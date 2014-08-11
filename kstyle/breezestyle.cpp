// krazy:excludeall=qclasses

//////////////////////////////////////////////////////////////////////////////
// breezestyle.cpp
// Breeze widget style for KDE Frameworks
// -------------------
//
// Copyright (C) 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License version 2 as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library; see the file COPYING.LIB.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.
//////////////////////////////////////////////////////////////////////////////

#include "breezestyle.h"
#include "breezestyle.moc"

#include "breeze.h"
#include "breezeanimations.h"
#include "breezeframeshadow.h"
#include "breezehelper.h"
#include "breezemdiwindowshadow.h"
#include "breezemnemonics.h"
#include "breezeshadowhelper.h"
#include "breezesplitterproxy.h"
#include "breezestyleconfigdata.h"
#include "breezewindowmanager.h"

#include <KColorUtils>

#include <QApplication>
#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QDial>
#include <QDBusConnection>
#include <QDockWidget>
#include <QFormLayout>
#include <QGraphicsView>
#include <QGroupBox>
#include <QLineEdit>
#include <QMainWindow>
#include <QMdiSubWindow>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QSplitterHandle>
#include <QTextEdit>
#include <QToolBox>
#include <QToolButton>

Q_LOGGING_CATEGORY(BREEZE, "breeze")

namespace BreezePrivate
{
    // needed to keep track of tabbars when being dragged
    class TabBarData: public QObject
    {

        public:

        //! constructor
        explicit TabBarData( QObject* parent ):
            QObject( parent )
        {}

        //! destructor
        virtual ~TabBarData( void )
        {}

        //! assign target tabBar
        void lock( const QWidget* widget )
        { _tabBar = widget; }

        //! true if tabbar is locked
        bool isLocked( const QWidget* widget ) const
        { return _tabBar && _tabBar.data() == widget; }

        //! release
        void release( void )
        { _tabBar.clear(); }

        private:

        //! pointer to target tabBar
        QPointer<const QWidget> _tabBar;

    };

}

namespace Breeze
{

    //_________________________________________________
    QStyle* Breeze::StylePlugin::create( const QString &key )
    {
        if( key.toLower() == QStringLiteral( "breeze" ) ) return new Style();
        else return nullptr;
    }

    //_________________________________________________
    Breeze::StylePlugin::~StylePlugin()
    {
        // Delete style when using ::exit() otherwise it'll outlive the unloaded plugin and we'll get a crash
        if (qApp) delete qApp->style();
    }

    //______________________________________________________________
    Style::Style( void ):
        _addLineButtons( SingleButton ),
        _subLineButtons( SingleButton ),
        _helper( new Helper( StyleConfigData::self()->sharedConfig() ) ),
        _shadowHelper( new ShadowHelper( this, *_helper ) ),
        _animations( new Animations( this ) ),
        _mnemonics( new Mnemonics( this ) ),
        _windowManager( new WindowManager( this ) ),
        _frameShadowFactory( new FrameShadowFactory( this ) ),
        _mdiWindowShadowFactory( new MdiWindowShadowFactory( this ) ),
        _splitterFactory( new SplitterFactory( this ) ),
        _tabBarData( new BreezePrivate::TabBarData( this ) ),
        SH_ArgbDndWindow( newStyleHint( QStringLiteral( "SH_ArgbDndWindow" ) ) )
    {

        // use DBus connection to update on breeze configuration change
        QDBusConnection dbus = QDBusConnection::sessionBus();
        dbus.connect( QString(),
            QStringLiteral( "/BreezeStyle" ),
            QStringLiteral( "org.kde.Breeze.Style" ),
            QStringLiteral( "reparseConfiguration" ), this, SLOT(configurationChanged()) );

        // enable debugging
        QLoggingCategory::setFilterRules(QStringLiteral("breeze.debug = false"));

        // call the slot directly; this initial call will set up things that also
        // need to be reset when the system palette changes
        loadConfiguration();

    }

    //______________________________________________________________
    Style::~Style( void )
    {
        delete _shadowHelper;
        delete _helper;
    }

    //______________________________________________________________
    void Style::polish( QWidget* widget )
    {
        if( !widget ) return;

        // register widget to animations
        _animations->registerWidget( widget );
        _windowManager->registerWidget( widget );
        _frameShadowFactory->registerWidget( widget, *_helper );
        _mdiWindowShadowFactory->registerWidget( widget );
        _shadowHelper->registerWidget( widget );
        _splitterFactory->registerWidget( widget );

        // enable mouse over effects for all necessary widgets
        if(
            qobject_cast<QAbstractItemView*>( widget )
            || qobject_cast<QAbstractSpinBox*>( widget )
            || qobject_cast<QCheckBox*>( widget )
            || qobject_cast<QComboBox*>( widget )
            || qobject_cast<QDial*>( widget )
            || qobject_cast<QLineEdit*>( widget )
            || qobject_cast<QPushButton*>( widget )
            || qobject_cast<QRadioButton*>( widget )
            || qobject_cast<QScrollBar*>( widget )
            || qobject_cast<QSlider*>( widget )
            || qobject_cast<QSplitterHandle*>( widget )
            || qobject_cast<QTabBar*>( widget )
            || qobject_cast<QTextEdit*>( widget )
            || qobject_cast<QToolButton*>( widget )
            )
        { widget->setAttribute( Qt::WA_Hover ); }

        // enforce translucency for drag and drop window
        if( widget->testAttribute( Qt::WA_X11NetWmWindowTypeDND ) && _helper->compositingActive() )
        {
            widget->setAttribute( Qt::WA_TranslucentBackground );
            widget->clearMask();
        }

        if( QAbstractItemView *itemView = qobject_cast<QAbstractItemView*>( widget ) )
        {

            // enable mouse over effects in itemviews' viewport
            itemView->viewport()->setAttribute( Qt::WA_Hover );

        } else if( QAbstractScrollArea* scrollArea = qobject_cast<QAbstractScrollArea*>( widget ) ) {

            // enable mouse over effect in sunken scrollareas that support focus
            if( scrollArea->frameShadow() == QFrame::Sunken && widget->focusPolicy()&Qt::StrongFocus )
            { widget->setAttribute( Qt::WA_Hover ); }

        } else if( QGroupBox* groupBox = qobject_cast<QGroupBox*>( widget ) )  {

            // checkable group boxes
            if( groupBox->isCheckable() )
            { groupBox->setAttribute( Qt::WA_Hover ); }

        } else if( qobject_cast<QAbstractButton*>( widget ) && qobject_cast<QDockWidget*>( widget->parent() ) ) {

            widget->setAttribute( Qt::WA_Hover );

        } else if( qobject_cast<QAbstractButton*>( widget ) && qobject_cast<QToolBox*>( widget->parent() ) ) {

            widget->setAttribute( Qt::WA_Hover );

        } else if( qobject_cast<QFrame*>( widget ) && widget->parent() && widget->parent()->inherits( "KTitleWidget" ) ) {

            widget->setAutoFillBackground( false );

        }

        if( qobject_cast<QScrollBar*>( widget ) )
        {

            // remove opaque painting for scrollbars
            widget->setAttribute( Qt::WA_OpaquePaintEvent, false );

        } else if( qobject_cast<QAbstractScrollArea*>( widget ) ) {

            addEventFilter( widget );

        } else if( QToolButton* toolButton = qobject_cast<QToolButton*>( widget ) ) {

            if( toolButton->autoRaise() )
            {
                // for flat toolbuttons, adjust foreground and background role accordingly
                widget->setBackgroundRole( QPalette::NoRole );
                widget->setForegroundRole( QPalette::WindowText );
            }

        } else if( qobject_cast<QDockWidget*>( widget ) ) {

            // add event filter on dock widgets
            // and alter palette
            widget->setAutoFillBackground( false );
            widget->setContentsMargins( Metrics::Frame_FrameWidth, Metrics::Frame_FrameWidth, Metrics::Frame_FrameWidth, Metrics::Frame_FrameWidth );
            addEventFilter( widget );

        } else if( qobject_cast<QMdiSubWindow*>( widget ) ) {

            widget->setAutoFillBackground( false );
            addEventFilter( widget );

        } else if( qobject_cast<QMenu*>( widget ) ) {

            setTranslucentBackground( widget );

        } else if( widget->inherits( "QComboBoxPrivateContainer" ) ) {

            addEventFilter( widget );
            setTranslucentBackground( widget );

        } else if( widget->inherits( "QTipLabel" ) ) {

            setTranslucentBackground( widget );

        }

        // alter palette for relevant framed widgets
        const QPalette palette( _helper->framePalette( QGuiApplication::palette() ) );
        if( qobject_cast<QDockWidget*>( widget ) ||
            qobject_cast<QGroupBox*>( widget ) ||
            qobject_cast<QTabWidget*>( widget ) ||
            qobject_cast<QMenu*>( widget ) ||
            widget->inherits( "QComboBoxPrivateContainer" ) )
            { widget->setPalette( palette ); }

        // base class polishing
        KStyle::polish( widget );

    }

    //_______________________________________________________________
    void Style::unpolish( QWidget* widget )
    {

        // register widget to animations
        _animations->unregisterWidget( widget );
        _frameShadowFactory->unregisterWidget( widget );
        _mdiWindowShadowFactory->unregisterWidget( widget );
        _shadowHelper->unregisterWidget( widget );
        _windowManager->unregisterWidget( widget );
        _splitterFactory->unregisterWidget( widget );

        KStyle::unpolish( widget );

    }


    //____________________________________________________________________
    QIcon Style::standardIcon( StandardPixmap standardPixmap, const QStyleOption* option, const QWidget* widget ) const
    {

        // lookup cache
        if( _iconCache.contains( standardPixmap ) ) return _iconCache.value( standardPixmap );

        QIcon icon;
        switch( standardPixmap )
        {

            case SP_TitleBarNormalButton:
            case SP_TitleBarMinButton:
            case SP_TitleBarMaxButton:
            case SP_TitleBarCloseButton:
            case SP_DockWidgetCloseButton:
            icon = titleBarButtonIcon( standardPixmap, option, widget );
            break;

            case SP_ToolBarHorizontalExtensionButton:
            case SP_ToolBarVerticalExtensionButton:
            icon = toolBarExtensionIcon( standardPixmap, option, widget );
            break;

            default:
            icon = KStyle::standardIcon( standardPixmap, option, widget );
            break;

        }

        const_cast<IconCache*>(&_iconCache)->insert( standardPixmap, icon );
        return icon;

    }

    //______________________________________________________________
    int Style::pixelMetric( PixelMetric metric, const QStyleOption* option, const QWidget* widget ) const
    {

        // handle special cases
        switch( metric )
        {

            // frame width
            case PM_DefaultFrameWidth:
            if( qobject_cast<const QLineEdit*>( widget ) ) return LineEdit_FrameWidth;
            else if( option && option->styleObject && option->styleObject->inherits( "QQuickStyleItem" ) )
            {
                const QString &elementType = option->styleObject->property( "elementType" ).toString();
                if ( elementType == QLatin1String( "edit" ) || elementType == QLatin1String( "spinbox" ) )
                {

                    return LineEdit_FrameWidth;

                } else if ( elementType == QLatin1String( "combobox" ) ) {

                    return ComboBox_FrameWidth;
                }

            }

            // fallback
            return Metrics::Frame_FrameWidth;

            case PM_ComboBoxFrameWidth:
            {
                const QStyleOptionComboBox* comboBoxOption( qstyleoption_cast< const QStyleOptionComboBox*>( option ) );
                return comboBoxOption && comboBoxOption->editable ? Metrics::LineEdit_FrameWidth : Metrics::ComboBox_FrameWidth;
            }

            case PM_SpinBoxFrameWidth:
            return Metrics::SpinBox_FrameWidth;

            case PM_ToolBarFrameWidth:
            return Metrics::ToolBar_FrameWidth;

            case PM_ToolTipLabelFrameWidth:
            return Metrics::ToolTip_FrameWidth;

            // layout
            case PM_LayoutLeftMargin:
            case PM_LayoutTopMargin:
            case PM_LayoutRightMargin:
            case PM_LayoutBottomMargin:
            {
                /*
                use either Child margin or TopLevel margin,
                depending on  widget type
                */
                if( ( option && ( option->state & QStyle::State_Window ) ) || ( widget && widget->isWindow() ) )
                {

                    return Metrics::Layout_TopLevelMarginWidth;

                } else {

                    return Metrics::Layout_ChildMarginWidth;

                }

            }

            case PM_LayoutHorizontalSpacing: return Metrics::Layout_DefaultSpacing;
            case PM_LayoutVerticalSpacing: return Metrics::Layout_DefaultSpacing;

            // buttons
            case PM_ButtonMargin: return Metrics::Button_MarginWidth;
            case PM_ButtonDefaultIndicator: return 0;
            case PM_ButtonShiftHorizontal: return 0;
            case PM_ButtonShiftVertical: return 0;

            // menubars
            case PM_MenuBarPanelWidth: return 0;
            case PM_MenuBarHMargin: return 0;
            case PM_MenuBarVMargin: return 0;
            case PM_MenuBarItemSpacing: return 0;
            case PM_MenuDesktopFrameWidth: return 0;

            // menu buttons
            case PM_MenuButtonIndicator: return Metrics::MenuButton_IndicatorWidth;

            // toolbars
            case PM_ToolBarHandleExtent: return Metrics::ToolBar_HandleExtent;
            case PM_ToolBarSeparatorExtent: return Metrics::ToolBar_SeparatorWidth;

            // need to make sure there is enough room to draw the icon
            case PM_ToolBarExtensionExtent:
            return pixelMetric( PM_SmallIconSize, option, widget ) + 2*Metrics::ToolButton_MarginWidth;

            // toolbar items
            case PM_ToolBarItemMargin: return 0;
            case PM_ToolBarItemSpacing: return Metrics::ToolBar_ItemSpacing;

            // tabbars
            case PM_TabBarTabShiftVertical: return 0;
            case PM_TabBarTabShiftHorizontal: return 0;
            case PM_TabBarTabOverlap: return Metrics::TabBar_TabOverlap;
            case PM_TabBarBaseOverlap: return Metrics::TabBar_BaseOverlap;
            case PM_TabBarTabHSpace: return 2*Metrics::TabBar_TabMarginWidth;
            case PM_TabBarTabVSpace: return 2*Metrics::TabBar_TabMarginHeight;

            // scrollbars
            case PM_ScrollBarExtent: return Metrics::ScrollBar_Extend;
            case PM_ScrollBarSliderMin: return Metrics::ScrollBar_MinSliderHeight;

            // title bar
            case PM_TitleBarHeight: return 2*Metrics::TitleBar_MarginWidth + pixelMetric( PM_SmallIconSize, option, widget );

            // sliders
            case PM_SliderThickness: return Metrics::Slider_ControlThickness;
            case PM_SliderControlThickness: return Metrics::Slider_ControlThickness;
            case PM_SliderLength: return Metrics::Slider_ControlThickness;

            // checkboxes and radio buttons
            case PM_IndicatorWidth: return CheckBox_Size;
            case PM_IndicatorHeight: return CheckBox_Size;
            case PM_ExclusiveIndicatorWidth: return CheckBox_Size;
            case PM_ExclusiveIndicatorHeight: return CheckBox_Size;

            // list heaaders
            case PM_HeaderMarkSize: return Metrics::Header_ArrowSize;
            case PM_HeaderMargin: return Metrics::Header_MarginWidth;

            // dock widget
            // return 0 here, since frame is handled directly in polish
            case PM_DockWidgetFrameWidth: return 0;
            case PM_DockWidgetTitleMargin: return Frame_FrameWidth;
            case PM_DockWidgetTitleBarButtonMargin: return ToolButton_MarginWidth;

            // fallback
            default: return KStyle::pixelMetric( metric, option, widget );

        }

    }

    //______________________________________________________________
    int Style::styleHint( StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData ) const
    {
        switch( hint )
        {


            case SH_RubberBand_Mask:
            {

                if( QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask*>( returnData ) )
                {

                    mask->region = option->rect;

                    // need to check on widget before removing inner region
                    // in order to still preserve rubberband in MainWindow and QGraphicsView
                    // in QMainWindow because it looks better
                    // in QGraphicsView because the painting fails completely otherwise
                    if( widget && (
                        qobject_cast<const QAbstractItemView*>( widget->parent() ) ||
                        qobject_cast<const QGraphicsView*>( widget->parent() ) ||
                        qobject_cast<const QMainWindow*>( widget->parent() ) ) )
                    { return true; }

                    // also check if widget's parent is some itemView viewport
                    if( widget && widget->parent() &&
                        qobject_cast<const QAbstractItemView*>( widget->parent()->parent() ) &&
                        static_cast<const QAbstractItemView*>( widget->parent()->parent() )->viewport() == widget->parent() )
                    { return true; }

                    // mask out center
                    mask->region -= insideMargin( option->rect, 1 );

                    return true;
                }
                return false;
            }

            // combobox
            case SH_ComboBox_ListMouseTracking: return true;

            // menubar
            case SH_MenuBar_MouseTracking: return true;

            // menu
            case SH_Menu_MouseTracking: return true;
            case SH_Menu_SubMenuPopupDelay: return 150;
            case SH_Menu_SloppySubMenus: return true;
            case SH_Menu_SupportsSections: return true;

            // groupboxes
            case SH_GroupBox_TextLabelVerticalAlignment: return Qt::AlignVCenter;

            // tabbar
            case SH_TabBar_Alignment: return Qt::AlignCenter;

            // toolbox
            case SH_ToolBox_SelectedPageTitleBold: return false;

            // scrollbars
            case SH_ScrollBar_MiddleClickAbsolutePosition: return true;

            // forms
            case SH_FormLayoutFormAlignment: return Qt::AlignLeft | Qt::AlignTop;
            case SH_FormLayoutLabelAlignment: return Qt::AlignRight;
            case SH_FormLayoutFieldGrowthPolicy: return QFormLayout::ExpandingFieldsGrow;
            case SH_FormLayoutWrapPolicy: return QFormLayout::DontWrapRows;

            // message box
            case SH_MessageBox_TextInteractionFlags: return Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse;

            // dialog buttons
            case SH_ProgressDialog_CenterCancelButton:
            case SH_MessageBox_CenterButtons:
            return false;

            // input panel
            case SH_RequestSoftwareInputPanel: return RSIP_OnMouseClick;

            // title bars
            case SH_TitleBar_NoBorder: return true;

            // dock widget buttons
            case SH_DockWidget_ButtonsHaveFrame: return false;

            // fallback
            default: return KStyle::styleHint( hint, option, widget, returnData );

        }

    }

    //______________________________________________________________
    QRect Style::subElementRect( SubElement element, const QStyleOption* option, const QWidget* widget ) const
    {
        switch( element )
        {

            // checkboxes and radio buttons
            case SE_CheckBoxContents: return checkBoxContentsRect( option, widget );
            case SE_RadioButtonContents: return checkBoxContentsRect( option, widget );

            // line edit content
            case SE_LineEditContents: return lineEditContentsRect( option, widget );

            // progress bars
            case SE_ProgressBarGroove: return progressBarGrooveRect( option, widget );
            case SE_ProgressBarContents: return progressBarContentsRect( option, widget );
            case SE_ProgressBarLabel: return progressBarLabelRect( option, widget );

            // headers
            case SE_HeaderArrow: return headerArrowRect( option, widget );
            case SE_HeaderLabel: return headerLabelRect( option, widget );

            // tabbars
            case SE_TabWidgetTabBar: return tabWidgetTabBarRect( option, widget );
            case SE_TabWidgetTabContents: return tabWidgetTabContentsRect( option, widget );
            case SE_TabWidgetTabPane: return tabWidgetTabPaneRect( option, widget );
            case SE_TabWidgetLeftCorner: return tabWidgetCornerRect( SE_TabWidgetLeftCorner, option, widget );
            case SE_TabWidgetRightCorner: return tabWidgetCornerRect( SE_TabWidgetRightCorner, option, widget );

            // toolbox
            case SE_ToolBoxTabContents: return toolBoxTabContentsRect( option, widget );

            // fallback
            default: return KStyle::subElementRect( element, option, widget );

        }
    }

    //______________________________________________________________
    QRect Style::subControlRect( ComplexControl element, const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget ) const
    {
        switch( element )
        {

            case CC_GroupBox: return groupBoxSubControlRect( option, subControl, widget );
            case CC_ToolButton: return toolButtonSubControlRect( option, subControl, widget );
            case CC_ComboBox: return comboBoxSubControlRect( option, subControl, widget );
            case CC_SpinBox: return spinBoxSubControlRect( option, subControl, widget );
            case CC_ScrollBar: return scrollBarSubControlRect( option, subControl, widget );
            case CC_Dial: return dialSubControlRect( option, subControl, widget );

            // fallback
            default: return KStyle::subControlRect( element, option, subControl, widget );

        }

    }

    //______________________________________________________________
    QSize Style::sizeFromContents( ContentsType element, const QStyleOption* option, const QSize& size, const QWidget* widget ) const
    {

        switch( element )
        {

            case CT_CheckBox: return checkBoxSizeFromContents( option, size, widget );
            case CT_RadioButton: return checkBoxSizeFromContents( option, size, widget );
            case CT_LineEdit: return lineEditSizeFromContents( option, size, widget );
            case CT_ComboBox: return comboBoxSizeFromContents( option, size, widget );
            case CT_SpinBox: return spinBoxSizeFromContents( option, size, widget );
            case CT_PushButton: return pushButtonSizeFromContents( option, size, widget );
            case CT_ToolButton: return toolButtonSizeFromContents( option, size, widget );
            case CT_MenuBar: return defaultSizeFromContents( option, size, widget );
            case CT_MenuBarItem: return menuBarItemSizeFromContents( option, size, widget );
            case CT_MenuItem: return menuItemSizeFromContents( option, size, widget );
            case CT_ProgressBar: return progressBarSizeFromContents( option, size, widget );

            // tabbars
            case CT_TabWidget: return tabWidgetSizeFromContents( option, size, widget );
            case CT_TabBarTab: return tabBarTabSizeFromContents( option, size, widget );

            // item views
            case CT_HeaderSection: return headerSectionSizeFromContents( option, size, widget );
            case CT_ItemViewItem: return itemViewItemSizeFromContents( option, size, widget );

            // fallback
            default: return KStyle::sizeFromContents( element, option, size, widget );

        }

    }

    //______________________________________________________________
    QStyle::SubControl Style::hitTestComplexControl( ComplexControl control, const QStyleOptionComplex* option, const QPoint& point, const QWidget* widget ) const
    {
        switch( control )
        {
            case CC_ScrollBar:
            {

                QRect groove = scrollBarSubControlRect( option, SC_ScrollBarGroove, widget );
                if ( groove.contains( point ) )
                {
                    //Must be either page up/page down, or just click on the slider.
                    //Grab the slider to compare
                    QRect slider = scrollBarSubControlRect( option, SC_ScrollBarSlider, widget );

                    if( slider.contains( point ) ) return SC_ScrollBarSlider;
                    else if( preceeds( point, slider, option ) ) return SC_ScrollBarSubPage;
                    else return SC_ScrollBarAddPage;

                }

                //This is one of the up/down buttons. First, decide which one it is.
                if( preceeds( point, groove, option ) )
                {

                    if( _subLineButtons == DoubleButton )
                    {
                        QRect buttonRect = scrollBarInternalSubControlRect( option, SC_ScrollBarSubLine );
                        return scrollBarHitTest( buttonRect, point, option );
                    } else return SC_ScrollBarSubLine;

                }

                if( _addLineButtons == DoubleButton )
                {

                    QRect buttonRect = scrollBarInternalSubControlRect( option, SC_ScrollBarAddLine );
                    return scrollBarHitTest( buttonRect, point, option );

                } else return SC_ScrollBarAddLine;
            }

            // fallback
            default: return KStyle::hitTestComplexControl( control, option, point, widget );

        }

    }

