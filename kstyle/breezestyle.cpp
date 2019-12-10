/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>    *
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 *************************************************************************/

#include "breezestyle.h"

#include "breeze.h"
#include "breezeanimations.h"
#include "breezeframeshadow.h"
#include "breezemdiwindowshadow.h"
#include "breezemnemonics.h"
#include "breezepropertynames.h"
#include "breezeshadowhelper.h"
#include "breezesplitterproxy.h"
#include "breezestyleconfigdata.h"
#include "breezewidgetexplorer.h"
#include "breezewindowmanager.h"
#include "breezeblurhelper.h"

#include <KColorUtils>

#include <QApplication>
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
#include <QItemDelegate>
#include <QSplitterHandle>
#include <QTextEdit>
#include <QToolBar>
#include <QToolBox>
#include <QToolButton>
#include <QTreeView>
#include <QWidgetAction>

#if BREEZE_HAVE_QTQUICK
#include <QQuickWindow>
#endif

namespace BreezePrivate
{

    // needed to keep track of tabbars when being dragged
    class TabBarData: public QObject
    {

        public:

        //* constructor
        explicit TabBarData( QObject* parent ):
            QObject( parent )
        {}

        //* assign target tabBar
        void lock( const QWidget* widget )
        { _tabBar = widget; }

        //* true if tabbar is locked
        bool isLocked( const QWidget* widget ) const
        { return _tabBar && _tabBar.data() == widget; }

        //* release
        void release()
        { _tabBar.clear(); }

        private:

        //* pointer to target tabBar
        Breeze::WeakPointer<const QWidget> _tabBar;

    };

    //* needed to have spacing added to items in combobox
    class ComboBoxItemDelegate: public QItemDelegate
    {

        public:

        //* constructor
        explicit ComboBoxItemDelegate( QAbstractItemView* parent ):
            QItemDelegate( parent ),
            _proxy( parent->itemDelegate() ),
            _itemMargin( Breeze::Metrics::ItemView_ItemMarginWidth )
        {}

        //* paint
        void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
        {
            // call either proxy or parent class
            if( _proxy ) _proxy.data()->paint( painter, option, index );
            else QItemDelegate::paint( painter, option, index );
        }

        //* size hint for index
        QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override
        {

            // get size from either proxy or parent class
            auto size( _proxy ?
                _proxy.data()->sizeHint( option, index ):
                QItemDelegate::sizeHint( option, index ) );

            // adjust and return
            if( size.isValid() ) { size.rheight() += _itemMargin*2; }
            return size;

        }

        private:

        //* proxy
        Breeze::WeakPointer<QAbstractItemDelegate> _proxy;

        //* margin
        int _itemMargin;

    };

    //_______________________________________________________________
    #if !BREEZE_USE_KDE4
    bool isProgressBarHorizontal( const QStyleOptionProgressBar* option )
    {  return option && ( (option->state & QStyle::State_Horizontal ) || option->orientation == Qt::Horizontal ); }
    #endif

}

namespace Breeze
{

    //______________________________________________________________
    Style::Style():

        #if BREEZE_USE_KDE4
        _helper( new Helper( "breeze" ) )
        #else
        _helper( new Helper( StyleConfigData::self()->sharedConfig() ) )
        #endif

        , _shadowHelper( new ShadowHelper( this, *_helper ) )
        , _animations( new Animations( this ) )
        , _mnemonics( new Mnemonics( this ) )

        #if !BREEZE_USE_KDE4
        , _blurHelper( new BlurHelper( this ) )
        #endif

        , _windowManager( new WindowManager( this ) )
        , _frameShadowFactory( new FrameShadowFactory( this ) )
        , _mdiWindowShadowFactory( new MdiWindowShadowFactory( this ) )
        , _splitterFactory( new SplitterFactory( this ) )
        , _widgetExplorer( new WidgetExplorer( this ) )
        , _tabBarData( new BreezePrivate::TabBarData( this ) )
        #if BREEZE_HAVE_KSTYLE||BREEZE_USE_KDE4
        , SH_ArgbDndWindow( newStyleHint( QStringLiteral( "SH_ArgbDndWindow" ) ) )
        , CE_CapacityBar( newControlElement( QStringLiteral( "CE_CapacityBar" ) ) )
        #endif
    {

        // use DBus connection to update on breeze configuration change
        auto dbus = QDBusConnection::sessionBus();
        dbus.connect( QString(),
            QStringLiteral( "/BreezeStyle" ),
            QStringLiteral( "org.kde.Breeze.Style" ),
            QStringLiteral( "reparseConfiguration" ), this, SLOT(configurationChanged()) );

        dbus.connect( QString(),
            QStringLiteral( "/BreezeDecoration" ),
            QStringLiteral( "org.kde.Breeze.Style" ),
            QStringLiteral( "reparseConfiguration" ), this, SLOT(configurationChanged()) );
        #if !BREEZE_USE_KDE4
        #if QT_VERSION < 0x050D00 // Check if Qt version < 5.13
        this->addEventFilter(qApp);
        #else
        connect(qApp, &QApplication::paletteChanged, this, &Style::configurationChanged);
        #endif
        #endif
        // call the slot directly; this initial call will set up things that also
        // need to be reset when the system palette changes
        loadConfiguration();

    }

