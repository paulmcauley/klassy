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

#include "breezeanimations.h"
#include "breezeanimationmodes.h"
#include "breezeframeshadow.h"
#include "breezehelper.h"
#include "breezemetrics.h"
#include "breezemnemonics.h"
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
#include <QGraphicsView>
#include <QGroupBox>
#include <QLineEdit>
#include <QMainWindow>
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
        _animations( new Animations( this ) ),
        _mnemonics( new Mnemonics( this ) ),
        _tabBarData( new BreezePrivate::TabBarData( this ) ),
        _windowManager( new WindowManager( this ) ),
        _frameShadowFactory( new FrameShadowFactory( this ) ),
        SH_ArgbDndWindow( newStyleHint( QStringLiteral( "SH_ArgbDndWindow" ) ) ),
        CE_CapacityBar( newControlElement( QStringLiteral( "CE_CapacityBar" ) ) )

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
    { delete _helper; }

    //______________________________________________________________
    void Style::polish( QWidget* widget )
    {
        if( !widget ) return;

        // register widget to animations
        _animations->registerWidget( widget );
        _windowManager->registerWidget( widget );
        _frameShadowFactory->registerWidget( widget, *_helper );

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


        // remove opaque painting for scrollbars
        if( qobject_cast<QScrollBar*>( widget ) )
        {

            widget->setAttribute( Qt::WA_OpaquePaintEvent, false );

        // add event filter
        } else if( qobject_cast<QDockWidget*>( widget ) ) {

            widget->setAutoFillBackground( false );
            widget->setBackgroundRole( QPalette::NoRole );
            widget->setContentsMargins( Metrics::Frame_FrameWidth, Metrics::Frame_FrameWidth, Metrics::Frame_FrameWidth, Metrics::Frame_FrameWidth );
            addEventFilter( widget );

        }

        // base class polishing
        KStyle::polish( widget );

    }

    //_______________________________________________________________
    void Style::unpolish( QWidget* widget )
    {

        // register widget to animations
        _animations->unregisterWidget( widget );
        _windowManager->unregisterWidget( widget );
        _frameShadowFactory->unregisterWidget( widget );

        KStyle::unpolish( widget );

    }

    //______________________________________________________________
    int Style::pixelMetric( PixelMetric metric, const QStyleOption* option, const QWidget* widget ) const
    {

        // handle special cases
        switch( metric )
        {

            // frame width
            case PM_DefaultFrameWidth:
            case PM_ComboBoxFrameWidth:
            case PM_SpinBoxFrameWidth:
            return Metrics::Frame_FrameWidth;

            // buttons
            case PM_ButtonMargin: return Metrics::Button_MarginWidth;
            case PM_ButtonDefaultIndicator: return 0;
            case PM_ButtonShiftHorizontal: return 0;
            case PM_ButtonShiftVertical: return 0;

            // tabbar
            case PM_TabBarTabShiftVertical: return 0;
            case PM_TabBarTabShiftHorizontal: return 0;
            case PM_TabBarTabOverlap: return Metrics::TabBar_TabOverlap;
            case PM_TabBarBaseOverlap: return Metrics::TabBar_BaseOverlap;
            case PM_TabBarTabHSpace: return 2*Metrics::TabBar_TabMarginWidth;
            case PM_TabBarTabVSpace: return 2*Metrics::TabBar_TabMarginHeight;

            // scrollbars
            case PM_ScrollBarExtent: return Metrics::ScrollBar_Extend;
            case PM_ScrollBarSliderMin: return Metrics::ScrollBar_MinSliderHeight;

            // sliders
            case PM_SliderThickness: return Metrics::Slider_ControlThickness;
            case PM_SliderControlThickness: return Metrics::Slider_ControlThickness;
            case PM_SliderLength: return Metrics::Slider_ControlThickness;

            // checkboxes and radio buttons
            case PM_IndicatorWidth: return CheckBox_Size;
            case PM_IndicatorHeight: return CheckBox_Size;
            case PM_ExclusiveIndicatorWidth: return RadioButton_Size;
            case PM_ExclusiveIndicatorHeight: return RadioButton_Size;

            // list heaaders
            case PM_HeaderMarkSize: return Metrics::Header_MarkSize;
            case PM_HeaderMargin: return Metrics::Header_MarginWidth;

            // dock widget
            // return 0 here, since frame is handled directly in polish
            case PM_DockWidgetFrameWidth: return 0;
            case PM_DockWidgetTitleMargin: return DockWidget_TitleMarginWidth;

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
                    mask->region -= option->rect.adjusted( 1,1,-1,-1 );

                    return true;
                }
                return false;
            }

            // mouse tracking
            case SH_ComboBox_ListMouseTracking: return true;
            case SH_MenuBar_MouseTracking: return true;
            case SH_Menu_MouseTracking: return true;

            // groupboxes
            case SH_GroupBox_TextLabelVerticalAlignment: return Qt::AlignVCenter;

            // tabbar
            case SH_TabBar_Alignment: return Qt::AlignCenter;

            // scrollbars
            case SH_ScrollBar_MiddleClickAbsolutePosition: return true;

            // fallback
            default: return KStyle::styleHint( hint, option, widget, returnData );

        }

    }

    //______________________________________________________________
    QRect Style::subElementRect( SubElement element, const QStyleOption* option, const QWidget* widget ) const
    {
        switch( element )
        {

            // checkboxes
            case SE_CheckBoxContents: return checkBoxContentsRect( option, widget );
            case SE_CheckBoxFocusRect: return checkBoxFocusRect( option, widget );

            // radio buttons
            case SE_RadioButtonContents: return radioButtonContentsRect( option, widget );
            case SE_RadioButtonFocusRect: return radioButtonFocusRect( option, widget );

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
            case CT_ProgressBar: return progressBarSizeFromContents( option, size, widget );
            case CT_HeaderSection: return headerSectionSizeFromContents( option, size, widget );

            // tabbar
            case CT_TabWidget: return tabWidgetSizeFromContents( option, size, widget );
            case CT_TabBarTab: return tabBarTabSizeFromContents( option, size, widget );

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

            // checkboxes and radio buttons
            case PE_IndicatorCheckBox: fcn = &Style::drawIndicatorCheckBoxPrimitive; break;
            case PE_IndicatorRadioButton: fcn = &Style::drawIndicatorRadioButtonPrimitive; break;

            // arrows
            case PE_IndicatorArrowUp: fcn = &Style::drawIndicatorArrowUpPrimitive; break;
            case PE_IndicatorArrowDown: fcn = &Style::drawIndicatorArrowDownPrimitive; break;
            case PE_IndicatorArrowLeft: fcn = &Style::drawIndicatorArrowLeftPrimitive; break;
            case PE_IndicatorArrowRight: fcn = &Style::drawIndicatorArrowRightPrimitive; break;
            case PE_IndicatorHeaderArrow: fcn = &Style::drawIndicatorHeaderArrowPrimitive; break;

            // frames
            case PE_FrameStatusBar: fcn = &Style::emptyPrimitive; break;
            case PE_Frame: fcn = &Style::drawFramePrimitive; break;
            case PE_FrameLineEdit: fcn = &Style::drawFramePrimitive; break;
            case PE_FrameGroupBox: fcn = &Style::drawFrameGroupBoxPrimitive; break;
            case PE_FrameTabBarBase: fcn = &Style::drawFrameTabBarBasePrimitive; break;
            case PE_FrameTabWidget: fcn = &Style::drawFrameTabWidgetPrimitive; break;
            case PE_FrameFocusRect: fcn = &Style::drawFrameFocusRectPrimitive; break;

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

            // combobox
            case CE_ComboBoxLabel: fcn = &Style::drawComboBoxLabelControl; break;

            // progress bars
            case CE_ProgressBar: fcn = &Style::drawProgressBarControl; break;
            case CE_ProgressBarContents: fcn = &Style::drawProgressBarContentsControl; break;
            case CE_ProgressBarGroove: fcn = &Style::drawProgressBarGrooveControl; break;
            case CE_ProgressBarLabel: fcn = &Style::drawProgressBarLabelControl; break;

            // scrollbars
            case CE_ScrollBarSlider: fcn = &Style::drawScrollBarSliderControl; break;
            case CE_ScrollBarAddLine: fcn = &Style::drawScrollBarAddLineControl; break;
            case CE_ScrollBarAddPage: fcn = &Style::drawScrollBarAddPageControl; break;
            case CE_ScrollBarSubLine: fcn = &Style::drawScrollBarSubLineControl; break;
            case CE_ScrollBarSubPage: fcn = &Style::drawScrollBarSubPageControl; break;

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

            case CC_ComboBox: fcn = &Style::drawComboBoxComplexControl; break;
            case CC_SpinBox: fcn = &Style::drawSpinBoxComplexControl; break;
            case CC_Slider: fcn = &Style::drawSliderComplexControl; break;
            case CC_Dial: fcn = &Style::drawDialComplexControl; break;

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

                const QPalette palette = _helper->mergePalettes( palette, _animations->widgetEnabilityEngine().opacity( widget, AnimationEnable )  );
                return KStyle::drawItemText( painter, r, flags, palette, enabled, text, textRole );

            }

        }

        // fallback
        return KStyle::drawItemText( painter, r, flags, palette, enabled, text, textRole );

    }


    //_____________________________________________________________________
    bool Style::eventFilter( QObject *object, QEvent *event )
    {

        // dock widgets
        if( QDockWidget* dockWidget = qobject_cast<QDockWidget*>( object ) ) { return eventFilterDockWidget( dockWidget, event ); }

        // fallback
        return KStyle::eventFilter( object, event );
    }

    //____________________________________________________________________________
    bool Style::eventFilterDockWidget( QDockWidget* dockWidget, QEvent* event )
    {
        switch( event->type() )
        {
            case QEvent::Paint:
            {

                // do nothing for detached docks
                if( dockWidget->isWindow() ) return false;

                // create painter and clip
                QPainter painter( dockWidget );
                QPaintEvent *paintEvent = static_cast<QPaintEvent*>( event );
                painter.setClipRegion( paintEvent->region() );

                // define color and render
                const QPalette& palette( dockWidget->palette() );
                const QColor outline( KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::WindowText ), 0.25 ) );
                _helper->renderFrame( &painter, dockWidget->rect(), QColor(), outline );

                return false;
            }

            default: return false;

        }

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
    }

    //___________________________________________________________________________________________________________________
    QRect Style::checkBoxFocusRect( const QStyleOption* option, const QWidget* ) const
    {

        // cast option
        const QStyleOptionButton* buttonOption( qstyleoption_cast<const QStyleOptionButton*>( option ) );
        if( !buttonOption ) return option->rect;

        // calculate text rect
        const QRect contentsRect( option->rect.adjusted( Metrics::CheckBox_Size + Metrics::CheckBox_BoxTextSpace, 0, 0, 0 ) );
        const QRect boundingRect( option->fontMetrics.boundingRect( contentsRect, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextShowMnemonic, buttonOption->text ) );
        return handleRTL( option, boundingRect );

    }

    //___________________________________________________________________________________________________________________
    QRect Style::lineEditContentsRect( const QStyleOption* option, const QWidget* ) const
    {
        // cast option and check
        const QStyleOptionFrame* frameOption( qstyleoption_cast<const QStyleOptionFrame*>( option ) );
        if( !frameOption ) return option->rect;

        // check flatness
        const bool flat( frameOption->lineWidth == 0 );
        return flat ? option->rect : insideMargin( option->rect, Metrics::LineEdit_MarginWidth + Metrics::Frame_FrameWidth );
    }

    //___________________________________________________________________________________________________________________
    QRect Style::progressBarGrooveRect( const QStyleOption* option, const QWidget* ) const
    {

        // cast option
        const QStyleOptionProgressBar* progressBarOption( qstyleoption_cast<const QStyleOptionProgressBar*>( option ) );
        if( !progressBarOption ) return option->rect;

        // get direction
        const State& flags( option->state );
        const bool horizontal( flags&State_Horizontal );
        const bool textVisible( progressBarOption->textVisible );
        const bool busy( progressBarOption->minimum == 0 && progressBarOption->maximum == 0 );

        QRect rect( option->rect );
        if( textVisible && !busy )
        {
            if( horizontal ) rect.setTop( rect.height() - Metrics::ProgressBar_Thickness );
            else {

                const bool reverseLayout( option->direction == Qt::RightToLeft );
                if( reverseLayout ) rect.setLeft( rect.width() - Metrics::ProgressBar_Thickness );
                else rect.setWidth( Metrics::ProgressBar_Thickness );

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
        const State& flags( option->state );
        const bool horizontal( flags&State_Horizontal );

        QRect rect( option->rect );
        if( horizontal ) rect.setHeight( rect.height() - Metrics::ProgressBar_Thickness - Metrics::ProgressBar_BoxTextSpace );
        else {

            const bool reverseLayout( option->direction == Qt::RightToLeft );
            if( reverseLayout ) rect.setWidth( rect.width() - Metrics::ProgressBar_Thickness - Metrics::ProgressBar_BoxTextSpace );
            else rect.setLeft( Metrics::ProgressBar_Thickness + Metrics::ProgressBar_BoxTextSpace );

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
        arrowRect.setLeft( arrowRect.right() - Metrics::Header_MarkSize );

        return handleRTL( option, arrowRect );

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

        labelRect.adjust( 0, 0, -Metrics::Header_MarkSize-Metrics::Header_BoxTextSpace, 0 );
        return handleRTL( option, labelRect );

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
            if( !tabOption->leftCornerWidgetSize.isEmpty() )
            {
                const QRect buttonRect( subElementRect( SE_TabWidgetLeftCorner, option, widget ) );
                rect.setLeft( buttonRect.width() - 1 );
            }

            if( !tabOption->rightCornerWidgetSize.isEmpty() )
            {
                const QRect buttonRect( subElementRect( SE_TabWidgetRightCorner, option, widget ) );
                rect.setRight( buttonRect.left() );
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

        return handleRTL( option, cornerRect );

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
                const int frameWidth( pixelMetric( PM_DefaultFrameWidth, option, widget ) );
                rect.adjust( frameWidth, frameWidth, -frameWidth, -frameWidth );

                // get flags
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
                const int frameWidth( pixelMetric( PM_DefaultFrameWidth, option, widget ) );
                rect.adjust( frameWidth, frameWidth, -frameWidth, -frameWidth );

                const bool emptyText( groupBoxOption->text.isEmpty() );
                const bool checkable( groupBoxOption->subControls & QStyle::SC_GroupBoxCheckBox );

                // calculate title height
                int titleHeight( 0 );
                int titleWidth( 0 );
                if( !emptyText )
                {
                    const QFontMetrics fontMetrics = option->fontMetrics;
                    titleHeight = qMax( titleHeight, fontMetrics.height() );
                    titleWidth += fontMetrics.size( Qt::TextShowMnemonic, groupBoxOption->text ).width();
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
                        rect.right() - Metrics::ComboBox_ButtonWidth,
                        rect.top(),
                        Metrics::ComboBox_ButtonWidth,
                        rect.height() );

                } else {

                    arrowRect = QRect(
                        rect.right() - Metrics::ComboBox_ButtonWidth,
                        rect.top(),
                        Metrics::ComboBox_ButtonWidth,
                        rect.height() );

                }

                arrowRect = centerRect( arrowRect, Metrics::ComboBox_ButtonWidth, Metrics::ComboBox_ButtonWidth );
                return handleRTL( option, arrowRect );

            }

            case SC_ComboBoxEditField:
            {

                // take out frame width
                if( !flat ) rect = insideMargin( rect, Metrics::Frame_FrameWidth );

                QRect labelRect;
                const bool editable( comboBoxOption->editable );

                if( editable )
                {

                    labelRect = QRect(
                        rect.left(), rect.top(),
                        rect.width() - Metrics::ComboBox_ButtonWidth,
                        rect.height() );

                    // remove line editor margins
                    if( !flat )
                    { labelRect.adjust( Metrics::LineEdit_MarginWidth, Metrics::LineEdit_MarginWidth, 0, -Metrics::LineEdit_MarginWidth ); }

                } else {

                    labelRect = QRect(
                        rect.left(), rect.top(),
                        rect.width() - Metrics::ComboBox_ButtonWidth - Metrics::ComboBox_BoxTextSpace,
                        rect.height() );

                    // remove button margin
                    labelRect.adjust( Metrics::ComboBox_MarginWidth, Metrics::ComboBox_MarginWidth, 0, -Metrics::ComboBox_MarginWidth );

                }

                return handleRTL( option, labelRect );

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
                    rect.right() - Metrics::SpinBox_ButtonWidth,
                    rect.top(),
                    Metrics::SpinBox_ButtonWidth,
                    rect.height() );

                arrowRect = centerRect( arrowRect, Metrics::SpinBox_ButtonWidth, Metrics::SpinBox_ButtonWidth );
                arrowRect.setHeight( Metrics::SpinBox_ButtonWidth/2 );
                if( subControl == SC_SpinBoxDown ) arrowRect.translate( 0, Metrics::SpinBox_ButtonWidth/2 );

                return handleRTL( option, arrowRect );

            }

            case SC_SpinBoxEditField:
            {

                // take out frame width
                if( !flat ) rect = insideMargin( rect, Metrics::Frame_FrameWidth );

                QRect labelRect;
                labelRect = QRect(
                    rect.left(), rect.top(),
                    rect.width() - Metrics::SpinBox_ButtonWidth,
                    rect.height() );

                // remove line editor margins
                if( !flat ) labelRect.adjust( Metrics::LineEdit_MarginWidth, Metrics::LineEdit_MarginWidth, 0, -Metrics::LineEdit_MarginWidth );

                return handleRTL( option, labelRect );

            }

            default: break;

        }

        return KStyle::subControlRect( CC_SpinBox, option, subControl, widget );

    }

    //___________________________________________________________________________________________________________________
    QRect Style::scrollBarInternalSubControlRect( const QStyleOptionComplex* option, SubControl subControl ) const
    {

        const QRect& r = option->rect;
        const State& flags( option->state );
        const bool horizontal( flags&State_Horizontal );

        switch ( subControl )
        {

            case SC_ScrollBarSubLine:
            {
                int majorSize( scrollBarButtonHeight( _subLineButtons ) );
                if( horizontal ) return handleRTL( option, QRect( r.x(), r.y(), majorSize, r.height() ) );
                else return handleRTL( option, QRect( r.x(), r.y(), r.width(), majorSize ) );

            }

            case SC_ScrollBarAddLine:
            {
                int majorSize( scrollBarButtonHeight( _addLineButtons ) );
                if( horizontal ) return handleRTL( option, QRect( r.right() - majorSize, r.y(), majorSize, r.height() ) );
                else return handleRTL( option, QRect( r.x(), r.bottom() - majorSize, r.width(), majorSize ) );
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

        // get relevant flags
        const State& flags( option->state );
        const bool horizontal( flags&State_Horizontal );

        switch ( subControl )
        {

            case SC_ScrollBarSubLine:
            case SC_ScrollBarAddLine:
            return scrollBarInternalSubControlRect( option, subControl );

            case SC_ScrollBarGroove:
            {
                QRect top = handleRTL( option, scrollBarInternalSubControlRect( option, SC_ScrollBarSubLine ) );
                QRect bot = handleRTL( option, scrollBarInternalSubControlRect( option, SC_ScrollBarAddLine ) );

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
                return handleRTL( option, QRect( topLeftCorner, botRightCorner )  );

            }

            case SC_ScrollBarSlider:
            {
                // We handle RTL here to unreflect things if need be
                QRect groove = handleRTL( option, scrollBarSubControlRect( option, SC_ScrollBarGroove, widget ) );

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
                if( horizontal ) return handleRTL( option, QRect( groove.x() + pos, groove.y(), sliderSize, groove.height() ) );
                else return handleRTL( option, QRect( groove.x(), groove.y() + pos, groove.width(), sliderSize ) );
            }

            case SC_ScrollBarSubPage:
            {

                //We do handleRTL here to unreflect things if need be
                QRect slider = handleRTL( option, scrollBarSubControlRect( option, SC_ScrollBarSlider, widget ) );
                QRect groove = handleRTL( option, scrollBarSubControlRect( option, SC_ScrollBarGroove, widget ) );

                if( horizontal ) return handleRTL( option, QRect( groove.x(), groove.y(), slider.x() - groove.x(), groove.height() ) );
                else return handleRTL( option, QRect( groove.x(), groove.y(), groove.width(), slider.y() - groove.y() ) );
            }

            case SC_ScrollBarAddPage:
            {

                //We do handleRTL here to unreflect things if need be
                QRect slider = handleRTL( option, scrollBarSubControlRect( option, SC_ScrollBarSlider, widget ) );
                QRect groove = handleRTL( option, scrollBarSubControlRect( option, SC_ScrollBarGroove, widget ) );

                if( horizontal ) return handleRTL( option, QRect( slider.right() + 1, groove.y(), groove.right() - slider.right(), groove.height() ) );
                else return handleRTL( option, QRect( groove.x(), slider.bottom() + 1, groove.width(), groove.bottom() - slider.bottom() ) );

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

        //Make sure we can fit the indicator
        QSize size( contentsSize );
        size.setHeight( qMax( size.height(), (int) Metrics::CheckBox_Size ) );

        //Add space for the indicator and the icon
        size.rwidth() += Metrics::CheckBox_Size + Metrics::CheckBox_BoxTextSpace;

        return size;

    }

    //______________________________________________________________
    QSize Style::lineEditSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* ) const
    {
        // cast option and check
        const QStyleOptionFrame* frameOption( qstyleoption_cast<const QStyleOptionFrame*>( option ) );
        if( !frameOption ) return contentsSize;

        const bool flat( frameOption->lineWidth == 0 );
        return flat ? contentsSize : expandSize( contentsSize, Metrics::LineEdit_MarginWidth + Metrics::Frame_FrameWidth );
    }

    //______________________________________________________________
    QSize Style::comboBoxSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionComboBox* comboBoxOption( qstyleoption_cast<const QStyleOptionComboBox*>( option ) );
        if( !comboBoxOption ) return contentsSize;

        const bool editable( comboBoxOption->editable );
        const bool flat( editable && !comboBoxOption->frame );

        // copy size
        QSize size( contentsSize );

        // add relevant margins
        if( editable && !flat ) size = expandSize( size, Metrics::LineEdit_MarginWidth );
        else if( !editable ) size = expandSize( size, Metrics::ComboBox_MarginWidth );

        // make sure there is enough height for the button
        size.setHeight( qMax( size.height(), (int)Metrics::ComboBox_ButtonWidth ) );

        // add button width and spacing
        size.rwidth() += Metrics::ComboBox_ButtonWidth;
        if( !editable ) size.rwidth() += Metrics::ComboBox_BoxTextSpace;

        // add framewidth if needed
        return flat ? size : expandSize( size, Metrics::Frame_FrameWidth );

    }

    //______________________________________________________________
    QSize Style::spinBoxSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionSpinBox *spinBoxOption( qstyleoption_cast<const QStyleOptionSpinBox*>( option ) );
        if( !spinBoxOption ) return contentsSize;

        const bool flat( !spinBoxOption->frame );

        // copy size
        QSize size( contentsSize );

        // add editor margins
        if( !flat ) size = expandSize( size, Metrics::LineEdit_MarginWidth );

        // make sure there is enough height for the button
        size.setHeight( qMax( size.height(), (int)Metrics::SpinBox_ButtonWidth ) );

        // add button width and spacing
        size.rwidth() += Metrics::SpinBox_ButtonWidth;

        // add framewidth if needed
        return flat ? size : expandSize( size, Metrics::Frame_FrameWidth );

    }

    //______________________________________________________________
    QSize Style::pushButtonSizeFromContents( const QStyleOption*, const QSize& contentsSize, const QWidget* ) const
    { return expandSize( contentsSize, Metrics::Button_MarginWidth + Metrics::Frame_FrameWidth ); }

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
        const State& flags( option->state );
        const bool horizontal( flags&State_Horizontal );

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
            contentsWidth += Metrics::Header_MarkSize + Metrics::Header_BoxTextSpace;
            contentsHeight = qMax( contentsHeight, (int) Metrics::Header_MarkSize );
        }

        // update contents size, add margins and return
        const QSize size( contentsSize.expandedTo( QSize( contentsWidth, contentsHeight ) ) );
        return expandSize( size, Metrics::Header_MarginWidth );

    }

    //______________________________________________________________
    QSize Style::tabWidgetSizeFromContents( const QStyleOption*, const QSize& contentsSize, const QWidget* ) const
    { return expandSize( contentsSize, Metrics::Frame_FrameWidth ); }

    //______________________________________________________________
    QSize Style::tabBarTabSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* ) const
    {
        const QStyleOptionTab *tabOption( qstyleoption_cast<const QStyleOptionTab*>( option ) );

        // add margins
        // QSize size( expandSize( contentsSize, Metrics::TabBar_TabMarginWidth ) );
        QSize size( contentsSize );

        // compare to minimum size
        const bool verticalTabs( tabOption && isVerticalTab( tabOption ) );
        if( verticalTabs )
        {

            size += QSize( 2*Metrics::TabBar_TabMarginHeight, 2*Metrics::TabBar_TabMarginWidth );
            size = size.expandedTo( QSize( Metrics::TabBar_TabMinHeight, Metrics::TabBar_TabMinWidth ) );

        } else {

            size += QSize( 2*Metrics::TabBar_TabMarginWidth, 2*Metrics::TabBar_TabMarginHeight );
            size = size.expandedTo( QSize( Metrics::TabBar_TabMinWidth, Metrics::TabBar_TabMinHeight ) );

        }

        return size;

    }

    //______________________________________________________________
    bool Style::drawFramePrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        const State& flags( option->state );

        // do nothing for flat frames
        const bool isTitleWidget( widget && widget->parent() && widget->parent()->inherits( "KTitleWidget" ) );

        if( !isTitleWidget && !( flags & (State_Sunken | State_Raised ) ) ) return true;

        const bool isQtQuickControl = !widget && option && option->styleObject && option->styleObject->inherits( "QQuickStyleItem" );
        const bool isInputWidget( ( widget && widget->testAttribute( Qt::WA_Hover ) ) ||
            ( isQtQuickControl && option->styleObject->property( "elementType" ).toString() == QStringLiteral( "edit") ) );

        const QPalette& palette( option->palette );
        const bool enabled( flags & State_Enabled );
        const bool mouseOver( enabled && isInputWidget && ( flags&State_MouseOver ) );
        const bool hasFocus( enabled && ( flags&State_HasFocus ) );

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
        const QColor color( isTitleWidget ? palette.color( widget->backgroundRole() ):QColor() );
        const QColor outline( _helper->frameOutlineColor( option->palette, mouseOver, hasFocus, opacity, mode ) );
        _helper->renderFrame( painter, option->rect, color, outline, hasFocus );

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
        const QColor color( KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::Base ), 0.3 ) );
        const QColor outline( KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::WindowText ), 0.25 ) );

        _helper->renderFrame( painter, option->rect, color, outline );

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
        Helper::Corners corners = Helper::CornersAll;

        // adjust corners to deal with oversized tabbars
        switch( tabOption->shape )
        {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
            if( tabBarSize.width() >= rect.width()-2 ) corners &= ~Helper::CornersTop;
            break;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            if( tabBarSize.width() >= rect.width()-2 ) corners &= ~Helper::CornersBottom;
            break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            if( tabBarSize.height() >= rect.height()-2 ) corners &= ~Helper::CornersLeft;
            break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            if( tabBarSize.height() >= rect.height()-2 ) corners &= ~Helper::CornersRight;
            break;

            default: break;
        }

        // define colors
        const QPalette& palette( option->palette );
        const QColor color( KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::Base ), 0.3 ) );
        const QColor outline( KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::WindowText ), 0.25 ) );
        _helper->renderTabWidgetFrame( painter, rect, color, outline, corners );

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
        const QPalette& palette( option->palette );
        const QColor outline( KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::WindowText ), 0.25 ) );

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
    bool Style::drawFrameFocusRectPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // checkboxes and radio buttons
        if(
            qobject_cast<const QCheckBox*>( widget ) ||
            qobject_cast<const QRadioButton*>( widget ) )
        {
            painter->translate( 0, 2 );
            painter->setPen( _helper->viewFocusBrush().brush( option->palette ).color() );
            painter->drawLine( option->rect.bottomLeft(), option->rect.bottomRight() );
        }

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorArrowPrimitive( ArrowOrientation orientation, const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        const QRectF rect( option->rect );
        const QPalette& palette( option->palette );
        const State& flags( option->state );
        const bool enabled( flags & State_Enabled );
        const bool mouseOver( enabled && ( flags & State_MouseOver ) );

        // color
        QColor color;
        if( mouseOver ) color = _helper->viewHoverBrush().brush( palette ).color();
        else color = palette.color( QPalette::WindowText );

        // arrow
        const QPolygonF arrow( genericArrow( orientation, ArrowNormal ) );
        const qreal penThickness = 1.5;

        painter->setRenderHint( QPainter::Antialiasing );
        painter->translate( rect.center() );
        painter->setPen( QPen( color, penThickness ) );
        painter->drawPolyline( arrow );

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorHeaderArrowPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {
        const QStyleOptionHeader *headerOption( qstyleoption_cast<const QStyleOptionHeader*>( option ) );
        const State& flags( option->state );

        // arrow orientation
        ArrowOrientation orientation( ArrowNone );
        if( flags&State_UpArrow || ( headerOption && headerOption->sortIndicator==QStyleOptionHeader::SortUp ) ) orientation = ArrowUp;
        else if( flags&State_DownArrow || ( headerOption && headerOption->sortIndicator==QStyleOptionHeader::SortDown ) ) orientation = ArrowDown;
        if( orientation == ArrowNone ) return true;

        // flags, rect and palette
        const QRectF& rect( option->rect );
        const QPalette& palette( option->palette );

        // define color and polygon for drawing arrow
        const QPolygonF arrow( genericArrow( orientation, ArrowNormal ) );
        const QColor color = palette.color( QPalette::WindowText );
        const qreal penThickness = 1.5;

        // render arrow
        painter->setRenderHint( QPainter::Antialiasing );
        painter->setPen( QPen( color, penThickness ) );
        painter->setBrush( Qt::NoBrush );
        painter->translate( rect.center() );
        painter->drawPolyline( arrow );

        return true;
    }

    //______________________________________________________________
    bool Style::drawPanelButtonCommandPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        const State& flags( option->state );
        const bool enabled( flags & State_Enabled );
        const bool mouseOver( enabled && ( flags & State_MouseOver ) );
        const bool hasFocus( enabled && ( flags & State_HasFocus ) );
        const bool sunken( flags & ( State_On|State_Sunken ) );
        QPalette palette( option->palette );

        // update button color from palette in case button is default
        const QStyleOptionButton* buttonOption( qstyleoption_cast< const QStyleOptionButton* >( option ) );
        if( enabled && buttonOption && buttonOption->features & QStyleOptionButton::DefaultButton )
        {
            const QColor button( palette.color( QPalette::Button ) );
            const QColor base( palette.color( QPalette::Base ) );
            palette.setColor( QPalette::Button, KColorUtils::mix( button, base, 0.7 ) );
        }

        // update animation state
        // mouse over takes precedence over focus
        _animations->widgetStateEngine().updateState( widget, AnimationHover, mouseOver );
        _animations->widgetStateEngine().updateState( widget, AnimationFocus, hasFocus && !mouseOver );

        const AnimationMode mode( _animations->widgetStateEngine().buttonAnimationMode( widget ) );
        const qreal opacity( _animations->widgetStateEngine().buttonOpacity( widget ) );

        const QColor shadow( _helper->alphaColor( palette.color( QPalette::Shadow ), 0.2 ) );
        const QColor outline( _helper->buttonOutlineColor( palette, mouseOver, hasFocus, opacity, mode ) );
        const QColor color( _helper->buttonPanelColor( palette, mouseOver, hasFocus, opacity, mode ) );

        // render
        _helper->renderButtonFrame( painter, option->rect, color, outline, shadow, hasFocus, sunken );

        return true;

    }

    //______________________________________________________________
    bool Style::drawPanelButtonToolPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        /*
        For toolbutton in TabBars, corresponding to expanding arrows, no frame is drawn
        However one needs to draw the window background, because the button rect might
        overlap with some tab below. ( this is a Qt bug )
        */
        const QTabBar* tabBar( widget ? qobject_cast<const QTabBar*>( widget->parent() ):nullptr );
        if( tabBar )
        {
            QRect rect( option->rect );

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

            painter->setPen( Qt::NoPen );
            painter->setBrush( tabBar->palette().color( QPalette::Window ) );
            painter->drawRect( rect );
            return true;
        }

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorCheckBoxPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // get rect
        const State& flags( option->state );
        const bool enabled( flags & State_Enabled );
        const bool mouseOver( enabled && ( flags & State_MouseOver ) );
        const bool sunken( flags & State_Sunken );

        Helper::CheckBoxState state( Helper::CheckOff );
        if( flags & State_NoChange ) state =Helper:: CheckPartial;
        else if( flags & State_On ) state = Helper::CheckOn;

        // color
        const QPalette& palette( option->palette );
        const QColor base( palette.color( QPalette::Window ) );
        const QColor disabled( KColorUtils::mix( base, palette.color( QPalette::WindowText ), 0.4 ) );
        const QColor normal( KColorUtils::mix( base, palette.color( QPalette::WindowText ), 0.5 ) );
        const QColor active( _helper->viewFocusBrush().brush( palette ).color() );
        const QColor hover( _helper->viewHoverBrush().brush( palette ).color() );
        QColor color;

        // update only mouse over state
        _animations->widgetStateEngine().updateState( widget, AnimationHover, mouseOver );

        // cannot use transparent colors because of the rendering of the shadow
        if( !enabled ) color = disabled;
        else if( _animations->widgetStateEngine().isAnimated( widget, AnimationHover ) )
        {

            const qreal opacity( _animations->widgetStateEngine().opacity( widget, AnimationHover ) );
            if( state != Helper::CheckOff ) color = KColorUtils::mix( active, hover, opacity );
            else color = KColorUtils::mix( normal, hover, opacity );

        } else if( mouseOver ) color = hover;
        else if( state != Helper::CheckOff ) color = active;
        else color = normal;

        // shadow color
        const QColor shadow( _helper->alphaColor( palette.color( QPalette::Shadow ), 0.2 ) );

        // render
        _helper->renderCheckBox( painter, option->rect, color, shadow, sunken, state );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawIndicatorRadioButtonPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // get rect
        const State& flags( option->state );
        const bool enabled( flags & State_Enabled );
        const bool mouseOver( enabled && ( flags & State_MouseOver ) );
        const bool sunken( flags & State_Sunken );
        const bool checked( flags & State_On );

        // color
        const QPalette& palette( option->palette );
        const QColor base( palette.color( QPalette::Window ) );
        const QColor disabled( KColorUtils::mix( base, palette.color( QPalette::WindowText ), 0.4 ) );
        const QColor normal( KColorUtils::mix( base, palette.color( QPalette::WindowText ), 0.5 ) );
        const QColor active( _helper->viewFocusBrush().brush( palette ).color() );
        const QColor hover( _helper->viewHoverBrush().brush( palette ).color() );
        QColor color;

        // update only mouse over
        _animations->widgetStateEngine().updateState( widget, AnimationHover, mouseOver );

        // cannot use transparent colors because of the rendering of the shadow
        if( !enabled ) color = disabled;
        else if( _animations->widgetStateEngine().isAnimated( widget, AnimationHover ) )
        {

            const qreal opacity( _animations->widgetStateEngine().opacity( widget, AnimationHover ) );
            if( checked ) color = KColorUtils::mix( active, hover, opacity );
            else color = KColorUtils::mix( normal, hover, opacity );

        } else if( mouseOver ) color = hover;
        else if( checked ) color = active;
        else color = normal;

        // shadow color
        const QColor shadow( _helper->alphaColor( palette.color( QPalette::Shadow ), 0.2 ) );

        // render
        _helper->renderRadioButton( painter, option->rect, color, shadow, sunken, checked );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawPushButtonLabelControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionButton* buttonOption( qstyleoption_cast<const QStyleOptionButton*>( option ) );
        if( !buttonOption ) return false;

        // need to alter palette for focused buttons
        const bool hasFocus( option->state & State_HasFocus );
        const bool mouseOver( option->state & State_MouseOver );
        if( mouseOver || !hasFocus ) return false;

        // copy option, alter palette, and call base class method
        QStyleOptionButton copy( *buttonOption );
        copy.palette.setColor( QPalette::ButtonText, copy.palette.color( QPalette::HighlightedText ) );
        KStyle::drawControl( CE_PushButtonLabel, &copy, painter, widget );
        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawComboBoxLabelControl( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        const QStyleOptionComboBox* comboBoxOption( qstyleoption_cast<const QStyleOptionComboBox*>( option ) );
        if( !comboBoxOption ) return false;
        if( comboBoxOption->editable ) return false;

        // need to alter palette for focused buttons
        const bool hasFocus( option->state & State_HasFocus );
        const bool mouseOver( option->state & State_MouseOver );
        if( hasFocus && !mouseOver )
        { painter->setPen( QPen( option->palette.color( QPalette::HighlightedText ), 1 ) ); }

        return false;

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
            indicatorRect = handleRTL( option, indicatorRect );
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
        const State& flags( option->state );
        const bool enabled( flags&State_Enabled );

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
        const State& flags( option->state );
        const bool horizontal( flags & State_Horizontal );
        if( horizontal ) handleRect = centerRect( option->rect, option->rect.width(), Metrics::ScrollBar_SliderWidth );
        else handleRect = centerRect( option->rect, Metrics::ScrollBar_SliderWidth, option->rect.height() );

        const bool enabled( flags&State_Enabled );
        const bool mouseOver( enabled && ( flags&State_MouseOver ) );

        QWidget* parent( scrollBarParent( widget ) );
        const bool focus( enabled && parent && parent->hasFocus() );

        // enable animation state
        _animations->scrollBarEngine().updateState( widget, enabled && ( sliderOption->activeSubControls & SC_ScrollBarSlider ) );
        const qreal opacity( _animations->scrollBarEngine().opacity( widget, SC_ScrollBarSlider ) );
        if( widget )
        {
            // render background
            // it is necessary to retrieve the complete widget rect, in order to properly handle overlaps
            // at the scrollbar boundary
            // define color
            const QPalette& palette( option->palette );
            const QColor color( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.3 ) );

            // adjust rect
            QStyleOptionSlider copy( *sliderOption );
            copy.rect = widget->rect();
            QRect backgroundRect( scrollBarSubControlRect( &copy, SC_ScrollBarGroove, widget ) );
            if( horizontal ) backgroundRect = centerRect( backgroundRect, backgroundRect.width(), Metrics::ScrollBar_SliderWidth );
            else backgroundRect = centerRect( backgroundRect, Metrics::ScrollBar_SliderWidth, backgroundRect.height() );

            _helper->renderScrollBarGroove( painter, backgroundRect, color );

        }

        {
            // render handle
            // define colors
            QColor color;
            const QPalette& palette( option->palette );

            const QColor base( focus ?
                _helper->viewFocusBrush().brush( palette ).color():
                _helper->alphaColor( palette.color( QPalette::WindowText ), 0.5 ) );

            const QColor highlight( _helper->viewHoverBrush().brush( palette ).color() );
            if( opacity >= 0 ) color = KColorUtils::mix( base, highlight, opacity );
            else if( mouseOver ) color = highlight;
            else color = base;

            // render
            _helper->renderScrollBarHandle( painter, handleRect, color, QColor() );

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

        const State& flags( option->state );
        const bool horizontal( flags & State_Horizontal );
        const bool reverseLayout( option->direction == Qt::RightToLeft );

        // adjust rect, based on number of buttons to be drawn
        QRect rect( scrollBarInternalSubControlRect( sliderOption, SC_ScrollBarAddLine ) );

        QColor color;
        QStyleOptionSlider localOption( *sliderOption );
        if( _addLineButtons == DoubleButton )
        {

            if( horizontal )
            {

                //Draw the arrows
                const QSize halfSize( rect.width()/2, rect.height() );
                const QRect leftSubButton( rect.topLeft(), halfSize );
                const QRect rightSubButton( leftSubButton.topRight() + QPoint( 1, 0 ), halfSize );

                localOption.rect = leftSubButton;
                color = scrollBarArrowColor( &localOption,  reverseLayout ? SC_ScrollBarAddLine:SC_ScrollBarSubLine, widget );
                renderScrollBarArrow( painter, leftSubButton, color, ArrowLeft );

                localOption.rect = rightSubButton;
                color = scrollBarArrowColor( &localOption,  reverseLayout ? SC_ScrollBarSubLine:SC_ScrollBarAddLine, widget );
                renderScrollBarArrow( painter, rightSubButton, color, ArrowRight );

            } else {

                const QSize halfSize( rect.width(), rect.height()/2 );
                const QRect topSubButton( rect.topLeft(), halfSize );
                const QRect botSubButton( topSubButton.bottomLeft() + QPoint( 0, 1 ), halfSize );

                localOption.rect = topSubButton;
                color = scrollBarArrowColor( &localOption, SC_ScrollBarSubLine, widget );
                renderScrollBarArrow( painter, topSubButton, color, ArrowUp );

                localOption.rect = botSubButton;
                color = scrollBarArrowColor( &localOption, SC_ScrollBarAddLine, widget );
                renderScrollBarArrow( painter, botSubButton, color, ArrowDown );

            }

        } else if( _addLineButtons == SingleButton ) {

            localOption.rect = rect;
            color = scrollBarArrowColor( &localOption,  SC_ScrollBarAddLine, widget );
            if( horizontal )
            {

                if( reverseLayout ) renderScrollBarArrow( painter, rect, color, ArrowLeft );
                else renderScrollBarArrow( painter, rect.translated( 1, 0 ), color, ArrowRight );

            } else renderScrollBarArrow( painter, rect.translated( 0, 1 ), color, ArrowDown );

        }

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawScrollBarAddPageControl( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionSlider* sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if ( !sliderOption ) return true;

        painter->setClipRect( option->rect );

        const QPalette& palette( option->palette );
        const QColor color( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.3 ) );
        const State& flags( option->state );

        // define tiles and adjust rect
        QRect backgroundRect;
        const bool horizontal( flags & State_Horizontal );
        const bool reverseLayout( sliderOption->direction == Qt::RightToLeft );

        if( horizontal )
        {

            backgroundRect = centerRect( option->rect, option->rect.width(), Metrics::ScrollBar_SliderWidth );
            if( reverseLayout ) backgroundRect.adjust( 0, 0, Metrics::ScrollBar_SliderWidth/2, 0 );
            else backgroundRect.adjust( -Metrics::ScrollBar_SliderWidth/2, 0, 0, 0 );


        } else {

            backgroundRect = centerRect( option->rect, Metrics::ScrollBar_SliderWidth, option->rect.height() );
            backgroundRect.adjust( 0, -Metrics::ScrollBar_SliderWidth/2, 0, 0 );

        }

        // render
        _helper->renderScrollBarGroove( painter, backgroundRect, color );

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

        const State& flags( option->state );
        const bool horizontal( flags & State_Horizontal );
        const bool reverseLayout( option->direction == Qt::RightToLeft );

        // colors
        const QPalette& palette( option->palette );
        const QColor background( palette.color( QPalette::Window ) );

        // adjust rect, based on number of buttons to be drawn
        QRect rect( scrollBarInternalSubControlRect( sliderOption, SC_ScrollBarSubLine ) );

        QColor color;
        QStyleOptionSlider localOption( *sliderOption );
        if( _subLineButtons == DoubleButton )
        {

            if( horizontal )
            {

                //Draw the arrows
                const QSize halfSize( rect.width()/2, rect.height() );
                const QRect leftSubButton( rect.topLeft(), halfSize );
                const QRect rightSubButton( leftSubButton.topRight() + QPoint( 1, 0 ), halfSize );

                localOption.rect = leftSubButton;
                color = scrollBarArrowColor( &localOption,  reverseLayout ? SC_ScrollBarAddLine:SC_ScrollBarSubLine, widget );
                renderScrollBarArrow( painter, leftSubButton, color, ArrowLeft );

                localOption.rect = rightSubButton;
                color = scrollBarArrowColor( &localOption,  reverseLayout ? SC_ScrollBarSubLine:SC_ScrollBarAddLine, widget );
                renderScrollBarArrow( painter, rightSubButton, color, ArrowRight );

            } else {

                const QSize halfSize( rect.width(), rect.height()/2 );
                const QRect topSubButton( rect.topLeft(), halfSize );
                const QRect botSubButton( topSubButton.bottomLeft() + QPoint( 0, 1 ), halfSize );

                localOption.rect = topSubButton;
                color = scrollBarArrowColor( &localOption, SC_ScrollBarSubLine, widget );
                renderScrollBarArrow( painter, topSubButton, color, ArrowUp );

                localOption.rect = botSubButton;
                color = scrollBarArrowColor( &localOption, SC_ScrollBarAddLine, widget );
                renderScrollBarArrow( painter, botSubButton, color, ArrowDown );

            }

        } else if( _subLineButtons == SingleButton ) {

            localOption.rect = rect;
            color = scrollBarArrowColor( &localOption,  SC_ScrollBarSubLine, widget );
            if( horizontal )
            {

                if( reverseLayout ) renderScrollBarArrow( painter, rect.translated( 1, 0 ), color, ArrowRight );
                else renderScrollBarArrow( painter, rect, color, ArrowLeft );

            } else renderScrollBarArrow( painter, rect, color, ArrowUp );

        }

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawScrollBarSubPageControl( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionSlider* sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if ( !sliderOption ) return true;

        painter->setClipRect( option->rect );

        const QPalette& palette( option->palette );
        const QColor color( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.3 ) );
        const State& flags( option->state );

        // define tiles and adjust rect
        QRect backgroundRect;
        const bool horizontal( flags & State_Horizontal );
        const bool reverseLayout( sliderOption->direction == Qt::RightToLeft );

        if( horizontal )
        {

            backgroundRect = centerRect( option->rect, option->rect.width(), Metrics::ScrollBar_SliderWidth );
            if( reverseLayout ) backgroundRect.adjust( -Metrics::ScrollBar_SliderWidth/2, 0, 0, 0 );
            else backgroundRect.adjust( 0, 0, Metrics::ScrollBar_SliderWidth/2-1, 0 );

        } else {

            backgroundRect = centerRect( option->rect, Metrics::ScrollBar_SliderWidth, option->rect.height() );
            backgroundRect.adjust( 0, 0, 0, Metrics::ScrollBar_SliderWidth/2-1 );

        }

        // render
        _helper->renderScrollBarGroove( painter, backgroundRect, color );

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
            {
                const QPalette& palette( option->palette );
                const QRect rect( option->rect );
                const QColor color( KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::WindowText ), 0.25 ) );
                painter->setBrush( Qt::NoBrush );
                painter->setPen( QPen( color, 1 ) );
                painter->drawLine( rect.topLeft(), rect.topRight() );
                return true;
            }

            case QFrame::VLine:
            {
                const QPalette& palette( option->palette );
                const QRect rect( option->rect );
                const QColor color( KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::WindowText ), 0.25 ) );
                painter->setBrush( Qt::NoBrush );
                painter->setPen( QPen( color, 1 ) );
                painter->drawLine( rect.topLeft(), rect.bottomLeft() );
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
        const State& flags( option->state );
        const bool enabled( flags & State_Enabled );
        const bool mouseOver( enabled && ( flags & State_MouseOver ) );
        const bool sunken( enabled && ( flags & (State_On|State_Sunken) ) );

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
        const QColor normal( palette.color( QPalette::Window ) );
        const QColor focus( KColorUtils::mix( normal, _helper->viewFocusBrush().brush( palette ).color(), 0.2 ) );
        const QColor hover( KColorUtils::mix( normal, _helper->viewHoverBrush().brush( palette ).color(), 0.2 ) );

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
        painter->setBrush( palette.color( QPalette::Window ) );
        painter->setPen( Qt::NoPen );
        painter->drawRect( rect );

        // outline
        painter->setBrush( Qt::NoBrush );
        painter->setPen( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.1 ) );

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

        // check focus
        const State& flags( option->state );
        const bool hasFocus( flags & State_HasFocus );
        const bool selected( flags & State_Selected );
        if( !( hasFocus && selected ) ) return true;

        // code is copied from QCommonStyle, but adds focus
        // cast option and check
        const QStyleOptionTab *tabOption( qstyleoption_cast<const QStyleOptionTab*>(option) );
        if( !tabOption || tabOption->text.isEmpty() ) return true;

        // tab option rect
        QRect rect = tabOption->rect;
        const bool verticalTabs( isVerticalTab( tabOption ) );
        const int alignment = Qt::AlignCenter | Qt::TextHideMnemonic;

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

        // render focus line
        painter->translate( 0, 2 );
        painter->setPen( _helper->viewFocusBrush().brush( option->palette ).color() );
        painter->drawLine( textRect.bottomLeft(), textRect.bottomRight() );

        if( verticalTabs ) painter->restore();

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawTabBarTabShapeControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        const QStyleOptionTab* tabOption( qstyleoption_cast<const QStyleOptionTab*>( option ) );
        if( !tabOption ) return true;

        // palette and flags
        const QPalette& palette( option->palette );
        const State& flags( option->state );
        const bool enabled( flags & State_Enabled );
        const bool selected( flags & State_Selected );
        const bool mouseOver( enabled && !selected && ( flags & State_MouseOver ) );

        // check if tab is being dragged
        const bool isDragged( widget && selected && painter->device() != widget );
        const bool isLocked( widget && _tabBarData->isLocked( widget ) );

        // store rect
        QRect rect( option->rect );

        // animation state
        _animations->tabBarEngine().updateState( widget, rect.topLeft(), mouseOver );
        const bool animated( enabled && !selected && _animations->tabBarEngine().isAnimated( widget, rect.topLeft() ) );
        const qreal opacity( _animations->tabBarEngine().opacity( widget, rect.topLeft() ) );

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

        // swap flags based on reverse layout, so that they become layout independent
        const bool reverseLayout( option->direction == Qt::RightToLeft );
        const bool verticalTabs( isVerticalTab( tabOption ) );
        if( reverseLayout && !verticalTabs )
        {
            qSwap( isFirst, isLast );
            qSwap( isLeftOfSelected, isRightOfSelected );
        }

        // adjust rect and define corners based on tabbar orientation
        Helper::Corners corners;
        switch( tabOption->shape )
        {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
            if( selected ) {

                corners = Helper::CornerTopLeft|Helper::CornerTopRight;
                rect.adjust( 0, 0, 0, Metrics::TabBar_TabRadius );

            } else {

                rect.adjust( 0, 0, 0, -1 );
                if( isFirst ) corners |= Helper::CornerTopLeft;
                if( isLast ) corners |= Helper::CornerTopRight;
                if( isRightOfSelected ) rect.adjust( -Metrics::TabBar_TabRadius, 0, 0, 0 );
                if( isLeftOfSelected ) rect.adjust( 0, 0, Metrics::TabBar_TabOverlap, 0 );
                else if( !isLast ) rect.adjust( 0, 0, Metrics::TabBar_TabOverlap, 0 );

            }
            break;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            if( selected ) {

                corners = Helper::CornerBottomLeft|Helper::CornerBottomRight;
                rect.adjust( 0, -Metrics::TabBar_TabRadius, 0, 0 );

            } else {

                rect.adjust( 0, 1, 0, 0 );
                if( isFirst ) corners |= Helper::CornerBottomLeft;
                if( isLast ) corners |= Helper::CornerBottomRight;
                if( isRightOfSelected ) rect.adjust( -Metrics::TabBar_TabRadius, 0, 0, 0 );
                if( isLeftOfSelected ) rect.adjust( 0, 0, Metrics::TabBar_TabOverlap, 0 );
                else if( !isLast ) rect.adjust( 0, 0, Metrics::TabBar_TabOverlap, 0 );

            }
            break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            if( selected )
            {
                corners = Helper::CornerTopLeft|Helper::CornerBottomLeft;
                rect.adjust( 0, 0, Metrics::TabBar_TabRadius, 0 );

            } else {

                rect.adjust( 0, 0, -1, 0 );
                if( isFirst ) corners |= Helper::CornerTopLeft;
                if( isLast ) corners |= Helper::CornerBottomLeft;
                if( isRightOfSelected ) rect.adjust( 0, -Metrics::TabBar_TabRadius, 0, 0 );
                if( isLeftOfSelected ) rect.adjust( 0, 0, 0, Metrics::TabBar_TabRadius );
                else if( !isLast ) rect.adjust( 0, 0, 0, Metrics::TabBar_TabOverlap );

            }
            break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            if( selected )
            {

                corners = Helper::CornerTopRight|Helper::CornerBottomRight;
                rect.adjust( -Metrics::TabBar_TabRadius, 0, 0, 0 );

            } else {

                rect.adjust( 1, 0, 0, 0 );
                if( isFirst ) corners |= Helper::CornerTopRight;
                if( isLast ) corners |= Helper::CornerBottomRight;
                if( isRightOfSelected ) rect.adjust( 0, -Metrics::TabBar_TabRadius, 0, 0 );
                if( isLeftOfSelected ) rect.adjust( 0, 0, 0, Metrics::TabBar_TabRadius );
                else if( !isLast ) rect.adjust( 0, 0, 0, Metrics::TabBar_TabOverlap );

            }
            break;

            default: break;
        }

        // color
        QColor color;
        if( selected ) color = KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::Base ), 0.3 );
        else {

            const QColor normal( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.2 ) );
            const QColor hover( _helper->alphaColor( _helper->viewHoverBrush().brush( palette ).color(), 0.2 ) );
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

    //______________________________________________________________
    bool Style::drawComboBoxComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionComboBox* comboBoxOption( qstyleoption_cast<const QStyleOptionComboBox*>( option ) );
        if( !comboBoxOption ) return true;

        const State& flags( option->state );
        const QPalette& palette( option->palette );
        const bool enabled( flags & State_Enabled );
        const bool mouseOver( enabled && ( flags & State_MouseOver ) );
        const bool hasFocus( flags & State_HasFocus );
        const bool editable( comboBoxOption->editable );
        const bool flat( editable && !comboBoxOption->frame );

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
                    painter->drawRect( option->rect );

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
                    _helper->renderFrame( painter, option->rect, color, outline, hasFocus );
                }

            } else {

                // read only comboboxes. Make it look like a button

                // update animation state
                // hover takes precedence over focus
                _animations->lineEditEngine().updateState( widget, AnimationHover, mouseOver );
                _animations->lineEditEngine().updateState( widget, AnimationFocus, hasFocus && !mouseOver );

                const AnimationMode mode( _animations->lineEditEngine().buttonAnimationMode( widget ) );
                const qreal opacity( _animations->lineEditEngine().buttonOpacity( widget ) );

                const QColor shadow( _helper->alphaColor( palette.color( QPalette::Shadow ), 0.2 ) );
                const QColor outline( _helper->buttonOutlineColor( palette, mouseOver, hasFocus, opacity, mode ) );
                const QColor color( _helper->buttonPanelColor( palette, mouseOver, hasFocus, opacity, mode ) );

                const bool sunken( flags & ( State_On|State_Sunken ) );

                _helper->renderButtonFrame( painter, option->rect, color, outline, shadow, hasFocus, sunken );

            }

        }

        // arrow
        if( option->subControls & SC_ComboBoxArrow )
        {

            // detect empty comboboxes
            const QComboBox* comboBox = qobject_cast<const QComboBox*>( widget );
            const bool empty( comboBox && !comboBox->count() );

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
                    const QColor hover( _helper->viewHoverBrush().brush( palette ).color() );

                    if( animated )
                    {
                        arrowColor = KColorUtils::mix( normal, hover, opacity );

                    } else if( subControlHover ) {

                        arrowColor = hover;

                    } else arrowColor = normal;

                }

            } else {

                if( empty || !enabled ) arrowColor = palette.color( QPalette::Disabled, QPalette::Text );
                else if( hasFocus && !mouseOver ) arrowColor = palette.color( QPalette::HighlightedText );
                else arrowColor = palette.color( QPalette::ButtonText );

            }

            const QRectF arrowRect( comboBoxSubControlRect( option, SC_ComboBoxArrow, widget ) );

            const QPolygonF arrow( genericArrow( ArrowDown, ArrowNormal ) );
            const qreal penThickness( 1.5 );

            painter->save();
            painter->translate( arrowRect.center() );
            painter->setRenderHint( QPainter::Antialiasing );
            painter->setPen( QPen( arrowColor, penThickness ) );
            painter->drawPolyline( arrow );
            painter->restore();

        }

        return true;

    }

    //______________________________________________________________
    bool Style::drawSpinBoxComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        const QStyleOptionSpinBox *spinBoxOption( qstyleoption_cast<const QStyleOptionSpinBox*>( option ) );
        if( !spinBoxOption ) return true;

        const State& flags( option->state );
        const QPalette& palette( option->palette );
        const bool enabled( flags & State_Enabled );
        const bool mouseOver( enabled && ( flags & State_MouseOver ) );
        const bool hasFocus( flags & State_HasFocus );
        const bool flat( !spinBoxOption->frame );

        if( option->subControls & SC_SpinBoxFrame )
        {

            const QColor color( palette.color( QPalette::Base ) );
            if( flat )
            {

                painter->setBrush( color );
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
                _helper->renderFrame( painter, option->rect, color, outline, hasFocus );
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
        const State& flags( option->state );
        const bool enabled( flags & State_Enabled );
        const bool mouseOver( enabled && ( flags & State_MouseOver ) );
        const bool hasFocus( enabled && ( flags & State_HasFocus ) );

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
            const QColor color( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.3 ) );

            if( !enabled ) _helper->renderSliderGroove( painter, grooveRect, color );
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
                    leftRect.setRight( handleRect.right()-1 );
                    _helper->renderSliderGroove( painter, leftRect, reverseLayout ? color:highlight );

                    QRect rightRect( grooveRect );
                    rightRect.setLeft( handleRect.left()+1 );
                    _helper->renderSliderGroove( painter, rightRect, reverseLayout ? highlight:color );

                } else {

                    QRect topRect( grooveRect );
                    topRect.setBottom( handleRect.bottom()-1 );
                    _helper->renderSliderGroove( painter, topRect, highlight );

                    QRect bottomRect( grooveRect );
                    bottomRect.setTop( handleRect.top()+1 );
                    _helper->renderSliderGroove( painter, bottomRect, color );

                }

            }

        }

        // handle
        if ( sliderOption->subControls & SC_SliderHandle )
        {

            // get rect and center
            QRect handleRect( subControlRect( CC_Slider, sliderOption, SC_SliderHandle, widget ) );
            handleRect = centerRect( handleRect, Metrics::Slider_ControlThickness, Metrics::Slider_ControlThickness );

            const bool handleActive( sliderOption->activeSubControls & SC_SliderHandle );

            // define colors
            const QColor color( palette.color( QPalette::Button ) );
            const QColor shadow( _helper->alphaColor( palette.color( QPalette::Shadow ), 0.2 ) );
            QColor outline;

            _animations->sliderEngine().updateState( widget, enabled && handleActive );
            const bool animated( _animations->sliderEngine().isAnimated( widget ) );
            const qreal opacity( _animations->sliderEngine().opacity( widget ) );

            const QColor hover( _helper->viewHoverBrush().brush( option->palette ).color() );
            const QColor focus( _helper->viewFocusBrush().brush( option->palette ).color() );
            const QColor defaultOutline( KColorUtils::mix( palette.color( QPalette::Button ), palette.color( QPalette::ButtonText ), 0.4 ) );

            if( animated )
            {

                if( hasFocus ) outline = KColorUtils::mix( focus, hover, opacity );
                else outline = KColorUtils::mix( defaultOutline, hover, opacity );

            } else if( handleActive && mouseOver ) outline = hover;
            else if( hasFocus ) outline = focus;
            else outline = defaultOutline;

            const bool sunken( flags & (State_On|State_Sunken) );
            _helper->renderSliderHandle( painter, handleRect, color, outline, shadow, hasFocus, sunken );

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
        const State& flags( option->state );
        const bool enabled( flags & State_Enabled );
        const bool mouseOver( enabled && ( flags & State_MouseOver ) );
        const bool hasFocus( enabled && ( flags & State_HasFocus ) );

        // do not render tickmarks
        if( sliderOption->subControls & SC_DialTickmarks )
        {}

        // groove
        if( sliderOption->subControls & SC_DialGroove )
        {

            // groove rect
            QRect grooveRect( subControlRect( CC_Dial, sliderOption, SC_SliderGroove, widget ) );

            // handle rect
            // base color
            const QColor color( KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::WindowText ), 0.3 ) );

            // render groove
            _helper->renderDialGroove( painter, grooveRect, color );

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

            // update animatiosn handle rect
            _animations->dialEngine().setHandleRect( widget, handleRect );
            const bool handleActive( mouseOver && handleRect.contains( _animations->dialEngine().position( widget ) ) );

            // define colors
            const QColor color( palette.color( QPalette::Button ) );
            const QColor shadow( _helper->alphaColor( palette.color( QPalette::Shadow ), 0.2 ) );
            QColor outline;

            _animations->dialEngine().updateState( widget, enabled && handleActive );
            const bool animated( _animations->dialEngine().isAnimated( widget ) );
            const qreal opacity( _animations->dialEngine().opacity( widget ) );

            const QColor hover( _helper->viewHoverBrush().brush( option->palette ).color() );
            const QColor focus( _helper->viewFocusBrush().brush( option->palette ).color() );
            const QColor defaultOutline( KColorUtils::mix( palette.color( QPalette::Button ), palette.color( QPalette::ButtonText ), 0.4 ) );

            if( animated )
            {

                if( hasFocus ) outline = KColorUtils::mix( focus, hover, opacity );
                else outline = KColorUtils::mix( defaultOutline, hover, opacity );

            } else if( handleActive && mouseOver ) outline = hover;
            else if( hasFocus ) outline = focus;
            else outline = defaultOutline;

            const bool sunken( flags & (State_On|State_Sunken) );
            _helper->renderSliderHandle( painter, handleRect, color, outline, shadow, hasFocus, sunken );

        }

        return true;
    }

    //______________________________________________________________________________
    void Style::renderScrollBarArrow(
        QPainter* painter, const QRect& rect, const QColor& color,
        ArrowOrientation orientation ) const
    {
        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        const qreal penThickness( 1.5 );
        const QPolygonF arrow( genericArrow( orientation, ArrowNormal ) );

        const QColor base( color );

        painter->save();
        painter->translate( QRectF(rect).center() );

        painter->setPen( QPen( base, penThickness, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );
        painter->drawPolyline( arrow );
        painter->restore();

        return;

    }

    //____________________________________________________________________________________________________
    void Style::renderSpinBoxArrow( QPainter* painter, const QStyleOptionSpinBox* option, const QWidget* widget, const SubControl& subControl ) const
    {

        const QPalette& palette( option->palette );
        const State& flags( option->state );

        // enable state
        bool enabled( flags & State_Enabled );

        // check steps enable step
        const bool atLimit(
            (subControl == SC_SpinBoxUp && !(option->stepEnabled & QAbstractSpinBox::StepUpEnabled )) ||
            (subControl == SC_SpinBoxDown && !(option->stepEnabled & QAbstractSpinBox::StepDownEnabled ) ) );

        // update enabled state accordingly
        enabled &= !atLimit;

        // update mouse-over effect
        const bool mouseOver( enabled && ( flags & State_MouseOver ) );

        // check animation state
        const bool subControlHover( enabled && mouseOver && ( option->activeSubControls & subControl ) );
        _animations->spinBoxEngine().updateState( widget, subControl, subControlHover );

        const bool animated( enabled && _animations->spinBoxEngine().isAnimated( widget, subControl ) );
        const qreal opacity( _animations->spinBoxEngine().opacity( widget, subControl ) );

        QColor color;
        if( animated )
        {

            QColor highlight = _helper->viewHoverBrush().brush( palette ).color();
            color = KColorUtils::mix( palette.color( QPalette::Text ), highlight, opacity );

        } else if( subControlHover ) {

            color = _helper->viewHoverBrush().brush( palette ).color();

        } else if( atLimit ) {

            color = palette.color( QPalette::Disabled, QPalette::Text );

        } else {

            color = palette.color( QPalette::Text );

        }

        const QPolygonF arrow( genericArrow( ( subControl == SC_SpinBoxUp ) ? ArrowUp:ArrowDown, ArrowNormal ) );
        const QRectF arrowRect( subControlRect( CC_SpinBox, option, subControl, widget ) );

        painter->save();
        painter->translate( arrowRect.center() );
        painter->setRenderHint( QPainter::Antialiasing );

        const qreal penThickness = 1.6;
        painter->setPen( QPen( color, penThickness ) );
        painter->drawPolyline( arrow );
        painter->restore();

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

            QColor highlight = _helper->viewHoverBrush().brush( palette ).color();
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
    QPolygonF Style::genericArrow( Style::ArrowOrientation orientation, Style::ArrowSize size ) const
    {

        QPolygonF a;
        switch( orientation )
        {
            case ArrowUp:
            {
                if( size == ArrowTiny ) a << QPointF( -1.75, 1.125 ) << QPointF( 0.5, -1.125 ) << QPointF( 2.75, 1.125 );
                else if( size == ArrowSmall ) a << QPointF( -2,1.5 ) << QPointF( 0.5, -1.5 ) << QPointF( 3,1.5 );
                else a << QPointF( -4,2 ) << QPointF( 0, -2 ) << QPointF( 4,2 );
                break;
            }

            case ArrowDown:
            {
                if( size == ArrowTiny ) a << QPointF( -1.75, -1.125 ) << QPointF( 0.5, 1.125 ) << QPointF( 2.75, -1.125 );
                else if( size == ArrowSmall ) a << QPointF( -2,-1.5 ) << QPointF( 0.5, 1.5 ) << QPointF( 3,-1.5 );
                else a << QPointF( -4,-2 ) << QPointF( 0, 2 ) << QPointF( 4,-2 );
                break;
            }

            case ArrowLeft:
            {
                if( size == ArrowTiny ) a << QPointF( 1.125, -1.75 ) << QPointF( -1.125, 0.5 ) << QPointF( 1.125, 2.75 );
                else if( size == ArrowSmall ) a << QPointF( 1.5,-2 ) << QPointF( -1.5, 0.5 ) << QPointF( 1.5,3 );
                else a << QPointF( 2, -4 ) << QPointF( -2, 0 ) << QPointF( 2, 4 );
                break;
            }

            case ArrowRight:
            {
                if( size == ArrowTiny ) a << QPointF( -1.125, -1.75 ) << QPointF( 1.125, 0.5 ) << QPointF( -1.125, 2.75 );
                else if( size == ArrowSmall ) a << QPointF( -1.5,-2 ) << QPointF( 1.5, 0.5 ) << QPointF( -1.5,3 );
                else a << QPointF( -2,-4 ) << QPointF( 2, 0 ) << QPointF( -2, 4 );
                break;
            }

            default: break;

        }

        return a;

    }

}