    //______________________________________________________________
    void Style::drawPrimitive( PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        StylePrimitive fcn( nullptr );
        switch( element )
        {

            // buttons
            case PE_PanelButtonCommand: fcn = &Style::drawPanelButtonCommandPrimitive; break;
            case PE_PanelButtonTool: fcn = &Style::drawPanelButtonToolPrimitive; break;

            // scroll areas
            case PE_PanelScrollAreaCorner: fcn = &Style::drawPanelScrollAreaCornerPrimitive; break;


            // menus
            case PE_PanelMenu: fcn = &Style::drawPanelMenuPrimitive; break;

            // tooltips
            case PE_PanelTipLabel: fcn = &Style::drawPanelTipLabelPrimitive; break;

            // item view
            case PE_PanelItemViewItem: fcn = &Style::drawPanelItemViewItemPrimitive; break;

            // checkboxes and radio buttons
            case PE_IndicatorCheckBox: fcn = &Style::drawIndicatorCheckBoxPrimitive; break;
            case PE_IndicatorRadioButton: fcn = &Style::drawIndicatorRadioButtonPrimitive; break;

            // menu indicator
            case PE_IndicatorButtonDropDown: fcn = &Style::drawIndicatorButtonDropDownPrimitive; break;

            // tabwidget tebs
            case PE_IndicatorTabClose: fcn = &Style::drawIndicatorTabClosePrimitive; break;
            case PE_IndicatorTabTear: fcn = &Style::drawIndicatorTabTearPrimitive; break;

            // arrows
            case PE_IndicatorArrowUp: fcn = &Style::drawIndicatorArrowUpPrimitive; break;
            case PE_IndicatorArrowDown: fcn = &Style::drawIndicatorArrowDownPrimitive; break;
            case PE_IndicatorArrowLeft: fcn = &Style::drawIndicatorArrowLeftPrimitive; break;
            case PE_IndicatorArrowRight: fcn = &Style::drawIndicatorArrowRightPrimitive; break;
            case PE_IndicatorHeaderArrow: fcn = &Style::drawIndicatorHeaderArrowPrimitive; break;
            case PE_IndicatorToolBarHandle: fcn = &Style::drawIndicatorToolBarHandlePrimitive; break;
            case PE_IndicatorToolBarSeparator: fcn = &Style::drawIndicatorToolBarSeparatorPrimitive; break;
            case PE_IndicatorBranch: fcn = &Style::drawIndicatorBranchPrimitive; break;

            // frames
            case PE_FrameStatusBar: fcn = &Style::emptyPrimitive; break;
            case PE_Frame: fcn = &Style::drawFramePrimitive; break;
            case PE_FrameLineEdit: fcn = &Style::drawFrameLineEditPrimitive; break;
            case PE_FrameMenu: fcn = &Style::drawFrameMenuPrimitive; break;
            case PE_FrameGroupBox: fcn = &Style::drawFrameGroupBoxPrimitive; break;
            case PE_FrameTabWidget: fcn = &Style::drawFrameTabWidgetPrimitive; break;
            case PE_FrameTabBarBase: fcn = &Style::drawFrameTabBarBasePrimitive; break;
            case PE_FrameWindow: fcn = &Style::drawFrameWindowPrimitive; break;

            // disable all focus rect rendering
            // it is handled directly in the relevant primitives
            case PE_FrameFocusRect: fcn = &Style::emptyPrimitive; break;

            // fallback
            default: break;

        }

        painter->save();

        // call function if implemented
        if( !( fcn && ( this->*fcn )( option, painter, widget ) ) )
        { KStyle::drawPrimitive( element, option, painter, widget ); }

        painter->restore();

    }

    //______________________________________________________________
    void Style::drawControl( ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        StyleControl fcn( nullptr );
        switch( element )
        {

            /*
            for CE_PushButtonBevel the only thing that is done is draw the PanelButtonCommand primitive
            since the prototypes are identical we register the second directly in the control map: fcn = without
            using an intermediate function
            */
            case CE_PushButtonBevel: fcn = &Style::drawPanelButtonCommandPrimitive; break;
            case CE_PushButtonLabel: fcn = &Style::drawPushButtonLabelControl; break;
            case CE_CheckBoxLabel: fcn = &Style::drawCheckBoxLabelControl; break;
            case CE_RadioButtonLabel: fcn = &Style::drawCheckBoxLabelControl; break;

            // combobox
            case CE_ComboBoxLabel: fcn = &Style::drawComboBoxLabelControl; break;

            // menu bars
            case CE_MenuBarEmptyArea: fcn = &Style::emptyControl; break;

            // menubar items
            case CE_MenuBarItem: fcn = &Style::drawMenuBarItemControl; break;

            // menu items
            case CE_MenuItem: fcn = &Style::drawMenuItemControl; break;

            // toolbar
            case CE_ToolBar: fcn = &Style::emptyControl; break;

            // progress bars
            case CE_ProgressBar: fcn = &Style::drawProgressBarControl; break;
            case CE_ProgressBarContents: fcn = &Style::drawProgressBarContentsControl; break;
            case CE_ProgressBarGroove: fcn = &Style::drawProgressBarGrooveControl; break;
            case CE_ProgressBarLabel: fcn = &Style::drawProgressBarLabelControl; break;

            // scrollbars
            case CE_ScrollBarSlider: fcn = &Style::drawScrollBarSliderControl; break;
            case CE_ScrollBarAddLine: fcn = &Style::drawScrollBarAddLineControl; break;
            case CE_ScrollBarSubLine: fcn = &Style::drawScrollBarSubLineControl; break;

            // these two are handled directly in CC_ScrollBar
            case CE_ScrollBarAddPage: fcn = &Style::emptyControl; break;
            case CE_ScrollBarSubPage: fcn = &Style::emptyControl; break;

            // frame
            case CE_ShapedFrame: fcn = &Style::drawShapedFrameControl; break;

            // rubber band
            case CE_RubberBand: fcn = &Style::drawRubberBandControl; break;

            // size grip
            // no size grip is rendered, since its usage is discouraged
            case CE_SizeGrip: fcn = &Style::emptyControl; break;

            // list headers
            case CE_HeaderSection: fcn = &Style::drawHeaderSectionControl; break;
            case CE_HeaderEmptyArea: fcn = &Style::drawHeaderEmptyAreaControl; break;

            // tabbar
            case CE_TabBarTabLabel: fcn = &Style::drawTabBarTabLabelControl; break;
            case CE_TabBarTabShape: fcn = &Style::drawTabBarTabShapeControl; break;

            // toolbox
            case CE_ToolBoxTabLabel: fcn = &Style::drawToolBoxTabLabelControl; break;
            case CE_ToolBoxTabShape: fcn = &Style::drawToolBoxTabShapeControl; break;

            // dock widget titlebar
            case CE_DockWidgetTitle: fcn = &Style::drawDockWidgetTitleControl; break;

            // fallback
            default: break;

        }

        painter->save();

        // call function if implemented
        if( !( fcn && ( this->*fcn )( option, painter, widget ) ) )
        { KStyle::drawControl( element, option, painter, widget ); }

        painter->restore();

    }

    //______________________________________________________________
    void Style::drawComplexControl( ComplexControl element, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        StyleComplexControl fcn( nullptr );
        switch( element )
        {

            case CC_GroupBox: fcn = &Style::drawGroupBoxComplexControl; break;
            case CC_ToolButton: fcn = &Style::drawToolButtonComplexControl; break;
            case CC_ComboBox: fcn = &Style::drawComboBoxComplexControl; break;
            case CC_SpinBox: fcn = &Style::drawSpinBoxComplexControl; break;
            case CC_Slider: fcn = &Style::drawSliderComplexControl; break;
            case CC_Dial: fcn = &Style::drawDialComplexControl; break;
            case CC_ScrollBar: fcn = &Style::drawScrollBarComplexControl; break;
            case CC_TitleBar: fcn = &Style::drawTitleBarComplexControl; break;

            // fallback
            default: break;

        }


        painter->save();

        // call function if implemented
        if( !( fcn && ( this->*fcn )( option, painter, widget ) ) )
        { KStyle::drawComplexControl( element, option, painter, widget ); }

        painter->restore();


    }


    //___________________________________________________________________________________
    void Style::drawItemText(
        QPainter* painter, const QRect& r, int flags, const QPalette& palette, bool enabled,
        const QString &text, QPalette::ColorRole textRole ) const
    {

        // hide mnemonics if requested
        if( !_mnemonics->enabled() && ( flags & Qt::TextShowMnemonic ) && !( flags&Qt::TextHideMnemonic ) )
        {
            flags &= ~Qt::TextShowMnemonic;
            flags |= Qt::TextHideMnemonic;
        }

        if( _animations->widgetEnabilityEngine().enabled() )
        {

            /*
            check if painter engine is registered to WidgetEnabilityEngine, and animated
            if yes, merge the palettes. Note: a static_cast is safe here, since only the address
            of the pointer is used, not the actual content.
            */
            const QWidget* widget( static_cast<const QWidget*>( painter->device() ) );
            if( _animations->widgetEnabilityEngine().isAnimated( widget, AnimationEnable ) )
            {

                const QPalette copy = _helper->disabledPalette( palette, _animations->widgetEnabilityEngine().opacity( widget, AnimationEnable )  );
                return KStyle::drawItemText( painter, r, flags, copy, enabled, text, textRole );

            }

        }

        // fallback
        return KStyle::drawItemText( painter, r, flags, palette, enabled, text, textRole );

    }


    //_____________________________________________________________________
    bool Style::eventFilter( QObject *object, QEvent *event )
    {

        if( QDockWidget* dockWidget = qobject_cast<QDockWidget*>( object ) ) { return eventFilterDockWidget( dockWidget, event ); }
        else if( QAbstractScrollArea* scrollArea = qobject_cast<QAbstractScrollArea*>( object ) ) { return eventFilterScrollArea( scrollArea, event ); }
        else if( QMdiSubWindow* subWindow = qobject_cast<QMdiSubWindow*>( object ) ) { return eventFilterMdiSubWindow( subWindow, event ); }

        // cast to QWidget
        QWidget *widget = static_cast<QWidget*>( object );
        if( widget->inherits( "QComboBoxPrivateContainer" ) ) { return eventFilterComboBoxContainer( widget, event ); }

        // fallback
        return KStyle::eventFilter( object, event );
    }

    //____________________________________________________________________________
    bool Style::eventFilterDockWidget( QDockWidget* dockWidget, QEvent* event )
    {
        if( event->type() == QEvent::Paint )
        {
            // create painter and clip
            QPainter painter( dockWidget );
            QPaintEvent *paintEvent = static_cast<QPaintEvent*>( event );
            painter.setClipRegion( paintEvent->region() );

            // store palette and set colors
            const QPalette& palette( dockWidget->palette() );
            const QColor background( palette.color( QPalette::Window ) );
            const QColor outline( _helper->frameOutlineColor( palette ) );

            // store rect
            const QRect rect( dockWidget->rect() );

            // render
            if( dockWidget->isWindow() ) _helper->renderMenuFrame( &painter, rect, background, outline, false );
            else _helper->renderFrame( &painter, rect, background, outline );

        }

        return false;

    }

    //____________________________________________________________________________
    bool Style::eventFilterScrollArea( QAbstractScrollArea* scrollArea, QEvent* event )
    {

        if( event->type() == QEvent::Paint )
        {

            // get scrollarea viewport
            QWidget* viewport( scrollArea->viewport() );
            if( !viewport ) return false;

            // get scrollarea horizontal and vertical containers
            QWidget* widget( nullptr );
            QList<QWidget*> widgets;
            if( viewport && ( widget = scrollArea->findChild<QWidget*>( "qt_scrollarea_vcontainer" ) ) && widget->isVisible() )
            { widgets.append( widget ); }

            if( viewport && ( widget = scrollArea->findChild<QWidget*>( "qt_scrollarea_hcontainer" ) ) && widget->isVisible() )
            { widgets.append( widget ); }

            if( widgets.empty() ) return false;

            // make sure proper background is rendered behind the containers
            QPainter p( scrollArea );
            p.setClipRegion( static_cast<QPaintEvent*>( event )->region() );

            p.setPen( Qt::NoPen );
            p.setBrush( widget->palette().color( viewport->backgroundRole() ) );

            foreach( QWidget* widget, widgets )
            { p.drawRect( widget->geometry() ); }

        }

        return false;
    }

    //____________________________________________________________________________
    bool Style::eventFilterMdiSubWindow( QMdiSubWindow* subWindow, QEvent* event )
    {

        if( event->type() == QEvent::Paint )
        {
            QPainter painter( subWindow );
            QPaintEvent* paintEvent( static_cast<QPaintEvent*>( event ) );
            painter.setClipRegion( paintEvent->region() );

            const QRect rect( subWindow->rect() );
            const QColor background( subWindow->palette().color( QPalette::Window ) );

            if( subWindow->isMaximized() )
            {

                // full painting
                painter.setPen( Qt::NoPen );
                painter.setBrush( background );
                painter.drawRect( rect );

            } else {

                // framed painting
                _helper->renderMenuFrame( &painter, rect, background, QColor() );

            }

        }

        // continue with normal painting
        return false;

    }

    //_________________________________________________________
    bool Style::eventFilterComboBoxContainer( QWidget* widget, QEvent* event )
    {
        if( event->type() == QEvent::Paint )
        {

            QPainter painter( widget );
            QPaintEvent *paintEvent = static_cast<QPaintEvent*>( event );
            painter.setClipRegion( paintEvent->region() );

            const QRect rect( widget->rect() );
            const QPalette& palette( widget->palette() );
            const QColor background( palette.color( QPalette::Window ) );
            const QColor outline( _helper->frameOutlineColor( palette ) );

            const bool hasAlpha( _helper->hasAlphaChannel( widget ) );
            if( hasAlpha )
            {

                painter.setCompositionMode( QPainter::CompositionMode_Source );
                _helper->renderMenuFrame( &painter, rect, background, outline, true );

            } else {

                _helper->renderMenuFrame( &painter, rect, background, outline, false );

            }

        }

        return false;

    }

    //_____________________________________________________________________
    void Style::configurationChanged()
    {

        // reparse breezerc
        StyleConfigData::self()->readConfig();

        // reload configuration
        loadConfiguration();
    }

    //_____________________________________________________________________
    void Style::loadConfiguration()
    {

        // load helper configuration
        _helper->loadConfig();

        // reinitialize engines
        _animations->setupEngines();
        _windowManager->initialize();

        // mnemonics
        _mnemonics->setMode( StyleConfigData::mnemonicsMode() );

        // splitter proxy
        _splitterFactory->setEnabled( StyleConfigData::splitterProxyEnabled() );

        // set mdiwindow factory shadow tiles
        _mdiWindowShadowFactory->setShadowTiles( _shadowHelper->shadowTiles() );

        // clear icon cache
        _iconCache.clear();

        // scrollbar buttons
        switch( StyleConfigData::scrollBarAddLineButtons() )
        {
            case 0: _addLineButtons = NoButton; break;
            case 1: _addLineButtons = SingleButton; break;

            default:
            case 2: _addLineButtons = DoubleButton; break;
        }

        switch( StyleConfigData::scrollBarSubLineButtons() )
        {
            case 0: _subLineButtons = NoButton; break;
            case 1: _subLineButtons = SingleButton; break;

            default:
            case 2: _subLineButtons = DoubleButton; break;
        }

    }

    //___________________________________________________________________________________________________________________
    QRect Style::lineEditContentsRect( const QStyleOption* option, const QWidget* widget ) const
    {
        // cast option and check
        const QStyleOptionFrame* frameOption( qstyleoption_cast<const QStyleOptionFrame*>( option ) );
        if( !frameOption ) return option->rect;

        // check flatness
        const bool flat( frameOption->lineWidth == 0 );
        if( flat ) return option->rect;

        // copy rect and take out margins
        QRect rect( option->rect );

        // take out margins if there is enough room
        const int frameWidth( pixelMetric( PM_DefaultFrameWidth, option, widget ) );
        if( rect.height() > option->fontMetrics.height() + 2*frameWidth ) return insideMargin( rect, frameWidth );
        else return rect;
    }

    //___________________________________________________________________________________________________________________
    QRect Style::progressBarGrooveRect( const QStyleOption* option, const QWidget* ) const
    {

        // cast option
        const QStyleOptionProgressBar* progressBarOption( qstyleoption_cast<const QStyleOptionProgressBar*>( option ) );
        if( !progressBarOption ) return option->rect;

        // get direction
        const State& state( option->state );
        const bool horizontal( state & State_Horizontal );
        const bool textVisible( progressBarOption->textVisible );
        const bool busy( progressBarOption->minimum == 0 && progressBarOption->maximum == 0 );

        QRect rect( option->rect );
        if( textVisible && !busy )
        {
            if( horizontal ) rect.setTop( rect.height() - Metrics::ProgressBar_Thickness );
            else {

                rect.setWidth( Metrics::ProgressBar_Thickness );
                rect = handleRightToLeftLayout( option, rect );

            }

        } else {

            // center everything, if text is hidden
            if( horizontal ) return centerRect( rect, rect.width(), Metrics::ProgressBar_Thickness );
            else return centerRect( rect, Metrics::ProgressBar_Thickness, rect.height() );

        }
        return rect;
    }

    //___________________________________________________________________________________________________________________
    QRect Style::progressBarContentsRect( const QStyleOption* option, const QWidget* widget ) const
    { return progressBarGrooveRect( option, widget ); }

    //___________________________________________________________________________________________________________________
    QRect Style::progressBarLabelRect( const QStyleOption* option, const QWidget* ) const
    {

        // cast option
        const QStyleOptionProgressBar* progressBarOption( qstyleoption_cast<const QStyleOptionProgressBar*>( option ) );
        if( !( progressBarOption && progressBarOption->textVisible ) ) return QRect();

        // get direction
        const State& state( option->state );
        const bool horizontal( state & State_Horizontal );

        QRect rect( option->rect );
        if( horizontal ) rect.setHeight( rect.height() - Metrics::ProgressBar_Thickness - Metrics::ProgressBar_BoxTextSpace );
        else {

            rect.setLeft( Metrics::ProgressBar_Thickness + Metrics::ProgressBar_BoxTextSpace );
            rect = handleRightToLeftLayout( option, rect );

        }

        return rect;

    }

    //___________________________________________________________________________________________________________________
    QRect Style::headerArrowRect( const QStyleOption* option, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionHeader* headerOption( qstyleoption_cast<const QStyleOptionHeader*>( option ) );
        if( !headerOption ) return option->rect;

        // check if arrow is necessary
        if( headerOption->sortIndicator == QStyleOptionHeader::None ) return QRect();

        QRect arrowRect( insideMargin( option->rect, Metrics::Header_MarginWidth ) );
        arrowRect.setLeft( arrowRect.right() - Metrics::Header_ArrowSize );

        return handleRightToLeftLayout( option, arrowRect );

    }

    //___________________________________________________________________________________________________________________
    QRect Style::headerLabelRect( const QStyleOption* option, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionHeader* headerOption( qstyleoption_cast<const QStyleOptionHeader*>( option ) );
        if( !headerOption ) return option->rect;

        // check if arrow is necessary
        QRect labelRect( insideMargin( option->rect, Metrics::Header_MarginWidth ) );
        if( headerOption->sortIndicator == QStyleOptionHeader::None ) return labelRect;

        labelRect.adjust( 0, 0, -Metrics::Header_ArrowSize-Metrics::Header_BoxTextSpace, 0 );
        return handleRightToLeftLayout( option, labelRect );

    }

    //____________________________________________________________________
    QRect Style::tabWidgetTabBarRect( const QStyleOption* option, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionTabWidgetFrame* tabOption = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>( option );
        if( !tabOption ) return option->rect;

        // do nothing if tabbar is hidden
        const QSize tabBarSize( tabOption->tabBarSize );
        if( tabBarSize.isEmpty() ) return option->rect;

        QRect rect( option->rect );
        QRect tabBarRect( QPoint(0, 0), tabBarSize );

        // horizontal positioning
        const bool verticalTabs( isVerticalTab( tabOption->shape ) );
        if( verticalTabs )
        {

            tabBarRect.setHeight( qMin( tabBarRect.height(), rect.height() - 2 ) );
            tabBarRect.moveTop( rect.top() + ( rect.height() - tabBarRect.height() )/2 );

        } else {

            // adjust rect to deal with corner buttons
            // need to properly deal with reverse layout
            const bool reverseLayout( option->direction == Qt::RightToLeft );
            if( !tabOption->leftCornerWidgetSize.isEmpty() )
            {
                const QRect buttonRect( subElementRect( SE_TabWidgetLeftCorner, option, widget ) );
                if( reverseLayout ) rect.setRight( buttonRect.left() );
                else rect.setLeft( buttonRect.width() - 1 );
            }

            if( !tabOption->rightCornerWidgetSize.isEmpty() )
            {
                const QRect buttonRect( subElementRect( SE_TabWidgetRightCorner, option, widget ) );
                if( reverseLayout ) rect.setLeft( buttonRect.width() - 1 );
                else rect.setRight( buttonRect.left() );
            }

            tabBarRect.setWidth( qMin( tabBarRect.width(), rect.width() - 2 ) );
            tabBarRect.moveLeft( rect.left() + (rect.width() - tabBarRect.width())/2 );

        }

        // vertical positioning
        switch( tabOption->shape )
        {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
            tabBarRect.moveTop( rect.top()+1 );
            break;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            tabBarRect.moveBottom( rect.bottom()-1 );
            break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            tabBarRect.moveLeft( rect.left()+1 );
            break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            tabBarRect.moveRight( rect.right()-1 );
            break;

            default: break;

        }

        return tabBarRect;

    }

    //____________________________________________________________________
    QRect Style::tabWidgetTabContentsRect( const QStyleOption* option, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionTabWidgetFrame* tabOption = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>( option );
        if( !tabOption ) return option->rect;

        // do nothing if tabbar is hidden
        if( tabOption->tabBarSize.isEmpty() ) return option->rect;
        const QRect rect = tabWidgetTabPaneRect( option, widget );

        const bool documentMode( tabOption->lineWidth == 0 );
        if( documentMode )
        {

            // add margin only to the relevant side
            switch( tabOption->shape )
            {
                case QTabBar::RoundedNorth:
                case QTabBar::TriangularNorth:
                return rect.adjusted( 0, Metrics::TabWidget_MarginWidth, 0, 0 );

                case QTabBar::RoundedSouth:
                case QTabBar::TriangularSouth:
                return rect.adjusted( 0, 0, 0, -Metrics::TabWidget_MarginWidth );

                case QTabBar::RoundedWest:
                case QTabBar::TriangularWest:
                return rect.adjusted( Metrics::TabWidget_MarginWidth, 0, 0, 0 );

                case QTabBar::RoundedEast:
                case QTabBar::TriangularEast:
                return rect.adjusted( 0, 0, -Metrics::TabWidget_MarginWidth, 0 );

                default: return rect;
            }

        } else return insideMargin( rect, Metrics::TabWidget_MarginWidth );

    }