    //______________________________________________________________
    Style::~Style()
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
            || widget->inherits( "KTextEditor::View" )
            )
        { widget->setAttribute( Qt::WA_Hover ); }

        // enforce translucency for drag and drop window
        if( widget->testAttribute( Qt::WA_X11NetWmWindowTypeDND ) && _helper->compositingActive() )
        {
            widget->setAttribute( Qt::WA_TranslucentBackground );
            widget->clearMask();
        }

        // scrollarea polishing is somewhat complex. It is moved to a dedicated method
        polishScrollArea( qobject_cast<QAbstractScrollArea*>( widget ) );

        if( auto itemView = qobject_cast<QAbstractItemView*>( widget ) )
        {

            // enable mouse over effects in itemviews' viewport
            itemView->viewport()->setAttribute( Qt::WA_Hover );

        } else if( auto groupBox = qobject_cast<QGroupBox*>( widget ) )  {

            // checkable group boxes
            if( groupBox->isCheckable() )
            { groupBox->setAttribute( Qt::WA_Hover ); }

        } else if( qobject_cast<QAbstractButton*>( widget ) && qobject_cast<QDockWidget*>( widget->parent() ) ) {

            widget->setAttribute( Qt::WA_Hover );

        } else if( qobject_cast<QAbstractButton*>( widget ) && qobject_cast<QToolBox*>( widget->parent() ) ) {

            widget->setAttribute( Qt::WA_Hover );

        } else if( qobject_cast<QFrame*>( widget ) && widget->parent() && widget->parent()->inherits( "KTitleWidget" ) ) {

            widget->setAutoFillBackground( false );
            if( !StyleConfigData::titleWidgetDrawFrame() )
            { widget->setBackgroundRole( QPalette::Window ); }

        }

        if( qobject_cast<QScrollBar*>( widget ) )
        {

            // remove opaque painting for scrollbars
            widget->setAttribute( Qt::WA_OpaquePaintEvent, false );

        } else if( widget->inherits( "KTextEditor::View" ) ) {

            addEventFilter( widget );

        } else if( auto toolButton = qobject_cast<QToolButton*>( widget ) ) {

            if( toolButton->autoRaise() )
            {
                // for flat toolbuttons, adjust foreground and background role accordingly
                widget->setBackgroundRole( QPalette::NoRole );
                widget->setForegroundRole( QPalette::WindowText );
            }

            if( widget->parentWidget() &&
                widget->parentWidget()->parentWidget() &&
                widget->parentWidget()->parentWidget()->inherits( "Gwenview::SideBarGroup" ) )
            { widget->setProperty( PropertyNames::toolButtonAlignment, Qt::AlignLeft ); }

        } else if( qobject_cast<QDockWidget*>( widget ) ) {

            // add event filter on dock widgets
            // and alter palette
            widget->setAutoFillBackground( false );
            widget->setContentsMargins( Metrics::Frame_FrameWidth, Metrics::Frame_FrameWidth, Metrics::Frame_FrameWidth, Metrics::Frame_FrameWidth );
            addEventFilter( widget );

        } else if( qobject_cast<QMdiSubWindow*>( widget ) ) {

            widget->setAutoFillBackground( false );
            addEventFilter( widget );

        } else if( qobject_cast<QToolBox*>( widget ) ) {

            widget->setBackgroundRole( QPalette::NoRole );
            widget->setAutoFillBackground( false );

        } else if( widget->parentWidget() && widget->parentWidget()->parentWidget() && qobject_cast<QToolBox*>( widget->parentWidget()->parentWidget()->parentWidget() ) ) {

            widget->setBackgroundRole( QPalette::NoRole );
            widget->setAutoFillBackground( false );
            widget->parentWidget()->setAutoFillBackground( false );

        } else if( qobject_cast<QMenu*>( widget ) ) {

            setTranslucentBackground( widget );

            #if !BREEZE_USE_KDE4
            if ( _helper->hasAlphaChannel( widget ) && StyleConfigData::menuOpacity() < 100 ) {
                _blurHelper->registerWidget( widget->window() );
            }
            #endif

        #if QT_VERSION >= 0x050000
        } else if( qobject_cast<QCommandLinkButton*>( widget ) ) {

            addEventFilter( widget );
        #endif
        } else if( auto comboBox = qobject_cast<QComboBox*>( widget ) ) {

            if( !hasParent( widget, "QWebView" ) )
            {
                auto itemView( comboBox->view() );
                if( itemView && itemView->itemDelegate() && itemView->itemDelegate()->inherits( "QComboBoxDelegate" ) )
                { itemView->setItemDelegate( new BreezePrivate::ComboBoxItemDelegate( itemView ) ); }
            }

        } else if( widget->inherits( "QComboBoxPrivateContainer" ) ) {

            addEventFilter( widget );
            setTranslucentBackground( widget );

        } else if( widget->inherits( "QTipLabel" ) ) {

            setTranslucentBackground( widget );

        }

        // base class polishing
        ParentStyleClass::polish( widget );

    }

    //______________________________________________________________
    void Style::polishScrollArea( QAbstractScrollArea* scrollArea )
    {

        // check argument
        if( !scrollArea ) return;

        // enable mouse over effect in sunken scrollareas that support focus
        if( scrollArea->frameShadow() == QFrame::Sunken && scrollArea->focusPolicy()&Qt::StrongFocus )
        { scrollArea->setAttribute( Qt::WA_Hover ); }

        if( scrollArea->viewport() && scrollArea->inherits( "KItemListContainer" ) && scrollArea->frameShape() == QFrame::NoFrame )
        {
            scrollArea->viewport()->setBackgroundRole( QPalette::Window );
            scrollArea->viewport()->setForegroundRole( QPalette::WindowText );
        }

        // add event filter, to make sure proper background is rendered behind scrollbars
        addEventFilter( scrollArea );

        // force side panels as flat, on option
        if( scrollArea->inherits( "KDEPrivate::KPageListView" ) || scrollArea->inherits( "KDEPrivate::KPageTreeView" ) )
        { scrollArea->setProperty( PropertyNames::sidePanelView, true ); }

        // for all side view panels, unbold font (design choice)
        if( scrollArea->property( PropertyNames::sidePanelView ).toBool() )
        {
            // upbold list font
            auto font( scrollArea->font() );
            font.setBold( false );
            scrollArea->setFont( font );
        }

        // disable autofill background for flat (== NoFrame) scrollareas, with QPalette::Window as a background
        // this fixes flat scrollareas placed in a tinted widget, such as groupboxes, tabwidgets or framed dock-widgets
        if( !(scrollArea->frameShape() == QFrame::NoFrame || scrollArea->backgroundRole() == QPalette::Window ) )
        { return; }

        // get viewport and check background role
        auto viewport( scrollArea->viewport() );
        if( !( viewport && viewport->backgroundRole() == QPalette::Window ) ) return;

        // change viewport autoFill background.
        // do the same for all children if the background role is QPalette::Window
        viewport->setAutoFillBackground( false );
        QList<QWidget*> children( viewport->findChildren<QWidget*>() );
        foreach( QWidget* child, children )
        {
            if( child->parent() == viewport && child->backgroundRole() == QPalette::Window )
            { child->setAutoFillBackground( false ); }
        }

        /*
        QTreeView animates expanding/collapsing branches. It paints them into a
        temp pixmap whose background is unconditionally filled with the palette's
        *base* color which is usually different from the window's color
        cf. QTreeViewPrivate::renderTreeToPixmapForAnimation()
        */
        if( auto treeView = qobject_cast<QTreeView *>( scrollArea ) )
        {
            if (treeView->isAnimated())
            {
                QPalette pal( treeView->palette() );
                pal.setColor( QPalette::Active, QPalette::Base, treeView->palette().color( treeView->backgroundRole() ) );
                treeView->setPalette(pal);
            }
        }

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

        #if !BREEZE_USE_KDE4
        _blurHelper->unregisterWidget( widget );
        #endif

        // remove event filter
        if( qobject_cast<QAbstractScrollArea*>( widget ) ||
            qobject_cast<QDockWidget*>( widget ) ||
            qobject_cast<QMdiSubWindow*>( widget ) ||
            widget->inherits( "QComboBoxPrivateContainer" ) )
            { widget->removeEventFilter( this ); }

        ParentStyleClass::unpolish( widget );

    }

    //______________________________________________________________
    int Style::pixelMetric( PixelMetric metric, const QStyleOption* option, const QWidget* widget ) const
    {

        // handle special cases
        switch( metric )
        {

            // frame width
            case PM_DefaultFrameWidth:
            if( qobject_cast<const QMenu*>( widget ) ) return Metrics::Menu_FrameWidth;
            if( qobject_cast<const QLineEdit*>( widget ) ) return Metrics::LineEdit_FrameWidth;
            #if QT_VERSION >= 0x050000
            else if( isQtQuickControl( option, widget ) )
            {
                const QString &elementType = option->styleObject->property( "elementType" ).toString();
                if( elementType == QLatin1String( "edit" ) || elementType == QLatin1String( "spinbox" ) )
                {

                    return Metrics::LineEdit_FrameWidth;

                } else if( elementType == QLatin1String( "combobox" ) ) {

                    return Metrics::ComboBox_FrameWidth;
                }

            }
            #endif

            // fallback
            return Metrics::Frame_FrameWidth;

            case PM_ComboBoxFrameWidth:
            {
                const auto comboBoxOption( qstyleoption_cast< const QStyleOptionComboBox*>( option ) );
                return comboBoxOption && comboBoxOption->editable ? Metrics::LineEdit_FrameWidth : Metrics::ComboBox_FrameWidth;
            }

            case PM_SpinBoxFrameWidth: return Metrics::SpinBox_FrameWidth;
            case PM_ToolBarFrameWidth: return Metrics::ToolBar_FrameWidth;
            case PM_ToolTipLabelFrameWidth: return Metrics::ToolTip_FrameWidth;

            // layout
            case PM_LayoutLeftMargin:
            case PM_LayoutTopMargin:
            case PM_LayoutRightMargin:
            case PM_LayoutBottomMargin:
            {
                /*
                 * use either Child margin or TopLevel margin,
                 * depending on  widget type
                 */
                if( ( option && ( option->state & QStyle::State_Window ) ) || ( widget && widget->isWindow() ) )
                {

                    return Metrics::Layout_TopLevelMarginWidth;

                } else if( widget && widget->inherits( "KPageView" ) ) {

                    return 0;

                } else {

                    return Metrics::Layout_ChildMarginWidth;

                }

            }

            case PM_LayoutHorizontalSpacing: return Metrics::Layout_DefaultSpacing;
            case PM_LayoutVerticalSpacing: return Metrics::Layout_DefaultSpacing;

            // buttons
            case PM_ButtonMargin:
            {
                // needs special case for kcalc buttons, to prevent the application to set too small margins
                if( widget && widget->inherits( "KCalcButton" ) ) return Metrics::Button_MarginWidth + 4;
                else return Metrics::Button_MarginWidth;
            }

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
            case PM_ToolBarExtensionExtent:
            return pixelMetric( PM_SmallIconSize, option, widget ) + 2*Metrics::ToolButton_MarginWidth;

            case PM_ToolBarItemMargin: return 0;
            case PM_ToolBarItemSpacing: return Metrics::ToolBar_ItemSpacing;

            // tabbars
            case PM_TabBarTabShiftVertical: return 0;
            case PM_TabBarTabShiftHorizontal: return 0;
            case PM_TabBarTabOverlap: return Metrics::TabBar_TabOverlap;
            case PM_TabBarBaseOverlap: return Metrics::TabBar_BaseOverlap;
            case PM_TabBarTabHSpace: return 2*Metrics::TabBar_TabMarginWidth;
            case PM_TabBarTabVSpace: return 2*Metrics::TabBar_TabMarginHeight;
            case PM_TabCloseIndicatorWidth:
            case PM_TabCloseIndicatorHeight:
            return pixelMetric( PM_SmallIconSize, option, widget );

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
            case PM_IndicatorWidth: return Metrics::CheckBox_Size;
            case PM_IndicatorHeight: return Metrics::CheckBox_Size;
            case PM_ExclusiveIndicatorWidth: return Metrics::CheckBox_Size;
            case PM_ExclusiveIndicatorHeight: return Metrics::CheckBox_Size;

            // list heaaders
            case PM_HeaderMarkSize: return Metrics::Header_ArrowSize;
            case PM_HeaderMargin: return Metrics::Header_MarginWidth;

            // dock widget
            // return 0 here, since frame is handled directly in polish
            case PM_DockWidgetFrameWidth: return 0;
            case PM_DockWidgetTitleMargin: return Metrics::Frame_FrameWidth;
            case PM_DockWidgetTitleBarButtonMargin: return Metrics::ToolButton_MarginWidth;

            case PM_SplitterWidth: return Metrics::Splitter_SplitterWidth;
            case PM_DockWidgetSeparatorExtent: return Metrics::Splitter_SplitterWidth;

            // fallback
            default: return ParentStyleClass::pixelMetric( metric, option, widget );

        }

    }

    //______________________________________________________________
    int Style::styleHint( StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData ) const
    {
        switch( hint )
        {

            case SH_RubberBand_Mask:
            {

                if( auto mask = qstyleoption_cast<QStyleHintReturnMask*>( returnData ) )
                {

                    mask->region = option->rect;

                    /*
                     * need to check on widget before removing inner region
                     * in order to still preserve rubberband in MainWindow and QGraphicsView
                     * in QMainWindow because it looks better
                     * in QGraphicsView because the painting fails completely otherwise
                     */
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

            case SH_ComboBox_ListMouseTracking: return true;
            case SH_MenuBar_MouseTracking: return true;
            case SH_Menu_MouseTracking: return true;
            case SH_Menu_SubMenuPopupDelay: return 150;
            case SH_Menu_SloppySubMenus: return true;

            #if QT_VERSION >= 0x050000
            // TODO Qt6: drop deprecated SH_Widget_Animate
            case SH_Widget_Animate: return StyleConfigData::animationsEnabled();
            case SH_Menu_SupportsSections: return true;
            #endif
            #if QT_VERSION >= 0x050A00
            case SH_Widget_Animation_Duration: return StyleConfigData::animationsEnabled() ? StyleConfigData::animationsDuration() : 0;
            #endif

            case SH_DialogButtonBox_ButtonsHaveIcons: return true;

            case SH_GroupBox_TextLabelVerticalAlignment: return Qt::AlignVCenter;
            case SH_TabBar_Alignment: return StyleConfigData::tabBarDrawCenteredTabs() ? Qt::AlignCenter:Qt::AlignLeft;
            case SH_ToolBox_SelectedPageTitleBold: return false;
            case SH_ScrollBar_MiddleClickAbsolutePosition: return true;
            case SH_ScrollView_FrameOnlyAroundContents: return false;
            case SH_FormLayoutFormAlignment: return Qt::AlignLeft | Qt::AlignTop;
            case SH_FormLayoutLabelAlignment: return Qt::AlignRight;
            case SH_FormLayoutFieldGrowthPolicy: return QFormLayout::ExpandingFieldsGrow;
            case SH_FormLayoutWrapPolicy: return QFormLayout::DontWrapRows;
            case SH_MessageBox_TextInteractionFlags: return Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse;
            case SH_ProgressDialog_CenterCancelButton: return false;
            case SH_MessageBox_CenterButtons: return false;

            case SH_RequestSoftwareInputPanel: return RSIP_OnMouseClick;
            case SH_TitleBar_NoBorder: return true;
            case SH_DockWidget_ButtonsHaveFrame: return false;
            default: return ParentStyleClass::styleHint( hint, option, widget, returnData );

        }

    }

    //______________________________________________________________
    QRect Style::subElementRect( SubElement element, const QStyleOption* option, const QWidget* widget ) const
    {
        switch( element )
        {

            case SE_PushButtonContents: return pushButtonContentsRect( option, widget );
            case SE_CheckBoxContents: return checkBoxContentsRect( option, widget );
            case SE_RadioButtonContents: return checkBoxContentsRect( option, widget );
            case SE_LineEditContents: return lineEditContentsRect( option, widget );
            case SE_ProgressBarGroove: return progressBarGrooveRect( option, widget );
            case SE_ProgressBarContents: return progressBarContentsRect( option, widget );
            case SE_ProgressBarLabel: return progressBarLabelRect( option, widget );
            case SE_FrameContents: return frameContentsRect( option, widget );
            case SE_HeaderArrow: return headerArrowRect( option, widget );
            case SE_HeaderLabel: return headerLabelRect( option, widget );
            case SE_TabBarTabLeftButton: return tabBarTabLeftButtonRect( option, widget );
            case SE_TabBarTabRightButton: return tabBarTabRightButtonRect( option, widget );
            case SE_TabWidgetTabBar: return tabWidgetTabBarRect( option, widget );
            case SE_TabWidgetTabContents: return tabWidgetTabContentsRect( option, widget );
            case SE_TabWidgetTabPane: return tabWidgetTabPaneRect( option, widget );
            case SE_TabWidgetLeftCorner: return tabWidgetCornerRect( SE_TabWidgetLeftCorner, option, widget );
            case SE_TabWidgetRightCorner: return tabWidgetCornerRect( SE_TabWidgetRightCorner, option, widget );
            case SE_ToolBoxTabContents: return toolBoxTabContentsRect( option, widget );

            // fallback
            default: return ParentStyleClass::subElementRect( element, option, widget );

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
            case CC_Slider: return sliderSubControlRect( option, subControl, widget );

            // fallback
            default: return ParentStyleClass::subControlRect( element, option, subControl, widget );

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
            case CT_Slider: return sliderSizeFromContents( option, size, widget );
            case CT_PushButton: return pushButtonSizeFromContents( option, size, widget );
            case CT_ToolButton: return toolButtonSizeFromContents( option, size, widget );
            case CT_MenuBar: return defaultSizeFromContents( option, size, widget );
            case CT_MenuBarItem: return menuBarItemSizeFromContents( option, size, widget );
            case CT_MenuItem: return menuItemSizeFromContents( option, size, widget );
            case CT_ProgressBar: return progressBarSizeFromContents( option, size, widget );
            case CT_TabWidget: return tabWidgetSizeFromContents( option, size, widget );
            case CT_TabBarTab: return tabBarTabSizeFromContents( option, size, widget );
            case CT_HeaderSection: return headerSectionSizeFromContents( option, size, widget );
            case CT_ItemViewItem: return itemViewItemSizeFromContents( option, size, widget );

            // fallback
            default: return ParentStyleClass::sizeFromContents( element, option, size, widget );
        }

    }

    //______________________________________________________________
    QStyle::SubControl Style::hitTestComplexControl( ComplexControl control, const QStyleOptionComplex* option, const QPoint& point, const QWidget* widget ) const
    {
        switch( control )
        {
            case CC_ScrollBar:
            {

                auto grooveRect = subControlRect( CC_ScrollBar, option, SC_ScrollBarGroove, widget );
                if( grooveRect.contains( point ) )
                {
                    // Must be either page up/page down, or just click on the slider.
                    auto sliderRect = subControlRect( CC_ScrollBar, option, SC_ScrollBarSlider, widget );

                    if( sliderRect.contains( point ) ) return SC_ScrollBarSlider;
                    else if( preceeds( point, sliderRect, option ) ) return SC_ScrollBarSubPage;
                    else return SC_ScrollBarAddPage;

                }

                // This is one of the up/down buttons. First, decide which one it is.
                if( preceeds( point, grooveRect, option ) )
                {

                    if( _subLineButtons == DoubleButton )
                    {

                        auto buttonRect = scrollBarInternalSubControlRect( option, SC_ScrollBarSubLine );
                        return scrollBarHitTest( buttonRect, point, option );

                    } else return SC_ScrollBarSubLine;

                }

                if( _addLineButtons == DoubleButton )
                {

                    auto buttonRect = scrollBarInternalSubControlRect( option, SC_ScrollBarAddLine );
                    return scrollBarHitTest( buttonRect, point, option );

                } else return SC_ScrollBarAddLine;
            }

            // fallback
            default: return ParentStyleClass::hitTestComplexControl( control, option, point, widget );

        }

    }

    //______________________________________________________________
    void Style::drawPrimitive( PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        StylePrimitive fcn;
        switch( element )
        {

            case PE_PanelButtonCommand: fcn = &Style::drawPanelButtonCommandPrimitive; break;
            case PE_PanelButtonTool: fcn = &Style::drawPanelButtonToolPrimitive; break;
            case PE_PanelScrollAreaCorner: fcn = &Style::drawPanelScrollAreaCornerPrimitive; break;
            case PE_PanelMenu: fcn = &Style::drawPanelMenuPrimitive; break;
            case PE_PanelTipLabel: fcn = &Style::drawPanelTipLabelPrimitive; break;
            case PE_PanelItemViewItem: fcn = &Style::drawPanelItemViewItemPrimitive; break;
            case PE_IndicatorCheckBox: fcn = &Style::drawIndicatorCheckBoxPrimitive; break;
            case PE_IndicatorRadioButton: fcn = &Style::drawIndicatorRadioButtonPrimitive; break;
            case PE_IndicatorButtonDropDown: fcn = &Style::drawIndicatorButtonDropDownPrimitive; break;
            case PE_IndicatorTabClose: fcn = &Style::drawIndicatorTabClosePrimitive; break;
            case PE_IndicatorTabTear: fcn = &Style::drawIndicatorTabTearPrimitive; break;
            case PE_IndicatorArrowUp: fcn = &Style::drawIndicatorArrowUpPrimitive; break;
            case PE_IndicatorArrowDown: fcn = &Style::drawIndicatorArrowDownPrimitive; break;
            case PE_IndicatorArrowLeft: fcn = &Style::drawIndicatorArrowLeftPrimitive; break;
            case PE_IndicatorArrowRight: fcn = &Style::drawIndicatorArrowRightPrimitive; break;
            case PE_IndicatorHeaderArrow: fcn = &Style::drawIndicatorHeaderArrowPrimitive; break;
            case PE_IndicatorToolBarHandle: fcn = &Style::drawIndicatorToolBarHandlePrimitive; break;
            case PE_IndicatorToolBarSeparator: fcn = &Style::drawIndicatorToolBarSeparatorPrimitive; break;
            case PE_IndicatorBranch: fcn = &Style::drawIndicatorBranchPrimitive; break;
            case PE_FrameStatusBar: fcn = &Style::emptyPrimitive; break;
            case PE_Frame: fcn = &Style::drawFramePrimitive; break;
            case PE_FrameLineEdit: fcn = &Style::drawFrameLineEditPrimitive; break;
            case PE_FrameMenu: fcn = &Style::drawFrameMenuPrimitive; break;
            case PE_FrameGroupBox: fcn = &Style::drawFrameGroupBoxPrimitive; break;
            case PE_FrameTabWidget: fcn = &Style::drawFrameTabWidgetPrimitive; break;
            case PE_FrameTabBarBase: fcn = &Style::drawFrameTabBarBasePrimitive; break;
            case PE_FrameWindow: fcn = &Style::drawFrameWindowPrimitive; break;
            case PE_FrameFocusRect: fcn = _frameFocusPrimitive; break;

            // fallback
            default: break;

        }

        painter->save();

        // call function if implemented
        if( !( fcn && fcn( *this, option, painter, widget ) ) )
        { ParentStyleClass::drawPrimitive( element, option, painter, widget ); }

        painter->restore();

    }

    //______________________________________________________________
    void Style::drawControl( ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        StyleControl fcn;

        #if BREEZE_HAVE_KSTYLE||BREEZE_USE_KDE4
        if( element == CE_CapacityBar )
        {
            fcn = &Style::drawProgressBarControl;

        } else
        #endif

        switch( element ) {

            case CE_PushButtonBevel: fcn = &Style::drawPanelButtonCommandPrimitive; break;
            case CE_PushButtonLabel: fcn = &Style::drawPushButtonLabelControl; break;
            case CE_CheckBoxLabel: fcn = &Style::drawCheckBoxLabelControl; break;
            case CE_RadioButtonLabel: fcn = &Style::drawCheckBoxLabelControl; break;
            case CE_ToolButtonLabel: fcn = &Style::drawToolButtonLabelControl; break;
            case CE_ComboBoxLabel: fcn = &Style::drawComboBoxLabelControl; break;
            case CE_MenuBarEmptyArea: fcn = &Style::emptyControl; break;
            case CE_MenuBarItem: fcn = &Style::drawMenuBarItemControl; break;
            case CE_MenuItem: fcn = &Style::drawMenuItemControl; break;
            case CE_ToolBar: fcn = &Style::emptyControl; break;
            case CE_ProgressBar: fcn = &Style::drawProgressBarControl; break;
            case CE_ProgressBarContents: fcn = &Style::drawProgressBarContentsControl; break;
            case CE_ProgressBarGroove: fcn = &Style::drawProgressBarGrooveControl; break;
            case CE_ProgressBarLabel: fcn = &Style::drawProgressBarLabelControl; break;
            case CE_ScrollBarSlider: fcn = &Style::drawScrollBarSliderControl; break;
            case CE_ScrollBarAddLine: fcn = &Style::drawScrollBarAddLineControl; break;
            case CE_ScrollBarSubLine: fcn = &Style::drawScrollBarSubLineControl; break;
            case CE_ScrollBarAddPage: fcn = &Style::emptyControl; break;
            case CE_ScrollBarSubPage: fcn = &Style::emptyControl; break;
            case CE_ShapedFrame: fcn = &Style::drawShapedFrameControl; break;
            case CE_RubberBand: fcn = &Style::drawRubberBandControl; break;
            case CE_SizeGrip: fcn = &Style::emptyControl; break;
            case CE_HeaderSection: fcn = &Style::drawHeaderSectionControl; break;
            case CE_HeaderEmptyArea: fcn = &Style::drawHeaderEmptyAreaControl; break;
            case CE_TabBarTabLabel: fcn = &Style::drawTabBarTabLabelControl; break;
            case CE_TabBarTabShape: fcn = &Style::drawTabBarTabShapeControl; break;
            case CE_ToolBoxTabLabel: fcn = &Style::drawToolBoxTabLabelControl; break;
            case CE_ToolBoxTabShape: fcn = &Style::drawToolBoxTabShapeControl; break;
            case CE_DockWidgetTitle: fcn = &Style::drawDockWidgetTitleControl; break;

            // fallback
            default: break;

        }

        painter->save();

        // call function if implemented
        if( !( fcn && fcn( *this, option, painter, widget ) ) )
        { ParentStyleClass::drawControl( element, option, painter, widget ); }

        painter->restore();

    }

    //______________________________________________________________
    void Style::drawComplexControl( ComplexControl element, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        StyleComplexControl fcn;
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
        if( !( fcn && fcn( *this, option, painter, widget ) ) )
        { ParentStyleClass::drawComplexControl( element, option, painter, widget ); }

        painter->restore();

    }


    //___________________________________________________________________________________
    void Style::drawItemText(
        QPainter* painter, const QRect& rect, int flags, const QPalette& palette, bool enabled,
        const QString &text, QPalette::ColorRole textRole ) const
    {

        // hide mnemonics if requested
        if( !_mnemonics->enabled() && ( flags&Qt::TextShowMnemonic ) && !( flags&Qt::TextHideMnemonic ) )
        {
            flags &= ~Qt::TextShowMnemonic;
            flags |= Qt::TextHideMnemonic;
        }

        // make sure vertical alignment is defined
        // fallback on Align::VCenter if not
        if( !(flags&Qt::AlignVertical_Mask) ) flags |= Qt::AlignVCenter;

        if( _animations->widgetEnabilityEngine().enabled() )
        {

            /*
             * check if painter engine is registered to WidgetEnabilityEngine, and animated
             * if yes, merge the palettes. Note: a static_cast is safe here, since only the address
             * of the pointer is used, not the actual content.
             */
            const QWidget* widget( static_cast<const QWidget*>( painter->device() ) );
            if( _animations->widgetEnabilityEngine().isAnimated( widget, AnimationEnable ) )
            {

                const QPalette copy( _helper->disabledPalette( palette, _animations->widgetEnabilityEngine().opacity( widget, AnimationEnable ) ) );
                return ParentStyleClass::drawItemText( painter, rect, flags, copy, enabled, text, textRole );

            }

        }

        // fallback
        return ParentStyleClass::drawItemText( painter, rect, flags, palette, enabled, text, textRole );

    }

    //_____________________________________________________________________
    bool Style::eventFilter( QObject *object, QEvent *event )
    {

        if( auto dockWidget = qobject_cast<QDockWidget*>( object ) ) { return eventFilterDockWidget( dockWidget, event ); }
        else if( auto subWindow = qobject_cast<QMdiSubWindow*>( object ) ) { return eventFilterMdiSubWindow( subWindow, event ); }
        #if QT_VERSION >= 0x050000
        else if( auto commandLinkButton = qobject_cast<QCommandLinkButton*>( object ) ) { return eventFilterCommandLinkButton( commandLinkButton, event ); }
        #endif
        #if QT_VERSION < 0x050D00 // Check if Qt version < 5.13
        else if( object == qApp && event->type() == QEvent::ApplicationPaletteChange ) { configurationChanged(); }
        #endif
        // cast to QWidget
        QWidget *widget = static_cast<QWidget*>( object );
        if( widget->inherits( "QAbstractScrollArea" ) || widget->inherits( "KTextEditor::View" ) ) { return eventFilterScrollArea( widget, event ); }
        else if( widget->inherits( "QComboBoxPrivateContainer" ) ) { return eventFilterComboBoxContainer( widget, event ); }

        // fallback
        return ParentStyleClass::eventFilter( object, event );

    }

    //____________________________________________________________________________
    bool Style::eventFilterScrollArea( QWidget* widget, QEvent* event )
    {

        switch( event->type() )
        {
            case QEvent::Paint:
            {

                // get scrollarea viewport
                auto scrollArea( qobject_cast<QAbstractScrollArea*>( widget ) );
                QWidget* viewport;
                if( !( scrollArea && (viewport = scrollArea->viewport()) ) ) break;

                // get scrollarea horizontal and vertical containers
                QWidget* child( nullptr );
                QList<QWidget*> children;
                if( ( child = scrollArea->findChild<QWidget*>( "qt_scrollarea_vcontainer" ) ) && child->isVisible() )
                { children.append( child ); }

                if( ( child = scrollArea->findChild<QWidget*>( "qt_scrollarea_hcontainer" ) ) && child->isVisible() )
                { children.append( child ); }

                if( children.empty() ) break;
                if( !scrollArea->styleSheet().isEmpty() ) break;

                // make sure proper background is rendered behind the containers
                QPainter painter( scrollArea );
                painter.setClipRegion( static_cast<QPaintEvent*>( event )->region() );

                painter.setPen( Qt::NoPen );

                // decide background color
                const QPalette::ColorRole role( viewport->backgroundRole() );
                QColor background;
                if( role == QPalette::Window && hasAlteredBackground( viewport ) ) background = _helper->frameBackgroundColor( viewport->palette() );
                else background = viewport->palette().color( role );
                painter.setBrush( background );

                // render
                foreach( auto* child, children )
                { painter.drawRect( child->geometry() ); }

            }
            break;

            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
            case QEvent::MouseMove:
            {

                // case event
                QMouseEvent* mouseEvent( static_cast<QMouseEvent*>( event ) );

                // get frame framewidth
                const int frameWidth( pixelMetric( PM_DefaultFrameWidth, nullptr, widget ) );

                // find list of scrollbars
                QList<QScrollBar*> scrollBars;
                if( auto scrollArea = qobject_cast<QAbstractScrollArea*>( widget ) )
                {

                    if( scrollArea->horizontalScrollBarPolicy() != Qt::ScrollBarAlwaysOff ) scrollBars.append( scrollArea->horizontalScrollBar() );
                    if( scrollArea->verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff )scrollBars.append( scrollArea->verticalScrollBar() );

                } else if( widget->inherits( "KTextEditor::View" ) ) {

                    scrollBars = widget->findChildren<QScrollBar*>();

                }

                // loop over found scrollbars
                foreach( QScrollBar* scrollBar, scrollBars )
                {

                    if( !( scrollBar && scrollBar->isVisible() ) ) continue;

                    QPoint offset;
                    if( scrollBar->orientation() == Qt::Horizontal ) offset = QPoint( 0, frameWidth );
                    else offset = QPoint( QApplication::isLeftToRight() ? frameWidth : -frameWidth, 0 );

                    // map position to scrollarea
                    QPoint position( scrollBar->mapFrom( widget, mouseEvent->pos() - offset ) );

                    // check if contains
                    if( !scrollBar->rect().contains( position ) ) continue;

                    // copy event, send and return
                    QMouseEvent copy(
                        mouseEvent->type(),
                        position,
                        mouseEvent->button(),
                        mouseEvent->buttons(), mouseEvent->modifiers());

                    QCoreApplication::sendEvent( scrollBar, &copy );
                    event->setAccepted( true );
                    return true;

                }

                break;

            }

            default: break;

        }

        return  ParentStyleClass::eventFilter( widget, event );

    }

    //_________________________________________________________
    bool Style::eventFilterComboBoxContainer( QWidget* widget, QEvent* event )
    {
        if( event->type() == QEvent::Paint )
        {

            QPainter painter( widget );
            auto paintEvent = static_cast<QPaintEvent*>( event );
            painter.setClipRegion( paintEvent->region() );

            const auto rect( widget->rect() );
            const auto& palette( widget->palette() );
            const auto background( _helper->frameBackgroundColor( palette ) );
            const auto outline( _helper->frameOutlineColor( palette ) );

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
            const auto& palette( dockWidget->palette() );
            const auto background( _helper->frameBackgroundColor( palette ) );
            const auto outline( _helper->frameOutlineColor( palette ) );

            // store rect
            const auto rect( dockWidget->rect() );

            // render
            if( dockWidget->isFloating() )
            {
                _helper->renderMenuFrame( &painter, rect, background, outline, false );

            } else if( StyleConfigData::dockWidgetDrawFrame() || (dockWidget->features()&QDockWidget::AllDockWidgetFeatures) ) {

                _helper->renderFrame( &painter, rect, background, outline );

            }

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

            const auto rect( subWindow->rect() );
            const auto background( subWindow->palette().color( QPalette::Window ) );

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

    //____________________________________________________________________________
    #if QT_VERSION >= 0x050000
    bool Style::eventFilterCommandLinkButton( QCommandLinkButton* button, QEvent* event )
    {

        if( event->type() == QEvent::Paint )
        {

            // painter
            QPainter painter( button );
            painter.setClipRegion( static_cast<QPaintEvent*>( event )->region() );

            const bool isFlat = false;

            // option
            QStyleOptionButton option;
            option.initFrom( button );
            option.features |= QStyleOptionButton::CommandLinkButton;
            if( isFlat ) option.features |= QStyleOptionButton::Flat;
            option.text = QString();
            option.icon = QIcon();

            if( button->isChecked() ) option.state|=State_On;
            if( button->isDown() ) option.state|=State_Sunken;

            // frame
            drawControl(QStyle::CE_PushButton, &option, &painter, button );

            // offset
            const int margin( Metrics::Button_MarginWidth + Metrics::Frame_FrameWidth );
            QPoint offset( margin, margin );

            if( button->isDown() && !isFlat ) painter.translate( 1, 1 );
            { offset += QPoint( 1, 1 ); }

            // state
            const State& state( option.state );
            const bool enabled( state & State_Enabled );
            bool mouseOver( enabled && ( state & State_MouseOver ) );
            bool hasFocus( enabled && ( state & State_HasFocus ) );

            // icon
            if( !button->icon().isNull() )
            {

                const auto pixmapSize( button->icon().actualSize( button->iconSize() ) );
                const QRect pixmapRect( QPoint( offset.x(), button->description().isEmpty() ? (button->height() - pixmapSize.height())/2:offset.y() ), pixmapSize );
                const QPixmap pixmap( button->icon().pixmap(pixmapSize,
                    enabled ? QIcon::Normal : QIcon::Disabled,
                    button->isChecked() ? QIcon::On : QIcon::Off) );
                drawItemPixmap( &painter, pixmapRect, Qt::AlignCenter, pixmap );

                offset.rx() += pixmapSize.width() + Metrics::Button_ItemSpacing;

            }

            // text rect
            QRect textRect( offset, QSize( button->size().width() - offset.x() - margin, button->size().height() - 2*margin ) );
            const QPalette::ColorRole textRole = (enabled && hasFocus && !mouseOver && !isFlat ) ? QPalette::HighlightedText : QPalette::ButtonText;
            if( !button->text().isEmpty() )
            {

                QFont font( button->font() );
                font.setBold( true );
                painter.setFont( font );
                if( button->description().isEmpty() )
                {

                    drawItemText( &painter, textRect, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextHideMnemonic, button->palette(), enabled, button->text(), textRole );

                } else {

                    drawItemText( &painter, textRect, Qt::AlignLeft|Qt::AlignTop|Qt::TextHideMnemonic, button->palette(), enabled, button->text(), textRole );
                    textRect.setTop( textRect.top() + QFontMetrics( font ).height() );

                }

                painter.setFont( button->font() );

            }

            if( !button->description().isEmpty() )
            { drawItemText( &painter, textRect, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextWordWrap, button->palette(), enabled, button->description(), textRole ); }

            return true;
        }

        // continue with normal painting
        return false;

    }
    #endif

    //_____________________________________________________________________
    void Style::configurationChanged()
    {

        // reload
        #if BREEZE_USE_KDE4
        StyleConfigData::self()->readConfig();
        #else
        StyleConfigData::self()->load();
        #endif

        // reload configuration
        loadConfiguration();

    }

    //____________________________________________________________________
    QIcon Style::standardIconImplementation( StandardPixmap standardPixmap, const QStyleOption* option, const QWidget* widget ) const
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
            break;

        }

        if( icon.isNull() )
        {

            // do not cache parent style icon, since it may change at runtime
            #if QT_VERSION >= 0x050000
            return  ParentStyleClass::standardIcon( standardPixmap, option, widget );
            #else
            return  ParentStyleClass::standardIconImplementation( standardPixmap, option, widget );
            #endif

        } else {
            const_cast<IconCache*>(&_iconCache)->insert( standardPixmap, icon );
            return icon;
        }

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

        // reset shadow tiles
        _shadowHelper->loadConfig();

        // set mdiwindow factory shadow tiles
        _mdiWindowShadowFactory->setShadowHelper( _shadowHelper );

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

        // frame focus
        if( StyleConfigData::viewDrawFocusIndicator() ) _frameFocusPrimitive = &Style::drawFrameFocusRectPrimitive;
        else _frameFocusPrimitive = &Style::emptyPrimitive;

        // widget explorer
        _widgetExplorer->setEnabled( StyleConfigData::widgetExplorerEnabled() );
        _widgetExplorer->setDrawWidgetRects( StyleConfigData::drawWidgetRects() );
    }

    //___________________________________________________________________________________________________________________
    QRect Style::pushButtonContentsRect( const QStyleOption* option, const QWidget* ) const
    { return insideMargin( option->rect, Metrics::Frame_FrameWidth ); }

    //___________________________________________________________________________________________________________________
    QRect Style::checkBoxContentsRect( const QStyleOption* option, const QWidget* ) const
    { return visualRect( option, option->rect.adjusted( Metrics::CheckBox_Size + Metrics::CheckBox_ItemSpacing, 0, 0, 0 ) ); }

    //___________________________________________________________________________________________________________________
    QRect Style::lineEditContentsRect( const QStyleOption* option, const QWidget* widget ) const
    {
        // cast option and check
        const auto frameOption( qstyleoption_cast<const QStyleOptionFrame*>( option ) );
        if( !frameOption ) return option->rect;

        // check flatness
        const bool flat( frameOption->lineWidth == 0 );
        if( flat ) return option->rect;

        // copy rect and take out margins
        auto rect( option->rect );

        // take out margins if there is enough room
        const int frameWidth( pixelMetric( PM_DefaultFrameWidth, option, widget ) );
        if( rect.height() >= option->fontMetrics.height() + 2*frameWidth ) return insideMargin( rect, frameWidth );
        else return rect;
    }

    //___________________________________________________________________________________________________________________
    QRect Style::progressBarGrooveRect( const QStyleOption* option, const QWidget* widget ) const
    {

        // cast option and check
        const auto progressBarOption( qstyleoption_cast<const QStyleOptionProgressBar*>( option ) );
        if( !progressBarOption ) return option->rect;

        // get flags and orientation
        const bool textVisible( progressBarOption->textVisible );
        const bool busy( progressBarOption->minimum == 0 && progressBarOption->maximum == 0 );

        #if BREEZE_USE_KDE4
        const auto progressBarOption2( qstyleoption_cast<const QStyleOptionProgressBarV2*>( option ) );
        const bool horizontal( !progressBarOption2 || progressBarOption2->orientation == Qt::Horizontal );
        #else
        const bool horizontal( BreezePrivate::isProgressBarHorizontal( progressBarOption ) );
        #endif

        // copy rectangle and adjust
        auto rect( option->rect );
        const int frameWidth( pixelMetric( PM_DefaultFrameWidth, option, widget ) );
        if( horizontal ) rect = insideMargin( rect, frameWidth, 0 );
        else rect = insideMargin( rect, 0, frameWidth );

        if( textVisible && !busy && horizontal )
        {

            auto textRect( subElementRect( SE_ProgressBarLabel, option, widget ) );
            textRect = visualRect( option, textRect );
            rect.setRight( textRect.left() - Metrics::ProgressBar_ItemSpacing - 1 );
            rect = visualRect( option, rect );
            rect = centerRect( rect, rect.width(), Metrics::ProgressBar_Thickness );

        } else if( horizontal ) {

            rect = centerRect( rect, rect.width(), Metrics::ProgressBar_Thickness );

        } else {

            rect = centerRect( rect, Metrics::ProgressBar_Thickness, rect.height() );

        }

        return rect;

    }

    //___________________________________________________________________________________________________________________
    QRect Style::progressBarContentsRect( const QStyleOption* option, const QWidget* widget ) const
    {

        // cast option and check
        const auto progressBarOption( qstyleoption_cast<const QStyleOptionProgressBar*>( option ) );
        if( !progressBarOption ) return QRect();

        // get groove rect
        const auto rect( progressBarGrooveRect( option, widget ) );

        // in busy mode, grooveRect is used
        const bool busy( progressBarOption->minimum == 0 && progressBarOption->maximum == 0 );
        if( busy ) return rect;

        // get orientation
        #if BREEZE_USE_KDE4
        const auto progressBarOption2( qstyleoption_cast<const QStyleOptionProgressBarV2*>( option ) );
        const bool horizontal( !progressBarOption2 || progressBarOption2->orientation == Qt::Horizontal );
        #else
        const bool horizontal( BreezePrivate::isProgressBarHorizontal( progressBarOption ) );
        #endif

        // check inverted appearance
        #if BREEZE_USE_KDE4
        const bool inverted( progressBarOption2 ? progressBarOption2->invertedAppearance : false );
        #else
        const bool inverted( progressBarOption->invertedAppearance );
        #endif

        // get progress and steps
        const qreal progress( progressBarOption->progress - progressBarOption->minimum );
        const int steps( qMax( progressBarOption->maximum  - progressBarOption->minimum, 1 ) );

        //Calculate width fraction
        const qreal widthFrac = qMin( qreal(1), progress/steps );

        // convert the pixel width
        const int indicatorSize( widthFrac*( horizontal ? rect.width():rect.height() ) );

        QRect indicatorRect;
        if( horizontal )
        {

            indicatorRect = QRect( inverted ? ( rect.right() - indicatorSize + 1):rect.left(), rect.y(), indicatorSize, rect.height() );
            indicatorRect = visualRect( option->direction, rect, indicatorRect );

        } else indicatorRect = QRect( rect.x(), inverted ? rect.top() : (rect.bottom() - indicatorSize + 1), rect.width(), indicatorSize );

        return indicatorRect;

    }

    //___________________________________________________________________________________________________________________
    QRect Style::frameContentsRect( const QStyleOption* option, const QWidget* widget ) const
    {
        if( !StyleConfigData::sidePanelDrawFrame() &&
            qobject_cast<const QAbstractScrollArea*>( widget ) &&
            widget->property( PropertyNames::sidePanelView ).toBool() )
        {

            // adjust margins for sidepanel widgets
            return option->rect.adjusted( 0, 0, -1, 0 );

        } else {

            // base class implementation
            return ParentStyleClass::subElementRect( SE_FrameContents, option, widget );

        }
    }

    //___________________________________________________________________________________________________________________
    QRect Style::progressBarLabelRect( const QStyleOption* option, const QWidget* ) const
    {

        // cast option and check
        const auto progressBarOption( qstyleoption_cast<const QStyleOptionProgressBar*>( option ) );
        if( !progressBarOption ) return QRect();

        // get flags and check
        const bool textVisible( progressBarOption->textVisible );
        const bool busy( progressBarOption->minimum == 0 && progressBarOption->maximum == 0 );
        if( !textVisible || busy ) return QRect();

        // get direction and check
        #if BREEZE_USE_KDE4
        const auto progressBarOption2( qstyleoption_cast<const QStyleOptionProgressBarV2*>( option ) );
        const bool horizontal( !progressBarOption2 || progressBarOption2->orientation == Qt::Horizontal );
        #else
        const bool horizontal( BreezePrivate::isProgressBarHorizontal( progressBarOption ) );
        #endif
        if( !horizontal ) return QRect();

        int textWidth = qMax(
            option->fontMetrics.size( _mnemonics->textFlags(), progressBarOption->text ).width(),
            option->fontMetrics.size( _mnemonics->textFlags(), QStringLiteral( "100%" ) ).width() );

        auto rect( insideMargin( option->rect, Metrics::Frame_FrameWidth, 0 ) );
        rect.setLeft( rect.right() - textWidth + 1 );
        rect = visualRect( option, rect );

        return rect;

    }

    //___________________________________________________________________________________________________________________
    QRect Style::headerArrowRect( const QStyleOption* option, const QWidget* ) const
    {

        // cast option and check
        const auto headerOption( qstyleoption_cast<const QStyleOptionHeader*>( option ) );
        if( !headerOption ) return option->rect;

        // check if arrow is necessary
        if( headerOption->sortIndicator == QStyleOptionHeader::None ) return QRect();

        auto arrowRect( insideMargin( option->rect, Metrics::Header_MarginWidth ) );
        arrowRect.setLeft( arrowRect.right() - Metrics::Header_ArrowSize + 1 );

        return visualRect( option, arrowRect );

    }

    //___________________________________________________________________________________________________________________
    QRect Style::headerLabelRect( const QStyleOption* option, const QWidget* ) const
    {

        // cast option and check
        const auto headerOption( qstyleoption_cast<const QStyleOptionHeader*>( option ) );
        if( !headerOption ) return option->rect;

        // check if arrow is necessary
        auto labelRect( insideMargin( option->rect, Metrics::Header_MarginWidth, 0 ) );
        if( headerOption->sortIndicator == QStyleOptionHeader::None ) return labelRect;

        labelRect.adjust( 0, 0, -Metrics::Header_ArrowSize-Metrics::Header_ItemSpacing, 0 );
        return visualRect( option, labelRect );

    }

    //____________________________________________________________________
    QRect Style::tabBarTabLeftButtonRect( const QStyleOption* option, const QWidget* ) const
    {

        // cast option and check
        #if BREEZE_USE_KDE4
        const auto tabOption( qstyleoption_cast<const QStyleOptionTabV3*>( option ) );
        #else
        const auto tabOption( qstyleoption_cast<const QStyleOptionTab*>( option ) );
        #endif
        if( !tabOption || tabOption->leftButtonSize.isEmpty() ) return QRect();

        const auto rect( option->rect );
        const QSize size( tabOption->leftButtonSize );
        QRect buttonRect( QPoint(0,0), size );

        // vertical positioning
        switch( tabOption->shape )
        {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            buttonRect.moveLeft( rect.left() + Metrics::TabBar_TabMarginWidth );
            buttonRect.moveTop( ( rect.height() - buttonRect.height() )/2 );
            buttonRect = visualRect( option, buttonRect );
            break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            buttonRect.moveBottom( rect.bottom() - Metrics::TabBar_TabMarginWidth );
            buttonRect.moveLeft( ( rect.width() - buttonRect.width() )/2 );
            break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            buttonRect.moveTop( rect.top() + Metrics::TabBar_TabMarginWidth );
            buttonRect.moveLeft( ( rect.width() - buttonRect.width() )/2 );
            break;

            default: break;
        }

        return buttonRect;

    }

    //____________________________________________________________________
    QRect Style::tabBarTabRightButtonRect( const QStyleOption* option, const QWidget* ) const
    {

        // cast option and check
        #if BREEZE_USE_KDE4
        const auto tabOption( qstyleoption_cast<const QStyleOptionTabV3*>( option ) );
        #else
        const auto tabOption( qstyleoption_cast<const QStyleOptionTab*>( option ) );
        #endif
        if( !tabOption || tabOption->rightButtonSize.isEmpty() ) return QRect();

        const auto rect( option->rect );
        const auto size( tabOption->rightButtonSize );
        QRect buttonRect( QPoint(0,0), size );

        // vertical positioning
        switch( tabOption->shape )
        {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            buttonRect.moveRight( rect.right() - Metrics::TabBar_TabMarginWidth );
            buttonRect.moveTop( ( rect.height() - buttonRect.height() )/2 );
            buttonRect = visualRect( option, buttonRect );
            break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            buttonRect.moveTop( rect.top() + Metrics::TabBar_TabMarginWidth );
            buttonRect.moveLeft( ( rect.width() - buttonRect.width() )/2 );
            break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            buttonRect.moveBottom( rect.bottom() - Metrics::TabBar_TabMarginWidth );
            buttonRect.moveLeft( ( rect.width() - buttonRect.width() )/2 );
            break;

            default: break;
        }

        return buttonRect;

    }

    //____________________________________________________________________
    QRect Style::tabWidgetTabBarRect( const QStyleOption* option, const QWidget* widget ) const
    {

        // cast option and check
        const auto tabOption = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>( option );
        if( !tabOption ) return ParentStyleClass::subElementRect( SE_TabWidgetTabBar, option, widget );

        // do nothing if tabbar is hidden
        const QSize tabBarSize( tabOption->tabBarSize );

        auto rect( option->rect );
        QRect tabBarRect( QPoint(0, 0), tabBarSize );

        Qt::Alignment tabBarAlignment( styleHint( SH_TabBar_Alignment, option, widget ) );

        // horizontal positioning
        const bool verticalTabs( isVerticalTab( tabOption->shape ) );
        if( verticalTabs )
        {

            tabBarRect.setHeight( qMin( tabBarRect.height(), rect.height() - 2 ) );
            if( tabBarAlignment == Qt::AlignCenter ) tabBarRect.moveTop( rect.top() + ( rect.height() - tabBarRect.height() )/2 );
            else tabBarRect.moveTop( rect.top()+1 );

        } else {

            // account for corner rects
            // need to re-run visualRect to remove right-to-left handling, since it is re-added on tabBarRect at the end
            const auto leftButtonRect( visualRect( option, subElementRect( SE_TabWidgetLeftCorner, option, widget ) ) );
            const auto rightButtonRect( visualRect( option, subElementRect( SE_TabWidgetRightCorner, option, widget ) ) );

            rect.setLeft( leftButtonRect.width() );
            rect.setRight( rightButtonRect.left() - 1 );

            tabBarRect.setWidth( qMin( tabBarRect.width(), rect.width() - 2 ) );
            if( tabBarAlignment == Qt::AlignCenter ) tabBarRect.moveLeft( rect.left() + (rect.width() - tabBarRect.width())/2 );
            else tabBarRect.moveLeft( rect.left() + 1 );

            tabBarRect = visualRect( option, tabBarRect );

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
        const auto tabOption = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>( option );
        if( !tabOption ) return option->rect;

        // do nothing if tabbar is hidden
        if( tabOption->tabBarSize.isEmpty() ) return option->rect;
        const auto rect = tabWidgetTabPaneRect( option, widget );

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
    QRect Style::tabWidgetTabPaneRect( const QStyleOption* option, const QWidget* ) const
    {

        const auto tabOption = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>( option );
        if( !tabOption || tabOption->tabBarSize.isEmpty() ) return option->rect;

        const int overlap = Metrics::TabBar_BaseOverlap - 1;
        const QSize tabBarSize( tabOption->tabBarSize - QSize( overlap, overlap ) );

        auto rect( option->rect );
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

            default: return QRect();
        }

        return rect;

    }

    //____________________________________________________________________
    QRect Style::tabWidgetCornerRect( SubElement element, const QStyleOption* option, const QWidget* ) const
    {

        // cast option and check
        const auto tabOption = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>( option );
        if( !tabOption ) return option->rect;

        // do nothing if tabbar is hidden
        const QSize tabBarSize( tabOption->tabBarSize );
        if( tabBarSize.isEmpty() ) return QRect();

        // do nothing for vertical tabs
        const bool verticalTabs( isVerticalTab( tabOption->shape ) );
        if( verticalTabs ) return QRect();

        const auto rect( option->rect );
        QRect cornerRect;
        switch( element )
        {
            case SE_TabWidgetLeftCorner:
            cornerRect = QRect( QPoint(0,0), tabOption->leftCornerWidgetSize );
            cornerRect.moveLeft( rect.left() );
            break;

            case SE_TabWidgetRightCorner:
            cornerRect = QRect( QPoint(0,0), tabOption->rightCornerWidgetSize );
            cornerRect.moveRight( rect.right() );
            break;

            default: break;

        }

        // expend height to tabBarSize, if needed, to make sure base is properly rendered
        cornerRect.setHeight( qMax( cornerRect.height(), tabBarSize.height() + 1 ) );

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
        cornerRect = visualRect( option, cornerRect );
        return cornerRect;

    }

    //____________________________________________________________________
    QRect Style::toolBoxTabContentsRect( const QStyleOption* option, const QWidget* widget ) const
    {

        // cast option and check
        const auto toolBoxOption( qstyleoption_cast<const QStyleOptionToolBox *>( option ) );
        if( !toolBoxOption ) return option->rect;

        // copy rect
        const auto& rect( option->rect );

        int contentsWidth(0);
        if( !toolBoxOption->icon.isNull() )
        {
            const int iconSize( pixelMetric( QStyle::PM_SmallIconSize, option, widget ) );
            contentsWidth += iconSize;

            if( !toolBoxOption->text.isEmpty() ) contentsWidth += Metrics::ToolBox_TabItemSpacing;
        }

        if( !toolBoxOption->text.isEmpty() )
        {

            const int textWidth = toolBoxOption->fontMetrics.size( _mnemonics->textFlags(), toolBoxOption->text ).width();
            contentsWidth += textWidth;

        }

        contentsWidth += 2*Metrics::ToolBox_TabMarginWidth;
        contentsWidth = qMin( contentsWidth, rect.width() );
        contentsWidth = qMax( contentsWidth, int(Metrics::ToolBox_TabMinWidth) );
        return centerRect( rect, contentsWidth, rect.height() );

    }

    //____________________________________________________________________
    QRect Style::genericLayoutItemRect( const QStyleOption* option, const QWidget* ) const
    { return insideMargin( option->rect, -Metrics::Frame_FrameWidth ); }

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
                const auto groupBoxOption = qstyleoption_cast<const QStyleOptionGroupBox*>( option );
                if( !groupBoxOption ) break;

                // take out frame width
                rect = insideMargin( rect, Metrics::Frame_FrameWidth );

                // get state
                const bool checkable( groupBoxOption->subControls & QStyle::SC_GroupBoxCheckBox );
                const bool emptyText( groupBoxOption->text.isEmpty() );

                // calculate title height
                int titleHeight( 0 );
                if( !emptyText ) titleHeight = groupBoxOption->fontMetrics.height();
                if( checkable ) titleHeight = qMax( titleHeight, int(Metrics::CheckBox_Size) );

                // add margin
                if( titleHeight > 0 ) titleHeight += 2*Metrics::GroupBox_TitleMarginWidth;

                rect.adjust( 0, titleHeight, 0, 0 );
                return rect;

            }

            case SC_GroupBoxCheckBox:
            case SC_GroupBoxLabel:
            {

                // cast option and check
                const auto groupBoxOption = qstyleoption_cast<const QStyleOptionGroupBox*>( option );
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
                    titleHeight = qMax( titleHeight, int(Metrics::CheckBox_Size) );
                    titleWidth += Metrics::CheckBox_Size;
                    if( !emptyText ) titleWidth += Metrics::CheckBox_ItemSpacing;
                }

                // adjust height
                auto titleRect( rect );
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
                    auto subRect( titleRect );
                    if( checkable ) subRect.adjust( Metrics::CheckBox_Size + Metrics::CheckBox_ItemSpacing, 0, 0, 0 );
                    return visualRect( option->direction, titleRect, subRect );

                }

            }

            default: break;

        }

        return ParentStyleClass::subControlRect( CC_GroupBox, option, subControl, widget );

    }

    //___________________________________________________________________________________________________________________
    QRect Style::toolButtonSubControlRect( const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget ) const
    {

        // cast option and check
        const auto toolButtonOption = qstyleoption_cast<const QStyleOptionToolButton*>( option );
        if( !toolButtonOption ) return ParentStyleClass::subControlRect( CC_ToolButton, option, subControl, widget );

        const bool hasPopupMenu( toolButtonOption->features & QStyleOptionToolButton::MenuButtonPopup );
        const bool hasInlineIndicator(
            toolButtonOption->features&QStyleOptionToolButton::HasMenu
            && toolButtonOption->features&QStyleOptionToolButton::PopupDelay
            && !hasPopupMenu );

        // store rect
        const auto& rect( option->rect );
        const int menuButtonWidth( Metrics::MenuButton_IndicatorWidth );
        switch( subControl )
        {
            case SC_ToolButtonMenu:
            {

                // check features
                if( !(hasPopupMenu || hasInlineIndicator ) ) return QRect();

                // check features
                auto menuRect( rect );
                menuRect.setLeft( rect.right() - menuButtonWidth + 1 );
                if( hasInlineIndicator )
                { menuRect.setTop( menuRect.bottom() - menuButtonWidth + 1 ); }

                return visualRect( option, menuRect );
            }

            case SC_ToolButton:
            {

                if( hasPopupMenu )
                {

                    auto contentsRect( rect );
                    contentsRect.setRight( rect.right() - menuButtonWidth );
                    return visualRect( option, contentsRect );

                } else return rect;

            }

            default: return QRect();

        }

    }

    //___________________________________________________________________________________________________________________
    QRect Style::comboBoxSubControlRect( const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget ) const
    {
        // cast option and check
        const auto comboBoxOption( qstyleoption_cast<const QStyleOptionComboBox*>( option ) );
        if( !comboBoxOption ) return ParentStyleClass::subControlRect( CC_ComboBox, option, subControl, widget );

        const bool editable( comboBoxOption->editable );
        const bool flat( editable && !comboBoxOption->frame );

        // copy rect
        auto rect( option->rect );

        switch( subControl )
        {
            case SC_ComboBoxFrame: return flat ? rect : QRect();
            case SC_ComboBoxListBoxPopup: return rect;

            case SC_ComboBoxArrow:
            {

                // take out frame width
                if( !flat ) rect = insideMargin( rect, Metrics::Frame_FrameWidth );

                QRect arrowRect(
                    rect.right() - Metrics::MenuButton_IndicatorWidth + 1,
                    rect.top(),
                    Metrics::MenuButton_IndicatorWidth,
                    rect.height() );

                arrowRect = centerRect( arrowRect, Metrics::MenuButton_IndicatorWidth, Metrics::MenuButton_IndicatorWidth );
                return visualRect( option, arrowRect );

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
                if( !flat && rect.height() >= option->fontMetrics.height() + 2*frameWidth )
                { labelRect.adjust( frameWidth, frameWidth, 0, -frameWidth ); }

                return visualRect( option, labelRect );

            }

            default: break;

        }

        return ParentStyleClass::subControlRect( CC_ComboBox, option, subControl, widget );

    }

    //___________________________________________________________________________________________________________________
    QRect Style::spinBoxSubControlRect( const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget ) const
    {

        // cast option and check
        const auto spinBoxOption( qstyleoption_cast<const QStyleOptionSpinBox*>( option ) );
        if( !spinBoxOption ) return ParentStyleClass::subControlRect( CC_SpinBox, option, subControl, widget );
        const bool flat( !spinBoxOption->frame );

        // copy rect
        auto rect( option->rect );

        switch( subControl )
        {
            case SC_SpinBoxFrame: return flat ? QRect():rect;

            case SC_SpinBoxUp:
            case SC_SpinBoxDown:
            {

                // take out frame width
                if( !flat && rect.height() >= 2*Metrics::Frame_FrameWidth + Metrics::SpinBox_ArrowButtonWidth ) rect = insideMargin( rect, Metrics::Frame_FrameWidth );

                QRect arrowRect;
                arrowRect = QRect(
                    rect.right() - Metrics::SpinBox_ArrowButtonWidth + 1,
                    rect.top(),
                    Metrics::SpinBox_ArrowButtonWidth,
                    rect.height() );

                const int arrowHeight( qMin( rect.height(), int(Metrics::SpinBox_ArrowButtonWidth) ) );
                arrowRect = centerRect( arrowRect, Metrics::SpinBox_ArrowButtonWidth, arrowHeight );
                arrowRect.setHeight( arrowHeight/2 );
                if( subControl == SC_SpinBoxDown ) arrowRect.translate( 0, arrowHeight/2 );

                return visualRect( option, arrowRect );

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
                if( !flat && labelRect.height() >= option->fontMetrics.height() + 2*frameWidth )
                { labelRect.adjust( frameWidth, frameWidth, 0, -frameWidth ); }

                return visualRect( option, labelRect );

            }

            default: break;

        }

        return ParentStyleClass::subControlRect( CC_SpinBox, option, subControl, widget );

    }

    //___________________________________________________________________________________________________________________
    QRect Style::scrollBarInternalSubControlRect( const QStyleOptionComplex* option, SubControl subControl ) const
    {

        const auto& rect = option->rect;
        const State& state( option->state );
        const bool horizontal( state & State_Horizontal );

        switch( subControl )
        {

            case SC_ScrollBarSubLine:
            {
                int majorSize( scrollBarButtonHeight( _subLineButtons ) );
                if( horizontal ) return visualRect( option, QRect( rect.left(), rect.top(), majorSize, rect.height() ) );
                else return visualRect( option, QRect( rect.left(), rect.top(), rect.width(), majorSize ) );

            }

            case SC_ScrollBarAddLine:
            {
                int majorSize( scrollBarButtonHeight( _addLineButtons ) );
                if( horizontal ) return visualRect( option, QRect( rect.right() - majorSize + 1, rect.top(), majorSize, rect.height() ) );
                else return visualRect( option, QRect( rect.left(), rect.bottom() - majorSize + 1, rect.width(), majorSize ) );
            }

            default: return QRect();

        }
    }

    //___________________________________________________________________________________________________________________
    QRect Style::scrollBarSubControlRect( const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget ) const
    {

        // cast option and check
        const auto sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if( !sliderOption ) return ParentStyleClass::subControlRect( CC_ScrollBar, option, subControl, widget );

        // get relevant state
        const State& state( option->state );
        const bool horizontal( state & State_Horizontal );

        switch( subControl )
        {

            case SC_ScrollBarSubLine:
            case SC_ScrollBarAddLine:
            return scrollBarInternalSubControlRect( option, subControl );

            case SC_ScrollBarGroove:
            {
                auto topRect = visualRect( option, scrollBarInternalSubControlRect( option, SC_ScrollBarSubLine ) );
                auto bottomRect = visualRect( option, scrollBarInternalSubControlRect( option, SC_ScrollBarAddLine ) );

                QPoint topLeftCorner;
                QPoint botRightCorner;

                if( horizontal )
                {

                    topLeftCorner  = QPoint( topRect.right() + 1, topRect.top() );
                    botRightCorner = QPoint( bottomRect.left()  - 1, topRect.bottom() );

                } else {

                    topLeftCorner  = QPoint( topRect.left(),  topRect.bottom() + 1 );
                    botRightCorner = QPoint( topRect.right(), bottomRect.top() - 1 );

                }

                // define rect
                return visualRect( option, QRect( topLeftCorner, botRightCorner )  );

            }

            case SC_ScrollBarSlider:
            {

                // handle RTL here to unreflect things if need be
                auto groove = visualRect( option, subControlRect( CC_ScrollBar, option, SC_ScrollBarGroove, widget ) );

                if( sliderOption->minimum == sliderOption->maximum ) return groove;

                // Figure out how much room there is
                int space( horizontal ? groove.width() : groove.height() );

                // Calculate the portion of this space that the slider should occupy
                int sliderSize = space * qreal( sliderOption->pageStep ) / ( sliderOption->maximum - sliderOption->minimum + sliderOption->pageStep );
                sliderSize = qMax( sliderSize, static_cast<int>(Metrics::ScrollBar_MinSliderHeight ) );
                sliderSize = qMin( sliderSize, space );

                space -= sliderSize;
                if( space <= 0 ) return groove;

                int pos = qRound( qreal( sliderOption->sliderPosition - sliderOption->minimum )/ ( sliderOption->maximum - sliderOption->minimum )*space );
                if( sliderOption->upsideDown ) pos = space - pos;
                if( horizontal ) return visualRect( option, QRect( groove.left() + pos, groove.top(), sliderSize, groove.height() ) );
                else return visualRect( option, QRect( groove.left(), groove.top() + pos, groove.width(), sliderSize ) );
            }

            case SC_ScrollBarSubPage:
            {

                // handle RTL here to unreflect things if need be
                auto slider = visualRect( option, subControlRect( CC_ScrollBar, option, SC_ScrollBarSlider, widget ) );
                auto groove = visualRect( option, subControlRect( CC_ScrollBar, option, SC_ScrollBarGroove, widget ) );

                if( horizontal ) return visualRect( option, QRect( groove.left(), groove.top(), slider.left() - groove.left(), groove.height() ) );
                else return visualRect( option, QRect( groove.left(), groove.top(), groove.width(), slider.top() - groove.top() ) );
            }

            case SC_ScrollBarAddPage:
            {

                // handle RTL here to unreflect things if need be
                auto slider = visualRect( option, subControlRect( CC_ScrollBar, option, SC_ScrollBarSlider, widget ) );
                auto groove = visualRect( option, subControlRect( CC_ScrollBar, option, SC_ScrollBarGroove, widget ) );

                if( horizontal ) return visualRect( option, QRect( slider.right() + 1, groove.top(), groove.right() - slider.right(), groove.height() ) );
                else return visualRect( option, QRect( groove.left(), slider.bottom() + 1, groove.width(), groove.bottom() - slider.bottom() ) );

            }

            default: return ParentStyleClass::subControlRect( CC_ScrollBar, option, subControl, widget );;
        }
    }

    //___________________________________________________________________________________________________________________
    QRect Style::dialSubControlRect( const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget ) const
    {

        // cast option and check
        const auto sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if( !sliderOption ) return ParentStyleClass::subControlRect( CC_Dial, option, subControl, widget );

        // adjust rect to be square, and centered
        auto rect( option->rect );
        const int dimension( qMin( rect.width(), rect.height() ) );
        rect = centerRect( rect, dimension, dimension );

        switch( subControl )
        {
            case QStyle::SC_DialGroove: return insideMargin( rect, (Metrics::Slider_ControlThickness - Metrics::Slider_GrooveThickness)/2 );
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

            default: return ParentStyleClass::subControlRect( CC_Dial, option, subControl, widget );;

        }

    }

    //___________________________________________________________________________________________________________________
    QRect Style::sliderSubControlRect( const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget ) const
    {

        // cast option and check
        const auto sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if( !sliderOption ) return ParentStyleClass::subControlRect( CC_Slider, option, subControl, widget );

        switch( subControl )
        {
            case SC_SliderGroove:
            {

                // direction
                const bool horizontal( sliderOption->orientation == Qt::Horizontal );

                // get base class rect
                auto grooveRect( ParentStyleClass::subControlRect( CC_Slider, option, subControl, widget ) );
                grooveRect = insideMargin( grooveRect, pixelMetric( PM_DefaultFrameWidth, option, widget ) );

                // centering
                if( horizontal ) grooveRect = centerRect( grooveRect, grooveRect.width(), Metrics::Slider_GrooveThickness );
                else grooveRect = centerRect( grooveRect, Metrics::Slider_GrooveThickness, grooveRect.height() );
                return grooveRect;

            }

            default: return ParentStyleClass::subControlRect( CC_Slider, option, subControl, widget );
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
        size.setHeight( qMax( size.height(), int(Metrics::CheckBox_Size) ) );

        // Add space for the indicator and the icon
        size.rwidth() += Metrics::CheckBox_Size + Metrics::CheckBox_ItemSpacing;

        // also add extra space, to leave room to the right of the label
        size.rwidth() += Metrics::CheckBox_ItemSpacing;

        return size;

    }

    //______________________________________________________________
    QSize Style::lineEditSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* widget ) const
    {
        // cast option and check
        const auto frameOption( qstyleoption_cast<const QStyleOptionFrame*>( option ) );
        if( !frameOption ) return contentsSize;

        const bool flat( frameOption->lineWidth == 0 );
        const int frameWidth( pixelMetric( PM_DefaultFrameWidth, option, widget ) );
        return flat ? contentsSize : expandSize( contentsSize, frameWidth );
    }

    //______________________________________________________________
    QSize Style::comboBoxSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* widget ) const
    {

        // cast option and check
        const auto comboBoxOption( qstyleoption_cast<const QStyleOptionComboBox*>( option ) );
        if( !comboBoxOption ) return contentsSize;

        // copy size
        QSize size( contentsSize );

        // add relevant margin
        const bool flat( !comboBoxOption->frame );
        const int frameWidth( pixelMetric( PM_ComboBoxFrameWidth, option, widget ) );
        if( !flat ) size = expandSize( size, frameWidth );

        // make sure there is enough height for the button
        size.setHeight( qMax( size.height(), int(Metrics::MenuButton_IndicatorWidth) ) );

        // add button width and spacing
        size.rwidth() += Metrics::MenuButton_IndicatorWidth+2;
        size.rwidth() += Metrics::Button_ItemSpacing;

        return size;

    }

    //______________________________________________________________
    QSize Style::spinBoxSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* widget ) const
    {

        // cast option and check
        const auto spinBoxOption( qstyleoption_cast<const QStyleOptionSpinBox*>( option ) );
        if( !spinBoxOption ) return contentsSize;

        const bool flat( !spinBoxOption->frame );

        // copy size
        QSize size( contentsSize );

        // add editor margins
        const int frameWidth( pixelMetric( PM_SpinBoxFrameWidth, option, widget ) );
        if( !flat ) size = expandSize( size, frameWidth );

        // make sure there is enough height for the button
        size.setHeight( qMax( size.height(), int(Metrics::SpinBox_ArrowButtonWidth) ) );

        // add button width and spacing
        size.rwidth() += Metrics::SpinBox_ArrowButtonWidth;

        return size;

    }

    //______________________________________________________________
    QSize Style::sliderSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* ) const
    {

        // cast option and check
        const auto sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if( !sliderOption ) return contentsSize;

        // store tick position and orientation
        const QSlider::TickPosition& tickPosition( sliderOption->tickPosition );
        const bool horizontal( sliderOption->orientation == Qt::Horizontal );
        const bool disableTicks( !StyleConfigData::sliderDrawTickMarks() );

        // do nothing if no ticks are requested
        if( tickPosition == QSlider::NoTicks ) return contentsSize;

        /*
         * Qt adds its own tick length directly inside QSlider.
         * Take it out and replace by ours, if needed
         */
        const int tickLength( disableTicks ? 0 : (
            Metrics::Slider_TickLength + Metrics::Slider_TickMarginWidth +
            (Metrics::Slider_GrooveThickness - Metrics::Slider_ControlThickness)/2 ) );

        const int builtInTickLength( 5 );

        QSize size( contentsSize );
        if( horizontal )
        {

            if(tickPosition & QSlider::TicksAbove) size.rheight() += tickLength - builtInTickLength;
            if(tickPosition & QSlider::TicksBelow) size.rheight() += tickLength - builtInTickLength;

        } else {

            if(tickPosition & QSlider::TicksAbove) size.rwidth() += tickLength - builtInTickLength;
            if(tickPosition & QSlider::TicksBelow) size.rwidth() += tickLength - builtInTickLength;

        }

        return size;

    }

    //______________________________________________________________
    QSize Style::pushButtonSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* widget ) const
    {

        // cast option and check
        const auto buttonOption( qstyleoption_cast<const QStyleOptionButton*>( option ) );
        if( !buttonOption ) return contentsSize;

        // output
        QSize size;

        // check text and icon
        const bool hasText( !buttonOption->text.isEmpty() );
        const bool flat( buttonOption->features & QStyleOptionButton::Flat );
        bool hasIcon( !buttonOption->icon.isNull() );

        if( !( hasText||hasIcon ) )
        {

            /*
            no text nor icon is passed.
            assume custom button and use contentsSize as a starting point
            */
            size = contentsSize;

        } else {

            /*
            rather than trying to guess what Qt puts into its contents size calculation,
            we recompute the button size entirely, based on button option
            this ensures consistency with the rendering stage
            */

            // update has icon to honour showIconsOnPushButtons, when possible
            hasIcon &= (showIconsOnPushButtons() || flat || !hasText );

            // text
            if( hasText ) size = buttonOption->fontMetrics.size( Qt::TextShowMnemonic, buttonOption->text );

            // icon
            if( hasIcon )
            {
                QSize iconSize = buttonOption->iconSize;
                if( !iconSize.isValid() ) iconSize = QSize( pixelMetric( PM_SmallIconSize, option, widget ), pixelMetric( PM_SmallIconSize, option, widget ) );

                size.setHeight( qMax( size.height(), iconSize.height() ) );
                size.rwidth() += iconSize.width();

                if( hasText ) size.rwidth() += Metrics::Button_ItemSpacing;
            }

        }

        // menu
        const bool hasMenu( buttonOption->features & QStyleOptionButton::HasMenu );
        if( hasMenu )
        {
            size.rwidth() += Metrics::MenuButton_IndicatorWidth;
            if( hasText||hasIcon ) size.rwidth() += Metrics::Button_ItemSpacing;
        }

        // expand with buttons margin
        size = expandSize( size, Metrics::Button_MarginWidth );

        // make sure buttons have a minimum width
        if( hasText )
        { size.setWidth( qMax( size.width(), int( Metrics::Button_MinWidth ) ) ); }

        // finally add frame margins
        return expandSize( size, Metrics::Frame_FrameWidth );

    }

    //______________________________________________________________
    QSize Style::toolButtonSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* ) const
    {

        // cast option and check
        const auto toolButtonOption = qstyleoption_cast<const QStyleOptionToolButton*>( option );
        if( !toolButtonOption ) return contentsSize;

        // copy size
        QSize size = contentsSize;

        // get relevant state flags
        const State& state( option->state );
        const bool autoRaise( state & State_AutoRaise );
        const bool hasPopupMenu( toolButtonOption->features & QStyleOptionToolButton::MenuButtonPopup );
        const bool hasInlineIndicator(
            toolButtonOption->features&QStyleOptionToolButton::HasMenu
            && toolButtonOption->features&QStyleOptionToolButton::PopupDelay
            && !hasPopupMenu );

        const int marginWidth( autoRaise ? Metrics::ToolButton_MarginWidth : Metrics::Button_MarginWidth + Metrics::Frame_FrameWidth );

        if( hasInlineIndicator ) size.rwidth() += Metrics::ToolButton_InlineIndicatorWidth;
        size = expandSize( size, marginWidth );

        return size;

    }

    //______________________________________________________________
    QSize Style::menuBarItemSizeFromContents( const QStyleOption*, const QSize& contentsSize, const QWidget* ) const
    {

        return expandSize( contentsSize, Metrics::MenuBarItem_MarginWidth, Metrics::MenuBarItem_MarginHeight );

    }

    //______________________________________________________________
    QSize Style::menuItemSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* widget ) const
    {

        // cast option and check
        const auto menuItemOption = qstyleoption_cast<const QStyleOptionMenuItem*>( option );
        if( !menuItemOption ) return contentsSize;

        /*
         * First calculate the intrinsic size of the item.
         * this must be kept consistent with what's in drawMenuItemControl
         */
        QSize size( contentsSize );
        switch( menuItemOption->menuItemType )
        {

            case QStyleOptionMenuItem::Normal:
            case QStyleOptionMenuItem::DefaultItem:
            case QStyleOptionMenuItem::SubMenu:
            {

                int iconWidth = 0;
                if( showIconsInMenuItems() ) iconWidth = isQtQuickControl( option, widget ) ? qMax( pixelMetric(PM_SmallIconSize, option, widget ), menuItemOption->maxIconWidth ) : menuItemOption->maxIconWidth;

                int leftColumnWidth = 0;

                // add icon width
                if( iconWidth > 0 )
                { leftColumnWidth += iconWidth + Metrics::MenuItem_ItemSpacing; }

                // add checkbox indicator width
                if( menuItemOption->menuHasCheckableItems )
                { leftColumnWidth += Metrics::CheckBox_Size + Metrics::MenuItem_ItemSpacing; }

                // add spacing for accelerator
                /*
                 * Note:
                 * The width of the accelerator itself is not included here since
                 * Qt will add that on separately after obtaining the
                 * sizeFromContents() for each menu item in the menu to be shown
                 * ( see QMenuPrivate::calcActionRects() )
                 */
                const bool hasAccelerator( menuItemOption->text.indexOf( QLatin1Char( '\t' ) ) >= 0 );
                if( hasAccelerator ) size.rwidth() += Metrics::MenuItem_AcceleratorSpace;

                // right column
                const int rightColumnWidth = Metrics::MenuButton_IndicatorWidth + Metrics::MenuItem_ItemSpacing;
                size.rwidth() += leftColumnWidth + rightColumnWidth;

                // make sure height is large enough for icon and arrow
                size.setHeight( qMax( size.height(), int(Metrics::MenuButton_IndicatorWidth) ) );
                size.setHeight( qMax( size.height(), int(Metrics::CheckBox_Size) ) );
                size.setHeight( qMax( size.height(), iconWidth ) );
                return expandSize( size, Metrics::MenuItem_MarginWidth, Metrics::MenuItem_MarginHeight );

            }

            case QStyleOptionMenuItem::Separator:
            {

                if( menuItemOption->text.isEmpty() && menuItemOption->icon.isNull() )
                {

                    return expandSize( QSize(0,1), Metrics::MenuItem_MarginWidth, Metrics::MenuItem_MarginHeight );

                } else {

                    // build toolbutton option
                    const QStyleOptionToolButton toolButtonOption( separatorMenuItemOption( menuItemOption, widget ) );

                    // make sure height is large enough for icon and text
                    const int iconWidth( menuItemOption->maxIconWidth );
                    const int textHeight( menuItemOption->fontMetrics.height() );
                    if( !menuItemOption->icon.isNull() ) size.setHeight( qMax( size.height(), iconWidth ) );
                    if( !menuItemOption->text.isEmpty() )
                    {
                        size.setHeight( qMax( size.height(), textHeight ) );
                        size.setWidth( qMax( size.width(), menuItemOption->fontMetrics.width( menuItemOption->text ) ) );
                    }

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
        const auto progressBarOption( qstyleoption_cast<const QStyleOptionProgressBar*>( option ) );
        if( !progressBarOption ) return contentsSize;

        #if BREEZE_USE_KDE4
        const auto progressBarOption2( qstyleoption_cast<const QStyleOptionProgressBarV2*>( option ) );
        const bool horizontal( !progressBarOption2 || progressBarOption2->orientation == Qt::Horizontal );
        #else
        const bool horizontal( BreezePrivate::isProgressBarHorizontal( progressBarOption ) );
        #endif

        // make local copy
        QSize size( contentsSize );

        if( horizontal )
        {

            // check text visibility
            const bool textVisible( progressBarOption->textVisible );

            size.setWidth( qMax( size.width(), int(Metrics::ProgressBar_Thickness) ) );
            size.setHeight( qMax( size.height(), int(Metrics::ProgressBar_Thickness) ) );
            if( textVisible ) size.setHeight( qMax( size.height(), option->fontMetrics.height() ) );

        } else {

            size.setHeight( qMax( size.height(), int(Metrics::ProgressBar_Thickness) ) );
            size.setWidth( qMax( size.width(), int(Metrics::ProgressBar_Thickness) ) );

        }

        return size;

    }

    //______________________________________________________________
    QSize Style::tabWidgetSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* widget ) const
    {
        // cast option and check
        const auto tabOption = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>( option );
        if( !tabOption ) return expandSize( contentsSize, Metrics::TabWidget_MarginWidth );

        // try find direct children of type QTabBar and QStackedWidget
        // this is needed in order to add TabWidget margins only if they are necessary around tabWidget content, not the tabbar
        if( !widget ) return expandSize( contentsSize, Metrics::TabWidget_MarginWidth );
        QTabBar* tabBar = nullptr;
        QStackedWidget* stack = nullptr;
        auto children( widget->children() );
        foreach( auto child, children )
        {
            if( !tabBar ) tabBar = qobject_cast<QTabBar*>( child );
            if( !stack ) stack = qobject_cast<QStackedWidget*>( child );
            if( tabBar && stack ) break;
        }

        if( !( tabBar && stack ) ) return expandSize( contentsSize, Metrics::TabWidget_MarginWidth );

        // tab orientation
        const bool verticalTabs( tabOption && isVerticalTab( tabOption->shape ) );
        if( verticalTabs )
        {
            const int tabBarHeight = tabBar->minimumSizeHint().height();
            const int stackHeight = stack->minimumSizeHint().height();
            if( contentsSize.height() == tabBarHeight && tabBarHeight + 2*(Metrics::Frame_FrameWidth - 1) >= stackHeight + 2*Metrics::TabWidget_MarginWidth ) return QSize( contentsSize.width() + 2*Metrics::TabWidget_MarginWidth, contentsSize.height() + 2*(Metrics::Frame_FrameWidth - 1) );
            else return expandSize( contentsSize, Metrics::TabWidget_MarginWidth );

        } else {

            const int tabBarWidth = tabBar->minimumSizeHint().width();
            const int stackWidth = stack->minimumSizeHint().width();
            if( contentsSize.width() == tabBarWidth && tabBarWidth + 2*(Metrics::Frame_FrameWidth - 1) >= stackWidth + 2*Metrics::TabWidget_MarginWidth) return QSize( contentsSize.width() + 2*(Metrics::Frame_FrameWidth - 1), contentsSize.height() + 2*Metrics::TabWidget_MarginWidth );
            else return expandSize( contentsSize, Metrics::TabWidget_MarginWidth );

        }

    }

    //______________________________________________________________
    QSize Style::tabBarTabSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* ) const
    {
        const auto tabOption( qstyleoption_cast<const QStyleOptionTab*>( option ) );
        const bool hasText( tabOption && !tabOption->text.isEmpty() );
        const bool hasIcon( tabOption && !tabOption->icon.isNull() );
        #if BREEZE_USE_KDE4
        const auto tabOptionV3( qstyleoption_cast<const QStyleOptionTabV3*>( option ) );
        const bool hasLeftButton( tabOptionV3 && !tabOptionV3->leftButtonSize.isEmpty() );
        const bool hasRightButton( tabOptionV3 && !tabOptionV3->leftButtonSize.isEmpty() );
        #else
        const bool hasLeftButton( tabOption && !tabOption->leftButtonSize.isEmpty() );
        const bool hasRightButton( tabOption && !tabOption->leftButtonSize.isEmpty() );
        #endif

        // calculate width increment for horizontal tabs
        int widthIncrement = 0;
        if( hasIcon && !( hasText || hasLeftButton || hasRightButton ) ) widthIncrement -= 4;
        if( hasText && hasIcon ) widthIncrement += Metrics::TabBar_TabItemSpacing;
        if( hasLeftButton && ( hasText || hasIcon ) )  widthIncrement += Metrics::TabBar_TabItemSpacing;
        if( hasRightButton && ( hasText || hasIcon || hasLeftButton ) )  widthIncrement += Metrics::TabBar_TabItemSpacing;

        // add margins
        QSize size( contentsSize );

        // compare to minimum size
        const bool verticalTabs( tabOption && isVerticalTab( tabOption ) );
        if( verticalTabs )
        {

            size.rheight() += widthIncrement;
            if( hasIcon && !hasText ) size = size.expandedTo( QSize( Metrics::TabBar_TabMinHeight, 0 ) );
            else size = size.expandedTo( QSize( Metrics::TabBar_TabMinHeight, Metrics::TabBar_TabMinWidth ) );

        } else {

            size.rwidth() += widthIncrement;
            if( hasIcon && !hasText ) size = size.expandedTo( QSize( 0, Metrics::TabBar_TabMinHeight ) );
            else size = size.expandedTo( QSize( Metrics::TabBar_TabMinWidth, Metrics::TabBar_TabMinHeight ) );

        }

        return size;

    }

    //______________________________________________________________
    QSize Style::headerSectionSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* ) const
    {

        // cast option and check
        const auto headerOption( qstyleoption_cast<const QStyleOptionHeader*>( option ) );
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
            if( hasText ) contentsWidth += Metrics::Header_ItemSpacing;
        }

        // contents height
        int contentsHeight( headerOption->fontMetrics.height() );
        if( hasIcon ) contentsHeight = qMax( contentsHeight, iconSize.height() );

        if( horizontal && headerOption->sortIndicator != QStyleOptionHeader::None )
        {
            // also add space for sort indicator
            contentsWidth += Metrics::Header_ArrowSize + Metrics::Header_ItemSpacing;
            contentsHeight = qMax( contentsHeight, int(Metrics::Header_ArrowSize) );
        }

        // update contents size, add margins and return
        const QSize size( contentsSize.expandedTo( QSize( contentsWidth, contentsHeight ) ) );
        return expandSize( size, Metrics::Header_MarginWidth );

    }

    //______________________________________________________________
    QSize Style::itemViewItemSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* widget ) const
    {
        // call base class
        const QSize size( ParentStyleClass::sizeFromContents( CT_ItemViewItem, option, contentsSize, widget ) );
        return expandSize( size, Metrics::ItemView_ItemMarginWidth );
    }

    //______________________________________________________________
    bool Style::drawFramePrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // copy palette and rect
        const auto& palette( option->palette );
        const auto& rect( option->rect );

        // detect title widgets
        const bool isTitleWidget(
            StyleConfigData::titleWidgetDrawFrame() &&
            widget &&
            widget->parent() &&
            widget->parent()->inherits( "KTitleWidget" ) );

        // copy state
        const State& state( option->state );
        if( !isTitleWidget && !( state & (State_Sunken | State_Raised ) ) ) return true;

        #if QT_VERSION >= 0x050000
        const bool isInputWidget( ( widget && widget->testAttribute( Qt::WA_Hover ) ) ||
            ( isQtQuickControl( option, widget ) && option->styleObject->property( "elementType" ).toString() == QStringLiteral( "edit") ) );
        #else
        const bool isInputWidget( ( widget && widget->testAttribute( Qt::WA_Hover ) ) );
        #endif

        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && isInputWidget && ( state & State_MouseOver ) );
        const bool hasFocus( enabled && isInputWidget && ( state & State_HasFocus ) );

        // focus takes precedence over mouse over
        _animations->inputWidgetEngine().updateState( widget, AnimationFocus, hasFocus );
        _animations->inputWidgetEngine().updateState( widget, AnimationHover, mouseOver && !hasFocus );

        // retrieve animation mode and opacity
        const AnimationMode mode( _animations->inputWidgetEngine().frameAnimationMode( widget ) );
        const qreal opacity( _animations->inputWidgetEngine().frameOpacity( widget ) );

        // render
        if( !StyleConfigData::sidePanelDrawFrame() && widget && widget->property( PropertyNames::sidePanelView ).toBool() )
        {

            const auto outline( _helper->sidePanelOutlineColor( palette, hasFocus, opacity, mode ) );
            const bool reverseLayout( option->direction == Qt::RightToLeft );
            const Side side( reverseLayout ? SideRight : SideLeft );
            _helper->renderSidePanelFrame( painter, rect, outline, side );

        } else {

            if( _frameShadowFactory->isRegistered( widget ) )
            {

                // update frame shadow factory
                _frameShadowFactory->updateShadowsGeometry( widget, rect );
                _frameShadowFactory->updateState( widget, hasFocus, mouseOver, opacity, mode );

            }

            const auto background( isTitleWidget ? palette.color( widget->backgroundRole() ):QColor() );
            const auto outline( _helper->frameOutlineColor( palette, mouseOver, hasFocus, opacity, mode ) );
            _helper->renderFrame( painter, rect, background, outline );

        }

        return true;

    }

    //______________________________________________________________
    bool Style::drawFrameLineEditPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {
        // copy palette and rect
        const auto& palette( option->palette );
        const auto& rect( option->rect );

        // make sure there is enough room to render frame
        if( rect.height() < 2*Metrics::LineEdit_FrameWidth + option->fontMetrics.height())
        {

            const auto &background = palette.color( QPalette::Base );

            painter->setPen( Qt::NoPen );
            painter->setBrush( background );
            painter->drawRect( rect );
            return true;

        } else {

            // copy state
            const State& state( option->state );
            const bool enabled( state & State_Enabled );
            const bool mouseOver( enabled && ( state & State_MouseOver ) );
            const bool hasFocus( enabled && ( state & State_HasFocus ) );

            // focus takes precedence over mouse over
            _animations->inputWidgetEngine().updateState( widget, AnimationFocus, hasFocus );
            _animations->inputWidgetEngine().updateState( widget, AnimationHover, mouseOver && !hasFocus );

            // retrieve animation mode and opacity
            const AnimationMode mode( _animations->inputWidgetEngine().frameAnimationMode( widget ) );
            const qreal opacity( _animations->inputWidgetEngine().frameOpacity( widget ) );

            // render
            const auto &background = palette.color( QPalette::Base );
            const auto outline( _helper->frameOutlineColor( palette, mouseOver, hasFocus, opacity, mode ) );
            _helper->renderFrame( painter, rect, background, outline );

        }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawFrameFocusRectPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {
        // no focus indicator on buttons / scrollbars, since it is rendered elsewhere
        if ( qobject_cast<const QAbstractButton*>( widget ) || qobject_cast<const QScrollBar*>( widget ) || qobject_cast<const QGroupBox*>( widget ) )
        { return true; }

        // no focus indicator on ComboBox list items
        if (widget && widget->inherits("QComboBoxListView"))
        { return true; }

        #if QT_VERSION >= 0x050000
        if ( option->styleObject && option->styleObject->property("elementType") == QLatin1String("button") )
        { return true; }
        #endif

        const State& state( option->state );

        // no focus indicator on selected list items
        if ((state & State_Selected) && qobject_cast<const QAbstractItemView *>(widget))
        { return true; }

        const auto rect( option->rect.adjusted( 0, 0, 0, 1 ) );
        const auto& palette( option->palette );

        if( rect.width() < 10 ) return true;

        const auto outlineColor( state & State_Selected ? palette.color( QPalette::HighlightedText ):palette.color( QPalette::Highlight ) );
        painter->setRenderHint( QPainter::Antialiasing, false );
        painter->setPen( outlineColor );
        painter->drawLine( QPoint( rect.bottomLeft() - QPoint( 0,1 ) ), QPoint( rect.bottomRight() - QPoint( 0,1 ) ) );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawFrameMenuPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {
        // only draw frame for (expanded) toolbars and QtQuick controls
        // do nothing for other cases, for which frame is rendered via drawPanelMenuPrimitive
        if( qobject_cast<const QToolBar*>( widget ) )
        {

            const auto& palette( option->palette );
            const auto background( _helper->frameBackgroundColor( palette ) );
            const auto outline( _helper->frameOutlineColor( palette ) );

            const bool hasAlpha( _helper->hasAlphaChannel( widget ) );
            _helper->renderMenuFrame( painter, option->rect, background, outline, hasAlpha );

        } else if( isQtQuickControl( option, widget ) ) {

            const auto& palette( option->palette );
            const auto background( _helper->frameBackgroundColor( palette ) );
            const auto outline( _helper->frameOutlineColor( palette ) );

            const bool hasAlpha( _helper->hasAlphaChannel( widget ) );
            _helper->renderMenuFrame( painter, option->rect, background, outline, hasAlpha );

        }

        return true;

    }

    //______________________________________________________________
    bool Style::drawFrameGroupBoxPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        // cast option and check
        const auto frameOption( qstyleoption_cast<const QStyleOptionFrame*>( option ) );
        if( !frameOption ) return true;

        // no frame for flat groupboxes
        #if BREEZE_USE_KDE4
        QStyleOptionFrameV2 frameOption2( *frameOption );
        if( frameOption2.features & QStyleOptionFrameV2::Flat ) return true;
        #else
        if( frameOption->features & QStyleOptionFrame::Flat ) return true;
        #endif

        // normal frame
        const auto& palette( option->palette );
        const auto background( _helper->frameBackgroundColor( palette ) );
        const auto outline( _helper->frameOutlineColor( palette ) );

        /*
         * need to reset painter's clip region in order to paint behind textbox label
         * (was taken out in QCommonStyle)
         */

        painter->setClipRegion( option->rect );
        _helper->renderFrame( painter, option->rect, background, outline );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawFrameTabWidgetPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        #if BREEZE_USE_KDE4
        const auto tabOption( qstyleoption_cast<const QStyleOptionTabWidgetFrameV2*>( option ) );
        #else
        const auto tabOption( qstyleoption_cast<const QStyleOptionTabWidgetFrame*>( option ) );
        #endif
        if( !tabOption ) return true;

        // do nothing if tabbar is hidden
        const bool isQtQuickControl( this->isQtQuickControl( option, widget ) );
        if( tabOption->tabBarSize.isEmpty() && !isQtQuickControl ) return true;

        // adjust rect to handle overlaps
        auto rect( option->rect );

        const auto tabBarRect( tabOption->tabBarRect );
        const QSize tabBarSize( tabOption->tabBarSize );
        Corners corners = AllCorners;

        // adjust corners to deal with oversized tabbars
        switch( tabOption->shape )
        {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
            if( isQtQuickControl ) rect.adjust( -1, -1, 1, 0 );
            if( tabBarSize.width() >= rect.width() - 2*Metrics::Frame_FrameRadius ) corners &= ~CornersTop;
            if( tabBarRect.left() < rect.left() + Metrics::Frame_FrameRadius ) corners &= ~CornerTopLeft;
            if( tabBarRect.right() > rect.right() - Metrics::Frame_FrameRadius ) corners &= ~CornerTopRight;
            break;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            if( isQtQuickControl ) rect.adjust( -1, 0, 1, 1 );
            if( tabBarSize.width() >= rect.width()-2*Metrics::Frame_FrameRadius ) corners &= ~CornersBottom;
            if( tabBarRect.left() < rect.left() + Metrics::Frame_FrameRadius ) corners &= ~CornerBottomLeft;
            if( tabBarRect.right() > rect.right() - Metrics::Frame_FrameRadius ) corners &= ~CornerBottomRight;
            break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            if( isQtQuickControl ) rect.adjust( -1, 0, 0, 0 );
            if( tabBarSize.height() >= rect.height()-2*Metrics::Frame_FrameRadius ) corners &= ~CornersLeft;
            if( tabBarRect.top() < rect.top() + Metrics::Frame_FrameRadius ) corners &= ~CornerTopLeft;
            if( tabBarRect.bottom() > rect.bottom() - Metrics::Frame_FrameRadius ) corners &= ~CornerBottomLeft;
            break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            if( isQtQuickControl ) rect.adjust( 0, 0, 1, 0 );
            if( tabBarSize.height() >= rect.height()-2*Metrics::Frame_FrameRadius ) corners &= ~CornersRight;
            if( tabBarRect.top() < rect.top() + Metrics::Frame_FrameRadius ) corners &= ~CornerTopRight;
            if( tabBarRect.bottom() > rect.bottom() - Metrics::Frame_FrameRadius ) corners &= ~CornerBottomRight;
            break;

            default: break;
        }

        // define colors
        const auto& palette( option->palette );
        const auto background( _helper->frameBackgroundColor( palette ) );
        const auto outline( _helper->frameOutlineColor( palette ) );
        _helper->renderTabWidgetFrame( painter, rect, background, outline, corners );

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawFrameTabBarBasePrimitive( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        // tabbar frame used either for 'separate' tabbar, or in 'document mode'

        // cast option and check
        const auto tabOption( qstyleoption_cast<const QStyleOptionTabBarBase*>( option ) );
        if( !tabOption ) return true;

        // get rect, orientation, palette
        const auto rect( option->rect );
        const auto outline( _helper->frameOutlineColor( option->palette ) );

        // setup painter
        painter->setBrush( Qt::NoBrush );
        painter->setRenderHint( QPainter::Antialiasing, false );
        painter->setPen( QPen( outline, 1 ) );

        // render
        switch( tabOption->shape )
        {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
            painter->drawLine( rect.bottomLeft() - QPoint( 1, 0 ), rect.bottomRight() + QPoint( 1, 0 ) );
            break;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            painter->drawLine( rect.topLeft() - QPoint( 1, 0 ), rect.topRight() + QPoint( 1, 0 ) );
            break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            painter->drawLine( rect.topRight() - QPoint( 0, 1 ), rect.bottomRight() + QPoint( 1, 0 ) );
            break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            painter->drawLine( rect.topLeft() - QPoint( 0, 1 ), rect.bottomLeft() + QPoint( 1, 0 ) );
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
        const auto& rect( option->rect );
        const auto& palette( option->palette );
        const State state( option->state );
        const bool selected( state & State_Selected );

        // render frame outline
        const auto outline( _helper->frameOutlineColor( palette, false, selected ) );
        _helper->renderMenuFrame( painter, rect, QColor(), outline );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorArrowPrimitive( ArrowOrientation orientation, const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // store rect and palette
        const auto& rect( option->rect );
        const auto& palette( option->palette );

        // store state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        bool mouseOver( enabled && ( state & State_MouseOver ) );
        bool hasFocus( enabled && ( state & State_HasFocus ) );

        // detect special buttons
        const bool inTabBar( widget && qobject_cast<const QTabBar*>( widget->parentWidget() ) );
        const bool inToolButton( qstyleoption_cast<const QStyleOptionToolButton *>( option ) );

        // color
        QColor color;
        if( inTabBar ) {

            // for tabbar arrows one uses animations to get the arrow color
            /*
             * get animation state
             * there is no need to update the engine since this was already done when rendering the frame
             */
            const AnimationMode mode( _animations->widgetStateEngine().buttonAnimationMode( widget ) );
            const qreal opacity( _animations->widgetStateEngine().buttonOpacity( widget ) );
            color = _helper->arrowColor( palette, mouseOver, hasFocus, opacity, mode );

        } else if( mouseOver && !inToolButton ) {

            color = _helper->hoverColor( palette );

        } else if( inToolButton ) {

            const bool flat( state & State_AutoRaise );

            // cast option
            const QStyleOptionToolButton* toolButtonOption( static_cast<const QStyleOptionToolButton*>( option ) );
            const bool hasMenu(
                ( toolButtonOption->subControls & SC_ToolButtonMenu ) ||
                ( toolButtonOption->features&QStyleOptionToolButton::HasMenu
                && toolButtonOption->features&QStyleOptionToolButton::PopupDelay ) );
            const bool sunken( state & (State_On | State_Sunken) );
            if( flat && hasMenu )
            {

                if( sunken && !mouseOver ) color = palette.color( QPalette::HighlightedText );
                else {

                    // for menu arrows in flat toolbutton one uses animations to get the arrow color
                    // handle arrow over animation
                    const bool arrowHover( mouseOver && ( toolButtonOption->activeSubControls & SC_ToolButtonMenu ) );
                    _animations->toolButtonEngine().updateState( widget, AnimationHover, arrowHover );

                    const bool animated( _animations->toolButtonEngine().isAnimated( widget, AnimationHover ) );
                    const qreal opacity( _animations->toolButtonEngine().opacity( widget, AnimationHover ) );

                    color = _helper->arrowColor( palette, arrowHover, false, opacity, animated ? AnimationHover:AnimationNone );

                }

            } else if( flat ) {

                if( sunken && hasFocus && !mouseOver ) color = palette.color( QPalette::HighlightedText );
                else color = _helper->arrowColor( palette, QPalette::WindowText );

            } else if( hasFocus && !mouseOver )  {

                color = palette.color( QPalette::HighlightedText );

            } else {

                color = _helper->arrowColor( palette, QPalette::ButtonText );

            }

        } else color = _helper->arrowColor( palette, QPalette::WindowText );

        // render
        _helper->renderArrow( painter, rect, color, orientation );

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorHeaderArrowPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {
        const auto headerOption( qstyleoption_cast<const QStyleOptionHeader*>( option ) );
        const State& state( option->state );

        // arrow orientation
        ArrowOrientation orientation( ArrowNone );
        if( state & State_UpArrow || ( headerOption && headerOption->sortIndicator==QStyleOptionHeader::SortUp ) ) orientation = ArrowUp;
        else if( state & State_DownArrow || ( headerOption && headerOption->sortIndicator==QStyleOptionHeader::SortDown ) ) orientation = ArrowDown;
        if( orientation == ArrowNone ) return true;

        // invert arrows if requested by (hidden) options
        if( StyleConfigData::viewInvertSortIndicator() ) orientation = (orientation == ArrowUp) ? ArrowDown:ArrowUp;

        // state, rect and palette
        const auto& rect( option->rect );
        const auto& palette( option->palette );

        // define color and polygon for drawing arrow
        const auto color = _helper->arrowColor( palette, QPalette::ButtonText );

        // render
        _helper->renderArrow( painter, rect, color, orientation );

        return true;
    }

    //______________________________________________________________
    bool Style::drawPanelButtonCommandPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const auto buttonOption( qstyleoption_cast< const QStyleOptionButton* >( option ) );
        if( !buttonOption ) return true;

        // rect and palette
        const auto& rect( option->rect );

        // button state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && ( state & State_MouseOver ) );
        const bool hasFocus( ( enabled && ( state & State_HasFocus ) ) && !( widget && widget->focusProxy()));
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
            const auto& palette( option->palette );
            const auto color( _helper->toolButtonColor( palette, mouseOver, hasFocus, sunken, opacity, mode ) );
            _helper->renderToolButtonFrame( painter, rect, color, sunken );

        } else {

            // update button color from palette in case button is default
            QPalette palette( option->palette );
            if( enabled && buttonOption->features & QStyleOptionButton::DefaultButton )
            {
                const auto button( palette.color( QPalette::Button ) );
                const auto base( palette.color( QPalette::Base ) );
                palette.setColor( QPalette::Button, KColorUtils::mix( button, base, 0.7 ) );
            }

            const auto shadow( _helper->shadowColor( palette ) );
            const auto outline( _helper->buttonOutlineColor( palette, mouseOver, hasFocus, opacity, mode ) );
            const auto background( _helper->buttonBackgroundColor( palette, mouseOver, hasFocus, sunken, opacity, mode ) );

            // render
            _helper->renderButtonFrame( painter, rect, background, outline, shadow, hasFocus, sunken );

        }

        return true;

    }

    //______________________________________________________________
    bool Style::drawPanelButtonToolPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // copy palette and rect
        const auto& palette( option->palette );
        auto rect( option->rect );

        // store relevant flags
        const State& state( option->state );
        const bool autoRaise( state & State_AutoRaise );
        const bool enabled( state & State_Enabled );
        const bool sunken( state & (State_On | State_Sunken) );
        const bool mouseOver( enabled && (option->state & State_MouseOver) );
        const bool hasFocus( enabled && (option->state & (State_HasFocus | State_Sunken)) );

        /*
         * get animation state
         * no need to update, this was already done in drawToolButtonComplexControl
         */
        const AnimationMode mode( _animations->widgetStateEngine().buttonAnimationMode( widget ) );
        const qreal opacity( _animations->widgetStateEngine().buttonOpacity( widget ) );

        if( !autoRaise )
        {

            // need to check widget for popup mode, because option is not set properly
            const auto toolButton( qobject_cast<const QToolButton*>( widget ) );
            const bool hasPopupMenu( toolButton && toolButton->popupMode() == QToolButton::MenuButtonPopup );

            // render as push button
            const auto shadow( _helper->shadowColor( palette ) );
            const auto outline( _helper->buttonOutlineColor( palette, mouseOver, hasFocus, opacity, mode ) );
            const auto background( _helper->buttonBackgroundColor( palette, mouseOver, hasFocus, sunken, opacity, mode ) );

            // adjust frame in case of menu
            if( hasPopupMenu )
            {
                painter->setClipRect( rect );
                rect.adjust( 0, 0, Metrics::Frame_FrameRadius + 2, 0 );
                rect = visualRect( option, rect );
            }

            // render
            _helper->renderButtonFrame( painter, rect, background, outline, shadow, hasFocus, sunken );

        } else {

            const auto color( _helper->toolButtonColor( palette, mouseOver, hasFocus, sunken, opacity, mode ) );
            _helper->renderToolButtonFrame( painter, rect, color, sunken );

        }

        return true;
    }

    //______________________________________________________________
    bool Style::drawTabBarPanelButtonToolPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // copy palette and rect
        auto rect( option->rect );

        // static_cast is safe here since check was already performed in calling function
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
        const auto& palette( parent ? parent->palette() : QApplication::palette() );
        const auto color = hasAlteredBackground(parent) ? _helper->frameBackgroundColor( palette ):palette.color( QPalette::Window );

        // render flat background
        painter->setPen( Qt::NoPen );
        painter->setBrush( color );
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
            const int frameWidth( pixelMetric( PM_DefaultFrameWidth, nullptr, scrollArea ) );
            painter->setClipRect( insideMargin( scrollArea->rect(), frameWidth ) );
            painter->setBrush( scrollArea->viewport()->palette().color( scrollArea->viewport()->backgroundRole() ) );
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
        /*
         * do nothing if menu is embedded in another widget
         * this corresponds to having a transparent background
         */
        if( widget && !widget->isWindow() ) return true;

        const auto& palette( option->palette );
        const auto outline( _helper->frameOutlineColor( palette ) );
        const bool hasAlpha( _helper->hasAlphaChannel( widget ) );
        auto background( _helper->frameBackgroundColor( palette ) );

        #if !BREEZE_USE_KDE4
        if ( hasAlpha ) {
            background.setAlphaF(StyleConfigData::menuOpacity() / 100.0);
        }
        #endif

        _helper->renderMenuFrame( painter, option->rect, background, outline, hasAlpha );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawPanelTipLabelPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // force registration of widget
        if( widget && widget->window() )
        { _shadowHelper->registerWidget( widget->window(), true ); }

        const auto& palette( option->palette );
        const auto &background = palette.color( QPalette::ToolTipBase );
        const auto outline( KColorUtils::mix( palette.color( QPalette::ToolTipBase ), palette.color( QPalette::ToolTipText ), 0.25 ) );
        const bool hasAlpha( _helper->hasAlphaChannel( widget ) );

        _helper->renderMenuFrame( painter, option->rect, background, outline, hasAlpha );
        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawPanelItemViewItemPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        #if BREEZE_USE_KDE4
        const auto viewItemOption = qstyleoption_cast<const QStyleOptionViewItemV4*>( option );
        #else
        const auto viewItemOption = qstyleoption_cast<const QStyleOptionViewItem*>( option );
        #endif
        if( !viewItemOption ) return false;

        // try cast widget
        const auto abstractItemView = qobject_cast<const QAbstractItemView *>( widget );

        // store palette and rect
        const auto& palette( option->palette );
        auto rect( option->rect );

        // store flags
        const State& state( option->state );
        const bool mouseOver( ( state & State_MouseOver ) && ( !abstractItemView || abstractItemView->selectionMode() != QAbstractItemView::NoSelection ) );
        const bool selected( state & State_Selected );
        const bool enabled( state & State_Enabled );
        const bool active( state & State_Active );

        const bool hasCustomBackground = viewItemOption->backgroundBrush.style() != Qt::NoBrush && !( state & State_Selected );
        const bool hasSolidBackground = !hasCustomBackground || viewItemOption->backgroundBrush.style() == Qt::SolidPattern;
        #if BREEZE_USE_KDE4
        const bool hasAlternateBackground( viewItemOption->features & QStyleOptionViewItemV2::Alternate );
        #else
        const bool hasAlternateBackground( viewItemOption->features & QStyleOptionViewItem::Alternate );
        #endif

        // do nothing if no background is to be rendered
        if( !( mouseOver || selected || hasCustomBackground || hasAlternateBackground ) )
        { return true; }

        // define color group
        QPalette::ColorGroup colorGroup;
        if( enabled ) colorGroup = active ? QPalette::Active : QPalette::Inactive;
        else colorGroup = QPalette::Disabled;

        // render alternate background
        if( hasAlternateBackground )
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

        // render
        _helper->renderSelection( painter, rect, color );

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorCheckBoxPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // copy rect and palette
        const auto& rect( option->rect );
        const auto& palette( option->palette );

        // store flags
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && ( state & State_MouseOver ) );
        const bool sunken( enabled && ( state & State_Sunken ) );
        const bool active( ( state & (State_On|State_NoChange) ) );

        // checkbox state
        CheckBoxState checkBoxState( CheckOff );
        if( state & State_NoChange ) checkBoxState = CheckPartial;
        else if( state & State_On ) checkBoxState = CheckOn;

        // detect checkboxes in lists
        const bool isSelectedItem( this->isSelectedItem( widget, rect.center() ) );

        // animation state
        _animations->widgetStateEngine().updateState( widget, AnimationHover, mouseOver );
        _animations->widgetStateEngine().updateState( widget, AnimationPressed, checkBoxState != CheckOff );
        if( _animations->widgetStateEngine().isAnimated( widget, AnimationPressed ) ) checkBoxState = CheckAnimated;
        const qreal animation( _animations->widgetStateEngine().opacity( widget, AnimationPressed ) );

        QColor color;
        if( isSelectedItem ) {

            color = _helper->checkBoxIndicatorColor( palette, false, enabled && active  );
            _helper->renderCheckBoxBackground( painter, rect, palette.color( QPalette::Base ), sunken );

        } else {

            const AnimationMode mode( _animations->widgetStateEngine().isAnimated( widget, AnimationHover ) ? AnimationHover:AnimationNone );
            const qreal opacity( _animations->widgetStateEngine().opacity( widget, AnimationHover ) );
            color = _helper->checkBoxIndicatorColor( palette, mouseOver, enabled && active, opacity, mode  );

        }

        // render
        const auto shadow( _helper->shadowColor( palette ) );
        _helper->renderCheckBox( painter, rect, color, shadow, sunken, checkBoxState, animation );
        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorRadioButtonPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // copy rect and palette
        const auto& rect( option->rect );
        const auto& palette( option->palette );

        // store flags
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && ( state & State_MouseOver ) );
        const bool sunken( state & State_Sunken );
        const bool checked( state & State_On );

        // radio button state
        RadioButtonState radioButtonState( state & State_On ? RadioOn:RadioOff );

        // detect radiobuttons in lists
        const bool isSelectedItem( this->isSelectedItem( widget, rect.center() ) );

        // animation state
        _animations->widgetStateEngine().updateState( widget, AnimationHover, mouseOver );
        _animations->widgetStateEngine().updateState( widget, AnimationPressed, radioButtonState != RadioOff );
        if( _animations->widgetStateEngine().isAnimated( widget, AnimationPressed ) ) radioButtonState = RadioAnimated;
        const qreal animation( _animations->widgetStateEngine().opacity( widget, AnimationPressed ) );

        // colors
        const auto shadow( _helper->shadowColor( palette ) );
        QColor color;
        if( isSelectedItem )
        {

            color = _helper->checkBoxIndicatorColor( palette, false, enabled && checked );
            _helper->renderRadioButtonBackground( painter, rect, palette.color( QPalette::Base ), sunken );

        } else {

            const AnimationMode mode( _animations->widgetStateEngine().isAnimated( widget, AnimationHover ) ? AnimationHover:AnimationNone );
            const qreal opacity( _animations->widgetStateEngine().opacity( widget, AnimationHover ) );
            color = _helper->checkBoxIndicatorColor( palette, mouseOver, enabled && checked, opacity, mode  );

        }

        // render
        _helper->renderRadioButton( painter, rect, color, shadow, sunken, radioButtonState, animation );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorButtonDropDownPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const auto toolButtonOption( qstyleoption_cast<const QStyleOptionToolButton*>( option ) );
        if( !toolButtonOption ) return true;

        // store state
        const State& state( option->state );
        const bool autoRaise( state & State_AutoRaise );

        // do nothing for autoraise buttons
        if( autoRaise || !(toolButtonOption->subControls & SC_ToolButtonMenu) ) return true;

        // store palette and rect
        const auto& palette( option->palette );
        const auto& rect( option->rect );

        // store state
        const bool enabled( state & State_Enabled );
        const bool hasFocus( enabled && ( state & ( State_HasFocus | State_Sunken ) ) );
        const bool mouseOver( enabled && ( state & State_MouseOver ) );
        const bool sunken( enabled && ( state & State_Sunken ) );

        // update animation state
        // mouse over takes precedence over focus
        _animations->widgetStateEngine().updateState( widget, AnimationHover, mouseOver );
        _animations->widgetStateEngine().updateState( widget, AnimationFocus, hasFocus && !mouseOver );

        const AnimationMode mode( _animations->widgetStateEngine().buttonAnimationMode( widget ) );
        const qreal opacity( _animations->widgetStateEngine().buttonOpacity( widget ) );

        // render as push button
        const auto shadow( _helper->shadowColor( palette ) );
        const auto outline( _helper->buttonOutlineColor( palette, mouseOver, hasFocus, opacity, mode ) );
        const auto background( _helper->buttonBackgroundColor( palette, mouseOver, hasFocus, false, opacity, mode ) );

        auto frameRect( rect );
        painter->setClipRect( rect );
        frameRect.adjust( -Metrics::Frame_FrameRadius - 1, 0, 0, 0 );
        frameRect = visualRect( option, frameRect );

        // render
        _helper->renderButtonFrame( painter, frameRect, background, outline, shadow, hasFocus, sunken );

        // also render separator
        auto separatorRect( rect.adjusted( 0, 2, -2, -2 ) );
        separatorRect.setWidth( 1 );
        separatorRect = visualRect( option, separatorRect );
        if( sunken ) separatorRect.translate( 1, 1 );
        _helper->renderSeparator( painter, separatorRect, outline, true );

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
        const auto tabOption( qstyleoption_cast<const QStyleOptionTab*>( option ) );
        if( !tabOption ) return true;

        // store palette and rect
        const auto& palette( option->palette );
        auto rect( option->rect );

        const bool reverseLayout( option->direction == Qt::RightToLeft );

        const auto color( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.2 ) );
        painter->setRenderHint( QPainter::Antialiasing, false );
        painter->setPen( color );
        painter->setBrush( Qt::NoBrush );
        switch( tabOption->shape )
        {

            case QTabBar::TriangularNorth:
            case QTabBar::RoundedNorth:
            rect.adjust( 0, 1, 0, 0 );
            if( reverseLayout ) painter->drawLine( rect.topRight(), rect.bottomRight() );
            else painter->drawLine( rect.topLeft(), rect.bottomLeft() );
            break;

            case QTabBar::TriangularSouth:
            case QTabBar::RoundedSouth:
            rect.adjust( 0, 0, 0, -1 );
            if( reverseLayout ) painter->drawLine( rect.topRight(), rect.bottomRight() );
            else painter->drawLine( rect.topLeft(), rect.bottomLeft() );
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

        // do nothing if disabled from options
        if( !StyleConfigData::toolBarDrawItemSeparator() ) return true;

        // store rect and palette
        auto rect( option->rect );
        const auto& palette( option->palette );

        // store state
        const State& state( option->state );
        const bool separatorIsVertical( state & State_Horizontal );

        // define color and render
        const auto color( _helper->separatorColor( palette ) );
        if( separatorIsVertical )
        {
            rect.setWidth( Metrics::ToolBar_HandleWidth );
            rect = centerRect( option->rect, rect.size() );
            rect.setWidth( 3 );
            _helper->renderSeparator( painter, rect, color, separatorIsVertical );

            rect.translate( 2, 0 );
            _helper->renderSeparator( painter, rect, color, separatorIsVertical );

        } else {

            rect.setHeight( Metrics::ToolBar_HandleWidth );
            rect = centerRect( option->rect, rect.size() );
            rect.setHeight( 3 );
            _helper->renderSeparator( painter, rect, color, separatorIsVertical );

            rect.translate( 0, 2 );
            _helper->renderSeparator( painter, rect, color, separatorIsVertical );

        }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorToolBarSeparatorPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        /*
         * do nothing if disabled from options
         * also need to check if widget is a combobox, because of Qt hack using 'toolbar' separator primitive
         * for rendering separators in comboboxes
         */
        if( !( StyleConfigData::toolBarDrawItemSeparator() || qobject_cast<const QComboBox*>( widget ) ) )
        { return true; }

        // store rect and palette
        const auto& rect( option->rect );
        const auto& palette( option->palette );

        // store state
        const State& state( option->state );
        const bool separatorIsVertical( state & State_Horizontal );

        // define color and render
        const auto color( _helper->separatorColor( palette ) );
        _helper->renderSeparator( painter, rect, color, separatorIsVertical );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorBranchPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        // copy rect and palette
        const auto& rect( option->rect );
        const auto& palette( option->palette );

        // state
        const State& state( option->state );
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
            expanderSize = qMin( expanderSize, int(Metrics::ItemView_ArrowSize) );
            expanderAdjust = expanderSize/2 + 1;
            const auto arrowRect = centerRect( rect, expanderSize, expanderSize );

            // get orientation from option
            ArrowOrientation orientation;
            if( expanderOpen ) orientation = ArrowDown;
            else if( reverseLayout ) orientation = ArrowLeft;
            else orientation = ArrowRight;

            // color
            const auto arrowColor( mouseOver ? _helper->hoverColor( palette ) : _helper->arrowColor( palette, QPalette::Text ) );

            // render
            _helper->renderArrow( painter, arrowRect, arrowColor, orientation );

        }

        // tree branches
        if( !StyleConfigData::viewDrawTreeBranchLines() ) return true;

        const auto center( rect.center() );
        const auto lineColor( KColorUtils::mix( palette.color( QPalette::Base ), palette.color( QPalette::Text ), 0.25 ) );
        painter->setRenderHint( QPainter::Antialiasing, true );
        painter->translate( 0.5, 0.5 );
        painter->setPen( QPen( lineColor, 1 ) );
        if( state & ( State_Item | State_Children | State_Sibling ) )
        {
            const QLineF line( QPointF( center.x(), rect.top() ), QPointF( center.x(), center.y() - expanderAdjust - 1 ) );
            painter->drawLine( line );
        }

        // The right/left (depending on direction) line gets drawn if we have an item
        if( state & State_Item )
        {
            const QLineF line = reverseLayout ?
                QLineF( QPointF( rect.left(), center.y() ), QPointF( center.x() - expanderAdjust, center.y() ) ):
                QLineF( QPointF( center.x() + expanderAdjust, center.y() ), QPointF( rect.right(), center.y() ) );
            painter->drawLine( line );

        }

        // The bottom if we have a sibling
        if( state & State_Sibling )
        {
            const QLineF line( QPointF( center.x(), center.y() + expanderAdjust ), QPointF( center.x(), rect.bottom() ) );
            painter->drawLine( line );
        }

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawPushButtonLabelControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const auto buttonOption( qstyleoption_cast<const QStyleOptionButton*>( option ) );
        if( !buttonOption ) return true;

        // copy rect and palette
        const auto& rect( option->rect );
        const auto& palette( option->palette );

        // state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool sunken( state & (State_On | State_Sunken) );
        const bool mouseOver( enabled && (option->state & State_MouseOver) );
        const bool hasFocus( enabled && !mouseOver && (option->state & State_HasFocus) );
        const bool flat( buttonOption->features & QStyleOptionButton::Flat );

        // content
        const bool hasText( !buttonOption->text.isEmpty() );
        const bool hasIcon( (showIconsOnPushButtons() || flat || !hasText ) && !buttonOption->icon.isNull() );

        // contents
        auto contentsRect( rect );
        if( sunken && !flat ) contentsRect.translate( 1, 1 );

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
            auto arrowRect( contentsRect );
            arrowRect.setLeft( contentsRect.right() - Metrics::MenuButton_IndicatorWidth + 1 );
            arrowRect = centerRect( arrowRect, Metrics::MenuButton_IndicatorWidth, Metrics::MenuButton_IndicatorWidth );

            contentsRect.setRight( arrowRect.left() - Metrics::Button_ItemSpacing - 1  );
            contentsRect.adjust( Metrics::Button_MarginWidth, 0, 0, 0 );

            arrowRect = visualRect( option, arrowRect );

            // define color
            const auto arrowColor( _helper->arrowColor( palette, textRole ) );
            _helper->renderArrow( painter, arrowRect, arrowColor, ArrowDown );

        }

        // icon size
        QSize iconSize;
        if( hasIcon )
        {
            iconSize = buttonOption->iconSize;
            if( !iconSize.isValid() )
            {
                const int metric( pixelMetric( PM_SmallIconSize, option, widget ) );
                iconSize = QSize( metric, metric );
            }
        }

        // text size
        const int textFlags( _mnemonics->textFlags() | Qt::AlignCenter );
        const QSize textSize( option->fontMetrics.size( textFlags, buttonOption->text ) );

        // adjust text and icon rect based on options
        QRect iconRect;
        QRect textRect;

        if( hasText && !hasIcon ) textRect = contentsRect;
        else if( hasIcon && !hasText ) iconRect = contentsRect;
        else {

            const int contentsWidth( iconSize.width() + textSize.width() + Metrics::Button_ItemSpacing );
            iconRect = QRect( QPoint( contentsRect.left() + (contentsRect.width() - contentsWidth )/2, contentsRect.top() + (contentsRect.height() - iconSize.height())/2 ), iconSize );
            textRect = QRect( QPoint( iconRect.right() + Metrics::ToolButton_ItemSpacing + 1, contentsRect.top() + (contentsRect.height() - textSize.height())/2 ), textSize );

        }

        // handle right to left
        if( iconRect.isValid() ) iconRect = visualRect( option, iconRect );
        if( textRect.isValid() ) textRect = visualRect( option, textRect );

        // make sure there is enough room for icon
        if( iconRect.isValid() ) iconRect = centerRect( iconRect, iconSize );

        // render icon
        if( hasIcon && iconRect.isValid() ) {

            // icon state and mode
            const QIcon::State iconState( sunken ? QIcon::On : QIcon::Off );
            QIcon::Mode iconMode;
            if( !enabled ) iconMode = QIcon::Disabled;
            else if( !flat && hasFocus ) iconMode = QIcon::Selected;
            else if( mouseOver && flat ) iconMode = QIcon::Active;
            else iconMode = QIcon::Normal;

            const auto pixmap = buttonOption->icon.pixmap( iconSize, iconMode, iconState );
            drawItemPixmap( painter, iconRect, Qt::AlignCenter, pixmap );

        }

        // render text
        if( hasText && textRect.isValid() )
        { drawItemText( painter, textRect, textFlags, palette, enabled, buttonOption->text, textRole ); }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawToolButtonLabelControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const auto toolButtonOption( qstyleoption_cast<const QStyleOptionToolButton*>(option) );

        // copy rect and palette
        const auto& rect = option->rect;
        const auto& palette = option->palette;

        // state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool sunken( state & (State_On | State_Sunken) );
        const bool mouseOver( enabled && (option->state & State_MouseOver) );
        const bool flat( state & State_AutoRaise );

        // focus flag is set to match the background color in either renderButtonFrame or renderToolButtonFrame
        bool hasFocus( false );
        if( flat ) hasFocus = enabled && !mouseOver && (option->state & State_HasFocus);
        else hasFocus = enabled && !mouseOver && (option->state & (State_HasFocus|State_Sunken) );

        const bool hasArrow( toolButtonOption->features & QStyleOptionToolButton::Arrow );
        const bool hasIcon( !( hasArrow || toolButtonOption->icon.isNull() ) );
        const bool hasText( !toolButtonOption->text.isEmpty() );

        // contents
        auto contentsRect( rect );
        if( sunken && !flat ) contentsRect.translate( 1, 1 );

        // icon size
        const QSize iconSize( toolButtonOption->iconSize );

        // text size
        int textFlags( _mnemonics->textFlags() );
        const QSize textSize( option->fontMetrics.size( textFlags, toolButtonOption->text ) );

        // adjust text and icon rect based on options
        QRect iconRect;
        QRect textRect;

        if( hasText && ( !(hasArrow||hasIcon) || toolButtonOption->toolButtonStyle == Qt::ToolButtonTextOnly ) )
        {

            // text only
            textRect = contentsRect;
            textFlags |= Qt::AlignCenter;

        } else if( (hasArrow||hasIcon) && (!hasText || toolButtonOption->toolButtonStyle == Qt::ToolButtonIconOnly ) ) {

            // icon only
            iconRect = contentsRect;

        } else if( toolButtonOption->toolButtonStyle == Qt::ToolButtonTextUnderIcon ) {

            const int contentsHeight( iconSize.height() + textSize.height() + Metrics::ToolButton_ItemSpacing );
            iconRect = QRect( QPoint( contentsRect.left() + (contentsRect.width() - iconSize.width())/2, contentsRect.top() + (contentsRect.height() - contentsHeight)/2 ), iconSize );
            textRect = QRect( QPoint( contentsRect.left() + (contentsRect.width() - textSize.width())/2, iconRect.bottom() + Metrics::ToolButton_ItemSpacing + 1 ), textSize );
            textFlags |= Qt::AlignCenter;

        } else {

            const bool leftAlign( widget && widget->property( PropertyNames::toolButtonAlignment ).toInt() == Qt::AlignLeft );
            if( leftAlign ) {
                const int marginWidth( Metrics::Button_MarginWidth + Metrics::Frame_FrameWidth + 1 );
                iconRect = QRect( QPoint( contentsRect.left() + marginWidth, contentsRect.top() + (contentsRect.height() - iconSize.height())/2 ), iconSize );
            }
            else {

                const int contentsWidth( iconSize.width() + textSize.width() + Metrics::ToolButton_ItemSpacing );
                iconRect = QRect( QPoint( contentsRect.left() + (contentsRect.width() - contentsWidth )/2, contentsRect.top() + (contentsRect.height() - iconSize.height())/2 ), iconSize );

            }

            textRect = QRect( QPoint( iconRect.right() + Metrics::ToolButton_ItemSpacing + 1, contentsRect.top() + (contentsRect.height() - textSize.height())/2 ), textSize );

            // handle right to left layouts
            iconRect = visualRect( option, iconRect );
            textRect = visualRect( option, textRect );

            textFlags |= Qt::AlignLeft | Qt::AlignVCenter;

        }

        // make sure there is enough room for icon
        if( iconRect.isValid() ) iconRect = centerRect( iconRect, iconSize );

        // render arrow or icon
        if( hasArrow && iconRect.isValid() )
        {

            QStyleOptionToolButton copy( *toolButtonOption );
            copy.rect = iconRect;
            switch( toolButtonOption->arrowType )
            {
                case Qt::LeftArrow: drawPrimitive( PE_IndicatorArrowLeft, &copy, painter, widget ); break;
                case Qt::RightArrow: drawPrimitive( PE_IndicatorArrowRight, &copy, painter, widget ); break;
                case Qt::UpArrow: drawPrimitive( PE_IndicatorArrowUp, &copy, painter, widget ); break;
                case Qt::DownArrow: drawPrimitive( PE_IndicatorArrowDown, &copy, painter, widget ); break;
                default: break;
            }

        } else if( hasIcon && iconRect.isValid() ) {

            // icon state and mode
            const QIcon::State iconState( sunken ? QIcon::On : QIcon::Off );
            QIcon::Mode iconMode;
            if( !enabled ) iconMode = QIcon::Disabled;
            else if( (!flat && hasFocus) || (flat && (state & State_Sunken) && !mouseOver) ) iconMode = QIcon::Selected;
            else if( mouseOver && flat ) iconMode = QIcon::Active;
            else iconMode = QIcon::Normal;

            const QPixmap pixmap = toolButtonOption->icon.pixmap( iconSize, iconMode, iconState );
            drawItemPixmap( painter, iconRect, Qt::AlignCenter, pixmap );

        }

        // render text
        if( hasText && textRect.isValid() )
        {

            QPalette::ColorRole textRole( QPalette::ButtonText );
            if( flat ) textRole = ( ((hasFocus&&sunken) || (state & State_Sunken))&&!mouseOver) ? QPalette::HighlightedText: QPalette::WindowText;
            else if( hasFocus&&!mouseOver ) textRole = QPalette::HighlightedText;

            painter->setFont(toolButtonOption->font);
            drawItemText( painter, textRect, textFlags, palette, enabled, toolButtonOption->text, textRole );

        }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawCheckBoxLabelControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const auto buttonOption( qstyleoption_cast<const QStyleOptionButton*>(option) );
        if( !buttonOption ) return true;

        // copy palette and rect
        const auto& palette( option->palette );
        const auto& rect( option->rect );

        // store state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );

        // text alignment
        const bool reverseLayout( option->direction == Qt::RightToLeft );
        const int textFlags( _mnemonics->textFlags() | Qt::AlignVCenter | (reverseLayout ? Qt::AlignRight:Qt::AlignLeft ) );

        // text rect
        auto textRect( rect );

        // render icon
        if( !buttonOption->icon.isNull() )
        {
            const QIcon::Mode mode( enabled ? QIcon::Normal : QIcon::Disabled );
            const QPixmap pixmap( buttonOption->icon.pixmap(  buttonOption->iconSize, mode ) );
            drawItemPixmap( painter, rect, textFlags, pixmap );

            // adjust rect (copied from QCommonStyle)
            textRect.setLeft( textRect.left() + buttonOption->iconSize.width() + 4 );
            textRect = visualRect( option, textRect );

        }

        // render text
        if( !buttonOption->text.isEmpty() )
        {
            textRect = option->fontMetrics.boundingRect( textRect, textFlags, buttonOption->text );
            drawItemText( painter, textRect, textFlags, palette, enabled, buttonOption->text, QPalette::WindowText );

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
        const auto comboBoxOption( qstyleoption_cast<const QStyleOptionComboBox*>( option ) );
        if( !comboBoxOption ) return false;
        if( comboBoxOption->editable ) return false;

        // need to alter palette for focused buttons
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool sunken( state & (State_On | State_Sunken) );
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

        // translate painter for pressed down comboboxes
        if( sunken && !flat )
        { painter->translate( 1, 1 ); }

        #if QT_VERSION >= 0x050000
        if (const auto cb = qstyleoption_cast<const QStyleOptionComboBox *>(option))
        {
            auto editRect = proxy()->subControlRect(CC_ComboBox, cb, SC_ComboBoxEditField, widget);
            painter->save();
            painter->setClipRect(editRect);
            if (!cb->currentIcon.isNull()) {
                QIcon::Mode mode;

                if ((cb->state & QStyle::State_Selected) && (cb->state & QStyle::State_Active)) {
                    mode = QIcon::Selected;
                } else if (cb->state & QStyle::State_Enabled) {
                    mode = QIcon::Normal;
                } else {
                    mode = QIcon::Disabled;
                }

                QWindow *window = nullptr;
                if (widget && widget->window()) {
                    window = widget->window()->windowHandle();
#if BREEZE_HAVE_QTQUICK
                } else if (QQuickItem *quickItem = qobject_cast<QQuickItem *>(option->styleObject)) {
                    window = quickItem->window();
#endif
                }

                const auto pixmap = cb->currentIcon.pixmap(window, cb->iconSize, mode);
                auto iconRect(editRect);
                iconRect.setWidth(cb->iconSize.width() + 4);
                iconRect = alignedRect(cb->direction,
                                       Qt::AlignLeft | Qt::AlignVCenter,
                                       iconRect.size(), editRect);
                if (cb->editable)
                    painter->fillRect(iconRect, option->palette.brush(QPalette::Base));
                proxy()->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);

                if (cb->direction == Qt::RightToLeft)
                    editRect.translate(-4 - cb->iconSize.width(), 0);
                else
                    editRect.translate(cb->iconSize.width() + 4, 0);
            }
            if (!cb->currentText.isEmpty() && !cb->editable) {
                proxy()->drawItemText(painter, editRect.adjusted(1, 0, -1, 0),
                             visualAlignment(cb->direction, Qt::AlignLeft | Qt::AlignVCenter),
                             cb->palette, cb->state & State_Enabled, cb->currentText);
            }
            painter->restore();
        }
        #else
        // call base class method
        ParentStyleClass::drawControl( CE_ComboBoxLabel, option, painter, widget );
        #endif

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawMenuBarItemControl( const QStyleOption* option, QPainter* painter, const QWidget* widget) const
    {

        // cast option and check
        const auto menuItemOption = qstyleoption_cast<const QStyleOptionMenuItem*>( option );
        if( !menuItemOption ) return true;

        // copy rect and palette
        const auto& rect( option->rect );
        const auto& palette( option->palette );

        // store state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool selected( enabled && (state & State_Selected) );
        const bool sunken( enabled && (state & State_Sunken) );
        const bool useStrongFocus( StyleConfigData::menuItemDrawStrongFocus() );

        // render hover and focus
        if( useStrongFocus && ( selected || sunken ) )
        {

            QColor outlineColor;
            if( sunken ) outlineColor = _helper->focusColor( palette );
            else if( selected ) outlineColor = _helper->hoverColor( palette );
            _helper->renderFocusRect( painter, rect, outlineColor );

        }

        /*
        check if item as an icon, in which case only the icon should be rendered
        consistently with comment in QMenuBarPrivate::calcActionRects
        */
        if( !menuItemOption->icon.isNull() )
        {
            // icon size is forced to SmallIconSize
            const auto iconSize = pixelMetric(QStyle::PM_SmallIconSize, nullptr, widget);
            const auto iconRect = centerRect( rect, iconSize, iconSize );

            // decide icon mode and state
            QIcon::Mode iconMode;
            QIcon::State iconState;
            if( !enabled )
            {
                iconMode = QIcon::Disabled;
                iconState = QIcon::Off;

            } else {

                if( useStrongFocus && sunken ) iconMode = QIcon::Selected;
                else if( useStrongFocus && selected ) iconMode = QIcon::Active;
                else iconMode = QIcon::Normal;

                iconState = sunken ? QIcon::On : QIcon::Off;

            }

            const auto pixmap = menuItemOption->icon.pixmap( iconSize, iconMode, iconState );
            drawItemPixmap( painter, iconRect, Qt::AlignCenter, pixmap );

            // render outline
            if( !useStrongFocus && ( selected || sunken ) )
            {

                QColor outlineColor;
                if( sunken ) outlineColor = _helper->focusColor( palette );
                else if( selected ) outlineColor = _helper->hoverColor( palette );

                _helper->renderFocusLine( painter, iconRect, outlineColor );

            }

        } else {

            // get text rect
            const int textFlags( Qt::AlignCenter|_mnemonics->textFlags() );
            const auto textRect = option->fontMetrics.boundingRect( rect, textFlags, menuItemOption->text );

            // render text
            const QPalette::ColorRole role = (useStrongFocus && sunken ) ? QPalette::HighlightedText : QPalette::WindowText;
            drawItemText( painter, textRect, textFlags, palette, enabled, menuItemOption->text, role );

            // render outline
            if( !useStrongFocus && ( selected || sunken ) )
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
    bool Style::drawMenuItemControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const auto menuItemOption = qstyleoption_cast<const QStyleOptionMenuItem*>( option );
        if( !menuItemOption ) return true;
        if( menuItemOption->menuItemType == QStyleOptionMenuItem::EmptyArea ) return true;

        // copy rect and palette
        const auto& rect( option->rect );
        const auto& palette( option->palette );

        // deal with separators
        if( menuItemOption->menuItemType == QStyleOptionMenuItem::Separator )
        {

            // normal separator
            if( menuItemOption->text.isEmpty() && menuItemOption->icon.isNull() )
            {

                const auto color( _helper->separatorColor( palette ) );
                _helper->renderSeparator( painter, rect, color );
                return true;

            } else {

                /*
                 * separator can have a title and an icon
                 * in that case they are rendered as menu title buttons
                 */
                QStyleOptionToolButton copy( separatorMenuItemOption( menuItemOption, widget ) );
                renderMenuTitle( &copy, painter, widget );

                return true;

            }

        }

        // store state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool selected( enabled && (state & State_Selected) );
        const bool sunken( enabled && (state & (State_On|State_Sunken) ) );
        const bool reverseLayout( option->direction == Qt::RightToLeft );
        const bool useStrongFocus( StyleConfigData::menuItemDrawStrongFocus() );

        // render hover and focus
        if( useStrongFocus && ( selected || sunken ) )
        {

            const auto color = _helper->focusColor( palette );
            const auto outlineColor = _helper->focusOutlineColor( palette );

            Sides sides = nullptr;
            if( !menuItemOption->menuRect.isNull() )
            {
                if( rect.top() <= menuItemOption->menuRect.top() ) sides |= SideTop;
                if( rect.bottom() >= menuItemOption->menuRect.bottom() ) sides |= SideBottom;
                if( rect.left() <= menuItemOption->menuRect.left() ) sides |= SideLeft;
                if( rect.right() >= menuItemOption->menuRect.right() ) sides |= SideRight;
            }

            _helper->renderFocusRect( painter, rect, color, outlineColor, sides );

        }

        // get rect available for contents
        auto contentsRect( insideMargin( rect,  Metrics::MenuItem_MarginWidth, Metrics::MenuItem_MarginHeight ) );

        // define relevant rectangles
        // checkbox
        QRect checkBoxRect;
        if( menuItemOption->menuHasCheckableItems )
        {
            checkBoxRect = QRect( contentsRect.left(), contentsRect.top() + (contentsRect.height()-Metrics::CheckBox_Size)/2, Metrics::CheckBox_Size, Metrics::CheckBox_Size );
            contentsRect.setLeft( checkBoxRect.right() + Metrics::MenuItem_ItemSpacing + 1 );
        }

        // render checkbox indicator
        if( menuItemOption->checkType == QStyleOptionMenuItem::NonExclusive )
        {

            checkBoxRect = visualRect( option, checkBoxRect );

            // checkbox state

            if( useStrongFocus && ( selected || sunken ) )
            { _helper->renderCheckBoxBackground( painter, checkBoxRect, palette.color( QPalette::Window ), sunken ); }

            CheckBoxState state( menuItemOption->checked ? CheckOn : CheckOff );
            const bool active( menuItemOption->checked );
            const auto shadow( _helper->shadowColor( palette ) );
            const auto color( _helper->checkBoxIndicatorColor( palette, false, enabled && active ) );
            _helper->renderCheckBox( painter, checkBoxRect, color, shadow, sunken, state );

        } else if( menuItemOption->checkType == QStyleOptionMenuItem::Exclusive ) {

            checkBoxRect = visualRect( option, checkBoxRect );

            if( useStrongFocus && ( selected || sunken ) )
            { _helper->renderRadioButtonBackground( painter, checkBoxRect, palette.color( QPalette::Window ), sunken ); }

            const bool active( menuItemOption->checked );
            const auto shadow( _helper->shadowColor( palette ) );
            const auto color( _helper->checkBoxIndicatorColor( palette, false, enabled && active ) );
            _helper->renderRadioButton( painter, checkBoxRect, color, shadow, sunken, active ? RadioOn:RadioOff );

        }

        // icon
        int iconWidth = 0;
        const bool showIcon( showIconsInMenuItems() );
        if( showIcon ) iconWidth = isQtQuickControl( option, widget ) ? qMax( pixelMetric(PM_SmallIconSize, option, widget ), menuItemOption->maxIconWidth ) : menuItemOption->maxIconWidth;

        QRect iconRect;
        if( showIcon && iconWidth > 0 )
        {
            iconRect = QRect( contentsRect.left(), contentsRect.top() + (contentsRect.height()-iconWidth)/2, iconWidth, iconWidth );
            contentsRect.setLeft( iconRect.right() + Metrics::MenuItem_ItemSpacing + 1 );
            const QSize iconSize( pixelMetric( PM_SmallIconSize, option, widget ), pixelMetric( PM_SmallIconSize, option, widget ) );
            iconRect = centerRect( iconRect, iconSize );
        }

        if( showIcon && !menuItemOption->icon.isNull() )
        {

            iconRect = visualRect( option, iconRect );

            // icon mode
            QIcon::Mode mode;
            if( selected && !useStrongFocus)  mode = QIcon::Active;
            else if( selected ) mode = QIcon::Selected;
            else if( enabled ) mode = QIcon::Normal;
            else mode = QIcon::Disabled;

            // icon state
            const QIcon::State iconState( sunken ? QIcon::On:QIcon::Off );
            const QPixmap icon = menuItemOption->icon.pixmap( iconRect.size(), mode, iconState );
            painter->drawPixmap( iconRect, icon );

        }

        // arrow
        QRect arrowRect( contentsRect.right() - Metrics::MenuButton_IndicatorWidth + 1, contentsRect.top() + (contentsRect.height()-Metrics::MenuButton_IndicatorWidth)/2, Metrics::MenuButton_IndicatorWidth, Metrics::MenuButton_IndicatorWidth );
        contentsRect.setRight( arrowRect.left() -  Metrics::MenuItem_ItemSpacing - 1 );

        if( menuItemOption->menuItemType == QStyleOptionMenuItem::SubMenu )
        {

            // apply right-to-left layout
            arrowRect = visualRect( option, arrowRect );

            // arrow orientation
            const ArrowOrientation orientation( reverseLayout ? ArrowLeft:ArrowRight );

            // color
            QColor arrowColor;
            if( useStrongFocus && ( selected || sunken ) ) arrowColor = palette.color( QPalette::HighlightedText );
            else if( sunken ) arrowColor = _helper->focusColor( palette );
            else if( selected ) arrowColor = _helper->hoverColor( palette );
            else arrowColor = _helper->arrowColor( palette, QPalette::WindowText );

            // render
            _helper->renderArrow( painter, arrowRect, arrowColor, orientation );

        }


        // text
        auto textRect = contentsRect;
        if( !menuItemOption->text.isEmpty() )
        {

            // adjust textRect
            QString text = menuItemOption->text;
            textRect = centerRect( textRect, textRect.width(), option->fontMetrics.size( _mnemonics->textFlags(), text ).height() );
            textRect = visualRect( option, textRect );

            // set font
            painter->setFont( menuItemOption->font );

            // color role
            const QPalette::ColorRole role = (useStrongFocus && ( selected || sunken )) ? QPalette::HighlightedText : QPalette::WindowText;

            // locate accelerator and render
            const int tabPosition( text.indexOf( QLatin1Char( '\t' ) ) );
            if( tabPosition >= 0 )
            {

                const int textFlags( Qt::AlignVCenter | Qt::AlignRight );
                QString accelerator( text.mid( tabPosition + 1 ) );
                text = text.left( tabPosition );
                drawItemText( painter, textRect, textFlags, palette, enabled, accelerator, role );

            }

            // render text
            const int textFlags( Qt::AlignVCenter | (reverseLayout ? Qt::AlignRight : Qt::AlignLeft ) | _mnemonics->textFlags() );
            textRect = option->fontMetrics.boundingRect( textRect, textFlags, text );
            drawItemText( painter, textRect, textFlags, palette, enabled, text, role );

            // render hover and focus
            if( !useStrongFocus && ( selected || sunken ) )
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

        const auto progressBarOption( qstyleoption_cast<const QStyleOptionProgressBar*>( option ) );
        if( !progressBarOption ) return true;

        // render groove
        #if BREEZE_USE_KDE4
        QStyleOptionProgressBarV2 progressBarOption2 = *progressBarOption;
        #else
        QStyleOptionProgressBar progressBarOption2 = *progressBarOption;
        #endif
        progressBarOption2.rect = subElementRect( SE_ProgressBarGroove, progressBarOption, widget );
        drawControl( CE_ProgressBarGroove, &progressBarOption2, painter, widget );

        #if QT_VERSION >= 0x050000
        const QObject* styleObject( widget ? widget:progressBarOption->styleObject );
        #else
        const QObject* styleObject( widget );
        #endif

        // enable busy animations
        // need to check both widget and passed styleObject, used for QML
        if( styleObject && _animations->busyIndicatorEngine().enabled() )
        {

            #if QT_VERSION >= 0x050000
            // register QML object if defined
            if( !widget && progressBarOption->styleObject )
            { _animations->busyIndicatorEngine().registerWidget( progressBarOption->styleObject ); }
            #endif

            _animations->busyIndicatorEngine().setAnimated( styleObject, progressBarOption->maximum == 0 && progressBarOption->minimum == 0 );

        }

        // check if animated and pass to option
        if( _animations->busyIndicatorEngine().isAnimated( styleObject ) )
        { progressBarOption2.progress = _animations->busyIndicatorEngine().value(); }

        // render contents
        progressBarOption2.rect = subElementRect( SE_ProgressBarContents, progressBarOption, widget );
        drawControl( CE_ProgressBarContents, &progressBarOption2, painter, widget );

        // render text
        const bool textVisible( progressBarOption->textVisible );
        const bool busy( progressBarOption->minimum == 0 && progressBarOption->maximum == 0 );
        if( textVisible && !busy )
        {
            progressBarOption2.rect = subElementRect( SE_ProgressBarLabel, progressBarOption, widget );
            drawControl( CE_ProgressBarLabel, &progressBarOption2, painter, widget );
        }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawProgressBarContentsControl( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        const auto progressBarOption( qstyleoption_cast<const QStyleOptionProgressBar*>( option ) );
        if( !progressBarOption ) return true;

        // copy rect and palette
        auto rect( option->rect );
        const auto& palette( option->palette );

        // get direction
        #if BREEZE_USE_KDE4
        const auto progressBarOption2( qstyleoption_cast<const QStyleOptionProgressBarV2*>( option ) );
        const bool horizontal = !progressBarOption2 || progressBarOption2->orientation == Qt::Horizontal;
        const bool inverted( progressBarOption2 ? progressBarOption2->invertedAppearance : false );
        #else
        const bool horizontal( BreezePrivate::isProgressBarHorizontal( progressBarOption ) );
        const bool inverted( progressBarOption->invertedAppearance );
        #endif
        bool reverse = horizontal && option->direction == Qt::RightToLeft;
        if( inverted ) reverse = !reverse;

        // check if anything is to be drawn
        const bool busy( ( progressBarOption->minimum == 0 && progressBarOption->maximum == 0 ) );
        if( busy )
        {

            const qreal progress( _animations->busyIndicatorEngine().value() );

            const auto &first = palette.color( QPalette::Highlight );
            const auto second( KColorUtils::mix( palette.color( QPalette::Highlight ), palette.color( QPalette::Window ), 0.7 ) );
            _helper->renderProgressBarBusyContents( painter, rect, first, second, horizontal, reverse, progress );

        } else {

            const QRegion oldClipRegion( painter->clipRegion() );
            if( horizontal )
            {
                if( rect.width() < Metrics::ProgressBar_Thickness )
                {
                    painter->setClipRect( rect, Qt::IntersectClip );
                    if( reverse ) rect.setLeft( rect.left() - Metrics::ProgressBar_Thickness + rect.width() );
                    else rect.setWidth( Metrics::ProgressBar_Thickness );
                }

            } else {

                if( rect.height() < Metrics::ProgressBar_Thickness )
                {
                    painter->setClipRect( rect, Qt::IntersectClip );
                    if( reverse ) rect.setHeight( Metrics::ProgressBar_Thickness );
                    else rect.setTop( rect.top() - Metrics::ProgressBar_Thickness + rect.height() );
                }

            }

            auto contentsColor(
                option->state.testFlag( QStyle::State_Selected ) ?
                palette.color( QPalette::HighlightedText ) :
                palette.color( QPalette::Highlight ) );

            _helper->renderProgressBarContents( painter, rect, contentsColor );
            painter->setClipRegion( oldClipRegion );
        }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawProgressBarGrooveControl( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {
        const auto& palette( option->palette );
        const auto color( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.3 ) );
        _helper->renderProgressBarGroove( painter, option->rect, color );
        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawProgressBarLabelControl( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        // cast option and check
        const auto progressBarOption( qstyleoption_cast<const QStyleOptionProgressBar*>( option ) );
        if( !progressBarOption ) return true;

        // get direction and check
        #if BREEZE_USE_KDE4
        const auto progressBarOption2( qstyleoption_cast<const QStyleOptionProgressBarV2*>( option ) );
        const bool horizontal = !progressBarOption2 || progressBarOption2->orientation == Qt::Horizontal;
        #else
        const bool horizontal( BreezePrivate::isProgressBarHorizontal( progressBarOption ) );
        #endif
        if( !horizontal ) return true;

        // store rect and palette
        const auto& rect( option->rect );
        const auto& palette( option->palette );

        // store state and direction
        const State& state( option->state );
        const bool enabled( state & State_Enabled );

        // define text rect
        Qt::Alignment hAlign( ( progressBarOption->textAlignment == Qt::AlignLeft ) ? Qt::AlignHCenter : progressBarOption->textAlignment );
        drawItemText( painter, rect, Qt::AlignVCenter | hAlign, palette, enabled, progressBarOption->text, QPalette::WindowText );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawScrollBarSliderControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const auto sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if( !sliderOption ) return true;

        // copy rect and palette
        const auto& rect( option->rect );
        const auto& palette( option->palette );

        //try to understand if anywhere the widget is under mouse, not just the handle, use _animations in case of QWidget, option->styleObject in case of QML
        #if QT_VERSION >= 0x050000
        bool widgetMouseOver( ( option->state & State_MouseOver ) );
        if( widget ) widgetMouseOver = _animations->scrollBarEngine().isHovered( widget, QStyle::SC_ScrollBarGroove );
        else if( option->styleObject ) widgetMouseOver = option->styleObject->property("hover").toBool();
        #else
        const bool widgetMouseOver( _animations->scrollBarEngine().isHovered( widget, QStyle::SC_ScrollBarGroove ) );
        #endif

        qreal grooveAnimationOpacity( _animations->scrollBarEngine().opacity( widget, QStyle::SC_ScrollBarGroove ) );
        if( grooveAnimationOpacity == AnimationData::OpacityInvalid ) grooveAnimationOpacity = (widgetMouseOver ? 1 : 0);
        const qreal handleSize = StyleConfigData::animationsEnabled() ? ((Metrics::ScrollBar_SliderWidth / 2.0) * (1 - grooveAnimationOpacity) + Metrics::ScrollBar_SliderWidth  * grooveAnimationOpacity) : (int)Metrics::ScrollBar_SliderWidth;

        // define handle rect
        QRect handleRect;
        const State& state( option->state );
        const bool horizontal( state & State_Horizontal );
        if( horizontal ) handleRect = centerRect( rect, rect.width(), handleSize );
        else handleRect = centerRect( rect, handleSize, rect.height() );

        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && ( state & State_MouseOver ) );

        // check focus from relevant parent
        const QWidget* parent( scrollBarParent( widget ) );
        const bool hasFocus( enabled && ( (widget && widget->hasFocus()) || (parent && parent->hasFocus()) ) );

        // enable animation state
        const bool handleActive( sliderOption->activeSubControls & SC_ScrollBarSlider );
        _animations->scrollBarEngine().updateState( widget, AnimationFocus, hasFocus );

        _animations->scrollBarEngine().updateState( widget, AnimationHover, mouseOver && handleActive );

        const auto mode( _animations->scrollBarEngine().animationMode( widget, SC_ScrollBarSlider ) );
        const qreal opacity( _animations->scrollBarEngine().opacity( widget, SC_ScrollBarSlider ) );
        auto color = _helper->scrollBarHandleColor( palette, mouseOver, hasFocus, opacity, mode );
        if (StyleConfigData::animationsEnabled()) {
            color.setAlphaF(color.alphaF() * (0.7 + 0.3 * grooveAnimationOpacity));
        }

        _helper->renderScrollBarHandle( painter, handleRect, color );
        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawScrollBarAddLineControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // do nothing if no buttons are defined
        if( _addLineButtons == NoButton ) return true;

        // cast option and check
        const auto sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if( !sliderOption ) return true;

        const State& state( option->state );
        const bool horizontal( state & State_Horizontal );
        const bool reverseLayout( option->direction == Qt::RightToLeft );

        // adjust rect, based on number of buttons to be drawn
        const auto rect( scrollBarInternalSubControlRect( sliderOption, SC_ScrollBarAddLine ) );

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
        const auto sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if( !sliderOption ) return true;

        const State& state( option->state );
        const bool horizontal( state & State_Horizontal );
        const bool reverseLayout( option->direction == Qt::RightToLeft );

        // colors
        const auto& palette( option->palette );
        const auto &background = palette.color( QPalette::Window );

        // adjust rect, based on number of buttons to be drawn
        const auto rect( scrollBarInternalSubControlRect( sliderOption, SC_ScrollBarSubLine ) );

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
    bool Style::drawShapedFrameControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        #if BREEZE_USE_KDE4
        const auto frameOpt = qstyleoption_cast<const QStyleOptionFrameV3*>( option );
        #else
        const auto frameOpt = qstyleoption_cast<const QStyleOptionFrame*>( option );
        #endif
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

                const auto& rect( option->rect );
                const auto color( _helper->separatorColor( option->palette ) );
                const bool isVertical( frameOpt->frameShape == QFrame::VLine );
                _helper->renderSeparator( painter, rect, color, isVertical );
                return true;
            }

            case QFrame::StyledPanel:
            {

                if( isQtQuickControl( option, widget ) )
                {

                    // ComboBox popup frame
                    drawFrameMenuPrimitive( option, painter, widget );
                    return true;

                } else break;
            }

            default: break;

        }

        return false;

    }

    //___________________________________________________________________________________
    bool Style::drawRubberBandControl( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        const auto& palette( option->palette );
        const auto rect( option->rect );

        auto color = palette.color( QPalette::Highlight );
        painter->setPen( KColorUtils::mix( color, palette.color( QPalette::Active, QPalette::WindowText ) ) );
        color.setAlpha( 50 );
        painter->setBrush( color );
        painter->setClipRegion( rect );
        painter->drawRect( rect.adjusted( 0, 0, -1, -1 ) );
        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawHeaderSectionControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        const auto& rect( option->rect );
        const auto& palette( option->palette );
        const auto& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && ( state & State_MouseOver ) );
        const bool sunken( enabled && ( state & (State_On|State_Sunken) ) );

        const auto headerOption( qstyleoption_cast<const QStyleOptionHeader*>( option ) );
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
        const auto &normal = palette.color( QPalette::Button );
        const auto focus( KColorUtils::mix( normal, _helper->focusColor( palette ), 0.2 ) );
        const auto hover( KColorUtils::mix( normal, _helper->hoverColor( palette ), 0.2 ) );

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
        const auto& rect( option->rect );
        auto palette( option->palette );

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
        ParentStyleClass::drawControl( CE_TabBarTabLabel, option, painter, widget );

        // store rect and palette
        const auto& rect( option->rect );
        const auto& palette( option->palette );

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
        const auto tabOption( qstyleoption_cast<const QStyleOptionTab*>(option) );
        if( !tabOption || tabOption->text.isEmpty() ) return true;

        // tab option rect
        const bool verticalTabs( isVerticalTab( tabOption ) );
        const int textFlags( Qt::AlignCenter | _mnemonics->textFlags() );

        // text rect
        auto textRect( subElementRect(SE_TabBarTabText, option, widget) );

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
        textRect = option->fontMetrics.boundingRect( textRect, textFlags, tabOption->text );

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

        const auto tabOption( qstyleoption_cast<const QStyleOptionTab*>( option ) );
        if( !tabOption ) return true;

        // palette and state
        const auto& palette( option->palette );
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool selected( state & State_Selected );
        const bool mouseOver( enabled && !selected && ( state & State_MouseOver ) );

        // check if tab is being dragged
        const bool isDragged( widget && selected && painter->device() != widget );
        const bool isLocked( widget && _tabBarData->isLocked( widget ) );

        // store rect
        auto rect( option->rect );

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
        const bool isQtQuickControl( this->isQtQuickControl( option, widget ) );
        bool isFirst( isSingle || position == QStyleOptionTab::Beginning );
        bool isLast( isSingle || position == QStyleOptionTab::End );
        bool isLeftOfSelected( !isLocked && tabOption->selectedPosition == QStyleOptionTab::NextIsSelected );
        bool isRightOfSelected( !isLocked && tabOption->selectedPosition == QStyleOptionTab::PreviousIsSelected );

        // true if widget is aligned to the frame
        // need to check for 'isRightOfSelected' because for some reason the isFirst flag is set when active tab is being moved
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

        // overlap
        // for QtQuickControls, ovelap is already accounted of in the option. Unlike in the qwidget case
        const int overlap( isQtQuickControl ? 0:Metrics::TabBar_TabOverlap );

        // adjust rect and define corners based on tabbar orientation
        Corners corners;
        switch( tabOption->shape )
        {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
            if( selected )
            {

                corners = CornerTopLeft|CornerTopRight;
                rect.adjust( 0, 0, 0, 1 );

            } else {

                rect.adjust( 0, 0, 0, -1 );
                if( isFirst ) corners |= CornerTopLeft;
                if( isLast ) corners |= CornerTopRight;
                if( isRightOfSelected ) rect.adjust( -Metrics::Frame_FrameRadius, 0, 0, 0 );
                if( isLeftOfSelected ) rect.adjust( 0, 0, Metrics::Frame_FrameRadius, 0 );
                else if( !isLast ) rect.adjust( 0, 0, overlap, 0 );

            }
            break;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            if( selected )
            {

                corners = CornerBottomLeft|CornerBottomRight;
                rect.adjust( 0, - 1, 0, 0 );

            } else {

                rect.adjust( 0, 1, 0, 0 );
                if( isFirst ) corners |= CornerBottomLeft;
                if( isLast ) corners |= CornerBottomRight;
                if( isRightOfSelected ) rect.adjust( -Metrics::Frame_FrameRadius, 0, 0, 0 );
                if( isLeftOfSelected ) rect.adjust( 0, 0, Metrics::Frame_FrameRadius, 0 );
                else if( !isLast ) rect.adjust( 0, 0, overlap, 0 );

            }
            break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            if( selected )
            {
                corners = CornerTopLeft|CornerBottomLeft;
                rect.adjust( 0, 0, 1, 0 );

            } else {

                rect.adjust( 0, 0, -1, 0 );
                if( isFirst ) corners |= CornerTopLeft;
                if( isLast ) corners |= CornerBottomLeft;
                if( isRightOfSelected ) rect.adjust( 0, -Metrics::Frame_FrameRadius, 0, 0 );
                if( isLeftOfSelected ) rect.adjust( 0, 0, 0, Metrics::Frame_FrameRadius );
                else if( !isLast ) rect.adjust( 0, 0, 0, overlap );

            }
            break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            if( selected )
            {

                corners = CornerTopRight|CornerBottomRight;
                rect.adjust( -1, 0, 0, 0 );

            } else {

                rect.adjust( 1, 0, 0, 0 );
                if( isFirst ) corners |= CornerTopRight;
                if( isLast ) corners |= CornerBottomRight;
                if( isRightOfSelected ) rect.adjust( 0, -Metrics::Frame_FrameRadius, 0, 0 );
                if( isLeftOfSelected ) rect.adjust( 0, 0, 0, Metrics::Frame_FrameRadius );
                else if( !isLast ) rect.adjust( 0, 0, 0, overlap );

            }
            break;

            default: break;
        }

        // color
        QColor color;
        if( selected )
        {

            #if QT_VERSION >= 0x050000
            bool documentMode = tabOption->documentMode;
            #else
            bool documentMode = false;
            if( const auto tabOptionV3 = qstyleoption_cast<const QStyleOptionTabV3*>( option ) )
            { documentMode = tabOptionV3->documentMode; }
            #endif

            // flag passed to QStyleOptionTab is unfortunately not reliable enough
            // also need to check on parent widget
            const auto tabWidget = ( widget && widget->parentWidget() ) ? qobject_cast<const QTabWidget *>( widget->parentWidget() ) : nullptr;
            documentMode |= ( tabWidget ? tabWidget->documentMode() : true );

            color = (documentMode&&!isQtQuickControl&&!hasAlteredBackground(widget)) ? palette.color( QPalette::Window ) : _helper->frameBackgroundColor( palette );

        } else {

            const auto normal( _helper->alphaColor( palette.color( QPalette::Shadow ), 0.2 ) );
            const auto hover( _helper->alphaColor( _helper->hoverColor( palette ), 0.2 ) );
            if( animated ) color = KColorUtils::mix( normal, hover, opacity );
            else if( mouseOver ) color = hover;
            else color = normal;

        }

        // outline
        const auto outline( selected ? _helper->alphaColor( palette.color( QPalette::WindowText ), 0.25 ) : QColor() );

        // render
        if( selected )
        {

            QRegion oldRegion( painter->clipRegion() );
            painter->setClipRect( option->rect, Qt::IntersectClip );
            _helper->renderTabBarTab( painter, rect, color, outline, corners );
            painter->setClipRegion( oldRegion );

        } else {

            _helper->renderTabBarTab( painter, rect, color, outline, corners );

        }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawToolBoxTabLabelControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // rendering is similar to drawPushButtonLabelControl
        // cast option and check
        const auto toolBoxOption( qstyleoption_cast<const QStyleOptionToolBox *>( option ) );
        if( !toolBoxOption ) return true;

        // copy palette
        const auto& palette( option->palette );

        const State& state( option->state );
        const bool enabled( state & State_Enabled );

        // text alignment
        const int textFlags( _mnemonics->textFlags() | Qt::AlignCenter );

        // contents rect
        const auto rect( subElementRect( SE_ToolBoxTabContents, option, widget ) );

        // store icon size
        const int iconSize( pixelMetric( QStyle::PM_SmallIconSize, option, widget ) );

        // find contents size and rect
        auto contentsRect( rect );
        QSize contentsSize;
        if( !toolBoxOption->text.isEmpty() )
        {
            contentsSize = option->fontMetrics.size( _mnemonics->textFlags(), toolBoxOption->text );
            if( !toolBoxOption->icon.isNull() ) contentsSize.rwidth() += Metrics::ToolBox_TabItemSpacing;
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
                contentsRect.setLeft( iconRect.right() + Metrics::ToolBox_TabItemSpacing + 1 );

            }

            iconRect = visualRect( option, iconRect );
            const QIcon::Mode mode( enabled ? QIcon::Normal : QIcon::Disabled );
            const QPixmap pixmap( toolBoxOption->icon.pixmap( iconSize, mode ) );
            drawItemPixmap( painter, iconRect, textFlags, pixmap );

        }

        // render text
        if( !toolBoxOption->text.isEmpty() )
        {
            contentsRect = visualRect( option, contentsRect );
            drawItemText( painter, contentsRect, textFlags, palette, enabled, toolBoxOption->text, QPalette::WindowText );
        }

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawToolBoxTabShapeControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const auto toolBoxOption( qstyleoption_cast<const QStyleOptionToolBox *>( option ) );
        if( !toolBoxOption ) return true;

        // copy rect and palette
        const auto& rect( option->rect );
        const auto tabRect( toolBoxTabContentsRect( option, widget ) );

        /*
         * important: option returns the wrong palette.
         * we use the widget palette instead, when set
         */
        const auto& palette( widget ? widget->palette() : option->palette );

        // store flags
        const State& flags( option->state );
        const bool enabled( flags&State_Enabled );
        const bool selected( flags&State_Selected );
        const bool mouseOver( enabled && !selected && ( flags&State_MouseOver ) );

        // update animation state
        /*
         * the proper widget ( the toolbox tab ) is not passed as argument by Qt.
         * What is passed is the toolbox directly. To implement animations properly,
         *the painter->device() is used instead
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
        const auto dockWidgetOption = qstyleoption_cast<const QStyleOptionDockWidget*>( option );
        if( !dockWidgetOption ) return true;

        const auto& palette( option->palette );
        const auto& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool reverseLayout( option->direction == Qt::RightToLeft );

        // cast to v2 to check vertical bar
        #if BREEZE_USE_KDE4
        const auto v2 = qstyleoption_cast<const QStyleOptionDockWidgetV2*>( option );
        const bool verticalTitleBar( v2 ? v2->verticalTitleBar : false );
        #else
        const bool verticalTitleBar( dockWidgetOption->verticalTitleBar );
        #endif

        const auto buttonRect( subElementRect( dockWidgetOption->floatable ? SE_DockWidgetFloatButton : SE_DockWidgetCloseButton, option, widget ) );

        // get rectangle and adjust to properly accounts for buttons
        auto rect( insideMargin( dockWidgetOption->rect, Metrics::Frame_FrameWidth ) );
        if( verticalTitleBar )
        {

            if( buttonRect.isValid() ) rect.setTop( buttonRect.bottom() + 1 );

        } else if( reverseLayout ) {

            if( buttonRect.isValid() ) rect.setLeft( buttonRect.right() + 1 );
            rect.adjust( 0, 0, -4, 0 );

        } else {

            if( buttonRect.isValid() ) rect.setRight( buttonRect.left() - 1 );
            rect.adjust( 4, 0, 0, 0 );

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
        ParentStyleClass::drawComplexControl( CC_GroupBox, option, painter, widget );

        // cast option and check
        const auto groupBoxOption = qstyleoption_cast<const QStyleOptionGroupBox*>( option );
        if( !groupBoxOption ) return true;

        // do nothing if either label is not selected or groupbox is empty
        if( !(option->subControls & QStyle::SC_GroupBoxLabel) || groupBoxOption->text.isEmpty() )
        { return true; }

        // store palette and rect
        const auto& palette( option->palette );

        // check focus state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool hasFocus( enabled && (option->state & State_HasFocus) );
        if( !hasFocus ) return true;

        // alignment
        const int textFlags( groupBoxOption->textAlignment | _mnemonics->textFlags() );

        // update animation state
        _animations->widgetStateEngine().updateState( widget, AnimationFocus, hasFocus );
        const bool isFocusAnimated( _animations->widgetStateEngine().isAnimated( widget, AnimationFocus ) );
        const qreal opacity( _animations->widgetStateEngine().opacity( widget, AnimationFocus ) );

        // get relevant rect
        auto textRect = subControlRect( CC_GroupBox, option, SC_GroupBoxLabel, widget );
        textRect = option->fontMetrics.boundingRect( textRect, textFlags, groupBoxOption->text );

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
        const auto toolButtonOption( qstyleoption_cast<const QStyleOptionToolButton*>( option ) );
        if( !toolButtonOption ) return true;

        // need to alter palette for focused buttons
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && (option->state & State_MouseOver) );
        const bool hasFocus( enabled && (option->state & State_HasFocus) );
        const bool sunken( state & (State_On | State_Sunken) );
        const bool flat( state & State_AutoRaise );

        // update animation state
        // mouse over takes precedence over focus
        _animations->widgetStateEngine().updateState( widget, AnimationHover, mouseOver );
        _animations->widgetStateEngine().updateState( widget, AnimationFocus, hasFocus && !mouseOver );

        // detect buttons in tabbar, for which special rendering is needed
        const bool inTabBar( widget && qobject_cast<const QTabBar*>( widget->parentWidget() ) );
        const bool isMenuTitle( this->isMenuTitle( widget ) );
        if( isMenuTitle )
        {
            // copy option to adjust state, and set font as not-bold
            QStyleOptionToolButton copy( *toolButtonOption );
            copy.font.setBold( false );
            copy.state = State_Enabled;

            // render
            renderMenuTitle( &copy, painter, widget );
            return true;
        }

        // copy option and alter palette
        QStyleOptionToolButton copy( *toolButtonOption );

        const bool hasPopupMenu( toolButtonOption->features & QStyleOptionToolButton::MenuButtonPopup );
        const bool hasInlineIndicator(
            toolButtonOption->features&QStyleOptionToolButton::HasMenu
            && toolButtonOption->features&QStyleOptionToolButton::PopupDelay
            && !hasPopupMenu );

        const auto buttonRect( subControlRect( CC_ToolButton, option, SC_ToolButton, widget ) );
        const auto menuRect( subControlRect( CC_ToolButton, option, SC_ToolButtonMenu, widget ) );

        // frame
        if( toolButtonOption->subControls & SC_ToolButton )
        {
            if( !flat ) copy.rect = buttonRect;
            if( inTabBar ) drawTabBarPanelButtonToolPrimitive( &copy, painter, widget );
            else drawPrimitive( PE_PanelButtonTool, &copy, painter, widget);
        }

        // arrow
        if( hasPopupMenu )
        {

            copy.rect = menuRect;
            if( !flat ) drawPrimitive( PE_IndicatorButtonDropDown, &copy, painter, widget );

            if( sunken && !flat ) copy.rect.translate( 1, 1 );
            drawPrimitive( PE_IndicatorArrowDown, &copy, painter, widget );

        } else if( hasInlineIndicator ) {

            copy.rect = menuRect;

            if( sunken && !flat ) copy.rect.translate( 1, 1 );
            drawPrimitive( PE_IndicatorArrowDown, &copy, painter, widget );

        }

        // contents
        {

            // restore state
            copy.state = state;

            // define contents rect
            auto contentsRect( buttonRect );

            // detect dock widget title button
            // for dockwidget title buttons, do not take out margins, so that icon do not get scaled down
            const bool isDockWidgetTitleButton( widget && widget->inherits( "QDockWidgetTitleButton" ) );
            if( isDockWidgetTitleButton )
            {

                // cast to abstract button
                // adjust state to have correct icon rendered
                const auto button( qobject_cast<const QAbstractButton*>( widget ) );
                if( button->isChecked() || button->isDown() ) copy.state |= State_On;

            } else if( !inTabBar && hasInlineIndicator ) {

                const int marginWidth( flat ? Metrics::ToolButton_MarginWidth : Metrics::Button_MarginWidth + Metrics::Frame_FrameWidth );
                contentsRect = insideMargin( contentsRect, marginWidth, 0 );
                contentsRect.setRight( contentsRect.right() - Metrics::ToolButton_InlineIndicatorWidth );
                contentsRect = visualRect( option, contentsRect );

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
        const auto comboBoxOption( qstyleoption_cast<const QStyleOptionComboBox*>( option ) );
        if( !comboBoxOption ) return true;

        // rect and palette
        const auto& rect( option->rect );
        const auto& palette( option->palette );

        // state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && ( state & State_MouseOver ) );
        const bool hasFocus( enabled && ( state & (State_HasFocus | State_Sunken ) ) );
        const bool editable( comboBoxOption->editable );
        const bool sunken( state & (State_On|State_Sunken) );
        bool flat( !comboBoxOption->frame );

        // frame
        if( option->subControls & SC_ComboBoxFrame )
        {

            if( editable )
            {

                flat |= ( rect.height() <= 2*Metrics::Frame_FrameWidth + Metrics::MenuButton_IndicatorWidth );
                if( flat )
                {

                    const auto &background = palette.color( QPalette::Base );

                    painter->setBrush( background );
                    painter->setPen( Qt::NoPen );
                    painter->drawRect( rect );

                } else {

                    drawPrimitive( PE_FrameLineEdit, option, painter, widget );

                }

            } else {

                // update animation state
                // hover takes precedence over focus
                _animations->inputWidgetEngine().updateState( widget, AnimationHover, mouseOver );
                _animations->inputWidgetEngine().updateState( widget, AnimationFocus, hasFocus && !mouseOver );
                const AnimationMode mode( _animations->inputWidgetEngine().buttonAnimationMode( widget ) );
                const qreal opacity( _animations->inputWidgetEngine().buttonOpacity( widget ) );

                if( flat ) {

                    // define colors and render
                    const auto color( _helper->toolButtonColor( palette, mouseOver, hasFocus, sunken, opacity, mode ) );
                    _helper->renderToolButtonFrame( painter, rect, color, sunken );

                } else {

                    // define colors
                    const auto shadow( _helper->shadowColor( palette ) );
                    const auto outline( _helper->buttonOutlineColor( palette, mouseOver, hasFocus, opacity, mode ) );
                    const auto background( _helper->buttonBackgroundColor( palette, mouseOver, hasFocus, false, opacity, mode ) );

                    // render
                    _helper->renderButtonFrame( painter, rect, background, outline, shadow, hasFocus, sunken );

                }

            }

        }

        // arrow
        if( option->subControls & SC_ComboBoxArrow )
        {

            // detect empty comboboxes
            const auto comboBox = qobject_cast<const QComboBox*>( widget );
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
                    const auto normal( _helper->arrowColor( palette, QPalette::WindowText ) );
                    const auto hover( _helper->hoverColor( palette ) );

                    if( animated )
                    {
                        arrowColor = KColorUtils::mix( normal, hover, opacity );

                    } else if( subControlHover ) {

                        arrowColor = hover;

                    } else arrowColor = normal;

                }

            } else if( flat )  {

                if( empty || !enabled ) arrowColor = _helper->arrowColor( palette, QPalette::Disabled, QPalette::WindowText );
                else if( hasFocus && !mouseOver && sunken ) arrowColor = palette.color( QPalette::HighlightedText );
                else arrowColor = _helper->arrowColor( palette, QPalette::WindowText );

            } else if( empty || !enabled ) arrowColor = _helper->arrowColor( palette, QPalette::Disabled, QPalette::ButtonText );
            else if( hasFocus && !mouseOver ) arrowColor = palette.color( QPalette::HighlightedText );
            else arrowColor = _helper->arrowColor( palette, QPalette::ButtonText );

            // arrow rect
            auto arrowRect( subControlRect( CC_ComboBox, option, SC_ComboBoxArrow, widget ) );

            // translate for non editable, non flat, sunken comboboxes
            if( sunken && !flat && !editable ) arrowRect.translate( 1, 1 );

            // render
            _helper->renderArrow( painter, arrowRect, arrowColor, ArrowDown );

        }

        return true;

    }

    //______________________________________________________________
    bool Style::drawSpinBoxComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        const auto spinBoxOption( qstyleoption_cast<const QStyleOptionSpinBox*>( option ) );
        if( !spinBoxOption ) return true;

        // store palette and rect
        const auto& palette( option->palette );
        const auto& rect( option->rect );


        if( option->subControls & SC_SpinBoxFrame )
        {

            // detect flat spinboxes
            bool flat( !spinBoxOption->frame );
            flat |= ( rect.height() < 2*Metrics::Frame_FrameWidth + Metrics::SpinBox_ArrowButtonWidth );
            if( flat )
            {

                const auto &background = palette.color( QPalette::Base );

                painter->setBrush( background );
                painter->setPen( Qt::NoPen );
                painter->drawRect( rect );

            } else {

                drawPrimitive( PE_FrameLineEdit, option, painter, widget );

            }

        }

        if( option->subControls & SC_SpinBoxUp ) renderSpinBoxArrow( SC_SpinBoxUp, spinBoxOption, painter, widget );
        if( option->subControls & SC_SpinBoxDown ) renderSpinBoxArrow( SC_SpinBoxDown, spinBoxOption, painter, widget );

        return true;

    }

    //______________________________________________________________
    bool Style::drawSliderComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const auto sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if( !sliderOption ) return true;

        // copy rect and palette
        const auto& rect( option->rect );
        const auto& palette( option->palette );

        // copy state
        const State& state( option->state );
        const bool enabled( state & State_Enabled );
        const bool mouseOver( enabled && ( state & State_MouseOver ) );
        const bool hasFocus( enabled && ( state & State_HasFocus ) );

        // direction
        const bool horizontal( sliderOption->orientation == Qt::Horizontal );

        // tickmarks
        if( StyleConfigData::sliderDrawTickMarks() && ( sliderOption->subControls & SC_SliderTickmarks ) )
        {
            const bool upsideDown( sliderOption->upsideDown );
            const int tickPosition( sliderOption->tickPosition );
            const int available( pixelMetric( PM_SliderSpaceAvailable, option, widget ) );
            int interval = sliderOption->tickInterval;
            if( interval < 1 ) interval = sliderOption->pageStep;
            if( interval >= 1 )
            {
                const int fudge( pixelMetric( PM_SliderLength, option, widget ) / 2 );
                int current( sliderOption->minimum );

                // store tick lines
                const auto grooveRect( subControlRect( CC_Slider, sliderOption, SC_SliderGroove, widget ) );
                QList<QLine> tickLines;
                if( horizontal )
                {

                    if( tickPosition & QSlider::TicksAbove ) tickLines.append( QLine( rect.left(), grooveRect.top() - Metrics::Slider_TickMarginWidth, rect.left(), grooveRect.top() - Metrics::Slider_TickMarginWidth - Metrics::Slider_TickLength ) );
                    if( tickPosition & QSlider::TicksBelow ) tickLines.append( QLine( rect.left(), grooveRect.bottom() + Metrics::Slider_TickMarginWidth, rect.left(), grooveRect.bottom() + Metrics::Slider_TickMarginWidth + Metrics::Slider_TickLength ) );

                } else {

                    if( tickPosition & QSlider::TicksAbove ) tickLines.append( QLine( grooveRect.left() - Metrics::Slider_TickMarginWidth, rect.top(), grooveRect.left() - Metrics::Slider_TickMarginWidth - Metrics::Slider_TickLength, rect.top() ) );
                    if( tickPosition & QSlider::TicksBelow ) tickLines.append( QLine( grooveRect.right() + Metrics::Slider_TickMarginWidth, rect.top(), grooveRect.right() + Metrics::Slider_TickMarginWidth + Metrics::Slider_TickLength, rect.top() ) );

                }

                // colors
                const auto base( _helper->separatorColor( palette ) );
                const auto &highlight = palette.color( QPalette::Highlight );

                while( current <= sliderOption->maximum )
                {

                    // adjust color
                    const auto color( (enabled && current <= sliderOption->sliderPosition) ? highlight:base );
                    painter->setPen( color );

                    // calculate positions and draw lines
                    int position( sliderPositionFromValue( sliderOption->minimum, sliderOption->maximum, current, available ) + fudge );
                    foreach( const QLine& tickLine, tickLines )
                    {
                        if( horizontal ) painter->drawLine( tickLine.translated( upsideDown ? (rect.width() - position) : position, 0 ) );
                        else painter->drawLine( tickLine.translated( 0, upsideDown ? (rect.height() - position):position ) );
                    }

                    // go to next position
                    current += interval;

                }
            }
        }

        // groove
        if( sliderOption->subControls & SC_SliderGroove )
        {
            // retrieve groove rect
            auto grooveRect( subControlRect( CC_Slider, sliderOption, SC_SliderGroove, widget ) );

            // base color
            const auto grooveColor( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.3 ) );

            if( !enabled ) _helper->renderSliderGroove( painter, grooveRect, grooveColor );
            else {

                const bool upsideDown( sliderOption->upsideDown );

                // handle rect
                auto handleRect( subControlRect( CC_Slider, sliderOption, SC_SliderHandle, widget ) );

                // highlight color
                const auto &highlight = palette.color( QPalette::Highlight );

                if( sliderOption->orientation == Qt::Horizontal )
                {

                    auto leftRect( grooveRect );
                    leftRect.setRight( handleRect.right() - Metrics::Slider_ControlThickness/2 );
                    _helper->renderSliderGroove( painter, leftRect, upsideDown ? grooveColor:highlight );

                    auto rightRect( grooveRect );
                    rightRect.setLeft( handleRect.left() + Metrics::Slider_ControlThickness/2 );
                    _helper->renderSliderGroove( painter, rightRect, upsideDown ? highlight:grooveColor );

                } else {

                    auto topRect( grooveRect );
                    topRect.setBottom( handleRect.bottom() - Metrics::Slider_ControlThickness/2 );
                    _helper->renderSliderGroove( painter, topRect, upsideDown ? grooveColor:highlight );

                    auto bottomRect( grooveRect );
                    bottomRect.setTop( handleRect.top() + Metrics::Slider_ControlThickness/2 );
                    _helper->renderSliderGroove( painter, bottomRect, upsideDown ? highlight:grooveColor );

                }

            }

        }

        // handle
        if( sliderOption->subControls & SC_SliderHandle )
        {

            // get rect and center
            auto handleRect( subControlRect( CC_Slider, sliderOption, SC_SliderHandle, widget ) );

            // handle state
            const bool handleActive( sliderOption->activeSubControls & SC_SliderHandle );
            const bool sunken( state & (State_On|State_Sunken) );

            // animation state
            _animations->widgetStateEngine().updateState( widget, AnimationHover, handleActive && mouseOver );
            _animations->widgetStateEngine().updateState( widget, AnimationFocus, hasFocus );
            const AnimationMode mode( _animations->widgetStateEngine().buttonAnimationMode( widget ) );
            const qreal opacity( _animations->widgetStateEngine().buttonOpacity( widget ) );

            // define colors
            const auto &background = palette.color( QPalette::Button );
            const auto outline( _helper->sliderOutlineColor( palette, handleActive && mouseOver, hasFocus, opacity, mode ) );
            const auto shadow( _helper->shadowColor( palette ) );

            // render
            _helper->renderSliderHandle( painter, handleRect, background, outline, shadow, sunken );

        }

        return true;
    }

    //______________________________________________________________
    bool Style::drawDialComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const auto sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if( !sliderOption ) return true;

        const auto& palette( option->palette );
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
            auto grooveRect( subControlRect( CC_Dial, sliderOption, SC_SliderGroove, widget ) );

            // groove
            const auto grooveColor( KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::WindowText ), 0.3 ) );

            // angles
            const qreal first( dialAngle( sliderOption, sliderOption->minimum ) );
            const qreal last( dialAngle( sliderOption, sliderOption->maximum ) );
            
            // render groove
            _helper->renderDialGroove( painter, grooveRect, grooveColor, first, last );

            if( enabled )
            {

                // highlight
                const auto &highlight = palette.color( QPalette::Highlight );

                // angles
                const qreal second( dialAngle( sliderOption, sliderOption->sliderPosition ) );

                // render contents
                _helper->renderDialContents( painter, grooveRect, highlight, first, second );

            }

        }

        // handle
        if( sliderOption->subControls & SC_DialHandle )
        {

            // get handle rect
            auto handleRect( subControlRect( CC_Dial, sliderOption, SC_DialHandle, widget ) );
            handleRect = centerRect( handleRect, Metrics::Slider_ControlThickness, Metrics::Slider_ControlThickness );

            // handle state
            const bool handleActive( mouseOver && handleRect.contains( _animations->dialEngine().position( widget ) ) );
            const bool sunken( state & (State_On|State_Sunken) );

            // animation state
            _animations->dialEngine().setHandleRect( widget, handleRect );
            _animations->dialEngine().updateState( widget, AnimationHover, handleActive && mouseOver );
            _animations->dialEngine().updateState( widget, AnimationFocus, hasFocus );
            const auto mode( _animations->dialEngine().buttonAnimationMode( widget ) );
            const qreal opacity( _animations->dialEngine().buttonOpacity( widget ) );

            // define colors
            const auto &background = palette.color( QPalette::Button );
            const auto outline( _helper->sliderOutlineColor( palette, handleActive && mouseOver, hasFocus, opacity, mode ) );
            const auto shadow( _helper->shadowColor( palette ) );

            // render
            _helper->renderSliderHandle( painter, handleRect, background, outline, shadow, sunken );

        }

        return true;
    }

    //______________________________________________________________
    bool Style::drawScrollBarComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {
        //the animation for QStyle::SC_ScrollBarGroove is special: it will animate
        //the opacity of everything else as well, included slider and arrows
        qreal opacity( _animations->scrollBarEngine().opacity( widget, QStyle::SC_ScrollBarGroove ) );
        const bool animated( StyleConfigData::animationsEnabled() && _animations->scrollBarEngine().isAnimated( widget,  AnimationHover, QStyle::SC_ScrollBarGroove ) );
        const bool mouseOver( option->state & State_MouseOver );

        if( opacity == AnimationData::OpacityInvalid ) opacity = 1;

        // render full groove directly, rather than using the addPage and subPage control element methods
        if( (!StyleConfigData::animationsEnabled() || mouseOver || animated) && option->subControls & SC_ScrollBarGroove )
        {
            // retrieve groove rectangle
            auto grooveRect( subControlRect( CC_ScrollBar, option, SC_ScrollBarGroove, widget ) );

            const auto& palette( option->palette );
            const auto color( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.3 * (animated ? opacity : 1) ) );
            const auto& state( option->state );
            const bool horizontal( state & State_Horizontal );

            if( horizontal ) grooveRect = centerRect( grooveRect, grooveRect.width(), Metrics::ScrollBar_SliderWidth );
            else grooveRect = centerRect( grooveRect, Metrics::ScrollBar_SliderWidth, grooveRect.height() );

            // render
            _helper->renderScrollBarGroove( painter, grooveRect, color );
        }

        // call base class primitive
        ParentStyleClass::drawComplexControl( CC_ScrollBar, option, painter, widget );

        return true;
    }

    //______________________________________________________________
    bool Style::drawTitleBarComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const auto titleBarOption( qstyleoption_cast<const QStyleOptionTitleBar *>( option ) );
        if( !titleBarOption ) return true;

        // store palette and rect
        auto palette( option->palette );
        const auto& rect( option->rect );

        const State& flags( option->state );
        const bool enabled( flags & State_Enabled );
        const bool active( enabled && ( titleBarOption->titleBarState & Qt::WindowActive ) );

        if( titleBarOption->subControls & SC_TitleBarLabel )
        {

            // render background
            painter->setClipRect( rect );
            const auto outline( active ? QColor():_helper->frameOutlineColor( palette, false, false ) );
            const auto background( _helper->titleBarColor( active ) );
            _helper->renderTabWidgetFrame( painter, rect.adjusted( -1, -1, 1, 3 ), background, outline, CornersTop );

            const bool useSeparator(
                active &&
                _helper->titleBarColor( active ) != palette.color( QPalette::Window ) &&
                !( titleBarOption->titleBarState & Qt::WindowMinimized ) );

            if( useSeparator )
            {
                painter->setRenderHint( QPainter::Antialiasing, false );
                painter->setBrush( Qt::NoBrush );
                painter->setPen( palette.color( QPalette::Highlight ) );
                painter->drawLine( rect.bottomLeft(), rect.bottomRight() );
            }

            // render text
            palette.setColor( QPalette::WindowText, _helper->titleBarTextColor( active ) );
            const auto textRect( subControlRect( CC_TitleBar, option, SC_TitleBarLabel, widget ) );
            ParentStyleClass::drawItemText( painter, textRect, Qt::AlignCenter, palette, active, titleBarOption->text, QPalette::WindowText );

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
        foreach( const SubControl& subControl, subControls )
        {

            // skip if not requested
            if( !( titleBarOption->subControls & subControl ) ) continue;

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
            auto iconRect( subControlRect( CC_TitleBar, option, subControl, widget ) );
            if( iconRect.isEmpty() ) continue;

            // active state
            const bool subControlActive( titleBarOption->activeSubControls & subControl );

            // mouse over state
            const bool mouseOver(
                !subControlActive &&
                widget &&
                iconRect.translated( widget->mapToGlobal( QPoint( 0,0 ) ) ).contains( QCursor::pos() ) );

            // adjust iconRect
            const int iconWidth( pixelMetric( PM_SmallIconSize, option, widget ) );
            const QSize iconSize( iconWidth, iconWidth );
            iconRect = centerRect( iconRect, iconSize );

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
    void Style::renderSpinBoxArrow( const SubControl& subControl, const QStyleOptionSpinBox* option, QPainter* painter, const QWidget* widget ) const
    {

        const auto& palette( option->palette );
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

        auto color = _helper->arrowColor( palette, QPalette::Text );
        if( animated )
        {

            auto highlight = _helper->hoverColor( palette );
            color = KColorUtils::mix( color, highlight, opacity );

        } else if( subControlHover ) {

            color = _helper->hoverColor( palette );

        } else if( atLimit ) {

            color = _helper->arrowColor( palette, QPalette::Disabled, QPalette::Text );

        }

        // arrow orientation
        ArrowOrientation orientation( ( subControl == SC_SpinBoxUp ) ? ArrowUp:ArrowDown );

        // arrow rect
        const auto arrowRect( subControlRect( CC_SpinBox, option, subControl, widget ) );

        // render
        _helper->renderArrow( painter, arrowRect, color, orientation );

    }

    //______________________________________________________________________________
    void Style::renderMenuTitle( const QStyleOptionToolButton* option, QPainter* painter, const QWidget* ) const
    {

        // render a separator at the bottom
        const auto& palette( option->palette );
        const auto color( _helper->separatorColor( palette ) );
        _helper->renderSeparator( painter, QRect( option->rect.bottomLeft()-QPoint( 0, Metrics::MenuItem_MarginHeight), QSize( option->rect.width(), 1 ) ), color );

        // render text in the center of the rect
        // icon is discarded on purpose
        painter->setFont( option->font );
        const auto contentsRect = insideMargin( option->rect, Metrics::MenuItem_MarginWidth, Metrics::MenuItem_MarginHeight );
        drawItemText( painter, contentsRect, Qt::AlignCenter, palette, true, option->text, QPalette::WindowText );

    }

    //______________________________________________________________________________
    qreal Style::dialAngle( const QStyleOptionSlider* sliderOption, int value ) const
    {

        // calculate angle at which handle needs to be drawn
        qreal angle( 0 );
        if( sliderOption->maximum == sliderOption->minimum ) angle = M_PI / 2;
        else {

            qreal fraction( qreal( value - sliderOption->minimum )/qreal( sliderOption->maximum - sliderOption->minimum ) );
            if( !sliderOption->upsideDown ) fraction = 1 - fraction;

            if( sliderOption->dialWrapping ) angle = 1.5*M_PI - fraction*2*M_PI;
            else  angle = ( M_PI*8 - fraction*10*M_PI )/6;

        }

        return angle;

    }

    //______________________________________________________________________________
    const QWidget* Style::scrollBarParent( const QWidget* widget ) const
    {

        // check widget and parent
        if( !(widget && widget->parentWidget() ) ) return nullptr;

        // try cast to scroll area. Must test both parent and grandparent
        QAbstractScrollArea* scrollArea;
        if( !(scrollArea = qobject_cast<QAbstractScrollArea*>( widget->parentWidget() ) ) )
        { scrollArea = qobject_cast<QAbstractScrollArea*>( widget->parentWidget()->parentWidget() ); }

        // check scrollarea
        if( scrollArea &&
            (widget == scrollArea->verticalScrollBar() ||
            widget == scrollArea->horizontalScrollBar() ) )
        {

            return scrollArea;

        } else if( widget->parentWidget()->inherits( "KTextEditor::View" ) ) {

            return widget->parentWidget();

        } else return nullptr;

    }

    //______________________________________________________________________________
    QColor Style::scrollBarArrowColor( const QStyleOptionSlider* option, const SubControl& control, const QWidget* widget ) const
    {

        const auto& rect( option->rect );
        const auto& palette( option->palette );
        auto color( _helper->arrowColor( palette, QPalette::WindowText ) );

        bool widgetMouseOver( ( option->state & State_MouseOver ) );
        if( widget ) widgetMouseOver = widget->underMouse();
        #if QT_VERSION >= 0x050000
        // in case this QStyle is used by QQuickControls QStyle wrapper
        else if( option->styleObject ) widgetMouseOver = option->styleObject->property("hover").toBool();
        #endif

        // check enabled state
        const bool enabled( option->state & State_Enabled );
        if( !enabled ) {
            if( StyleConfigData::animationsEnabled() ) {
                // finally, global opacity when ScrollBarShowOnMouseOver
                const qreal globalOpacity( _animations->scrollBarEngine().opacity( widget, QStyle::SC_ScrollBarGroove ) );
                if( globalOpacity >= 0 ) color.setAlphaF( globalOpacity );
                // no mouse over and no animation in progress, don't draw arrows at all
                else if( !widgetMouseOver ) return Qt::transparent;
            }
            return color;
        }

        if(
            ( control == SC_ScrollBarSubLine && option->sliderValue == option->minimum ) ||
            ( control == SC_ScrollBarAddLine && option->sliderValue == option->maximum ) )
        {

            // manually disable arrow, to indicate that scrollbar is at limit
            color = _helper->arrowColor( palette, QPalette::Disabled, QPalette::WindowText );
            if( StyleConfigData::animationsEnabled() ) {
                // finally, global opacity when ScrollBarShowOnMouseOver
                const qreal globalOpacity( _animations->scrollBarEngine().opacity( widget, QStyle::SC_ScrollBarGroove ) );
                if( globalOpacity >= 0 ) color.setAlphaF( globalOpacity );
                // no mouse over and no animation in progress, don't draw arrows at all
                else if( !widgetMouseOver ) return Qt::transparent;
            }
            return color;
        }

        const bool mouseOver( _animations->scrollBarEngine().isHovered( widget, control ) );
        const bool animated( _animations->scrollBarEngine().isAnimated( widget, AnimationHover, control ) );
        const qreal opacity( _animations->scrollBarEngine().opacity( widget, control ) );

        // retrieve mouse position from engine
        QPoint position( mouseOver ? _animations->scrollBarEngine().position( widget ) : QPoint( -1, -1 ) );
        if( mouseOver && rect.contains( position ) )
        {
            /*
             * need to update the arrow controlRect on fly because there is no
             * way to get it from the styles directly, outside of repaint events
             */
            _animations->scrollBarEngine().setSubControlRect( widget, control, rect );
        }


        if( rect.intersects(  _animations->scrollBarEngine().subControlRect( widget, control ) ) )
        {

            auto highlight = _helper->hoverColor( palette );
            if( animated )
            {
                color = KColorUtils::mix( color, highlight, opacity );

            } else if( mouseOver ) {

                color = highlight;

            }

        }

        if( StyleConfigData::animationsEnabled() ) {
            // finally, global opacity when ScrollBarShowOnMouseOver
            const qreal globalOpacity( _animations->scrollBarEngine().opacity( widget, QStyle::SC_ScrollBarGroove ) );
            if( globalOpacity >= 0 ) color.setAlphaF( globalOpacity );
            // no mouse over and no animation in progress, don't draw arrows at all
            else if( !widgetMouseOver ) return Qt::transparent;
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
        toolButtonOption.state = State_Enabled|State_AutoRaise;
        toolButtonOption.subControls = SC_ToolButton;
        toolButtonOption.icon =  QIcon();
        toolButtonOption.iconSize = QSize();
        toolButtonOption.text = menuItemOption->text;

        toolButtonOption.toolButtonStyle = Qt::ToolButtonTextBesideIcon;
        return toolButtonOption;

    }

    //____________________________________________________________________________________
    QIcon Style::toolBarExtensionIcon( StandardPixmap standardPixmap, const QStyleOption* option, const QWidget* widget ) const
    {

        // store palette
        // due to Qt, it is not always safe to assume that either option, nor widget are defined
        QPalette palette;
        if( option ) palette = option->palette;
        else if( widget ) palette = widget->palette();
        else palette = QApplication::palette();

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

        // default icon sizes
        static const QList<int> iconSizes = { 8, 16, 22, 32, 48 };

        // decide arrow orientation
        const ArrowOrientation orientation( standardPixmap == SP_ToolBarHorizontalExtensionButton ? ArrowRight : ArrowDown );

        // create icon and fill
        QIcon icon;
        foreach( const IconData& iconData, iconTypes )
        {

            foreach( const int& iconSize, iconSizes )
            {

                // create pixmap
                QPixmap pixmap( iconSize, iconSize );
                pixmap.fill( Qt::transparent );

                // render
                QPainter painter( &pixmap );

                // icon size
                const int fixedIconSize( pixelMetric( QStyle::PM_SmallIconSize, option, widget ) );
                const QRect fixedRect( 0, 0, fixedIconSize, fixedIconSize );

                painter.setWindow( fixedRect );
                painter.translate( standardPixmap == SP_ToolBarHorizontalExtensionButton ? QPoint( 1, 0 ) : QPoint( 0, 1 ) );
                _helper->renderArrow( &painter, fixedRect, iconData._color, orientation );
                painter.end();

                // add to icon
                icon.addPixmap( pixmap, iconData._mode, iconData._state );

            }

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
        // due to Qt, it is not always safe to assume that either option, nor widget are defined
        QPalette palette;
        if( option ) palette = option->palette;
        else if( widget ) palette = widget->palette();
        else palette = QApplication::palette();

        const bool isCloseButton( buttonType == ButtonClose && StyleConfigData::outlineCloseButton() );

        palette.setCurrentColorGroup( QPalette::Active );
        const auto base( palette.color( QPalette::WindowText ) );
        const auto selected( palette.color( QPalette::HighlightedText ) );
        const auto negative( buttonType == ButtonClose ? _helper->negativeText( palette ):base );
        const auto negativeSelected( buttonType == ButtonClose ? _helper->negativeText( palette ):selected );

        const bool invertNormalState( isCloseButton );

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
            { KColorUtils::mix( palette.color( QPalette::Window ), base,  0.5 ), invertNormalState, QIcon::Normal, QIcon::Off },
            { KColorUtils::mix( palette.color( QPalette::Window ), selected, 0.5 ), invertNormalState, QIcon::Selected, QIcon::Off },
            { KColorUtils::mix( palette.color( QPalette::Window ), negative, 0.5 ), true, QIcon::Active, QIcon::Off },
            { KColorUtils::mix( palette.color( QPalette::Window ), base, 0.2 ), invertNormalState, QIcon::Disabled, QIcon::Off },

            // state on icons
            { KColorUtils::mix( palette.color( QPalette::Window ), negative, 0.7 ), true, QIcon::Normal, QIcon::On },
            { KColorUtils::mix( palette.color( QPalette::Window ), negativeSelected, 0.7 ), true, QIcon::Selected, QIcon::On },
            { KColorUtils::mix( palette.color( QPalette::Window ), negative, 0.7 ), true, QIcon::Active, QIcon::On },
            { KColorUtils::mix( palette.color( QPalette::Window ), base, 0.2 ), invertNormalState, QIcon::Disabled, QIcon::On }

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
                _helper->renderDecorationButton( &painter, pixmap.rect(), iconData._color, buttonType, iconData._inverted );

                painter.end();

                // store
                icon.addPixmap( pixmap, iconData._mode, iconData._state );
            }

        }

        return icon;

    }

    //______________________________________________________________________________
    const QAbstractItemView* Style::itemViewParent( const QWidget* widget ) const
    {

        const QAbstractItemView* itemView( nullptr );

        // check widget directly
        if( ( itemView = qobject_cast<const QAbstractItemView*>( widget ) ) ) return itemView;

        // check widget grand-parent
        else if(
            widget &&
            widget->parentWidget() &&
            ( itemView = qobject_cast<const QAbstractItemView*>( widget->parentWidget()->parentWidget() ) ) &&
            itemView->viewport() == widget->parentWidget() )
        { return itemView; }

        // return null otherwise
        else return nullptr;
    }

    //____________________________________________________________________
    bool Style::isSelectedItem( const QWidget* widget, const QPoint& localPosition ) const
    {

        // get relevant itemview parent and check
        const QAbstractItemView* itemView( itemViewParent( widget ) );
        if( !( itemView && itemView->hasFocus() && itemView->selectionModel() ) ) return false;

        #if QT_VERSION >= 0x050000
        QPoint position = widget->mapTo( itemView, localPosition );
        #else
        // qt4 misses a const for mapTo argument, although nothing is actually changed to the passed widget
        QPoint position = widget->mapTo( const_cast<QAbstractItemView*>( itemView ), localPosition );
        #endif

        // get matching QModelIndex and check
        const QModelIndex index( itemView->indexAt( position ) );
        if( !index.isValid() ) return false;

        // check whether index is selected
        return itemView->selectionModel()->isSelected( index );

    }

    //____________________________________________________________________
    bool Style::isQtQuickControl( const QStyleOption* option, const QWidget* widget ) const
    {
        #if QT_VERSION >= 0x050000 && BREEZE_HAVE_QTQUICK
        const bool is = (widget == nullptr) && option && option->styleObject && option->styleObject->inherits( "QQuickItem" );
        if ( is ) _windowManager->registerQuickItem( static_cast<QQuickItem*>( option->styleObject ) );
        return is;
        #else
        Q_UNUSED( widget );
        Q_UNUSED( option );
        return false;
        #endif
    }

    //____________________________________________________________________
    bool Style::showIconsInMenuItems() const
    {
        const KConfigGroup g(KSharedConfig::openConfig(), "KDE");
        return g.readEntry("ShowIconsInMenuItems", true);
    }

    //____________________________________________________________________
    bool Style::showIconsOnPushButtons() const
    {
        const KConfigGroup g(KSharedConfig::openConfig(), "KDE");
        return g.readEntry("ShowIconsOnPushButtons", true);
    }

    //____________________________________________________________________
    bool Style::isMenuTitle( const QWidget* widget ) const
    {

        // check widget
        if( !widget ) return false;

        // check property
        const QVariant property( widget->property( PropertyNames::menuTitle ) );
        if( property.isValid() ) return property.toBool();

        // detect menu toolbuttons
        QWidget* parent = widget->parentWidget();
        if( qobject_cast<QMenu*>( parent ) )
        {
            foreach( auto child, parent->findChildren<QWidgetAction*>() )
            {
                if( child->defaultWidget() != widget ) continue;
                const_cast<QWidget*>(widget)->setProperty( PropertyNames::menuTitle, true );
                return true;
            }

        }

        const_cast<QWidget*>(widget)->setProperty( PropertyNames::menuTitle, false );
        return false;

    }

    //____________________________________________________________________
    bool Style::hasAlteredBackground( const QWidget* widget ) const
    {

        // check widget
        if( !widget ) return false;

        // check property
        const QVariant property( widget->property( PropertyNames::alteredBackground ) );
        if( property.isValid() ) return property.toBool();

        // check if widget is of relevant type
        bool hasAlteredBackground( false );
        if( const auto groupBox = qobject_cast<const QGroupBox*>( widget ) ) hasAlteredBackground = !groupBox->isFlat();
        else if( const auto tabWidget = qobject_cast<const QTabWidget*>( widget ) ) hasAlteredBackground = !tabWidget->documentMode();
        else if( qobject_cast<const QMenu*>( widget ) ) hasAlteredBackground = true;
        else if( StyleConfigData::dockWidgetDrawFrame() && qobject_cast<const QDockWidget*>( widget ) ) hasAlteredBackground = true;

        if( widget->parentWidget() && !hasAlteredBackground ) hasAlteredBackground = this->hasAlteredBackground( widget->parentWidget() );
        const_cast<QWidget*>(widget)->setProperty( PropertyNames::alteredBackground, hasAlteredBackground );
        return hasAlteredBackground;

    }

}