    //____________________________________________________________________
    QRect Style::tabWidgetCornerRect( SubElement element, const QStyleOption* option, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionTabWidgetFrame* tabOption = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>( option );
        if( !tabOption ) return option->rect;

        // do nothing if tabbar is hidden
        const QSize tabBarSize( tabOption->tabBarSize );
        if( tabBarSize.isEmpty() ) return QRect();

        // do nothing for vertical tabs
        const bool verticalTabs( isVerticalTab( tabOption->shape ) );
        if( verticalTabs ) return QRect();

        const QRect rect( option->rect );
        QRect cornerRect( QPoint( 0, 0 ), QSize( tabBarSize.height(), tabBarSize.height() + 1 ) );

        if( element == SE_TabWidgetRightCorner ) cornerRect.moveRight( rect.right() );
        else cornerRect.moveLeft( rect.left() );

        switch( tabOption->shape )
        {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
            cornerRect.moveTop( rect.top() );
            break;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            cornerRect.moveBottom( rect.bottom() );
            break;

            default: break;
        }

        // return cornerRect;
        cornerRect = handleRightToLeftLayout( option, cornerRect );
        return cornerRect;

    }

    //____________________________________________________________________
    QRect Style::tabWidgetTabPaneRect( const QStyleOption* option, const QWidget* ) const
    {

        const QStyleOptionTabWidgetFrame* tabOption = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>( option );
        if( !tabOption || tabOption->tabBarSize.isEmpty() ) return option->rect;

        const int overlap = Metrics::TabBar_BaseOverlap - 1;
        const QSize tabBarSize( tabOption->tabBarSize - QSize( overlap, overlap ) );

        QRect rect( option->rect );
        switch( tabOption->shape )
        {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
            rect.adjust( 0, tabBarSize.height(), 0, 0 );
            break;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            rect.adjust( 0, 0, 0, -tabBarSize.height() );
            break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            rect.adjust( tabBarSize.width(), 0, 0, 0 );
            break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            rect.adjust( 0, 0, -tabBarSize.width(), 0 );
            break;

            default: break;
        }

        return rect;

    }

    //____________________________________________________________________
    QRect Style::toolBoxTabContentsRect( const QStyleOption* option, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionToolBox* toolBoxOption( qstyleoption_cast<const QStyleOptionToolBox *>( option ) );
        if( !toolBoxOption ) return option->rect;

        // copy rect
        const QRect& rect( option->rect );

        int contentsWidth(0);
        if( !toolBoxOption->icon.isNull() )
        {
            const int iconSize( pixelMetric( QStyle::PM_SmallIconSize, option, widget ) );
            contentsWidth += iconSize;

            if( !toolBoxOption->text.isEmpty() ) contentsWidth += Metrics::ToolBox_TabBoxTextSpace;
        }

        if( !toolBoxOption->text.isEmpty() )
        {

            const int textWidth = toolBoxOption->fontMetrics.size( _mnemonics->textFlags(), toolBoxOption->text ).width();
            contentsWidth += textWidth;

        }

        contentsWidth = qMin( contentsWidth, rect.width() );
        contentsWidth = qMax( contentsWidth, (int)Metrics::ToolBox_TabMinWidth );
        return centerRect( rect, contentsWidth, rect.height() );

    }

    //______________________________________________________________
    QRect Style::groupBoxSubControlRect( const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget ) const
    {

        QRect rect = option->rect;
        switch( subControl )
        {

            case SC_GroupBoxFrame: return rect;

            case SC_GroupBoxContents:
            {

                // cast option and check
                const QStyleOptionGroupBox *groupBoxOption = qstyleoption_cast<const QStyleOptionGroupBox*>( option );
                if( !groupBoxOption ) break;

                // take out frame width
                rect = insideMargin( rect, Metrics::Frame_FrameWidth );

                // get state
                const bool checkable( groupBoxOption->subControls & QStyle::SC_GroupBoxCheckBox );
                const bool emptyText( groupBoxOption->text.isEmpty() );

                // calculate title height
                int titleHeight( 0 );
                if( !emptyText ) titleHeight = groupBoxOption->fontMetrics.height();
                if( checkable ) titleHeight = qMax( titleHeight, (int) Metrics::CheckBox_Size );

                // add margin
                if( titleHeight > 0 ) titleHeight += 2*Metrics::GroupBox_TitleMarginWidth;

                rect.adjust( 0, titleHeight, 0, 0 );
                return rect;

            }

            case SC_GroupBoxCheckBox:
            case SC_GroupBoxLabel:
            {

                // cast option and check
                const QStyleOptionGroupBox *groupBoxOption = qstyleoption_cast<const QStyleOptionGroupBox*>( option );
                if( !groupBoxOption ) break;

                // take out frame width
                rect = insideMargin( rect, Metrics::Frame_FrameWidth );

                const bool emptyText( groupBoxOption->text.isEmpty() );
                const bool checkable( groupBoxOption->subControls & QStyle::SC_GroupBoxCheckBox );

                // calculate title height
                int titleHeight( 0 );
                int titleWidth( 0 );
                if( !emptyText )
                {
                    const QFontMetrics fontMetrics = option->fontMetrics;
                    titleHeight = qMax( titleHeight, fontMetrics.height() );
                    titleWidth += fontMetrics.size( _mnemonics->textFlags(), groupBoxOption->text ).width();
                }

                if( checkable )
                {
                    titleHeight = qMax( titleHeight, (int) Metrics::CheckBox_Size );
                    titleWidth += Metrics::CheckBox_Size;
                    if( !emptyText ) titleWidth += Metrics::CheckBox_BoxTextSpace;
                }

                // adjust height
                QRect titleRect( rect );
                titleRect.setHeight( titleHeight );
                titleRect.translate( 0, Metrics::GroupBox_TitleMarginWidth );

                // center
                titleRect = centerRect( titleRect, titleWidth, titleHeight );

                if( subControl == SC_GroupBoxCheckBox )
                {

                    // vertical centering
                    titleRect = centerRect( titleRect, titleWidth, Metrics::CheckBox_Size );

                    // horizontal positioning
                    const QRect subRect( titleRect.topLeft(), QSize( Metrics::CheckBox_Size, titleRect.height() ) );
                    return visualRect( option->direction, titleRect, subRect );

                } else {

                    // vertical centering
                    QFontMetrics fontMetrics = option->fontMetrics;
                    titleRect = centerRect( titleRect, titleWidth, fontMetrics.height() );

                    // horizontal positioning
                    QRect subRect( titleRect );
                    if( checkable ) subRect.adjust( Metrics::CheckBox_Size + Metrics::CheckBox_BoxTextSpace, 0, 0, 0 );
                    return visualRect( option->direction, titleRect, subRect );

                }

            }

            default: break;

        }

        return KStyle::subControlRect( CC_GroupBox, option, subControl, widget );

    }

    //___________________________________________________________________________________________________________________
    QRect Style::toolButtonSubControlRect( const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionToolButton* toolButtonOption = qstyleoption_cast<const QStyleOptionToolButton*>( option );
        if( !toolButtonOption ) return KStyle::subControlRect( CC_ToolButton, option, subControl, widget );

        const bool hasPopupMenu( toolButtonOption->features & QStyleOptionToolButton::MenuButtonPopup );
        const bool hasInlineIndicator( toolButtonOption->features & QStyleOptionToolButton::HasMenu && !hasPopupMenu );

        // store rect
        const QRect& rect( option->rect );
        const int menuButtonWidth( Metrics::MenuButton_IndicatorWidth );
        switch( subControl )
        {
            case SC_ToolButtonMenu:
            {

                // check fratures
                if( !(hasPopupMenu || hasInlineIndicator ) ) return QRect();

                // check features
                QRect menuRect( rect );
                menuRect.setLeft( rect.right() + 1 - menuButtonWidth );
                if( hasInlineIndicator )
                { menuRect.setTop( menuRect.bottom() + 1 - menuButtonWidth ); }

                return handleRightToLeftLayout( option, menuRect );
            }

            case SC_ToolButton:
            {

                if( hasPopupMenu )
                {

                    QRect contentsRect( rect );
                    contentsRect.setRight( rect.right() - menuButtonWidth );
                    return handleRightToLeftLayout( option, contentsRect );

                } else return rect;

            }

            default: return QRect();

        }

    }

    //___________________________________________________________________________________________________________________
    QRect Style::comboBoxSubControlRect( const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget ) const
    {
        // cast option and check
        const QStyleOptionComboBox *comboBoxOption( qstyleoption_cast<const QStyleOptionComboBox*>( option ) );
        if( !comboBoxOption ) return KStyle::subControlRect( CC_ComboBox, option, subControl, widget );

        const bool editable( comboBoxOption->editable );
        const bool flat( editable && !comboBoxOption->frame );

        // copy rect
        QRect rect( option->rect );

        switch( subControl )
        {
            case SC_ComboBoxFrame: return flat ? rect : QRect();
            case SC_ComboBoxListBoxPopup: return rect;

            case SC_ComboBoxArrow:
            {

                // take out frame width
                if( !flat ) rect = insideMargin( rect, Metrics::Frame_FrameWidth );

                QRect arrowRect;
                if( editable )
                {

                    arrowRect = QRect(
                        rect.right() - Metrics::MenuButton_IndicatorWidth,
                        rect.top(),
                        Metrics::MenuButton_IndicatorWidth,
                        rect.height() );

                } else {

                    arrowRect = QRect(
                        rect.right() - Metrics::MenuButton_IndicatorWidth,
                        rect.top(),
                        Metrics::MenuButton_IndicatorWidth,
                        rect.height() );

                }

                arrowRect = centerRect( arrowRect, Metrics::MenuButton_IndicatorWidth, Metrics::MenuButton_IndicatorWidth );
                return handleRightToLeftLayout( option, arrowRect );

            }

            case SC_ComboBoxEditField:
            {

                QRect labelRect;
                const int frameWidth( pixelMetric( PM_ComboBoxFrameWidth, option, widget ) );
                labelRect = QRect(
                    rect.left(), rect.top(),
                    rect.width() - Metrics::MenuButton_IndicatorWidth,
                    rect.height() );

                // remove margins
                if( !flat && rect.height() > option->fontMetrics.height() + 2*frameWidth )
                { labelRect.adjust( frameWidth, frameWidth, 0, -frameWidth ); }

                return handleRightToLeftLayout( option, labelRect );

            }

            default: break;

        }

        return KStyle::subControlRect( CC_ComboBox, option, subControl, widget );

    }

    //___________________________________________________________________________________________________________________
    QRect Style::spinBoxSubControlRect( const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionSpinBox *spinBoxOption( qstyleoption_cast<const QStyleOptionSpinBox*>( option ) );
        if( !spinBoxOption ) return KStyle::subControlRect( CC_SpinBox, option, subControl, widget );
        const bool flat( !spinBoxOption->frame );

        // copy rect
        QRect rect( option->rect );

        switch( subControl )
        {
            case SC_SpinBoxFrame: return flat ? QRect():rect;

            case SC_SpinBoxUp:
            case SC_SpinBoxDown:
            {

                // take out frame width
                if( !flat ) rect = insideMargin( rect, Metrics::Frame_FrameWidth );

                QRect arrowRect;
                arrowRect = QRect(
                    rect.right() - Metrics::SpinBox_ArrowButtonWidth,
                    rect.top(),
                    Metrics::SpinBox_ArrowButtonWidth,
                    rect.height() );

                arrowRect = centerRect( arrowRect, Metrics::SpinBox_ArrowButtonWidth, Metrics::SpinBox_ArrowButtonWidth );
                arrowRect.setHeight( Metrics::SpinBox_ArrowButtonWidth/2 );
                if( subControl == SC_SpinBoxDown ) arrowRect.translate( 0, Metrics::SpinBox_ArrowButtonWidth/2 );

                return handleRightToLeftLayout( option, arrowRect );

            }

            case SC_SpinBoxEditField:
            {

                QRect labelRect;
                labelRect = QRect(
                    rect.left(), rect.top(),
                    rect.width() - Metrics::SpinBox_ArrowButtonWidth,
                    rect.height() );

                // remove right side line editor margins
                const int frameWidth( pixelMetric( PM_SpinBoxFrameWidth, option, widget ) );
                if( !flat && labelRect.height() > option->fontMetrics.height() + 2*frameWidth )
                { labelRect.adjust( frameWidth, frameWidth, 0, -frameWidth ); }

                return handleRightToLeftLayout( option, labelRect );

            }

            default: break;

        }

        return KStyle::subControlRect( CC_SpinBox, option, subControl, widget );

    }

    //___________________________________________________________________________________________________________________
    QRect Style::scrollBarInternalSubControlRect( const QStyleOptionComplex* option, SubControl subControl ) const
    {

        const QRect& r = option->rect;
        const State& state( option->state );
        const bool horizontal( state & State_Horizontal );

        switch ( subControl )
        {

            case SC_ScrollBarSubLine:
            {
                int majorSize( scrollBarButtonHeight( _subLineButtons ) );
                if( horizontal ) return handleRightToLeftLayout( option, QRect( r.x(), r.y(), majorSize, r.height() ) );
                else return handleRightToLeftLayout( option, QRect( r.x(), r.y(), r.width(), majorSize ) );

            }

            case SC_ScrollBarAddLine:
            {
                int majorSize( scrollBarButtonHeight( _addLineButtons ) );
                if( horizontal ) return handleRightToLeftLayout( option, QRect( r.right() - majorSize, r.y(), majorSize, r.height() ) );
                else return handleRightToLeftLayout( option, QRect( r.x(), r.bottom() - majorSize, r.width(), majorSize ) );
            }

            default: return QRect();

        }
    }

    //___________________________________________________________________________________________________________________
    QRect Style::scrollBarSubControlRect( const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionSlider* sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if( !sliderOption ) return KStyle::subControlRect( CC_ScrollBar, option, subControl, widget );

        // get relevant state
        const State& state( option->state );
        const bool horizontal( state & State_Horizontal );

        switch ( subControl )
        {

            case SC_ScrollBarSubLine:
            case SC_ScrollBarAddLine:
            return scrollBarInternalSubControlRect( option, subControl );

            case SC_ScrollBarGroove:
            {
                QRect top = handleRightToLeftLayout( option, scrollBarInternalSubControlRect( option, SC_ScrollBarSubLine ) );
                QRect bot = handleRightToLeftLayout( option, scrollBarInternalSubControlRect( option, SC_ScrollBarAddLine ) );

                QPoint topLeftCorner;
                QPoint botRightCorner;

                if( horizontal )
                {

                    topLeftCorner  = QPoint( top.right() + 1, top.top() );
                    botRightCorner = QPoint( bot.left()  - 1, top.bottom() );

                } else {

                    topLeftCorner  = QPoint( top.left(),  top.bottom() + 1 );
                    botRightCorner = QPoint( top.right(), bot.top()    - 1 );

                }

                // define rect
                return handleRightToLeftLayout( option, QRect( topLeftCorner, botRightCorner )  );

            }

            case SC_ScrollBarSlider:
            {
                // We handle RTL here to unreflect things if need be
                QRect groove = handleRightToLeftLayout( option, scrollBarSubControlRect( option, SC_ScrollBarGroove, widget ) );

                if ( sliderOption->minimum == sliderOption->maximum ) return groove;

                //Figure out how much room we have..
                int space( horizontal ? groove.width() : groove.height() );

                //Calculate the portion of this space that the slider should take up.
                int sliderSize = space * qreal( sliderOption->pageStep ) / ( sliderOption->maximum - sliderOption->minimum + sliderOption->pageStep );
                sliderSize = qMax( sliderSize, static_cast<int>(Metrics::ScrollBar_MinSliderHeight ) );
                sliderSize = qMin( sliderSize, space );

                space -= sliderSize;
                if( space <= 0 ) return groove;

                int pos = qRound( qreal( sliderOption->sliderPosition - sliderOption->minimum )/ ( sliderOption->maximum - sliderOption->minimum )*space );
                if( sliderOption->upsideDown ) pos = space - pos;
                if( horizontal ) return handleRightToLeftLayout( option, QRect( groove.x() + pos, groove.y(), sliderSize, groove.height() ) );
                else return handleRightToLeftLayout( option, QRect( groove.x(), groove.y() + pos, groove.width(), sliderSize ) );
            }

            case SC_ScrollBarSubPage:
            {

                //We do handleRightToLeftLayout here to unreflect things if need be
                QRect slider = handleRightToLeftLayout( option, scrollBarSubControlRect( option, SC_ScrollBarSlider, widget ) );
                QRect groove = handleRightToLeftLayout( option, scrollBarSubControlRect( option, SC_ScrollBarGroove, widget ) );

                if( horizontal ) return handleRightToLeftLayout( option, QRect( groove.x(), groove.y(), slider.x() - groove.x(), groove.height() ) );
                else return handleRightToLeftLayout( option, QRect( groove.x(), groove.y(), groove.width(), slider.y() - groove.y() ) );
            }

            case SC_ScrollBarAddPage:
            {

                //We do handleRightToLeftLayout here to unreflect things if need be
                QRect slider = handleRightToLeftLayout( option, scrollBarSubControlRect( option, SC_ScrollBarSlider, widget ) );
                QRect groove = handleRightToLeftLayout( option, scrollBarSubControlRect( option, SC_ScrollBarGroove, widget ) );

                if( horizontal ) return handleRightToLeftLayout( option, QRect( slider.right() + 1, groove.y(), groove.right() - slider.right(), groove.height() ) );
                else return handleRightToLeftLayout( option, QRect( groove.x(), slider.bottom() + 1, groove.width(), groove.bottom() - slider.bottom() ) );

            }

            default: return KStyle::subControlRect( CC_ScrollBar, option, subControl, widget );;
        }
    }

    //___________________________________________________________________________________________________________________
    QRect Style::dialSubControlRect( const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionSlider* sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if( !sliderOption ) return KStyle::subControlRect( CC_Dial, option, subControl, widget );

        // adjust rect to be square, and centered
        QRect rect( option->rect );
        const int dimension( qMin( rect.width(), rect.height() ) );
        rect = centerRect( rect, dimension, dimension );

        switch( subControl )
        {
            case QStyle::SC_DialGroove: return insideMargin( rect, (Metrics::Slider_ControlThickness - Metrics::Slider_Thickness)/2 );
            case QStyle::SC_DialHandle:
            {

                // calculate angle at which handle needs to be drawn
                const qreal angle( dialAngle( sliderOption, sliderOption->sliderPosition ) );

                // groove rect
                const QRectF grooveRect( insideMargin( rect, Metrics::Slider_ControlThickness/2 ) );
                qreal radius( grooveRect.width()/2 );

                // slider center
                QPointF center( grooveRect.center() + QPointF( radius*std::cos( angle ), -radius*std::sin( angle ) ) );

                // slider rect
                QRect handleRect( 0, 0, Metrics::Slider_ControlThickness, Metrics::Slider_ControlThickness );
                handleRect.moveCenter( center.toPoint() );
                return handleRect;

            }

            default: return KStyle::subControlRect( CC_Dial, option, subControl, widget );;

        }

    }

    //______________________________________________________________
    QSize Style::checkBoxSizeFromContents( const QStyleOption*, const QSize& contentsSize, const QWidget* ) const
    {
        // get contents size
        QSize size( contentsSize );

        // add focus height
        size = expandSize( size, 0, Metrics::CheckBox_FocusMarginWidth );

        // make sure there is enough height for indicator
        size.setHeight( qMax( size.height(), (int) Metrics::CheckBox_Size ) );

        // Add space for the indicator and the icon
        size.rwidth() += Metrics::CheckBox_Size + Metrics::CheckBox_BoxTextSpace;

        // also add extra space, to leave room to the right of the label
        size.rwidth() += Metrics::CheckBox_BoxTextSpace;

        return size;

    }

    //______________________________________________________________
    QSize Style::lineEditSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* widget ) const
    {
        // cast option and check
        const QStyleOptionFrame* frameOption( qstyleoption_cast<const QStyleOptionFrame*>( option ) );
        if( !frameOption ) return contentsSize;

        const bool flat( frameOption->lineWidth == 0 );
        const int frameWidth( pixelMetric( PM_DefaultFrameWidth, option, widget ) );
        return flat ? contentsSize : expandSize( contentsSize, frameWidth );
    }

    //______________________________________________________________
    QSize Style::comboBoxSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionComboBox* comboBoxOption( qstyleoption_cast<const QStyleOptionComboBox*>( option ) );
        if( !comboBoxOption ) return contentsSize;

        // copy size
        QSize size( contentsSize );

        // add relevant margin
        const bool flat( !comboBoxOption->frame );
        const int frameWidth( pixelMetric( PM_ComboBoxFrameWidth, option, widget ) );
        if( !flat ) size = expandSize( size, frameWidth );

        // make sure there is enough height for the button
        size.setHeight( qMax( size.height(), (int)Metrics::MenuButton_IndicatorWidth ) );

        // add button width and spacing
        size.rwidth() += Metrics::MenuButton_IndicatorWidth;

        return size;

    }

    //______________________________________________________________
    QSize Style::spinBoxSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionSpinBox *spinBoxOption( qstyleoption_cast<const QStyleOptionSpinBox*>( option ) );
        if( !spinBoxOption ) return contentsSize;

        const bool flat( !spinBoxOption->frame );

        // copy size
        QSize size( contentsSize );

        // add editor margins
        const int frameWidth( pixelMetric( PM_SpinBoxFrameWidth, option, widget ) );
        if( !flat ) size = expandSize( size, frameWidth );

        // make sure there is enough height for the button
        size.setHeight( qMax( size.height(), (int)Metrics::SpinBox_ArrowButtonWidth ) );

        // add button width and spacing
        size.rwidth() += Metrics::SpinBox_ArrowButtonWidth;

        return size;

    }

    //______________________________________________________________
    QSize Style::pushButtonSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionButton* buttonOption( qstyleoption_cast<const QStyleOptionButton*>( option ) );
        if( !buttonOption ) return contentsSize;

        const bool flat( buttonOption->features & QStyleOptionButton::Flat );

        QSize size( contentsSize );

        // add space for arrow
        if( buttonOption->features & QStyleOptionButton::HasMenu )
        {
            size.rheight() += 2*Metrics::Button_MarginWidth;
            size.setHeight( qMax( size.height(), int( Metrics::MenuButton_IndicatorWidth ) ) );
            size.rwidth() += Metrics::Button_MarginWidth;

            if( !( buttonOption->icon.isNull() && buttonOption->text.isEmpty() ) )
            { size.rwidth() += Metrics::Button_BoxTextSpace; }

        }  else size = expandSize( size, Metrics::Button_MarginWidth );

        // add space for icon
        if( !buttonOption->icon.isNull() )
        {

            QSize iconSize = buttonOption->iconSize;
            if( !iconSize.isValid() ) iconSize = QSize( pixelMetric( PM_SmallIconSize, option, widget ), pixelMetric( PM_SmallIconSize, option, widget ) );

            size.setHeight( qMax( size.height(), iconSize.height() ) );

            if( !buttonOption->text.isEmpty() )
            { size.rwidth() += Metrics::Button_BoxTextSpace; }

        }

        // finally add margins
        return flat ? size : expandSize( size, Metrics::Frame_FrameWidth );

    }

    //______________________________________________________________
    QSize Style::toolButtonSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionToolButton* toolButtonOption = qstyleoption_cast<const QStyleOptionToolButton*>( option );
        if( !toolButtonOption ) return contentsSize;

        // copy size
        QSize size = contentsSize;

        // get relevant state flags
        const State& state( option->state );
        const bool autoRaise( state & State_AutoRaise );
        const bool hasPopupMenu( toolButtonOption->subControls & SC_ToolButtonMenu );
        const bool hasInlineIndicator( toolButtonOption->features & QStyleOptionToolButton::HasMenu && !hasPopupMenu );
        const int marginWidth( autoRaise ? Metrics::ToolButton_MarginWidth : Metrics::Button_MarginWidth + Metrics::Frame_FrameWidth );

        if( hasInlineIndicator ) size.rwidth() += Metrics::ToolButton_BoxTextSpace;
        size = expandSize( size, marginWidth );

        return size;
    }

    //______________________________________________________________
    QSize Style::menuBarItemSizeFromContents( const QStyleOption*, const QSize& contentsSize, const QWidget* ) const
    { return expandSize( contentsSize, Metrics::MenuBarItem_MarginWidth, Metrics::MenuBarItem_MarginHeight ); }


    //______________________________________________________________
    QSize Style::menuItemSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionMenuItem* menuItemOption = qstyleoption_cast<const QStyleOptionMenuItem*>( option );
        if( !menuItemOption ) return contentsSize;

        // First, we calculate the intrinsic size of the item.
        // this must be kept consistent with what's in drawMenuItemControl
        QSize size( contentsSize );
        switch ( menuItemOption->menuItemType )
        {

            case QStyleOptionMenuItem::Normal:
            case QStyleOptionMenuItem::DefaultItem:
            case QStyleOptionMenuItem::SubMenu:
            {

                const int iconWidth( qMax( menuItemOption->maxIconWidth, pixelMetric( PM_SmallIconSize, option, widget ) ) );
                int leftColumnWidth( iconWidth );

                // add space with respect to text
                leftColumnWidth += Metrics::MenuItem_BoxTextSpace;

                // add checkbox indicator width
                if( menuItemOption->menuHasCheckableItems )
                { leftColumnWidth += Metrics::CheckBox_Size + Metrics::MenuItem_BoxTextSpace; }

                // add spacing for accelerator
                /*
                Note:
                The width of the accelerator itself is not included here since
                Qt will add that on separately after obtaining the
                sizeFromContents() for each menu item in the menu to be shown
                ( see QMenuPrivate::calcActionRects() )
                */
                const bool hasAccelerator( menuItemOption->text.indexOf( QLatin1Char( '\t' ) ) >= 0 );
                if( hasAccelerator ) size.rwidth() += Metrics::MenuItem_BoxTextSpace;

                // right column
                const int rightColumnWidth = Metrics::MenuButton_IndicatorWidth + Metrics::MenuItem_BoxTextSpace;
                size.rwidth() += leftColumnWidth + rightColumnWidth;

                // make sure height is large enough for icon and arrow
                size.setHeight( qMax( size.height(), (int) Metrics::MenuButton_IndicatorWidth ) );
                size.setHeight( qMax( size.height(), (int) Metrics::CheckBox_Size ) );
                size.setHeight( qMax( size.height(), (int) iconWidth ) );
                return expandSize( size, Metrics::MenuItem_MarginWidth );

            }

            case QStyleOptionMenuItem::Separator:
            {

                if( menuItemOption->text.isEmpty() && menuItemOption->icon.isNull() )
                {

                    return expandSize( QSize(0,1), Metrics::MenuItem_MarginWidth );


                } else {

                    // build toolbutton option
                    const QStyleOptionToolButton toolButtonOption( separatorMenuItemOption( menuItemOption, widget ) );

                    // make sure height is large enough for icon
                    const int iconWidth( qMax( menuItemOption->maxIconWidth, pixelMetric( PM_SmallIconSize, option, widget ) ) );
                    size.setHeight( qMax( size.height(), (int) iconWidth ) );

                    // return size from CT_ToolButton
                    return sizeFromContents( CT_ToolButton, &toolButtonOption, size, widget );
                }

            }

            // for all other cases, return input
            default: return contentsSize;
        }

    }

    //______________________________________________________________
    QSize Style::progressBarSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* ) const
    {

        // cast option
        const QStyleOptionProgressBar* progressBarOption( qstyleoption_cast<const QStyleOptionProgressBar*>( option ) );
        if( !progressBarOption ) return contentsSize;

        const bool textVisible( progressBarOption->textVisible );

        // make local copy
        QSize size( contentsSize );

        // get direction
        const State& state( option->state );
        const bool horizontal( state & State_Horizontal );

        if( horizontal ) {

            size.setWidth( qMax( size.width(), (int) Metrics::ProgressBar_Thickness ) );
            if( textVisible ) size.rheight() += Metrics::ProgressBar_Thickness + Metrics::ProgressBar_BoxTextSpace;
            else size.setHeight( Metrics::ProgressBar_Thickness );

        } else {

            size.setHeight( qMax( size.height(), (int) Metrics::ProgressBar_Thickness ) );
            if( textVisible ) size.rwidth() += Metrics::ProgressBar_Thickness + Metrics::ProgressBar_BoxTextSpace;
            else size.setWidth( Metrics::ProgressBar_Thickness );

        }

        return size;

    }

    //______________________________________________________________
    QSize Style::headerSectionSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionHeader* headerOption( qstyleoption_cast<const QStyleOptionHeader*>( option ) );
        if( !headerOption ) return contentsSize;

        // get text size
        const bool horizontal( headerOption->orientation == Qt::Horizontal );
        const bool hasText( !headerOption->text.isEmpty() );
        const bool hasIcon( !headerOption->icon.isNull() );

        const QSize textSize( hasText ? headerOption->fontMetrics.size( 0, headerOption->text ) : QSize() );
        const QSize iconSize( hasIcon ? QSize( 22,22 ) : QSize() );

        // contents width
        int contentsWidth( 0 );
        if( hasText ) contentsWidth += textSize.width();
        if( hasIcon )
        {
            contentsWidth += iconSize.width();
            if( hasText ) contentsWidth += Metrics::Header_BoxTextSpace;
        }

        // contents height
        int contentsHeight( 0 );
        if( hasText ) contentsHeight = textSize.height();
        if( hasIcon ) contentsHeight = qMax( contentsHeight, iconSize.height() );

        if( horizontal )
        {
            // also add space for icon
            contentsWidth += Metrics::Header_ArrowSize + Metrics::Header_BoxTextSpace;
            contentsHeight = qMax( contentsHeight, (int) Metrics::Header_ArrowSize );
        }

        // update contents size, add margins and return
        const QSize size( contentsSize.expandedTo( QSize( contentsWidth, contentsHeight ) ) );
        return expandSize( size, Metrics::Header_MarginWidth );

    }

    //______________________________________________________________
    QSize Style::itemViewItemSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* widget ) const
    {
        // call base class
        QSize size( KStyle::sizeFromContents( CT_ItemViewItem, option, contentsSize, widget ) );

        // add margins
        return expandSize( size, Metrics::ItemView_ItemMarginWidth );

    }

    //______________________________________________________________
    QSize Style::tabWidgetSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionTabWidgetFrame* tabOption = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>( option );
        if( !tabOption ) return expandSize( contentsSize, Metrics::Frame_FrameWidth );

        // tab orientation
        const bool verticalTabs( tabOption && isVerticalTab( tabOption->shape ) );

        // need to reduce the size in the tabbar direction, due to a bug in QTabWidget::minimumSize
        return verticalTabs ?
            expandSize( contentsSize, Metrics::Frame_FrameWidth, Metrics::Frame_FrameWidth - 1 ):
            expandSize( contentsSize, Metrics::Frame_FrameWidth - 1, Metrics::Frame_FrameWidth );

    }

    //______________________________________________________________
    QSize Style::tabBarTabSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* ) const
    {
        const QStyleOptionTab *tabOption( qstyleoption_cast<const QStyleOptionTab*>( option ) );

        // add margins
        QSize size( contentsSize );

        // compare to minimum size
        const bool verticalTabs( tabOption && isVerticalTab( tabOption ) );
        if( verticalTabs )
        {

            size = expandSize( size, Metrics::TabBar_TabMarginHeight, Metrics::TabBar_TabMarginWidth );
            size = size.expandedTo( QSize( Metrics::TabBar_TabMinHeight, Metrics::TabBar_TabMinWidth ) );

        } else {

            size = expandSize( size, Metrics::TabBar_TabMarginWidth, Metrics::TabBar_TabMarginHeight );
            size = size.expandedTo( QSize( Metrics::TabBar_TabMinWidth, Metrics::TabBar_TabMinHeight ) );

        }

        return size;

    }

    //______________________________________________________________
    bool Style::drawFramePrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        const State& state( option->state );

        // do nothing for flat frames
        const bool isTitleWidget( widget && widget->parent() && widget->parent()->inherits( "KTitleWidget" ) );

        if( !isTitleWidget && !( state & (State_Sunken | State_Raised ) ) ) return true;

        const bool isQtQuickControl = !widget && option && option->styleObject && option->styleObject->inherits( "QQuickStyleItem" );
        const bool isInputWidget( ( widget && widget->testAttribute( Qt::WA_Hover ) ) ||
            ( isQtQuickControl && option->styleObject->property( "elementType" ).toString() == QStringLiteral( "edit") ) );

        const QPalette& palette( option->palette );
        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && isInputWidget && ( state & State_MouseOver ) );
        const bool hasFocus( enabled && ( state & State_HasFocus ) );

        // focus takes precedence over mouse over
        _animations->lineEditEngine().updateState( widget, AnimationFocus, hasFocus );
        _animations->lineEditEngine().updateState( widget, AnimationHover, mouseOver && !hasFocus );

        // retrieve animation mode and opacity
        const AnimationMode mode( _animations->lineEditEngine().frameAnimationMode( widget ) );
        const qreal opacity( _animations->lineEditEngine().frameOpacity( widget ) );

        // update frame shadow factory
        if( _frameShadowFactory->isRegistered( widget ) )
        { _frameShadowFactory->updateState( widget, hasFocus, mouseOver, opacity, mode ); }

        // render
        const QColor background( isTitleWidget ? palette.color( widget->backgroundRole() ):QColor() );
        const QColor outline( _helper->frameOutlineColor( option->palette, mouseOver, hasFocus, opacity, mode ) );
        _helper->renderFrame( painter, option->rect, background, outline, hasFocus );

        return true;

    }

    //______________________________________________________________
    bool Style::drawFrameLineEditPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // render background
        const QColor background( option->palette.color( QPalette::Base ) );
        _helper->renderFrame( painter, option->rect, background );

        // render outline
        drawFramePrimitive( option, painter, widget );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawFrameMenuPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        const QPalette& palette( option->palette );
        const QColor background( palette.color( QPalette::Window ) );
        const QColor outline( _helper->frameOutlineColor( palette ) );

        const bool hasAlpha( _helper->hasAlphaChannel( widget ) );
        if( hasAlpha )
        {

            painter->setCompositionMode( QPainter::CompositionMode_Source );
            _helper->renderMenuFrame( painter, option->rect, background, outline, true );

        } else {

            _helper->renderMenuFrame( painter, option->rect, background, outline, false );

        }

        return true;

    }

    //______________________________________________________________
    bool Style::drawFrameGroupBoxPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionFrame *frameOption( qstyleoption_cast<const QStyleOptionFrame*>( option ) );
        if( !frameOption ) return true;

        // no frame for flat groupboxes
        QStyleOptionFrameV2 frameOption2( *frameOption );
        if( frameOption2.features & QStyleOptionFrameV2::Flat ) return true;

        // normal frame
        const QPalette& palette( option->palette );
        const QColor background( palette.color( QPalette::Window ) );
        const QColor outline( _helper->frameOutlineColor( palette ) );

        // need to reset painter's clip region in order to paint behind textbox label
        // (was taken out in QCommonStyle)
        painter->setClipRegion( option->rect );
        _helper->renderFrame( painter, option->rect, background, outline );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawFrameTabWidgetPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        // tabwidget frame
        // cast option and check
        const QStyleOptionTabWidgetFrame* tabOption( qstyleoption_cast<const QStyleOptionTabWidgetFrame*>( option ) );
        if( !tabOption ) return true;

        // do nothing if tabbar is hidden
        if( tabOption->tabBarSize.isEmpty() ) return true;

        // adjust rect to handle overlaps
        QRect rect( option->rect );

        const QSize tabBarSize( tabOption->tabBarSize );
        Corners corners = CornersAll;

        // adjust corners to deal with oversized tabbars
        switch( tabOption->shape )
        {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
            if( tabBarSize.width() >= rect.width()-2*Metrics::TabBar_TabRadius ) corners &= ~CornersTop;
            break;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            if( tabBarSize.width() >= rect.width()-2*Metrics::TabBar_TabRadius ) corners &= ~CornersBottom;
            break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            if( tabBarSize.height() >= rect.height()-2*Metrics::TabBar_TabRadius ) corners &= ~CornersLeft;
            break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            if( tabBarSize.height() >= rect.height()-2*Metrics::TabBar_TabRadius ) corners &= ~CornersRight;
            break;

            default: break;
        }

        // define colors
        const QPalette& palette( option->palette );
        const QColor background( palette.color( QPalette::Window ) );
        const QColor outline( _helper->frameOutlineColor( palette ) );
        _helper->renderTabWidgetFrame( painter, rect, background, outline, corners );

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawFrameTabBarBasePrimitive( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        /*
        tabbar frame
        used either for 'separate' tabbar, or in 'document mode'
        */

        // cast option and check
        const QStyleOptionTabBarBase* tabOption( qstyleoption_cast<const QStyleOptionTabBarBase*>( option ) );
        if( !tabOption ) return true;

        // get rect, orientation, palette
        const QRect rect( option->rect );
        const QColor outline( _helper->frameOutlineColor( option->palette ) );

        // setup painter
        painter->setBrush( Qt::NoBrush );
        painter->setRenderHint( QPainter::Antialiasing, false );
        painter->setPen( QPen( outline, 1 ) );

        // render
        switch( tabOption->shape )
        {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
            painter->drawLine( rect.bottomLeft(), rect.bottomRight() );
            break;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            painter->drawLine( rect.topLeft(), rect.topRight() );
            break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            painter->drawLine( rect.topRight(), rect.bottomRight() );
            break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            painter->drawLine( rect.topLeft(), rect.bottomLeft() );
            break;

            default:
            break;

        }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawFrameWindowPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        // copy rect and palette
        const QRect& rect( option->rect );
        const QPalette& palette( option->palette );
        const State state( option->state );
        const bool selected( state & State_Selected );

        // render frame outline
        const QColor outline( _helper->frameOutlineColor( palette, false, selected ) );
        _helper->renderMenuFrame( painter, rect, QColor(), outline );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorArrowPrimitive( ArrowOrientation orientation, const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // store rect and palette
        const QRect& rect( option->rect );
        const QPalette& palette( option->palette );

        // store state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        bool mouseOver( enabled && ( state & State_MouseOver ) );
        bool hasFocus( enabled && ( state & State_HasFocus ) );

        // detect buttons in tabbar, for which special rendering is needed
        const bool inTabBar( widget && qobject_cast<const QTabBar*>( widget->parentWidget() ) );

        // get animation state
        /* there is no need to update the engine since this was already done when rendering the frame */
        const AnimationMode mode( _animations->widgetStateEngine().buttonAnimationMode( widget ) );
        const qreal opacity( _animations->widgetStateEngine().buttonOpacity( widget ) );

        // color
        QColor color;
        if( mouseOver ) color = _helper->hoverColor( palette );
        else if( inTabBar && hasFocus ) color = _helper->arrowColor( palette, mouseOver, hasFocus, opacity, mode );
        else color = palette.color( QPalette::WindowText );

        // render
        _helper->renderArrow( painter, rect, color, orientation );

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorHeaderArrowPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {
        const QStyleOptionHeader *headerOption( qstyleoption_cast<const QStyleOptionHeader*>( option ) );
        const State& state( option->state );

        // arrow orientation
        ArrowOrientation orientation( ArrowNone );
        if( state & State_UpArrow || ( headerOption && headerOption->sortIndicator==QStyleOptionHeader::SortUp ) ) orientation = ArrowUp;
        else if( state & State_DownArrow || ( headerOption && headerOption->sortIndicator==QStyleOptionHeader::SortDown ) ) orientation = ArrowDown;
        if( orientation == ArrowNone ) return true;

        // state, rect and palette
        const QRect& rect( option->rect );
        const QPalette& palette( option->palette );

        // define color and polygon for drawing arrow
        const QColor color = palette.color( QPalette::ButtonText );

        // render
        _helper->renderArrow( painter, rect, color, orientation );

        return true;
    }

    //______________________________________________________________
    bool Style::drawPanelButtonCommandPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionButton* buttonOption( qstyleoption_cast< const QStyleOptionButton* >( option ) );
        if( !buttonOption ) return true;

        // rect and palette
        const QRect& rect( option->rect );

        // button state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && ( state & State_MouseOver ) );
        const bool hasFocus( enabled && ( state & State_HasFocus ) );
        const bool sunken( state & ( State_On|State_Sunken ) );
        const bool flat( buttonOption->features & QStyleOptionButton::Flat );

        // update animation state
        // mouse over takes precedence over focus
        _animations->widgetStateEngine().updateState( widget, AnimationHover, mouseOver );
        _animations->widgetStateEngine().updateState( widget, AnimationFocus, hasFocus && !mouseOver );

        const AnimationMode mode( _animations->widgetStateEngine().buttonAnimationMode( widget ) );
        const qreal opacity( _animations->widgetStateEngine().buttonOpacity( widget ) );

        if( flat )
        {

            // define colors and render
            const QPalette& palette( option->palette );
            const QColor color( _helper->toolButtonColor( palette, mouseOver, hasFocus || sunken, opacity, mode ) );
            _helper->renderToolButtonFrame( painter, rect, color, sunken );

        } else {

            // update button color from palette in case button is default
            QPalette palette( option->palette );
            if( enabled && buttonOption->features & QStyleOptionButton::DefaultButton )
            {
                const QColor button( palette.color( QPalette::Button ) );
                const QColor base( palette.color( QPalette::Base ) );
                palette.setColor( QPalette::Button, KColorUtils::mix( button, base, 0.7 ) );
            }

            const QColor shadow( _helper->shadowColor( palette ) );
            const QColor outline( _helper->buttonOutlineColor( palette, mouseOver, hasFocus, opacity, mode ) );
            const QColor background( _helper->buttonBackgroundColor( palette, mouseOver, hasFocus, opacity, mode ) );

            // render
            _helper->renderButtonFrame( painter, rect, background, outline, shadow, hasFocus, sunken );

        }

        return true;

    }

    //______________________________________________________________
    bool Style::drawPanelButtonToolPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // copy palette and rect
        const QPalette& palette( option->palette );
        QRect rect( option->rect );

        // store relevant flags
        const State& state( option->state );
        const bool autoRaise( state & State_AutoRaise );
        const bool enabled( state & State_Enabled );
        const bool sunken( ( state & State_On ) || ( state & State_Sunken ) );
        const bool mouseOver( enabled && (option->state & State_MouseOver) );
        const bool hasFocus( enabled && (option->state & State_HasFocus) );

        // get animation state
        // no need to update, this was already done in drawToolButtonComplexControl
        const AnimationMode mode( _animations->widgetStateEngine().buttonAnimationMode( widget ) );
        const qreal opacity( _animations->widgetStateEngine().buttonOpacity( widget ) );

        if( !autoRaise )
        {

            /* need to check widget for popup mode, because option is not set properly */
            const QToolButton* toolButton( qobject_cast<const QToolButton*>( widget ) );
            const bool hasPopupMenu( toolButton && toolButton->popupMode() == QToolButton::MenuButtonPopup );

            // render as push button
            const QColor shadow( _helper->shadowColor( palette ) );
            const QColor outline( _helper->buttonOutlineColor( palette, mouseOver, hasFocus, opacity, mode ) );
            const QColor background( _helper->buttonBackgroundColor( palette, mouseOver, hasFocus, opacity, mode ) );

            // adjust frame in case of menu
            if( hasPopupMenu )
            {
                painter->setClipRect( rect );
                rect.adjust( 0, 0, Metrics::Frame_FrameRadius, 0 );
                rect = handleRightToLeftLayout( option, rect );
            }

            // render
            _helper->renderButtonFrame( painter, rect, background, outline, shadow, hasFocus, sunken );

        } else {

            const QColor color( _helper->toolButtonColor( palette, mouseOver, hasFocus || sunken, opacity, mode ) );
            _helper->renderToolButtonFrame( painter, rect, color, sunken );

        }

        return true;
    }

    //______________________________________________________________
    bool Style::drawTabBarPanelButtonToolPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // copy palette and rect
        QRect rect( option->rect );

        /* static_cast is safe here since check was already performed in calling function */
        const QTabBar* tabBar( static_cast<QTabBar*>( widget->parentWidget() ) );

        // overlap.
        // subtract 1, because of the empty pixel left the tabwidget frame
        const int overlap( Metrics::TabBar_BaseOverlap - 1 );

        // adjust rect based on tabbar shape
        switch( tabBar->shape() )
        {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
            rect.adjust( 0, 0, 0, -overlap );
            break;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            rect.adjust( 0, overlap, 0, 0 );
            break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            rect.adjust( 0, 0, -overlap, 0 );
            break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            rect.adjust( overlap, 0, 0, 0 );
            break;

            default: break;

        }

        // get the relevant palette
        const QWidget* parent( tabBar->parentWidget() );
        if( qobject_cast<const QTabWidget*>( parent ) ) parent = parent->parentWidget();
        const QPalette palette( parent ? parent->palette() : QGuiApplication::palette() );

        // render flat background
        painter->setPen( Qt::NoPen );
        painter->setBrush( palette.color( QPalette::Window ) );
        painter->drawRect( rect );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawPanelScrollAreaCornerPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // make sure background role matches viewport
        const QAbstractScrollArea* scrollArea;
        if( ( scrollArea = qobject_cast<const QAbstractScrollArea*>( widget ) ) && scrollArea->viewport() )
        {

            // need to adjust clipRect in order not to render outside of frame
            painter->setClipRect( insideMargin( scrollArea->rect(), Metrics::Frame_FrameWidth ) );
            painter->setBrush( option->palette.color( scrollArea->viewport()->backgroundRole() ) );
            painter->setPen( Qt::NoPen );
            painter->drawRect( option->rect );
            return true;

        } else {

            return false;

        }

    }
    //___________________________________________________________________________________
    bool Style::drawPanelMenuPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // do nothing if menu is embedded in another widget
        // this corresponds to having a transparent background
        if( widget && !widget->isWindow() ) return true;

        const QPalette& palette( option->palette );
        const QColor background( _helper->frameBackgroundColor( palette ) );
        const QColor outline( _helper->frameOutlineColor( palette ) );

        const bool hasAlpha( _helper->hasAlphaChannel( widget ) );
        if( hasAlpha )
        {

            painter->setCompositionMode( QPainter::CompositionMode_Source );
            _helper->renderMenuFrame( painter, option->rect, background, outline, true );

        } else {

            _helper->renderMenuFrame( painter, option->rect, background, outline, false );

        }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawPanelTipLabelPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // force registration of widget
        if( widget && widget->window() )
        { _shadowHelper->registerWidget( widget->window(), true ); }

        const QPalette& palette( option->palette );
        const QColor background( palette.color( QPalette::ToolTipBase ) );
        const QColor outline( KColorUtils::mix( palette.color( QPalette::ToolTipBase ), palette.color( QPalette::ToolTipText ), 0.25 ) );

        const bool hasAlpha( _helper->hasAlphaChannel( widget ) );
        if( hasAlpha )
        {

            painter->setCompositionMode( QPainter::CompositionMode_Source );
            _helper->renderMenuFrame( painter, option->rect, background, outline, true );

        } else {

            _helper->renderMenuFrame( painter, option->rect, background, outline, false );

        }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawPanelItemViewItemPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionViewItem *viewItemOption = qstyleoption_cast<const QStyleOptionViewItem*>( option );
        if( !viewItemOption ) return false;

        // try cast widget
        const QAbstractItemView *abstractItemView = qobject_cast<const QAbstractItemView *>( widget );

        // store palette and rect
        const QPalette& palette( option->palette );
        const QRect& rect( option->rect );

        // store flags
        const State& state( option->state );
        const bool mouseOver( ( state & State_MouseOver ) && ( !abstractItemView || abstractItemView->selectionMode() != QAbstractItemView::NoSelection ) );
        const bool selected( state & State_Selected );
        const bool enabled( state & State_Enabled );
        const bool active( state & State_Active );

        const bool hasCustomBackground = viewItemOption->backgroundBrush.style() != Qt::NoBrush && !( state & State_Selected );
        const bool hasSolidBackground = !hasCustomBackground || viewItemOption->backgroundBrush.style() == Qt::SolidPattern;
        const bool hasAlternateBackground( viewItemOption->features & QStyleOptionViewItemV2::Alternate );

        // do nothing if no background is to be rendered
        if( !( mouseOver || selected || hasCustomBackground || hasAlternateBackground ) )
        { return true; }

        // define color group
        QPalette::ColorGroup colorGroup;
        if( enabled ) colorGroup = active ? QPalette::Active : QPalette::Inactive;
        else colorGroup = QPalette::Disabled;

        // render alternate background
        if( viewItemOption && ( viewItemOption->features & QStyleOptionViewItemV2::Alternate ) )
        {
            painter->setPen( Qt::NoPen );
            painter->setBrush( palette.brush( colorGroup, QPalette::AlternateBase ) );
            painter->drawRect( rect );
        }

        // stop here if no highlight is needed
        if( !( mouseOver || selected ||hasCustomBackground ) )
        { return true; }

        // render custom background
        if( hasCustomBackground && !hasSolidBackground )
        {

            painter->setBrushOrigin( viewItemOption->rect.topLeft() );
            painter->setBrush( viewItemOption->backgroundBrush );
            painter->setPen( Qt::NoPen );
            painter->drawRect( viewItemOption->rect );
            return true;

        }

        // render selection
        // define color
        QColor color;
        if( hasCustomBackground && hasSolidBackground ) color = viewItemOption->backgroundBrush.color();
        else color = palette.color( colorGroup, QPalette::Highlight );

        // change color to implement mouse over
        if( mouseOver && !hasCustomBackground )
        {
            if( !selected ) color.setAlphaF( 0.2 );
            else color = color.lighter( 110 );
        }

        // get selection path
        Corners corners;
        const bool hasSingleSelection( abstractItemView && abstractItemView->selectionMode() == QAbstractItemView::SingleSelection );
        if( hasSingleSelection )
        {

            // round relevant corners
            if(
                viewItemOption->viewItemPosition == QStyleOptionViewItemV4::OnlyOne ||
                viewItemOption->viewItemPosition == QStyleOptionViewItemV4::Invalid ||
                ( abstractItemView && abstractItemView->selectionBehavior() != QAbstractItemView::SelectRows ) )
            {

                corners = CornersAll;

            } else {

                if( viewItemOption->viewItemPosition == QStyleOptionViewItemV4::Beginning ) corners |= CornersLeft;
                if( viewItemOption->viewItemPosition == QStyleOptionViewItemV4::End ) corners |= CornersRight;

            }

        }

        // render
        _helper->renderSelection( painter, rect, color, corners );

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorCheckBoxPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // get rect
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && ( state & State_MouseOver ) );
        const bool sunken( enabled && ( state & State_Sunken ) );
        const bool active( ( state & (State_On|State_NoChange) ) );

        // checkbox state
        CheckBoxState checkBoxState( CheckOff );
        if( state & State_NoChange ) checkBoxState = CheckPartial;
        else if( state & State_On ) checkBoxState = CheckOn;

        // animation state
        _animations->widgetStateEngine().updateState( widget, AnimationHover, mouseOver );
        const AnimationMode mode( _animations->widgetStateEngine().isAnimated( widget, AnimationHover ) ? AnimationHover:AnimationNone );
        const qreal opacity( _animations->widgetStateEngine().opacity( widget, AnimationHover ) );

        // colors
        const QPalette& palette( option->palette );
        const QColor color( _helper->checkBoxIndicatorColor( palette, mouseOver, enabled && active, opacity, mode  ) );
        const QColor shadow( _helper->shadowColor( palette ) );

        // render
        _helper->renderCheckBox( painter, option->rect, color, shadow, sunken, checkBoxState );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorRadioButtonPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // get rect
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && ( state & State_MouseOver ) );
        const bool sunken( state & State_Sunken );
        const bool checked( state & State_On );

        // update only mouse over
        _animations->widgetStateEngine().updateState( widget, AnimationHover, mouseOver );
        const AnimationMode mode( _animations->widgetStateEngine().isAnimated( widget, AnimationHover ) ? AnimationHover:AnimationNone );
        const qreal opacity( _animations->widgetStateEngine().opacity( widget, AnimationHover ) );

        // colors
        const QPalette& palette( option->palette );
        const QColor color( _helper->checkBoxIndicatorColor( palette, mouseOver, enabled && checked, opacity, mode  ) );
        const QColor shadow( _helper->shadowColor( palette ) );

        // render
        _helper->renderRadioButton( painter, option->rect, color, shadow, sunken, checked );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorButtonDropDownPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionToolButton* toolButtonOption( qstyleoption_cast<const QStyleOptionToolButton*>( option ) );
        if( !toolButtonOption ) return true;

        // store state
        const State& state( option->state );
        const bool autoRaise( state & State_AutoRaise );

        // for toolbuttons, need to render the relevant part of the frame
        if( (toolButtonOption->subControls & SC_ToolButtonMenu) && !autoRaise )
        {

            // store palette and rect
            const QPalette& palette( option->palette );
            const QRect& rect( option->rect );

            // store state
            const bool enabled( state & State_Enabled );
            const bool hasFocus( enabled && ( state & State_HasFocus ) );
            const bool mouseOver( enabled && ( state & State_MouseOver ) );
            const bool sunken( enabled && ( state & State_Sunken ) );

            // update animation state
            // mouse over takes precedence over focus
            _animations->widgetStateEngine().updateState( widget, AnimationHover, mouseOver );
            _animations->widgetStateEngine().updateState( widget, AnimationFocus, hasFocus && !mouseOver );

            const AnimationMode mode( _animations->widgetStateEngine().buttonAnimationMode( widget ) );
            const qreal opacity( _animations->widgetStateEngine().buttonOpacity( widget ) );

            // render as push button
            const QColor shadow( _helper->shadowColor( palette ) );
            const QColor outline( _helper->buttonOutlineColor( palette, mouseOver, hasFocus, opacity, mode ) );
            const QColor background( _helper->buttonBackgroundColor( palette, mouseOver, hasFocus, opacity, mode ) );

            QRect frameRect( rect );
            painter->setClipRect( rect );
            frameRect.adjust( -Metrics::Frame_FrameRadius, 0, 0, 0 );
            frameRect = handleRightToLeftLayout( option, frameRect );

            // render
            _helper->renderButtonFrame( painter, frameRect, background, outline, shadow, hasFocus, sunken );

            // also render separator
            QRect separatorRect( rect.adjusted( 0, 2, -2, -2 ) );
            separatorRect.setWidth( 1 );
            separatorRect = handleRightToLeftLayout( option, separatorRect );
            _helper->renderSeparator( painter, separatorRect, outline, true );

        }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorTabClosePrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // get icon and check
        QIcon icon( standardIcon( SP_TitleBarCloseButton, option, widget ) );
        if( icon.isNull() ) return false;

        // store state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool active( state & State_Raised );
        const bool sunken( state & State_Sunken );

        // decide icon mode and state
        QIcon::Mode iconMode;
        QIcon::State iconState;
        if( !enabled )
        {
            iconMode = QIcon::Disabled;
            iconState = QIcon::Off;

        } else {

            if( active ) iconMode = QIcon::Active;
            else iconMode = QIcon::Normal;

            iconState = sunken ? QIcon::On : QIcon::Off;
        }

        // icon size
        const int iconWidth( pixelMetric(QStyle::PM_SmallIconSize, option, widget ) );
        const QSize iconSize( iconWidth, iconWidth );

        // get pixmap
        const QPixmap pixmap( icon.pixmap( iconSize, iconMode, iconState ) );

        // render
        drawItemPixmap( painter, option->rect, Qt::AlignCenter, pixmap );
        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorTabTearPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionTab* tabOption( qstyleoption_cast<const QStyleOptionTab*>( option ) );
        if( !tabOption ) return true;

        // store palette and rect
        const QPalette& palette( option->palette );
        QRect rect( option->rect );

        const QColor color( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.2 ) );
        painter->setRenderHint( QPainter::Antialiasing, false );
        painter->setPen( color );
        painter->setBrush( Qt::NoBrush );
        switch( tabOption->shape )
        {

            case QTabBar::TriangularNorth:
            case QTabBar::RoundedNorth:
            rect.adjust( 0, 1, 0, 0 );
            painter->drawLine( rect.topLeft(), rect.bottomLeft() );
            break;

            case QTabBar::TriangularSouth:
            case QTabBar::RoundedSouth:
            rect.adjust( 0, 0, 0, -1 );
            painter->drawLine( rect.topLeft(), rect.bottomLeft() );
            break;

            case QTabBar::TriangularWest:
            case QTabBar::RoundedWest:
            rect.adjust( 1, 0, 0, 0 );
            painter->drawLine( rect.topLeft(), rect.topRight() );
            break;

            case QTabBar::TriangularEast:
            case QTabBar::RoundedEast:
            rect.adjust( 0, 0, -1, 0 );
            painter->drawLine( rect.topLeft(), rect.topRight() );
            break;

            default: break;
        }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorToolBarHandlePrimitive( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {
        // store rect and palette
        const QRect& rect( option->rect );
        const QPalette& palette( option->palette );

        // store flags
        const State& state( option->state );
        const bool horizontal( state & State_Horizontal );

        // handle rect
        QRect handleRect( rect );
        if( horizontal ) handleRect.setWidth( Metrics::ToolBar_HandleWidth );
        else handleRect.setHeight( Metrics::ToolBar_HandleWidth );

        handleRect = centerRect( rect, handleRect.size() );

        // color
        QColor color( KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::WindowText ), 0.3 ) );

        // render
        _helper->renderToolBarHandle( painter, handleRect, color );
        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorToolBarSeparatorPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        // do nothing if disabled from options
        if( !StyleConfigData::toolBarDrawItemSeparator() ) return true;

        const State& state( option->state );
        const bool separatorIsVertical( state & State_Horizontal );
        const QRect& rect( option->rect );
        const QColor color( _helper->separatorColor( option->palette ) );

        _helper->renderSeparator( painter, rect, color, separatorIsVertical );
        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorBranchPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        const State& state( option->state );
        const QRect& rect( option->rect );
        const QPalette& palette( option->palette );

        const bool reverseLayout( option->direction == Qt::RightToLeft );

        //draw expander
        int expanderAdjust = 0;
        if( state & State_Children )
        {

            // state
            const bool expanderOpen( state & State_Open );
            const bool enabled( state & State_Enabled );
            const bool mouseOver( enabled && ( state & State_MouseOver ) );

            // expander rect
            int expanderSize = qMin( rect.width(), rect.height() );
            expanderSize = qMin( expanderSize, (int) Metrics::ItemView_ArrowSize );
            expanderAdjust = expanderSize/2 + 1;
            const QRect arrowRect = centerRect( rect, expanderSize, expanderSize );

            // get orientation from option
            ArrowOrientation orientation;
            if( expanderOpen ) orientation = ArrowDown;
            else if( reverseLayout ) orientation = ArrowLeft;
            else orientation = ArrowRight;

            // color
            const QColor arrowColor( mouseOver ? _helper->hoverColor( palette ) : palette.color( QPalette::Text ) );

            // render
            _helper->renderArrow( painter, arrowRect, arrowColor, orientation );

        }

        // tree branches
        if( !StyleConfigData::viewDrawTreeBranchLines() ) return true;

        const QPoint center( rect.center() );
        const QColor lineColor( KColorUtils::mix( palette.color( QPalette::Base ), palette.color( QPalette::Text ), 0.25 ) );
        painter->setRenderHint( QPainter::Antialiasing, false );
        painter->setPen( lineColor );
        if ( state & ( State_Item | State_Children | State_Sibling ) )
        {
            const QLine line( QPoint( center.x(), rect.top() ), QPoint( center.x(), center.y() - expanderAdjust ) );
            painter->drawLine( line );
        }

        //The right/left (depending on direction ) line gets drawn if we have an item
        if ( state & State_Item )
        {
            const QLine line = reverseLayout ?
                QLine( QPoint( rect.left(), center.y() ), QPoint( center.x() - expanderAdjust, center.y() ) ):
                QLine( QPoint( center.x() + expanderAdjust, center.y() ), QPoint( rect.right(), center.y() ) );
            painter->drawLine( line );

        }

        //The bottom if we have a sibling
        if ( state & State_Sibling )
        {
            const QLine line( QPoint( center.x(), center.y() + expanderAdjust ), QPoint( center.x(), rect.bottom() ) );
            painter->drawLine( line );
        }

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawPushButtonLabelControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionButton* buttonOption( qstyleoption_cast<const QStyleOptionButton*>( option ) );
        if( !buttonOption ) return true;

        // rect
        const QRect rect( option->rect );
        QRect contentsRect( insideMargin( rect, Metrics::Frame_FrameWidth ) );

        // palette
        const QPalette& palette( option->palette );

        // state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool sunken( ( state & State_On ) || ( state & State_Sunken ) );
        const bool mouseOver( enabled && (option->state & State_MouseOver) );
        const bool hasFocus( enabled && !mouseOver && (option->state & State_HasFocus) );
        const bool flat( buttonOption->features & QStyleOptionButton::Flat );

        // alignment
        const int alignment = _mnemonics->textFlags() | Qt::AlignCenter;

        // color role
        QPalette::ColorRole textRole;
        if( flat )
        {

            if( hasFocus && sunken ) textRole = QPalette::HighlightedText;
            else textRole = QPalette::WindowText;

        } else if( hasFocus ) textRole = QPalette::HighlightedText;
        else textRole = QPalette::ButtonText;

        // menu arrow
        if( buttonOption->features & QStyleOptionButton::HasMenu )
        {

            // define rect
            QRect arrowRect( contentsRect );
            arrowRect.setLeft( contentsRect.right() - Metrics::MenuButton_IndicatorWidth );
            arrowRect = centerRect( arrowRect, Metrics::MenuButton_IndicatorWidth, Metrics::MenuButton_IndicatorWidth );

            contentsRect.setRight( arrowRect.left() - Metrics::Button_BoxTextSpace - 1  );
            contentsRect.adjust( Metrics::Button_MarginWidth, Metrics::Button_MarginWidth, 0, -Metrics::Button_MarginWidth );

            arrowRect = handleRightToLeftLayout( option, arrowRect );

            // define color
            const QColor arrowColor( palette.color( textRole ) );

            // render
            _helper->renderArrow( painter, arrowRect, arrowColor, ArrowDown );

        } else contentsRect.adjust( Metrics::Button_MarginWidth, 0, -Metrics::Button_MarginWidth, 0 );

        // text size
        QSize contentsSize;
        if( !buttonOption->text.isEmpty() )
        {
            contentsSize = option->fontMetrics.size( _mnemonics->textFlags(), buttonOption->text );
            if( !buttonOption->icon.isNull() ) contentsSize.rwidth() += Metrics::Button_BoxTextSpace;
        }

        // icon size
        QSize iconSize;
        if( !buttonOption->icon.isNull() )
        {
            iconSize = buttonOption->iconSize;
            if( !iconSize.isValid() ) iconSize = QSize( pixelMetric( PM_SmallIconSize ), pixelMetric( PM_SmallIconSize, option, widget ) );

            contentsSize.setHeight( qMax( contentsSize.height(), iconSize.height() ) );
            contentsSize.rwidth() += iconSize.width();
        }

        // adjust contents rect
        contentsRect = centerRect( contentsRect, contentsSize );

        if( !buttonOption->icon.isNull() )
        {

            // icon rect
            QRect iconRect;
            if( buttonOption->text.isEmpty() ) iconRect = centerRect( contentsRect, iconSize );
            else {

                iconRect = contentsRect;
                iconRect.setWidth( iconSize.width() );
                iconRect = centerRect( iconRect, iconSize );
                contentsRect.setLeft( iconRect.right() + 1 + Metrics::Button_BoxTextSpace );

            }

            iconRect = handleRightToLeftLayout( option, iconRect );

            // icon mode
            QIcon::Mode mode;
            if( hasFocus ) mode = QIcon::Active;
            else if( enabled ) mode = QIcon::Normal;
            else mode = QIcon::Disabled;

            // icon state
            QIcon::State iconState = sunken ? QIcon::On : QIcon::Off;

            // icon
            QPixmap pixmap = buttonOption->icon.pixmap( iconSize, mode, iconState );
            drawItemPixmap( painter, iconRect, alignment, pixmap );

        }

        // text
        if( !buttonOption->text.isEmpty() )
        {
            contentsRect = handleRightToLeftLayout( option, contentsRect );
            drawItemText( painter, contentsRect, alignment, palette, enabled, buttonOption->text, textRole );
        }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawCheckBoxLabelControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionButton* buttonOption( qstyleoption_cast<const QStyleOptionButton*>(option) );
        if( !buttonOption ) return true;

        // copy palette and rect
        const QPalette& palette( option->palette );
        const QRect& rect( option->rect );

        // store state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );

        // text alignment
        const bool reverseLayout( option->direction == Qt::RightToLeft );
        const int alignment = _mnemonics->textFlags() | Qt::AlignVCenter | (reverseLayout ? Qt::AlignRight:Qt::AlignLeft );

        // text rect
        QRect textRect( rect );

        // render icon
        if( !buttonOption->icon.isNull() )
        {
            const QIcon::Mode mode( enabled ? QIcon::Normal : QIcon::Disabled );
            const QPixmap pixmap( buttonOption->icon.pixmap(  buttonOption->iconSize, mode ) );
            drawItemPixmap( painter, rect, alignment, pixmap );

            // adjust rect (copied from QCommonStyle)
            textRect.setLeft( textRect.left() + buttonOption->iconSize.width() + 4 );
            textRect = handleRightToLeftLayout( option, textRect );

        }

        // render text
        if( !buttonOption->text.isEmpty() )
        {
            textRect = option->fontMetrics.boundingRect( textRect, alignment, buttonOption->text );
            drawItemText( painter, textRect, alignment, palette, enabled, buttonOption->text, QPalette::WindowText );

            // check focus state
            const bool hasFocus( enabled && ( state & State_HasFocus ) );

            // update animation state
            _animations->widgetStateEngine().updateState( widget, AnimationFocus, hasFocus );
            const bool isFocusAnimated( _animations->widgetStateEngine().isAnimated( widget, AnimationFocus ) );
            const qreal opacity( _animations->widgetStateEngine().opacity( widget, AnimationFocus ) );

            // focus color
            QColor focusColor;
            if( isFocusAnimated ) focusColor = _helper->alphaColor( _helper->focusColor( palette ), opacity );
            else if( hasFocus ) focusColor =  _helper->focusColor( palette );

            // render focus
            _helper->renderFocusLine( painter, textRect, focusColor );

        }

        return true;

    }


    //___________________________________________________________________________________
    bool Style::drawComboBoxLabelControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        const QStyleOptionComboBox* comboBoxOption( qstyleoption_cast<const QStyleOptionComboBox*>( option ) );
        if( !comboBoxOption ) return false;
        if( comboBoxOption->editable ) return false;

        // need to alter palette for focused buttons
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool sunken( ( state & State_On ) || ( state & State_Sunken ) );
        const bool mouseOver( enabled && (option->state & State_MouseOver) );
        const bool hasFocus( enabled && !mouseOver && (option->state & State_HasFocus) );
        const bool flat( !comboBoxOption->frame );

        QPalette::ColorRole textRole;
        if( flat )  {

            if( hasFocus && sunken ) textRole = QPalette::HighlightedText;
            else textRole = QPalette::WindowText;

        } else if( hasFocus ) textRole = QPalette::HighlightedText;
        else textRole = QPalette::ButtonText;

        // change pen color directly
        painter->setPen( QPen( option->palette.color( textRole ), 1 ) );

        // call base class method
        KStyle::drawControl( CE_ComboBoxLabel, option, painter, widget );
        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawMenuBarItemControl( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionMenuItem* menuItemOption = qstyleoption_cast<const QStyleOptionMenuItem*>( option );
        if ( !menuItemOption ) return true;

        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool selected( enabled && (state & State_Selected) );
        const bool sunken( enabled && (state & State_Sunken) );

        const QPalette& palette( option->palette );
        const QRect& rect( option->rect );

        // get text rect
        const int alignment( Qt::AlignCenter|_mnemonics->textFlags() );
        const QRect textRect = option->fontMetrics.boundingRect( rect, alignment, menuItemOption->text );

        // render text
        drawItemText( painter, textRect, alignment, palette, enabled, menuItemOption->text, QPalette::WindowText );

        // render outline
        if( selected || sunken )
        {

            QColor outlineColor;
            if( sunken ) outlineColor = _helper->focusColor( palette );
            else if( selected ) outlineColor = _helper->hoverColor( palette );

            _helper->renderFocusLine( painter, textRect, outlineColor );

        }

        return true;

    }


    //___________________________________________________________________________________
    bool Style::drawMenuItemControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionMenuItem* menuItemOption = qstyleoption_cast<const QStyleOptionMenuItem*>( option );
        if( !menuItemOption ) return true;
        if( menuItemOption->menuItemType == QStyleOptionMenuItem::EmptyArea ) return true;

        // copy rect and palette
        const QRect& rect( option->rect );
        const QPalette& palette( option->palette );

        // get rect available for contents
        QRect contentsRect( insideMargin( rect,  Metrics::MenuItem_MarginWidth ) );

        // deal with separators
        if( menuItemOption->menuItemType == QStyleOptionMenuItem::Separator )
        {

            // normal separator
            if( menuItemOption->text.isEmpty() && menuItemOption->icon.isNull() )
            {

                const QColor color( _helper->separatorColor( palette ) );
                _helper->renderSeparator( painter, contentsRect, color );
                return true;

            } else {

                // separator can have a title and an icon
                // in that case they are rendered as sunken flat toolbuttons
                QStyleOptionToolButton toolButtonOption( separatorMenuItemOption( menuItemOption, widget ) );
                drawComplexControl( CC_ToolButton, &toolButtonOption, painter, widget );
                return true;

            }

        }

        // store state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool selected( enabled && (state & State_Selected) );
        const bool sunken( enabled && (state & (State_On|State_Sunken) ) );
        const bool reverseLayout( option->direction == Qt::RightToLeft );

        // define relevant rectangles
        // checkbox
        QRect checkBoxRect;
        if( menuItemOption->menuHasCheckableItems )
        {
            checkBoxRect = QRect( contentsRect.left(), contentsRect.top() + (contentsRect.height()-Metrics::CheckBox_Size)/2, Metrics::CheckBox_Size, Metrics::CheckBox_Size );
            contentsRect.setLeft( checkBoxRect.right() + Metrics::MenuItem_BoxTextSpace + 1 );
        }

        // render checkbox indicator
        if( menuItemOption->checkType == QStyleOptionMenuItem::NonExclusive )
        {

            checkBoxRect = handleRightToLeftLayout( option, checkBoxRect );

            // checkbox state
            CheckBoxState state( menuItemOption->checked ? CheckOn : CheckOff );
            const bool active( menuItemOption->checked );
            const QColor color( _helper->checkBoxIndicatorColor( palette, enabled && selected, enabled && active ) );
            const QColor shadow( _helper->shadowColor( palette ) );
            _helper->renderCheckBox( painter, checkBoxRect, color, shadow, sunken, state );

        } else if( menuItemOption->checkType == QStyleOptionMenuItem::Exclusive ) {

            checkBoxRect = handleRightToLeftLayout( option, checkBoxRect );

            const bool active( menuItemOption->checked );
            const QColor color( _helper->checkBoxIndicatorColor( palette, enabled && selected, enabled && active ) );
            const QColor shadow( _helper->shadowColor( palette ) );
            _helper->renderRadioButton( painter, checkBoxRect, color, shadow, sunken, active );

        }

        // icon
        const int iconWidth( qMax( menuItemOption->maxIconWidth, pixelMetric( PM_SmallIconSize, option, widget ) ) );
        QRect iconRect( contentsRect.left(), contentsRect.top() + (contentsRect.height()-iconWidth)/2, iconWidth, iconWidth );
        contentsRect.setLeft( iconRect.right() + Metrics::MenuItem_BoxTextSpace + 1 );

        const QSize iconSize( pixelMetric( PM_SmallIconSize, option, widget ), pixelMetric( PM_SmallIconSize, option, widget ) );
        iconRect = centerRect( iconRect, iconSize );

        if( !menuItemOption->icon.isNull() )
        {

            iconRect = handleRightToLeftLayout( option, iconRect );

            // icon mode
            QIcon::Mode mode;
            if( selected ) mode = QIcon::Active;
            else if( enabled ) mode = QIcon::Normal;
            else mode = QIcon::Disabled;

            // icon state
            const QIcon::State iconState( sunken ? QIcon::On:QIcon::Off );
            const QPixmap icon = menuItemOption->icon.pixmap( iconRect.size(), mode, iconState );
            painter->drawPixmap( iconRect, icon );

        }

        // arrow
        QRect arrowRect( contentsRect.right() - Metrics::MenuButton_IndicatorWidth, contentsRect.top() + (contentsRect.height()-Metrics::MenuButton_IndicatorWidth)/2, Metrics::MenuButton_IndicatorWidth, Metrics::MenuButton_IndicatorWidth );
        contentsRect.setRight( arrowRect.left() -  Metrics::MenuItem_BoxTextSpace - 1 );

        if( menuItemOption->menuItemType == QStyleOptionMenuItem::SubMenu )
        {

            // apply right-to-left layout
            arrowRect = handleRightToLeftLayout( option, arrowRect );

            // arrow orientation
            const ArrowOrientation orientation( reverseLayout ? ArrowLeft:ArrowRight );

            // color
            QColor arrowColor;
            if( sunken ) arrowColor = _helper->focusColor( palette );
            else if( selected ) arrowColor = _helper->hoverColor( palette );
            else arrowColor = palette.color( QPalette::WindowText );

            // render
            _helper->renderArrow( painter, arrowRect, arrowColor, orientation );

        }


        // text
        QRect textRect = contentsRect;
        if( !menuItemOption->text.isEmpty() )
        {

            // adjust textRect
            QString text = menuItemOption->text;
            textRect = centerRect( textRect, textRect.width(), option->fontMetrics.size( _mnemonics->textFlags(), text ).height() );
            textRect = handleRightToLeftLayout( option, textRect );

            // set font
            painter->setFont( menuItemOption->font );

            // locate accelerator and render
            const int tabPosition( text.indexOf( QLatin1Char( '\t' ) ) );
            if( tabPosition >= 0 )
            {
                QString accelerator( text.mid( tabPosition + 1 ) );
                text = text.left( tabPosition );
                drawItemText( painter, textRect, Qt::AlignRight | Qt::AlignVCenter | _mnemonics->textFlags(), palette, enabled, accelerator, QPalette::WindowText );
            }

            // render text
            const int textFlags( Qt::AlignVCenter | (reverseLayout ? Qt::AlignRight : Qt::AlignLeft ) | _mnemonics->textFlags() );

            textRect = option->fontMetrics.boundingRect( textRect, textFlags, text );
            drawItemText( painter, textRect, textFlags, palette, enabled, text, QPalette::WindowText );

            // render hover and focus
            if( selected || sunken )
            {

                QColor outlineColor;
                if( sunken ) outlineColor = _helper->focusColor( palette );
                else if( selected ) outlineColor = _helper->hoverColor( palette );

                _helper->renderFocusLine( painter, textRect, outlineColor );

            }

        }

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawProgressBarControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        const QStyleOptionProgressBar* progressBarOption( qstyleoption_cast<const QStyleOptionProgressBar*>( option ) );
        if( !progressBarOption ) return true;

        // render groove
        QStyleOptionProgressBarV2 progressBarOption2 = *progressBarOption;
        progressBarOption2.rect = subElementRect( SE_ProgressBarGroove, progressBarOption, widget );
        drawProgressBarGrooveControl( &progressBarOption2, painter, widget );

        // enable busy animations
        /* need to check both widget and passed styleObject, used for QML */
        if(( widget || progressBarOption->styleObject ) && _animations->busyIndicatorEngine().enabled() )
        {

            // register QML object if defined
            if( !widget && progressBarOption->styleObject )
            { _animations->busyIndicatorEngine().registerWidget( progressBarOption->styleObject ); }

            _animations->busyIndicatorEngine().setAnimated( widget ? widget : progressBarOption->styleObject, progressBarOption->maximum == 0 && progressBarOption->minimum == 0 );

        }

        // check if animated and pass to option
        if( _animations->busyIndicatorEngine().isAnimated( widget ? widget : progressBarOption->styleObject ) )
        { progressBarOption2.progress = _animations->busyIndicatorEngine().value(); }

        // render contents
        progressBarOption2.rect = subElementRect( SE_ProgressBarContents, progressBarOption, widget );
        drawProgressBarContentsControl( &progressBarOption2, painter, widget );

        // render text
        if( progressBarOption->textVisible )
        {
            progressBarOption2.rect = subElementRect( SE_ProgressBarLabel, progressBarOption, widget );
            drawProgressBarLabelControl( &progressBarOption2, painter, widget );
        }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawProgressBarContentsControl( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        const QStyleOptionProgressBar* progressBarOption( qstyleoption_cast<const QStyleOptionProgressBar*>( option ) );
        if( !progressBarOption ) return true;

        const QStyleOptionProgressBarV2* progressBarOption2( qstyleoption_cast<const QStyleOptionProgressBarV2*>( option ) );

        const QRect& rect( option->rect );
        const QPalette& palette( option->palette );

        // check if anything is to be drawn
        qreal progress = progressBarOption->progress - progressBarOption->minimum;
        const bool busyIndicator = ( progressBarOption->minimum == 0 && progressBarOption->maximum == 0 );
        if( busyIndicator ) progress = _animations->busyIndicatorEngine().value();

        if( busyIndicator )
        {

            const bool horizontal = !progressBarOption2 || progressBarOption2->orientation == Qt::Horizontal;
            const bool reverse = horizontal && option->direction == Qt::RightToLeft;

            const QColor first( palette.color( QPalette::Highlight ) );
            const QColor second( KColorUtils::mix( palette.color( QPalette::Highlight ), palette.color( QPalette::Window ), 0.7 ) );
            _helper->renderProgressBarBusyContents( painter, rect, first, second, horizontal, reverse, progress );

        } else if( progress ) {

            const int steps = qMax( progressBarOption->maximum  - progressBarOption->minimum, 1 );
            const bool horizontal = !progressBarOption2 || progressBarOption2->orientation == Qt::Horizontal;

            //Calculate width fraction
            qreal widthFrac( busyIndicator ?  ProgressBar_BusyIndicatorSize/100.0 : progress/steps );
            widthFrac = qMin( (qreal)1.0, widthFrac );

            // convert the pixel width
            const int indicatorSize( widthFrac*( horizontal ? rect.width():rect.height() ) );

            // do nothing if indicator size is too small
            if( indicatorSize < Metrics::ProgressBar_Thickness ) return true;

            QRect indicatorRect;

            if ( horizontal ) indicatorRect = QRect( rect.x(), rect.y(), indicatorSize, rect.height() );
            else indicatorRect = QRect( rect.x(), rect.bottom()- indicatorSize + 1, rect.width(), indicatorSize );

            // handle right to left
            indicatorRect = handleRightToLeftLayout( option, indicatorRect );
            _helper->renderProgressBarContents( painter, indicatorRect, palette.color( QPalette::Highlight ) );

        }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawProgressBarGrooveControl( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {
        const QPalette& palette( option->palette );
        const QColor color( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.3 ) );
        _helper->renderProgressBarGroove( painter, option->rect, color );
        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawProgressBarLabelControl( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {
        const QStyleOptionProgressBar* progressBarOption( qstyleoption_cast<const QStyleOptionProgressBar*>( option ) );
        if( !progressBarOption ) return true;

        const QRect& rect( option->rect );
        const QPalette& palette( option->palette );
        const State& state( option->state );
        const bool enabled( state & State_Enabled );

        const QStyleOptionProgressBarV2* progressBarOption2( qstyleoption_cast<const QStyleOptionProgressBarV2*>( option ) );
        const bool horizontal = !progressBarOption2 || progressBarOption2->orientation == Qt::Horizontal;
        const bool reverseLayout = ( option->direction == Qt::RightToLeft );

        // rotate label for vertical layout
        if( ! ( horizontal || reverseLayout ) )
        {

            painter->translate( rect.topRight() );
            painter->rotate( 90.0 );

        } else if( !horizontal ) {

            painter->translate( rect.bottomLeft() );
            painter->rotate( -90.0 );

        }

        // define text rect
        const QRect textRect( horizontal? rect : QRect( 0, 0, rect.height(), rect.width() ) );
        Qt::Alignment hAlign( ( progressBarOption->textAlignment == Qt::AlignLeft ) ? Qt::AlignHCenter : progressBarOption->textAlignment );
        drawItemText( painter, textRect, Qt::AlignBottom | hAlign, palette, enabled, progressBarOption->text, QPalette::WindowText );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawScrollBarSliderControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionSlider *sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if( !sliderOption ) return true;

        painter->setClipRect( option->rect );

        // define handle rect
        QRect handleRect;
        const State& state( option->state );
        const bool horizontal( state & State_Horizontal );
        if( horizontal ) handleRect = centerRect( option->rect, option->rect.width(), Metrics::ScrollBar_SliderWidth );
        else handleRect = centerRect( option->rect, Metrics::ScrollBar_SliderWidth, option->rect.height() );

        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && ( state & State_MouseOver ) );

        // check focus from relevant parent
        const QWidget* parent( scrollBarParent( widget ) );
        const bool focus( enabled && parent && parent->hasFocus() );

        // enable animation state
        _animations->scrollBarEngine().updateState( widget, enabled && ( sliderOption->activeSubControls & SC_ScrollBarSlider ) );
        const qreal opacity( _animations->scrollBarEngine().opacity( widget, SC_ScrollBarSlider ) );
        {
            // render handle
            // define colors
            QColor color;
            const QPalette& palette( option->palette );

            const QColor base( focus ?
                _helper->focusColor( palette ):
                _helper->alphaColor( palette.color( QPalette::WindowText ), 0.5 ) );

            const QColor highlight( _helper->hoverColor( palette ) );
            if( opacity >= 0 ) color = KColorUtils::mix( base, highlight, opacity );
            else if( mouseOver ) color = highlight;
            else color = base;

            // render
            _helper->renderScrollBarHandle( painter, handleRect, color );

        }

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawScrollBarAddLineControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // do nothing if no buttons are defined
        if( _addLineButtons == NoButton ) return true;

        // cast option and check
        const QStyleOptionSlider* sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if ( !sliderOption ) return true;

        const State& state( option->state );
        const bool horizontal( state & State_Horizontal );
        const bool reverseLayout( option->direction == Qt::RightToLeft );

        // adjust rect, based on number of buttons to be drawn
        QRect rect( scrollBarInternalSubControlRect( sliderOption, SC_ScrollBarAddLine ) );

        QColor color;
        QStyleOptionSlider copy( *sliderOption );
        if( _addLineButtons == DoubleButton )
        {

            if( horizontal )
            {

                //Draw the arrows
                const QSize halfSize( rect.width()/2, rect.height() );
                const QRect leftSubButton( rect.topLeft(), halfSize );
                const QRect rightSubButton( leftSubButton.topRight() + QPoint( 1, 0 ), halfSize );

                copy.rect = leftSubButton;
                color = scrollBarArrowColor( &copy,  reverseLayout ? SC_ScrollBarAddLine:SC_ScrollBarSubLine, widget );
                _helper->renderArrow( painter, leftSubButton, color, ArrowLeft );

                copy.rect = rightSubButton;
                color = scrollBarArrowColor( &copy,  reverseLayout ? SC_ScrollBarSubLine:SC_ScrollBarAddLine, widget );
                _helper->renderArrow( painter, rightSubButton, color, ArrowRight );

            } else {

                const QSize halfSize( rect.width(), rect.height()/2 );
                const QRect topSubButton( rect.topLeft(), halfSize );
                const QRect botSubButton( topSubButton.bottomLeft() + QPoint( 0, 1 ), halfSize );

                copy.rect = topSubButton;
                color = scrollBarArrowColor( &copy, SC_ScrollBarSubLine, widget );
                _helper->renderArrow( painter, topSubButton, color, ArrowUp );

                copy.rect = botSubButton;
                color = scrollBarArrowColor( &copy, SC_ScrollBarAddLine, widget );
                _helper->renderArrow( painter, botSubButton, color, ArrowDown );

            }

        } else if( _addLineButtons == SingleButton ) {

            copy.rect = rect;
            color = scrollBarArrowColor( &copy,  SC_ScrollBarAddLine, widget );
            if( horizontal )
            {

                if( reverseLayout ) _helper->renderArrow( painter, rect, color, ArrowLeft );
                else _helper->renderArrow( painter, rect.translated( 1, 0 ), color, ArrowRight );

            } else _helper->renderArrow( painter, rect.translated( 0, 1 ), color, ArrowDown );

        }

        return true;
    }


    //___________________________________________________________________________________
    bool Style::drawScrollBarSubLineControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // do nothing if no buttons are set
        if( _subLineButtons == NoButton ) return true;

        // cast option and check
        const QStyleOptionSlider* sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if ( !sliderOption ) return true;

        const State& state( option->state );
        const bool horizontal( state & State_Horizontal );
        const bool reverseLayout( option->direction == Qt::RightToLeft );

        // colors
        const QPalette& palette( option->palette );
        const QColor background( palette.color( QPalette::Window ) );

        // adjust rect, based on number of buttons to be drawn
        QRect rect( scrollBarInternalSubControlRect( sliderOption, SC_ScrollBarSubLine ) );

        QColor color;
        QStyleOptionSlider copy( *sliderOption );
        if( _subLineButtons == DoubleButton )
        {

            if( horizontal )
            {

                //Draw the arrows
                const QSize halfSize( rect.width()/2, rect.height() );
                const QRect leftSubButton( rect.topLeft(), halfSize );
                const QRect rightSubButton( leftSubButton.topRight() + QPoint( 1, 0 ), halfSize );

                copy.rect = leftSubButton;
                color = scrollBarArrowColor( &copy,  reverseLayout ? SC_ScrollBarAddLine:SC_ScrollBarSubLine, widget );
                _helper->renderArrow( painter, leftSubButton, color, ArrowLeft );

                copy.rect = rightSubButton;
                color = scrollBarArrowColor( &copy,  reverseLayout ? SC_ScrollBarSubLine:SC_ScrollBarAddLine, widget );
                _helper->renderArrow( painter, rightSubButton, color, ArrowRight );

            } else {

                const QSize halfSize( rect.width(), rect.height()/2 );
                const QRect topSubButton( rect.topLeft(), halfSize );
                const QRect botSubButton( topSubButton.bottomLeft() + QPoint( 0, 1 ), halfSize );

                copy.rect = topSubButton;
                color = scrollBarArrowColor( &copy, SC_ScrollBarSubLine, widget );
                _helper->renderArrow( painter, topSubButton, color, ArrowUp );

                copy.rect = botSubButton;
                color = scrollBarArrowColor( &copy, SC_ScrollBarAddLine, widget );
                _helper->renderArrow( painter, botSubButton, color, ArrowDown );

            }

        } else if( _subLineButtons == SingleButton ) {

            copy.rect = rect;
            color = scrollBarArrowColor( &copy,  SC_ScrollBarSubLine, widget );
            if( horizontal )
            {

                if( reverseLayout ) _helper->renderArrow( painter, rect.translated( 1, 0 ), color, ArrowRight );
                else _helper->renderArrow( painter, rect, color, ArrowLeft );

            } else _helper->renderArrow( painter, rect, color, ArrowUp );

        }

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawShapedFrameControl( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionFrameV3* frameOpt = qstyleoption_cast<const QStyleOptionFrameV3*>( option );
        if( !frameOpt ) return false;

        switch( frameOpt->frameShape )
        {

            case QFrame::Box:
            {
                if( option->state & State_Sunken ) return true;
                else break;
            }

            case QFrame::HLine:
            case QFrame::VLine:
            {

                const QRect& rect( option->rect );
                const QColor color( _helper->separatorColor( option->palette ) );
                const bool isVertical( frameOpt->frameShape == QFrame::VLine );
                _helper->renderSeparator( painter, rect, color, isVertical );
                return true;
            }

            default: break;

        }

        return false;

    }

    //___________________________________________________________________________________
    bool Style::drawRubberBandControl( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        const QPalette& palette( option->palette );
        const QRect rect( option->rect );

        QColor color = palette.color( QPalette::Highlight );
        painter->setPen( KColorUtils::mix( color, palette.color( QPalette::Active, QPalette::WindowText ) ) );
        color.setAlpha( 50 );
        painter->setBrush( color );
        painter->setClipRegion( rect );
        painter->drawRect( rect.adjusted( 0,0,-1,-1 ) );
        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawHeaderSectionControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        const QRect& rect( option->rect );
        const QPalette& palette( option->palette );
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && ( state & State_MouseOver ) );
        const bool sunken( enabled && ( state & (State_On|State_Sunken) ) );

        const QStyleOptionHeader* headerOption( qstyleoption_cast<const QStyleOptionHeader*>( option ) );
        if( !headerOption ) return true;

        const bool horizontal( headerOption->orientation == Qt::Horizontal );
        const bool isFirst( horizontal && ( headerOption->position == QStyleOptionHeader::Beginning ) );
        const bool isCorner( widget && widget->inherits( "QTableCornerButton" ) );
        const bool reverseLayout( option->direction == Qt::RightToLeft );

        // update animation state
        _animations->headerViewEngine().updateState( widget, rect.topLeft(), mouseOver );
        const bool animated( enabled && _animations->headerViewEngine().isAnimated( widget, rect.topLeft() ) );
        const qreal opacity( _animations->headerViewEngine().opacity( widget, rect.topLeft() ) );

        // fill
        const QColor normal( palette.color( QPalette::Button ) );
        const QColor focus( KColorUtils::mix( normal, _helper->focusColor( palette ), 0.2 ) );
        const QColor hover( KColorUtils::mix( normal, _helper->hoverColor( palette ), 0.2 ) );

        QColor color;
        if( sunken ) color = focus;
        else if( animated ) color = KColorUtils::mix( normal, hover, opacity );
        else if( mouseOver ) color = hover;
        else color = normal;

        painter->setRenderHint( QPainter::Antialiasing, false );
        painter->setBrush( color );
        painter->setPen( Qt::NoPen );
        painter->drawRect( rect );

        // outline
        painter->setBrush( Qt::NoBrush );
        painter->setPen( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.1 ) );

        if( isCorner )
        {

            if( reverseLayout ) painter->drawPoint( rect.bottomLeft() );
            else painter->drawPoint( rect.bottomRight() );


        } else if( horizontal ) {

            painter->drawLine( rect.bottomLeft(), rect.bottomRight() );

        } else {

            if( reverseLayout ) painter->drawLine( rect.topLeft(), rect.bottomLeft() );
            else painter->drawLine( rect.topRight(), rect.bottomRight() );

        }

        // separators
        painter->setPen( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.2 ) );

        if( horizontal )
        {
            if( headerOption->section != 0 || isFirst )
            {

                if( reverseLayout ) painter->drawLine( rect.topLeft(), rect.bottomLeft() - QPoint( 0, 1 ) );
                else painter->drawLine( rect.topRight(), rect.bottomRight() - QPoint( 0, 1 ) );

            }

        } else {

            if( reverseLayout ) painter->drawLine( rect.bottomLeft()+QPoint( 1, 0 ), rect.bottomRight() );
            else painter->drawLine( rect.bottomLeft(), rect.bottomRight() - QPoint( 1, 0 ) );

        }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawHeaderEmptyAreaControl( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        // use the same background as in drawHeaderPrimitive
        const QRect& rect( option->rect );
        QPalette palette( option->palette );

        const bool horizontal( option->state & QStyle::State_Horizontal );
        const bool reverseLayout( option->direction == Qt::RightToLeft );

        // fill
        painter->setRenderHint( QPainter::Antialiasing, false );
        painter->setBrush( palette.color( QPalette::Button ) );
        painter->setPen( Qt::NoPen );
        painter->drawRect( rect );

        // outline
        painter->setBrush( Qt::NoBrush );
        painter->setPen( _helper->alphaColor( palette.color( QPalette::ButtonText ), 0.1 ) );

        if( horizontal ) {

            painter->drawLine( rect.bottomLeft(), rect.bottomRight() );

        } else {

            if( reverseLayout ) painter->drawLine( rect.topLeft(), rect.bottomLeft() );
            else painter->drawLine( rect.topRight(), rect.bottomRight() );

        }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawTabBarTabLabelControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // call parent style method
        KStyle::drawControl( CE_TabBarTabLabel, option, painter, widget );

        // store rect and palette
        const QRect& rect( option->rect );
        const QPalette& palette( option->palette );

        // check focus
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool selected( state & State_Selected );
        const bool hasFocus( enabled && selected && (state & State_HasFocus) );

        // update mouse over animation state
        _animations->tabBarEngine().updateState( widget, rect.topLeft(), AnimationFocus, hasFocus );
        const bool animated( enabled && selected && _animations->tabBarEngine().isAnimated( widget, rect.topLeft(), AnimationFocus ) );
        const qreal opacity( _animations->tabBarEngine().opacity( widget, rect.topLeft(), AnimationFocus ) );

        if( !( hasFocus || animated ) ) return true;

        // code is copied from QCommonStyle, but adds focus
        // cast option and check
        const QStyleOptionTab *tabOption( qstyleoption_cast<const QStyleOptionTab*>(option) );
        if( !tabOption || tabOption->text.isEmpty() ) return true;

        // tab option rect
        const bool verticalTabs( isVerticalTab( tabOption ) );
        const int alignment = Qt::AlignCenter | _mnemonics->textFlags();

        // text rect
        QRect textRect( subElementRect(SE_TabBarTabText, option, widget) );

        if( verticalTabs )
        {

            // properly rotate painter
            painter->save();
            int newX, newY, newRot;
            if( tabOption->shape == QTabBar::RoundedEast || tabOption->shape == QTabBar::TriangularEast)
            {

                newX = rect.width() + rect.x();
                newY = rect.y();
                newRot = 90;

            } else {

                newX = rect.x();
                newY = rect.y() + rect.height();
                newRot = -90;

            }

            QTransform transform;
            transform.translate( newX, newY );
            transform.rotate(newRot);
            painter->setTransform( transform, true );

        }

        // adjust text rect based on font metrics
        textRect = option->fontMetrics.boundingRect( textRect, alignment, tabOption->text );

        // focus color
        QColor focusColor;
        if( animated ) focusColor = _helper->alphaColor( _helper->focusColor( palette ), opacity );
        else if( hasFocus ) focusColor =  _helper->focusColor( palette );

        // render focus line
        _helper->renderFocusLine( painter, textRect, focusColor );

        if( verticalTabs ) painter->restore();

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawTabBarTabShapeControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        const QStyleOptionTab* tabOption( qstyleoption_cast<const QStyleOptionTab*>( option ) );
        if( !tabOption ) return true;

        // palette and state
        const QPalette& palette( option->palette );
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool selected( state & State_Selected );
        const bool mouseOver( enabled && !selected && ( state & State_MouseOver ) );

        // check if tab is being dragged
        const bool isDragged( widget && selected && painter->device() != widget );
        const bool isLocked( widget && _tabBarData->isLocked( widget ) );

        // store rect
        QRect rect( option->rect );

        // update mouse over animation state
        _animations->tabBarEngine().updateState( widget, rect.topLeft(), AnimationHover, mouseOver );
        const bool animated( enabled && !selected && _animations->tabBarEngine().isAnimated( widget, rect.topLeft(), AnimationHover ) );
        const qreal opacity( _animations->tabBarEngine().opacity( widget, rect.topLeft(), AnimationHover ) );

        // lock state
        if( selected && widget && isDragged ) _tabBarData->lock( widget );
        else if( widget && selected  && _tabBarData->isLocked( widget ) ) _tabBarData->release();

        // tab position
        const QStyleOptionTab::TabPosition& position = tabOption->position;
        const bool isSingle( position == QStyleOptionTab::OnlyOneTab );
        bool isFirst( isSingle || position == QStyleOptionTab::Beginning );
        bool isLast( isSingle || position == QStyleOptionTab::End );
        bool isLeftOfSelected( !isLocked && tabOption->selectedPosition == QStyleOptionTab::NextIsSelected );
        bool isRightOfSelected( !isLocked && tabOption->selectedPosition == QStyleOptionTab::PreviousIsSelected );

        // true if widget is aligned to the frame
        /* need to check for 'isRightOfSelected' because for some reason the isFirst flag is set when active tab is being moved */
        isFirst &= !isRightOfSelected;
        isLast &= !isLeftOfSelected;

        // swap state based on reverse layout, so that they become layout independent
        const bool reverseLayout( option->direction == Qt::RightToLeft );
        const bool verticalTabs( isVerticalTab( tabOption ) );
        if( reverseLayout && !verticalTabs )
        {
            qSwap( isFirst, isLast );
            qSwap( isLeftOfSelected, isRightOfSelected );
        }

        // adjust rect and define corners based on tabbar orientation
        Corners corners;
        switch( tabOption->shape )
        {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
            if( selected ) {

                corners = CornerTopLeft|CornerTopRight;
                rect.adjust( 0, 0, 0, Metrics::TabBar_TabRadius );

            } else {

                rect.adjust( 0, 0, 0, -1 );
                if( isFirst ) corners |= CornerTopLeft;
                if( isLast ) corners |= CornerTopRight;
                if( isRightOfSelected ) rect.adjust( -Metrics::TabBar_TabRadius, 0, 0, 0 );
                if( isLeftOfSelected ) rect.adjust( 0, 0, Metrics::TabBar_TabOverlap, 0 );
                else if( !isLast ) rect.adjust( 0, 0, Metrics::TabBar_TabOverlap, 0 );

            }
            break;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            if( selected ) {

                corners = CornerBottomLeft|CornerBottomRight;
                rect.adjust( 0, -Metrics::TabBar_TabRadius, 0, 0 );

            } else {

                rect.adjust( 0, 1, 0, 0 );
                if( isFirst ) corners |= CornerBottomLeft;
                if( isLast ) corners |= CornerBottomRight;
                if( isRightOfSelected ) rect.adjust( -Metrics::TabBar_TabRadius, 0, 0, 0 );
                if( isLeftOfSelected ) rect.adjust( 0, 0, Metrics::TabBar_TabOverlap, 0 );
                else if( !isLast ) rect.adjust( 0, 0, Metrics::TabBar_TabOverlap, 0 );

            }
            break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            if( selected )
            {
                corners = CornerTopLeft|CornerBottomLeft;
                rect.adjust( 0, 0, Metrics::TabBar_TabRadius, 0 );

            } else {

                rect.adjust( 0, 0, -1, 0 );
                if( isFirst ) corners |= CornerTopLeft;
                if( isLast ) corners |= CornerBottomLeft;
                if( isRightOfSelected ) rect.adjust( 0, -Metrics::TabBar_TabRadius, 0, 0 );
                if( isLeftOfSelected ) rect.adjust( 0, 0, 0, Metrics::TabBar_TabRadius );
                else if( !isLast ) rect.adjust( 0, 0, 0, Metrics::TabBar_TabOverlap );

            }
            break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            if( selected )
            {

                corners = CornerTopRight|CornerBottomRight;
                rect.adjust( -Metrics::TabBar_TabRadius, 0, 0, 0 );

            } else {

                rect.adjust( 1, 0, 0, 0 );
                if( isFirst ) corners |= CornerTopRight;
                if( isLast ) corners |= CornerBottomRight;
                if( isRightOfSelected ) rect.adjust( 0, -Metrics::TabBar_TabRadius, 0, 0 );
                if( isLeftOfSelected ) rect.adjust( 0, 0, 0, Metrics::TabBar_TabRadius );
                else if( !isLast ) rect.adjust( 0, 0, 0, Metrics::TabBar_TabOverlap );

            }
            break;

            default: break;
        }

        // color
        QColor color;
        if( selected ) color = palette.color( QPalette::Window );
        else {

            const QColor normal( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.2 ) );
            const QColor hover( _helper->alphaColor( _helper->hoverColor( palette ), 0.2 ) );
            if( animated ) color = KColorUtils::mix( normal, hover, opacity );
            else if( mouseOver ) color = hover;
            else color = normal;

        }

        // outline
        const QColor outline( selected ? _helper->alphaColor( palette.color( QPalette::WindowText ), 0.25 ) : QColor() );

        // render
        _helper->renderTabBarTab( painter, rect, color, outline, corners );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawToolBoxTabLabelControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // rendering is similar to drawPushButtonLabelControl

        // cast option and check
        const QStyleOptionToolBox* toolBoxOption( qstyleoption_cast<const QStyleOptionToolBox *>( option ) );
        if( !toolBoxOption ) return true;

        // copy palette
        const QPalette& palette( option->palette );

        const State& state( option->state );
        const bool enabled( state & State_Enabled );

        // text alignment
        const int alignment = _mnemonics->textFlags() | Qt::AlignCenter;

        // contents rect
        const QRect rect( toolBoxTabContentsRect( option, widget ) );

        // store icon size
        const int iconSize( pixelMetric( QStyle::PM_SmallIconSize, option, widget ) );

        // find contents size and rect
        QRect contentsRect( rect );
        QSize contentsSize;
        if( !toolBoxOption->text.isEmpty() )
        {
            contentsSize = option->fontMetrics.size( _mnemonics->textFlags(), toolBoxOption->text );
            if( !toolBoxOption->icon.isNull() ) contentsSize.rwidth() += Metrics::ToolBox_TabBoxTextSpace;
        }

        // icon size
        if( !toolBoxOption->icon.isNull() )
        {

            contentsSize.setHeight( qMax( contentsSize.height(), iconSize ) );
            contentsSize.rwidth() += iconSize;

        }

        // adjust contents rect
        contentsRect = centerRect( contentsRect, contentsSize );

        // render icon
        if( !toolBoxOption->icon.isNull() )
        {

            // icon rect
            QRect iconRect;
            if( toolBoxOption->text.isEmpty() ) iconRect = centerRect( contentsRect, iconSize, iconSize );
            else {

                iconRect = contentsRect;
                iconRect.setWidth( iconSize );
                iconRect = centerRect( iconRect, iconSize, iconSize );
                contentsRect.setLeft( iconRect.right() + 1 + Metrics::ToolBox_TabBoxTextSpace );

            }

            iconRect = handleRightToLeftLayout( option, iconRect );
            const QIcon::Mode mode( enabled ? QIcon::Normal : QIcon::Disabled );
            const QPixmap pixmap( toolBoxOption->icon.pixmap( iconSize, mode ) );
            drawItemPixmap( painter, iconRect, alignment, pixmap );

        }

        // render text
        if( !toolBoxOption->text.isEmpty() )
        {
            contentsRect = handleRightToLeftLayout( option, contentsRect );
            drawItemText( painter, contentsRect, alignment, palette, enabled, toolBoxOption->text, QPalette::WindowText );
        }

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawToolBoxTabShapeControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionToolBox* toolBoxOption( qstyleoption_cast<const QStyleOptionToolBox *>( option ) );
        if( !toolBoxOption ) return true;

        // copy rect and palette
        const QRect& rect( option->rect );
        const QRect tabRect( toolBoxTabContentsRect( option, widget ) );

        /*
        important: option returns the wrong palette.
        we use the widget palette instead, when set
        */
        const QPalette palette( widget ? widget->palette() : option->palette );

        // store flags
        const State& flags( option->state );
        const bool enabled( flags&State_Enabled );
        const bool selected( flags&State_Selected );
        const bool mouseOver( enabled && !selected && ( flags&State_MouseOver ) );

        // update animation state
        /*
        the proper widget ( the toolbox tab ) is not passed as argument by Qt.
        What is passed is the toolbox directly. To implement animations properly,
        the painter->device() is used instead
        */
        bool isAnimated( false );
        qreal opacity( AnimationData::OpacityInvalid );
        QPaintDevice* device = painter->device();
        if( enabled && device )
        {
            _animations->toolBoxEngine().updateState( device, mouseOver );
            isAnimated = _animations->toolBoxEngine().isAnimated( device );
            opacity = _animations->toolBoxEngine().opacity( device );
        }

        // color
        QColor outline;
        if( selected ) outline = _helper->focusColor( palette );
        else outline = _helper->frameOutlineColor( palette, mouseOver, false, opacity, isAnimated ? AnimationHover:AnimationNone );

        // render
        _helper->renderToolBoxFrame( painter, rect, tabRect.width(), outline );

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawDockWidgetTitleControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionDockWidget* dockWidgetOption = ::qstyleoption_cast<const QStyleOptionDockWidget*>( option );
        if ( !dockWidgetOption ) return true;

        const QPalette& palette( option->palette );
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool reverseLayout( option->direction == Qt::RightToLeft );

        // cast to v2 to check vertical bar
        const QStyleOptionDockWidgetV2 *v2 = qstyleoption_cast<const QStyleOptionDockWidgetV2*>( option );
        const bool verticalTitleBar( v2 ? v2->verticalTitleBar : false );

        const QRect buttonRect( subElementRect( dockWidgetOption->floatable ? SE_DockWidgetFloatButton : SE_DockWidgetCloseButton, option, widget ) );

        // get rectangle and adjust to properly accounts for buttons
        QRect rect( insideMargin( dockWidgetOption->rect, Frame_FrameWidth ) );
        if( verticalTitleBar )
        {

            if( buttonRect.isValid() ) rect.setTop( buttonRect.bottom()+1 );

        } else if( reverseLayout ) {

            if( buttonRect.isValid() ) rect.setLeft( buttonRect.right()+1 );
            rect.adjust( 0,0,-4,0 );

        } else {

            if( buttonRect.isValid() ) rect.setRight( buttonRect.left()-1 );
            rect.adjust( 4,0,0,0 );

        }

        QString title( dockWidgetOption->title );
        int titleWidth = dockWidgetOption->fontMetrics.size( _mnemonics->textFlags(), title ).width();
        int width = verticalTitleBar ? rect.height() : rect.width();
        if( width < titleWidth ) title = dockWidgetOption->fontMetrics.elidedText( title, Qt::ElideRight, width, Qt::TextShowMnemonic );

        if( verticalTitleBar )
        {

            QSize size = rect.size();
            size.transpose();
            rect.setSize( size );

            painter->save();
            painter->translate( rect.left(), rect.top() + rect.width() );
            painter->rotate( -90 );
            painter->translate( -rect.left(), -rect.top() );
            drawItemText( painter, rect, Qt::AlignLeft | Qt::AlignVCenter | _mnemonics->textFlags(), palette, enabled, title, QPalette::WindowText );
            painter->restore();


        } else {

            drawItemText( painter, rect, Qt::AlignLeft | Qt::AlignVCenter | _mnemonics->textFlags(), palette, enabled, title, QPalette::WindowText );

        }

        return true;


    }

    //______________________________________________________________
    bool Style::drawGroupBoxComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        // base class method
        KStyle::drawComplexControl( CC_GroupBox, option, painter, widget );

        // cast option and check
        const QStyleOptionGroupBox *groupBoxOption = qstyleoption_cast<const QStyleOptionGroupBox*>( option );
        if( !groupBoxOption ) return true;

        // do nothing if either label is not selected or groupbox is empty
        if( !(option->subControls & QStyle::SC_GroupBoxLabel) || groupBoxOption->text.isEmpty() )
        { return true; }

        // store palette and rect
        const QPalette& palette( option->palette );

        // check focus state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool hasFocus( enabled && (option->state & State_HasFocus) );
        if( !hasFocus ) return true;

        // alignment
        const int alignment = groupBoxOption->textAlignment | _mnemonics->textFlags();

        // update animation state
        _animations->widgetStateEngine().updateState( widget, AnimationFocus, hasFocus );
        const bool isFocusAnimated( _animations->widgetStateEngine().isAnimated( widget, AnimationFocus ) );
        const qreal opacity( _animations->widgetStateEngine().opacity( widget, AnimationFocus ) );

        // get relevant rect
        QRect textRect = subControlRect( CC_GroupBox, option, SC_GroupBoxLabel, widget );
        textRect = option->fontMetrics.boundingRect( textRect, alignment, groupBoxOption->text );

        // focus color
        QColor focusColor;
        if( isFocusAnimated ) focusColor = _helper->alphaColor( _helper->focusColor( palette ), opacity );
        else if( hasFocus ) focusColor =  _helper->focusColor( palette );

        // render focus
        _helper->renderFocusLine( painter, textRect, focusColor );

        return true;

    }

    //______________________________________________________________
    bool Style::drawToolButtonComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {
        // cast option and check
        const QStyleOptionToolButton* toolButtonOption( qstyleoption_cast<const QStyleOptionToolButton*>( option ) );
        if( !toolButtonOption ) return true;

        // need to alter palette for focused buttons
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool sunken( ( state & State_On ) || ( state & State_Sunken ) );
        const bool mouseOver( enabled && (option->state & State_MouseOver) );
        const bool hasFocus( enabled && (option->state & State_HasFocus) );
        const bool autoRaise( state & State_AutoRaise );

        // update animation state
        // mouse over takes precedence over focus
        _animations->widgetStateEngine().updateState( widget, AnimationHover, mouseOver );
        _animations->widgetStateEngine().updateState( widget, AnimationFocus, hasFocus && !mouseOver );

        // detect buttons in tabbar, for which special rendering is needed
        const bool inTabBar( widget && qobject_cast<const QTabBar*>( widget->parentWidget() ) );

        // define text role for label and arrow
        QPalette::ColorRole textRole;
        if( inTabBar ) {

            textRole = QPalette::WindowText;

        } else if( autoRaise ) {

            if( sunken && !mouseOver ) textRole = QPalette::HighlightedText;
            else textRole = QPalette::WindowText;

        } else if( hasFocus && !mouseOver ) textRole = QPalette::HighlightedText;
        else textRole = QPalette::ButtonText;

        // copy option and alter palette
        QStyleOptionToolButton copy( *toolButtonOption );
        copy.palette.setColor( QPalette::WindowText, option->palette.color( textRole ) );
        copy.palette.setColor( QPalette::ButtonText, option->palette.color( textRole ) );

        const bool hasPopupMenu( toolButtonOption->subControls & SC_ToolButtonMenu );
        const bool hasInlineIndicator( toolButtonOption->features & QStyleOptionToolButton::HasMenu && !hasPopupMenu );

        const QRect buttonRect( subControlRect( CC_ToolButton, option, SC_ToolButton, widget ) );
        const QRect menuRect( subControlRect( CC_ToolButton, option, SC_ToolButtonMenu, widget ) );

        // frame
        if( toolButtonOption->subControls & SC_ToolButton )
        {
            copy.rect = buttonRect;
            if( inTabBar ) drawTabBarPanelButtonToolPrimitive( &copy, painter, widget );
            else drawPrimitive( PE_PanelButtonTool, &copy, painter, widget);
        }

        // arrow
        if( toolButtonOption->subControls & SC_ToolButtonMenu )
        {
            copy.rect = menuRect;
            if( !autoRaise )
            { drawPrimitive( PE_IndicatorButtonDropDown, &copy, painter, widget ); }

            if( !( autoRaise && ( toolButtonOption->activeSubControls & SC_ToolButtonMenu ) ) )
            { copy.state &= ~State_MouseOver; }

            drawPrimitive( PE_IndicatorArrowDown, &copy, painter, widget );

        } else if( hasInlineIndicator ) {

            copy.rect = menuRect;
            copy.state &= ~State_MouseOver;
            drawPrimitive( PE_IndicatorArrowDown, &copy, painter, widget );

        }

        // contents
        {

            // restore state
            copy.state = state;

            // define contents rect
            QRect contentsRect( buttonRect );

            // detect dock widget title button
            /* for dockwidget title buttons, do not take out margins, so that icon do not get scaled down */
            const bool isDockWidgetTitleButton( widget && widget->inherits( "QDockWidgetTitleButton" ) );
            if( isDockWidgetTitleButton )
            {

                // cast to abstract button
                // adjust state to have correct icon rendered
                const QAbstractButton* button( qobject_cast<const QAbstractButton*>( widget ) );
                if( button->isChecked() || button->isDown() ) copy.state |= State_On;

            } else {

                // take out margins
                const int marginWidth( autoRaise ? Metrics::ToolButton_MarginWidth : Metrics::Button_MarginWidth + Metrics::Frame_FrameWidth );
                contentsRect.adjust( marginWidth, 0, -marginWidth, 0 );
                if( hasInlineIndicator ) contentsRect.setRight( contentsRect.right() - Metrics::ToolButton_BoxTextSpace );

                // adjust state
                if( (toolButtonOption->features & QStyleOptionToolButton::Arrow) && !inTabBar )
                { copy.state &= ~State_MouseOver; }

            }

            copy.rect = contentsRect;

            // render
            drawControl( CE_ToolButtonLabel, &copy, painter, widget);

        }

        return true;
    }

    //______________________________________________________________
    bool Style::drawComboBoxComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionComboBox* comboBoxOption( qstyleoption_cast<const QStyleOptionComboBox*>( option ) );
        if( !comboBoxOption ) return true;

        // rect and palette
        const QRect& rect( option->rect );
        const QPalette& palette( option->palette );

        // state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && ( state & State_MouseOver ) );
        const bool hasFocus( state & State_HasFocus );
        const bool editable( comboBoxOption->editable );
        const bool sunken( state & (State_On|State_Sunken) );
        const bool flat( !comboBoxOption->frame );

        // frame
        if( option->subControls & SC_ComboBoxFrame )
        {

            if( editable )
            {

                const QColor color( palette.color( QPalette::Base ) );
                if( flat )
                {

                    painter->setBrush( color );
                    painter->setPen( Qt::NoPen );
                    painter->drawRect( rect );

                } else {

                    // editable combobox. Make it look like a LineEdit
                    // update animation state
                    // focus takes precedence over hover
                    _animations->lineEditEngine().updateState( widget, AnimationFocus, hasFocus );
                    _animations->lineEditEngine().updateState( widget, AnimationHover, mouseOver && !hasFocus );

                    // outline color
                    const QColor outline( _helper->frameOutlineColor( palette, mouseOver, hasFocus,
                    _animations->lineEditEngine().frameOpacity( widget ),
                    _animations->lineEditEngine().frameAnimationMode( widget ) ) );

                    // render
                    _helper->renderFrame( painter, rect, color, outline, hasFocus );
                }

            } else {

                // update animation state
                // hover takes precedence over focus
                _animations->lineEditEngine().updateState( widget, AnimationHover, mouseOver );
                _animations->lineEditEngine().updateState( widget, AnimationFocus, hasFocus && !mouseOver );
                const AnimationMode mode( _animations->lineEditEngine().buttonAnimationMode( widget ) );
                const qreal opacity( _animations->lineEditEngine().buttonOpacity( widget ) );

                if( flat ) {

                    // define colors and render
                    const QColor color( _helper->toolButtonColor( palette, mouseOver, hasFocus || sunken, opacity, mode ) );
                    _helper->renderToolButtonFrame( painter, rect, color, sunken );

                } else {

                    // define colors
                    const QColor shadow( _helper->shadowColor( palette ) );
                    const QColor outline( _helper->buttonOutlineColor( palette, mouseOver, hasFocus, opacity, mode ) );
                    const QColor background( _helper->buttonBackgroundColor( palette, mouseOver, hasFocus, opacity, mode ) );

                    // render
                    _helper->renderButtonFrame( painter, rect, background, outline, shadow, hasFocus, sunken );

                }

            }

        }

        // arrow
        if( option->subControls & SC_ComboBoxArrow )
        {

            // detect empty comboboxes
            const QComboBox* comboBox = qobject_cast<const QComboBox*>( widget );
            const bool empty( comboBox && !comboBox->count() );

            // arrow color
            QColor arrowColor;
            if( editable )
            {

                if( empty || !enabled ) arrowColor = palette.color( QPalette::Disabled, QPalette::Text );
                else {

                    // check animation state
                    const bool subControlHover( enabled && mouseOver && comboBoxOption->activeSubControls&SC_ComboBoxArrow );
                    _animations->comboBoxEngine().updateState( widget, AnimationHover, subControlHover  );

                    const bool animated( enabled && _animations->comboBoxEngine().isAnimated( widget, AnimationHover ) );
                    const qreal opacity( _animations->comboBoxEngine().opacity( widget, AnimationHover ) );

                    // color
                    const QColor normal( palette.color( QPalette::Text ) );
                    const QColor hover( _helper->hoverColor( palette ) );

                    if( animated )
                    {
                        arrowColor = KColorUtils::mix( normal, hover, opacity );

                    } else if( subControlHover ) {

                        arrowColor = hover;

                    } else arrowColor = normal;

                }

            } else if( flat )  {

                if( empty || !enabled ) arrowColor = palette.color( QPalette::Disabled, QPalette::WindowText );
                else if( hasFocus && !mouseOver && sunken ) arrowColor = palette.color( QPalette::HighlightedText );
                else arrowColor = palette.color( QPalette::WindowText );

            } else if( empty || !enabled ) arrowColor = palette.color( QPalette::Disabled, QPalette::ButtonText );
            else if( hasFocus && !mouseOver ) arrowColor = palette.color( QPalette::HighlightedText );
            else arrowColor = palette.color( QPalette::ButtonText );

            // arrow rect
            const QRect arrowRect( comboBoxSubControlRect( option, SC_ComboBoxArrow, widget ) );

            // render
            _helper->renderArrow( painter, arrowRect, arrowColor, ArrowDown );

        }

        return true;

    }

    //______________________________________________________________
    bool Style::drawSpinBoxComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        const QStyleOptionSpinBox *spinBoxOption( qstyleoption_cast<const QStyleOptionSpinBox*>( option ) );
        if( !spinBoxOption ) return true;

        const State& state( option->state );
        const QPalette& palette( option->palette );
        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && ( state & State_MouseOver ) );
        const bool hasFocus( enabled && ( state & State_HasFocus ) );
        const bool flat( !spinBoxOption->frame );

        if( option->subControls & SC_SpinBoxFrame )
        {

            const QColor background( palette.color( QPalette::Base ) );
            if( flat )
            {

                painter->setBrush( background );
                painter->setPen( Qt::NoPen );
                painter->drawRect( option->rect );

            } else {

                // update animation state
                // focus takes precedence over hover
                _animations->lineEditEngine().updateState( widget, AnimationFocus, hasFocus );
                _animations->lineEditEngine().updateState( widget, AnimationHover, mouseOver && !hasFocus );

                // outline color
                const QColor outline( _helper->frameOutlineColor( palette, mouseOver, hasFocus,
                    _animations->lineEditEngine().frameOpacity( widget ),
                    _animations->lineEditEngine().frameAnimationMode( widget ) ) );

                // render
                _helper->renderFrame( painter, option->rect, background, outline, hasFocus );
            }

        }

        if( option->subControls & SC_SpinBoxUp ) renderSpinBoxArrow( painter, spinBoxOption, widget, SC_SpinBoxUp );
        if( option->subControls & SC_SpinBoxDown ) renderSpinBoxArrow( painter, spinBoxOption, widget, SC_SpinBoxDown );

        return true;

    }

    //______________________________________________________________
    bool Style::drawSliderComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        const QStyleOptionSlider *sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if( !sliderOption ) return true;

        const QPalette& palette( option->palette );
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && ( state & State_MouseOver ) );
        const bool hasFocus( enabled && ( state & State_HasFocus ) );

        // do not render tickmarks
        if( sliderOption->subControls & SC_SliderTickmarks )
        {}

        // groove
        if( sliderOption->subControls & SC_SliderGroove )
        {
            // retrieve groove rect
            QRect grooveRect( subControlRect( CC_Slider, sliderOption, SC_SliderGroove, widget ) );

            // adjustments
            if( sliderOption->orientation == Qt::Horizontal )
            {

                grooveRect = centerRect( grooveRect, grooveRect.width()-Metrics::Slider_Thickness, Metrics::Slider_Thickness );

            } else {

                grooveRect = centerRect( grooveRect, Metrics::Slider_Thickness, grooveRect.height()-Metrics::Slider_Thickness );

            }

            // base color
            const QColor grooveColor( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.3 ) );

            if( !enabled ) _helper->renderSliderGroove( painter, grooveRect, grooveColor );
            else {

                // handle rect
                QRect handleRect( subControlRect( CC_Slider, sliderOption, SC_SliderHandle, widget ) );
                handleRect = centerRect( handleRect, Metrics::Slider_ControlThickness, Metrics::Slider_ControlThickness );

                // highlight color
                const QColor highlight( palette.color( QPalette::Highlight ) );

                if( sliderOption->orientation == Qt::Horizontal )
                {

                    const bool reverseLayout( option->direction == Qt::RightToLeft );

                    QRect leftRect( grooveRect );
                    leftRect.setRight( handleRect.right()-2 );
                    _helper->renderSliderGroove( painter, leftRect, reverseLayout ? grooveColor:highlight );

                    QRect rightRect( grooveRect );
                    rightRect.setLeft( handleRect.left()+2 );
                    _helper->renderSliderGroove( painter, rightRect, reverseLayout ? highlight:grooveColor );

                } else {

                    QRect topRect( grooveRect );
                    topRect.setBottom( handleRect.bottom()-2 );
                    _helper->renderSliderGroove( painter, topRect, highlight );

                    QRect bottomRect( grooveRect );
                    bottomRect.setTop( handleRect.top()+2 );
                    _helper->renderSliderGroove( painter, bottomRect, grooveColor );

                }

            }

        }

        // handle
        if( sliderOption->subControls & SC_SliderHandle )
        {

            // get rect and center
            QRect handleRect( subControlRect( CC_Slider, sliderOption, SC_SliderHandle, widget ) );
            handleRect = centerRect( handleRect, Metrics::Slider_ControlThickness, Metrics::Slider_ControlThickness );

            // handle state
            const bool handleActive( sliderOption->activeSubControls & SC_SliderHandle );
            const bool sunken( state & (State_On|State_Sunken) );

            // animation state
            _animations->sliderEngine().updateState( widget, enabled && handleActive );
            const AnimationMode mode( _animations->sliderEngine().isAnimated( widget ) ? AnimationHover:AnimationNone );
            const qreal opacity( _animations->sliderEngine().opacity( widget ) );

            // define colors
            const QColor background( palette.color( QPalette::Button ) );
            const QColor outline( _helper->sliderOutlineColor( palette, handleActive && mouseOver, hasFocus, opacity, mode ) );
            const QColor shadow( _helper->shadowColor( palette ) );

            // render
            _helper->renderSliderHandle( painter, handleRect, background, outline, shadow, hasFocus, sunken );

        }

        return true;
    }

    //______________________________________________________________
    bool Style::drawDialComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionSlider *sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if( !sliderOption ) return true;

        const QPalette& palette( option->palette );
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && ( state & State_MouseOver ) );
        const bool hasFocus( enabled && ( state & State_HasFocus ) );

        // do not render tickmarks
        if( sliderOption->subControls & SC_DialTickmarks )
        {}

        // groove
        if( sliderOption->subControls & SC_DialGroove )
        {

            // groove rect
            QRect grooveRect( subControlRect( CC_Dial, sliderOption, SC_SliderGroove, widget ) );

            // groove
            const QColor grooveColor( KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::WindowText ), 0.3 ) );

            // render groove
            _helper->renderDialGroove( painter, grooveRect, grooveColor );

            if( enabled )
            {

                // highlight
                const QColor highlight( palette.color( QPalette::Highlight ) );

                // angles
                const qreal first( dialAngle( sliderOption, sliderOption->minimum ) );
                const qreal second( dialAngle( sliderOption, sliderOption->sliderPosition ) );

                // render contents
                _helper->renderDialContents( painter, grooveRect, highlight, first, second );

            }

        }

        // handle
        if ( sliderOption->subControls & SC_DialHandle )
        {

            // get handle rect
            QRect handleRect( subControlRect( CC_Dial, sliderOption, SC_DialHandle, widget ) );
            handleRect = centerRect( handleRect, Metrics::Slider_ControlThickness, Metrics::Slider_ControlThickness );

            // handle state
            const bool handleActive( mouseOver && handleRect.contains( _animations->dialEngine().position( widget ) ) );
            const bool sunken( state & (State_On|State_Sunken) );

            // animation state
            _animations->dialEngine().setHandleRect( widget, handleRect );
            _animations->dialEngine().updateState( widget, enabled && handleActive );
            const AnimationMode mode( _animations->dialEngine().isAnimated( widget ) ? AnimationHover:AnimationNone );
            const qreal opacity( _animations->dialEngine().opacity( widget ) );

            // define colors
            const QColor background( palette.color( QPalette::Button ) );
            const QColor outline( _helper->sliderOutlineColor( palette, handleActive && mouseOver, hasFocus, opacity, mode ) );
            const QColor shadow( _helper->shadowColor( palette ) );

            // render
            _helper->renderSliderHandle( painter, handleRect, background, outline, shadow, hasFocus, sunken );

        }

        return true;
    }

    //______________________________________________________________
    bool Style::drawScrollBarComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        // render full groove directly, rather than using the addPage and subPage control element methods
        if( option->subControls && SC_ScrollBarGroove )
        {
            // retrieve groove rectangle
            QRect grooveRect( subControlRect( CC_ScrollBar, option, SC_ScrollBarGroove, widget ) );

            const QPalette& palette( option->palette );
            const QColor color( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.3 ) );
            const State& state( option->state );
            const bool horizontal( state & State_Horizontal );

            if( horizontal ) grooveRect = centerRect( grooveRect, grooveRect.width(), Metrics::ScrollBar_SliderWidth );
            else grooveRect = centerRect( grooveRect, Metrics::ScrollBar_SliderWidth, grooveRect.height() );

            // render
            _helper->renderScrollBarGroove( painter, grooveRect, color );

        }

        // call base class primitive
        KStyle::drawComplexControl( CC_ScrollBar, option, painter, widget );
        return true;
    }

    //______________________________________________________________
    bool Style::drawTitleBarComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionTitleBar *titleBarOption( qstyleoption_cast<const QStyleOptionTitleBar *>( option ) );
        if( !titleBarOption ) return true;

        // store palette and rect
        QPalette palette( option->palette );
        const QRect& rect( option->rect );

        const State& flags( option->state );
        const bool enabled( flags & State_Enabled );
        const bool active( enabled && ( titleBarOption->titleBarState & Qt::WindowActive ) );

        if( titleBarOption->subControls & SC_TitleBarLabel )
        {

            // render background
            painter->setClipRect( rect );
            const QColor outline( _helper->frameOutlineColor( palette, false, false ) );
            const QColor background( palette.color( active ? QPalette::Active : QPalette::Disabled, QPalette::Highlight ) );
            _helper->renderTabWidgetFrame( painter, rect.adjusted( -1, -1, 1, 3 ), background, outline, CornersTop );

            // render text
            const QRect textRect( subControlRect( CC_TitleBar, option, SC_TitleBarLabel, widget ) );
            KStyle::drawItemText( painter, textRect, Qt::AlignCenter, palette, active, titleBarOption->text, active ? QPalette::HighlightedText : QPalette::WindowText );

        }

        // buttons
        static const QList<SubControl> subControls =
        {
            SC_TitleBarMinButton,
            SC_TitleBarMaxButton,
            SC_TitleBarCloseButton,
            SC_TitleBarNormalButton,
            SC_TitleBarSysMenu
        };

        // loop over supported buttons
        foreach( const SubControl subControl, subControls )
        {

            // skip if not requested
            if( !titleBarOption->subControls & subControl ) continue;

            // find matching icon
            QIcon icon;
            switch( subControl )
            {
                case SC_TitleBarMinButton: icon = standardIcon( SP_TitleBarMinButton, option, widget ); break;
                case SC_TitleBarMaxButton: icon = standardIcon( SP_TitleBarMaxButton, option, widget ); break;
                case SC_TitleBarCloseButton: icon = standardIcon( SP_TitleBarCloseButton, option, widget ); break;
                case SC_TitleBarNormalButton: icon = standardIcon( SP_TitleBarNormalButton, option, widget ); break;
                case SC_TitleBarSysMenu: icon = titleBarOption->icon; break;
                default: break;
            }

            // check icon
            if( icon.isNull() ) continue;

            // define icon rect
            QRect iconRect( subControlRect( CC_TitleBar, option, subControl, widget ) );
            if( iconRect.isEmpty() ) continue;

            const int iconWidth( pixelMetric( PM_SmallIconSize, option, widget ) );
            const QSize iconSize( iconWidth, iconWidth );
            iconRect = centerRect( iconRect, iconSize );

            // active state
            const bool subControlActive( titleBarOption->activeSubControls & subControl );

            // mouse over state
            const bool mouseOver(
                !subControlActive &&
                widget &&
                iconRect.translated( widget->mapToGlobal( QPoint( 0,0 ) ) ).contains( QCursor::pos() ) );

            _animations->mdiWindowEngine().updateState( widget, subControl, mouseOver );

            // set icon mode and state
            QIcon::Mode iconMode;
            QIcon::State iconState;

            if( !enabled )
            {
                iconMode = QIcon::Disabled;
                iconState = QIcon::Off;

            } else {

                if( mouseOver ) iconMode = QIcon::Active;
                else if( active ) iconMode = QIcon::Selected;
                else iconMode = QIcon::Normal;

                iconState = subControlActive ? QIcon::On : QIcon::Off;

            }

            // get pixmap and render
            const QPixmap pixmap = icon.pixmap( iconSize, iconMode, iconState );
            painter->drawPixmap( iconRect, pixmap );

        }

        return true;

    }

    //____________________________________________________________________________________________________
    void Style::renderSpinBoxArrow( QPainter* painter, const QStyleOptionSpinBox* option, const QWidget* widget, const SubControl& subControl ) const
    {

        const QPalette& palette( option->palette );
        const State& state( option->state );

        // enable state
        bool enabled( state & State_Enabled );

        // check steps enable step
        const bool atLimit(
            (subControl == SC_SpinBoxUp && !(option->stepEnabled & QAbstractSpinBox::StepUpEnabled )) ||
            (subControl == SC_SpinBoxDown && !(option->stepEnabled & QAbstractSpinBox::StepDownEnabled ) ) );

        // update enabled state accordingly
        enabled &= !atLimit;

        // update mouse-over effect
        const bool mouseOver( enabled && ( state & State_MouseOver ) );

        // check animation state
        const bool subControlHover( enabled && mouseOver && ( option->activeSubControls & subControl ) );
        _animations->spinBoxEngine().updateState( widget, subControl, subControlHover );

        const bool animated( enabled && _animations->spinBoxEngine().isAnimated( widget, subControl ) );
        const qreal opacity( _animations->spinBoxEngine().opacity( widget, subControl ) );

        QColor color;
        if( animated )
        {

            QColor highlight = _helper->hoverColor( palette );
            color = KColorUtils::mix( palette.color( QPalette::Text ), highlight, opacity );

        } else if( subControlHover ) {

            color = _helper->hoverColor( palette );

        } else if( atLimit ) {

            color = palette.color( QPalette::Disabled, QPalette::Text );

        } else {

            color = palette.color( QPalette::Text );

        }

        // arrow orientation
        ArrowOrientation orientation( ( subControl == SC_SpinBoxUp ) ? ArrowUp:ArrowDown );

        // arrow rect
        const QRect arrowRect( subControlRect( CC_SpinBox, option, subControl, widget ) );

        // render
        _helper->renderArrow( painter, arrowRect, color, orientation );

        return;

    }

    //______________________________________________________________________________
    qreal Style::dialAngle( const QStyleOptionSlider* sliderOption, int value ) const
    {

        // calculate angle at which handle needs to be drawn
        qreal angle( 0 );
        if( sliderOption->maximum == sliderOption->minimum ) angle = M_PI / 2;
        else {

            qreal fraction( qreal( value - sliderOption->minimum )/qreal( sliderOption->maximum - sliderOption->minimum ) );
            if( !sliderOption->upsideDown ) fraction = 1.0 - fraction;

            if( sliderOption->dialWrapping ) angle = 1.5*M_PI - fraction*2*M_PI;
            else  angle = ( M_PI*8 - fraction*10*M_PI )/6;

        }

        return angle;

    }

    //______________________________________________________________________________
    QWidget* Style::scrollBarParent( const QWidget* widget ) const
    {

        // check widget and parent
        if( !(widget && widget->parentWidget() ) ) return nullptr;

        // try cast to scroll area. Must test both parent and grandparent
        QAbstractScrollArea* scrollArea;
        if( !(scrollArea = qobject_cast<QAbstractScrollArea*>( widget->parentWidget() ) ) )
        { scrollArea = qobject_cast<QAbstractScrollArea*>( widget->parentWidget()->parentWidget() ); }

        if( scrollArea &&
            (widget == scrollArea->verticalScrollBar() ||
            widget == scrollArea->horizontalScrollBar() ) ) return scrollArea;
        else return nullptr;

    }

    //______________________________________________________________________________
    QColor Style::scrollBarArrowColor( const QStyleOptionSlider* option, const SubControl& control, const QWidget* widget ) const
    {

        const QRect& rect( option->rect );
        const QPalette& palette( option->palette );
        QColor color( palette.color( QPalette::WindowText ) );

        // check enabled state
        const bool enabled( option->state & State_Enabled );
        if( !enabled ) return color;

        if(
            ( control == SC_ScrollBarSubLine && option->sliderValue == option->minimum ) ||
            ( control == SC_ScrollBarAddLine && option->sliderValue == option->maximum ) )
        {

            // manually disable arrow, to indicate that scrollbar is at limit
            return palette.color( QPalette::Disabled, QPalette::WindowText );

        }

        const bool mouseOver( _animations->scrollBarEngine().isHovered( widget, control ) );
        const bool animated( _animations->scrollBarEngine().isAnimated( widget, control ) );
        const qreal opacity( _animations->scrollBarEngine().opacity( widget, control ) );

        // retrieve mouse position from engine
        QPoint position( mouseOver ? _animations->scrollBarEngine().position( widget ) : QPoint( -1, -1 ) );
        if( mouseOver && rect.contains( position ) )
        {
            // need to update the arrow controlRect on fly because there is no
            // way to get it from the styles directly, outside of repaint events
            _animations->scrollBarEngine().setSubControlRect( widget, control, rect );
        }


        if( rect.intersects(  _animations->scrollBarEngine().subControlRect( widget, control ) ) )
        {

            QColor highlight = _helper->hoverColor( palette );
            if( animated )
            {
                color = KColorUtils::mix( color, highlight, opacity );

            } else if( mouseOver ) {

                color = highlight;

            }

        }

        return color;

    }

    //____________________________________________________________________________________
    void Style::setTranslucentBackground( QWidget* widget ) const
    {
        widget->setAttribute( Qt::WA_TranslucentBackground );

        #ifdef Q_WS_WIN
        // FramelessWindowHint is needed on windows to make WA_TranslucentBackground work properly
        widget->setWindowFlags( widget->windowFlags() | Qt::FramelessWindowHint );
        #endif

    }

    //____________________________________________________________________________________
    QStyleOptionToolButton Style::separatorMenuItemOption( const QStyleOptionMenuItem* menuItemOption, const QWidget* widget ) const
    {

        // separator can have a title and an icon
        // in that case they are rendered as sunken flat toolbuttons
        QStyleOptionToolButton toolButtonOption;
        toolButtonOption.initFrom( widget );
        toolButtonOption.rect = menuItemOption->rect;
        toolButtonOption.features = QStyleOptionToolButton::None;
        toolButtonOption.state = State_On|State_Sunken|State_Enabled|State_AutoRaise;
        toolButtonOption.subControls = SC_ToolButton;
        toolButtonOption.icon =  menuItemOption->icon;

        int iconWidth( pixelMetric( PM_SmallIconSize, menuItemOption, widget ) );
        toolButtonOption.iconSize = QSize( iconWidth, iconWidth );
        toolButtonOption.text = menuItemOption->text;

        toolButtonOption.toolButtonStyle = Qt::ToolButtonTextBesideIcon;

        return toolButtonOption;

    }

    //____________________________________________________________________________________
    QIcon Style::toolBarExtensionIcon( StandardPixmap standardPixmap, const QStyleOption* option, const QWidget* widget ) const
    {

        // store palette
        /* due to Qt, it is not always safe to assume that either option, nor widget are defined */
        QPalette palette;
        if( option ) palette = option->palette;
        else if( widget ) palette = widget->palette();
        else palette = QGuiApplication::palette();

        // convenience class to map color to icon mode
        struct IconData
        {
            QColor _color;
            QIcon::Mode _mode;
            QIcon::State _state;
        };

        // map colors to icon states
        const QList<IconData> iconTypes =
        {
            { palette.color( QPalette::Active, QPalette::WindowText ), QIcon::Normal, QIcon::Off },
            { palette.color( QPalette::Active, QPalette::WindowText ), QIcon::Selected, QIcon::Off },
            { palette.color( QPalette::Active, QPalette::WindowText ), QIcon::Active, QIcon::Off },
            { palette.color( QPalette::Disabled, QPalette::WindowText ), QIcon::Disabled, QIcon::Off },

            { palette.color( QPalette::Active, QPalette::HighlightedText ), QIcon::Normal, QIcon::On },
            { palette.color( QPalette::Active, QPalette::HighlightedText ), QIcon::Selected, QIcon::On },
            { palette.color( QPalette::Active, QPalette::WindowText ), QIcon::Active, QIcon::On },
            { palette.color( QPalette::Disabled, QPalette::WindowText ), QIcon::Disabled, QIcon::On }
        };

        // decide arrow orientation
        const ArrowOrientation orientation( standardPixmap == SP_ToolBarHorizontalExtensionButton ? ArrowRight : ArrowDown );

        // icon size
        const int iconWidth( pixelMetric( QStyle::PM_SmallIconSize, option, widget ) );

        // create icon and fill
        QIcon icon;
        foreach( const IconData& iconData, iconTypes )
        {
            // create pixmap
            QPixmap pixmap( iconWidth, iconWidth );
            pixmap.fill( Qt::transparent );

            // render
            {
                QPainter painter( &pixmap );
                painter.translate( standardPixmap == SP_ToolBarHorizontalExtensionButton ? QPoint( 1, 0 ) : QPoint( 0, 1 ) );
                _helper->renderArrow( &painter, pixmap.rect(), iconData._color, orientation );
            }

            // add to icon
            icon.addPixmap( pixmap, iconData._mode, iconData._state );

        }

        return icon;

    }

    //____________________________________________________________________________________
    QIcon Style::titleBarButtonIcon( StandardPixmap standardPixmap, const QStyleOption* option, const QWidget* widget ) const
    {

        // map standardPixmap to button type
        ButtonType buttonType;
        switch( standardPixmap )
        {
            case SP_TitleBarNormalButton: buttonType = ButtonRestore; break;
            case SP_TitleBarMinButton: buttonType = ButtonMinimize; break;
            case SP_TitleBarMaxButton: buttonType = ButtonMaximize; break;
            case SP_TitleBarCloseButton:
            case SP_DockWidgetCloseButton:
            buttonType = ButtonClose;
            break;

            default: return QIcon();
        }

        // store palette
        /* due to Qt, it is not always safe to assume that either option, nor widget are defined */
        QPalette palette;
        if( option ) palette = option->palette;
        else if( widget ) palette = widget->palette();
        else palette = QGuiApplication::palette();

        palette.setCurrentColorGroup( QPalette::Active );
        const QColor base( palette.color( QPalette::WindowText ) );
        const QColor selected( palette.color( QPalette::HighlightedText ) );
        const QColor negative( ( buttonType == ButtonClose ) ? _helper->negativeText( palette ):base );
        const QColor negativeSelected( ( buttonType == ButtonClose ) ? _helper->negativeText( palette ):selected );

        const bool invertNormalState( buttonType == ButtonClose );

        // convenience class to map color to icon mode
        struct IconData
        {
            QColor _color;
            bool _inverted;
            QIcon::Mode _mode;
            QIcon::State _state;
        };

        // map colors to icon states
        const QList<IconData> iconTypes =
        {
            // state off icons
            { _helper->alphaColor( base, 0.5 ), invertNormalState, QIcon::Normal, QIcon::Off },
            { _helper->alphaColor( selected, 0.5 ), invertNormalState, QIcon::Selected, QIcon::Off },
            { _helper->alphaColor( negative, 0.3 ), true, QIcon::Active, QIcon::Off },
            { _helper->alphaColor( base, 0.2 ), invertNormalState, QIcon::Disabled, QIcon::Off },

            // state on icons
            { _helper->alphaColor( negative, 0.7 ), true, QIcon::Normal, QIcon::On },
            { _helper->alphaColor( negativeSelected, 0.7 ), true, QIcon::Selected, QIcon::On },
            { _helper->alphaColor( negative, 0.7 ), true, QIcon::Active, QIcon::On },
            { _helper->alphaColor( base, 0.2 ), invertNormalState, QIcon::Disabled, QIcon::On }
        };

        // default icon sizes
        static const QList<int> iconSizes = { 8, 16, 22, 32, 48 };

        // output icon
        QIcon icon;

        foreach( const IconData& iconData, iconTypes )
        {

            foreach( const int& iconSize, iconSizes )
            {
                // create pixmap
                QPixmap pixmap( iconSize, iconSize );
                pixmap.fill( Qt::transparent );

                // create painter and render
                QPainter painter( &pixmap );
                _helper->renderButton( &painter, pixmap.rect(), iconData._color, buttonType, iconData._inverted );

                painter.end();

                // store
                icon.addPixmap( pixmap, iconData._mode, iconData._state );
            }

        }

        return icon;

    }

}
