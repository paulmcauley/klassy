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
#include <QGroupBox>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QSplitterHandle>
#include <QTextEdit>
#include <QToolBox>
#include <QToolButton>

Q_LOGGING_CATEGORY(BREEZE, "breeze")

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

        // enable hover effects for all necessary widgets
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

            // enable hover effects in itemviews' viewport
            itemView->viewport()->setAttribute( Qt::WA_Hover );

        } else if( QAbstractScrollArea* scrollArea = qobject_cast<QAbstractScrollArea*>( widget ) ) {

            // enable hover effect in sunken scrollareas that support focus
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

        }

        // remove opaque painting for scrollbars
        if( qobject_cast<QScrollBar*>( widget ) )
        {

            widget->setAttribute( Qt::WA_OpaquePaintEvent, false );

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
            case PM_DockWidgetFrameWidth:
            case PM_SpinBoxFrameWidth:
            return Metrics::Frame_FrameWidth;

            // buttons
            case PM_ButtonMargin: return Metrics::Button_MarginWidth;
            case PM_ButtonDefaultIndicator: return 0;
            case PM_ButtonShiftHorizontal: return 0;
            case PM_ButtonShiftVertical: return 0;

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


            // fallback
            default: return KStyle::pixelMetric( metric, option, widget );

        }

    }

    //______________________________________________________________
    int Style::styleHint( StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData ) const
    {
        switch( hint )
        {

            // mouse tracking
            case SH_ComboBox_ListMouseTracking: return true;
            case SH_MenuBar_MouseTracking: return true;
            case SH_Menu_MouseTracking: return true;

            // groupboxes
            case SH_GroupBox_TextLabelVerticalAlignment: return Qt::AlignVCenter;

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

            // checkboxes and radio buttons
            case PE_IndicatorCheckBox: fcn = &Style::drawIndicatorCheckBoxPrimitive; break;
            case PE_IndicatorRadioButton: fcn = &Style::drawIndicatorRadioButtonPrimitive; break;

            // frames
            case PE_FrameStatusBar: fcn = &Style::emptyPrimitive; break;
            case PE_Frame: fcn = &Style::drawFramePrimitive; break;
            case PE_FrameLineEdit: fcn = &Style::drawFramePrimitive; break;
            case PE_FrameGroupBox: fcn = &Style::drawFrameGroupBoxPrimitive; break;
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

            // size grip
            case CE_SizeGrip: fcn = &Style::emptyControl; break;

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

                const QPalette pal = _helper->mergePalettes( palette, _animations->widgetEnabilityEngine().opacity( widget, AnimationEnable )  );
                return KStyle::drawItemText( painter, r, flags, pal, enabled, text, textRole );

            }

        }

        // fallback
        return KStyle::drawItemText( painter, r, flags, palette, enabled, text, textRole );

    }


    //_____________________________________________________________________
    bool Style::eventFilter( QObject *object, QEvent *event )
    {
        // fallback
        return KStyle::eventFilter( object, event );
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
        const bool isFlat( frameOption->lineWidth == 0 );
        return isFlat ? option->rect : insideMargin( option->rect, Metrics::LineEdit_MarginWidth + Metrics::Frame_FrameWidth );
    }

    //___________________________________________________________________________________________________________________
    QRect Style::progressBarGrooveRect( const QStyleOption* option, const QWidget* ) const
    {

        // cast option
        const QStyleOptionProgressBar* progressBarOption = qstyleoption_cast<const QStyleOptionProgressBar*>( option );
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
        const QStyleOptionProgressBar* progressBarOption = qstyleoption_cast<const QStyleOptionProgressBar*>( option );
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
                const QStyleOptionGroupBox *groupBoxOption = qstyleoption_cast<const QStyleOptionGroupBox *>( option );
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
                const QStyleOptionGroupBox *groupBoxOption = qstyleoption_cast<const QStyleOptionGroupBox *>( option );
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

        QRect rect( option->rect );
        const QStyleOptionComboBox *comboBoxOption = qstyleoption_cast<const QStyleOptionComboBox *>( option );
        if( !comboBoxOption ) return KStyle::subControlRect( CC_ComboBox, option, subControl, widget );

        switch( subControl )
        {
            case SC_ComboBoxFrame: return comboBoxOption->frame ? rect : QRect();
            case SC_ComboBoxListBoxPopup: return rect;

            case SC_ComboBoxArrow:
            {

                // take out frame width
                rect = insideMargin( rect, Metrics::Frame_FrameWidth );

                QRect arrowRect;
                const bool editable( comboBoxOption->editable );
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
                rect = insideMargin( rect, Metrics::Frame_FrameWidth );

                QRect labelRect;
                const bool editable( comboBoxOption->editable );

                if( editable )
                {

                    labelRect = QRect(
                        rect.left(), rect.top(),
                        rect.width() - Metrics::ComboBox_ButtonWidth,
                        rect.height() );

                    // remove line editor margins
                    labelRect.adjust( Metrics::LineEdit_MarginWidth, Metrics::LineEdit_MarginWidth, 0, -Metrics::LineEdit_MarginWidth );

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

        QRect rect( option->rect );

        switch( subControl )
        {
            case SC_SpinBoxFrame: return rect;

            case SC_SpinBoxUp:
            case SC_SpinBoxDown:
            {

                // take out frame width
                rect = insideMargin( rect, Metrics::Frame_FrameWidth );

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
                rect = insideMargin( rect, Metrics::Frame_FrameWidth );

                QRect labelRect;
                labelRect = QRect(
                    rect.left(), rect.top(),
                    rect.width() - Metrics::SpinBox_ButtonWidth,
                    rect.height() );

                // remove line editor margins
                labelRect.adjust( Metrics::LineEdit_MarginWidth, Metrics::LineEdit_MarginWidth, 0, -Metrics::LineEdit_MarginWidth );

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
        const State& flags( option->state );
        const bool horizontal( flags&State_Horizontal );

        switch ( subControl )
        {

            case SC_ScrollBarSubLine:
            case SC_ScrollBarAddLine:
            return scrollBarInternalSubControlRect( option, subControl );

            //The main groove area. This is used to compute the others...
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
                const QStyleOptionSlider* sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
                if( !sliderOption ) return QRect();

                //We do handleRTL here to unreflect things if need be
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

            default: return QRect();
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

        // check flatness
        const bool isFlat( frameOption->lineWidth == 0 );
        if( isFlat ) return contentsSize;
        else return expandSize( contentsSize, Metrics::LineEdit_MarginWidth + Metrics::Frame_FrameWidth );
    }

    //______________________________________________________________
    QSize Style::comboBoxSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionComboBox* comboBoxOption( qstyleoption_cast<const QStyleOptionComboBox *>( option ) );
        if( !comboBoxOption ) return contentsSize;

        QSize size( contentsSize );

        // add relevant margins
        const bool editable( comboBoxOption->editable );
        if( editable ) size = expandSize( size, Metrics::LineEdit_MarginWidth );
        else size = expandSize( size, Metrics::ComboBox_MarginWidth );

        // make sure there is enough height for the button
        size.setHeight( qMax( size.height(), (int)Metrics::ComboBox_ButtonWidth ) );

        // add button width and spacing
        size.rwidth() += Metrics::ComboBox_ButtonWidth;
        if( !editable ) size.rwidth() += Metrics::ComboBox_BoxTextSpace;

        // add framewidth
        return expandSize( size, Metrics::Frame_FrameWidth );
    }

    //______________________________________________________________
    QSize Style::spinBoxSizeFromContents( const QStyleOption*, const QSize& contentsSize, const QWidget* ) const
    {

        QSize size( contentsSize );

        // add editor margins
        size = expandSize( size, Metrics::LineEdit_MarginWidth );

        // make sure there is enough height for the button
        size.setHeight( qMax( size.height(), (int)Metrics::SpinBox_ButtonWidth ) );

        // add button width and spacing
        size.rwidth() += Metrics::SpinBox_ButtonWidth;

        // add framewidth
        return expandSize( size, Metrics::Frame_FrameWidth );

    }

    //______________________________________________________________
    QSize Style::pushButtonSizeFromContents( const QStyleOption*, const QSize& contentsSize, const QWidget* ) const
    { return expandSize( contentsSize, Metrics::Button_MarginWidth + Metrics::Frame_FrameWidth ); }

    //______________________________________________________________
    QSize Style::progressBarSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget* ) const
    {

        // cast option
        const QStyleOptionProgressBar* progressBarOption = qstyleoption_cast<const QStyleOptionProgressBar*>( option );
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
    bool Style::drawFramePrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        const State& flags( option->state );

        const bool isQtQuickControl = !widget && option && option->styleObject && option->styleObject->inherits( "QQuickStyleItem" );
        const bool isInputWidget( ( widget && widget->testAttribute( Qt::WA_Hover ) )
                                  || ( isQtQuickControl && option->styleObject->property( "elementType" ).toString() == QStringLiteral( "edit") ) );

        const bool enabled( flags & State_Enabled );
        const bool mouseOver( enabled && isInputWidget && ( flags&State_MouseOver ) );
        const bool hasFocus( enabled && ( flags&State_HasFocus ) );

        // focus takes precedence over hover
        _animations->lineEditEngine().updateState( widget, AnimationFocus, hasFocus );
        _animations->lineEditEngine().updateState( widget, AnimationHover, mouseOver && !hasFocus );

        // do nothing for flat frames
        if( !( flags & (State_Sunken | State_Raised ) ) ) return true;

        // retrieve animation mode and opacity
        const AnimationMode mode( _animations->lineEditEngine().frameAnimationMode( widget ) );
        const qreal opacity( _animations->lineEditEngine().frameOpacity( widget ) );

        // update frame shadow factory
        if( _frameShadowFactory->isRegistered( widget ) )
        { _frameShadowFactory->updateState( widget, hasFocus, mouseOver, opacity, mode ); }

        // render
        const QColor outline( _helper->frameOutlineColor( option->palette, mouseOver, hasFocus, opacity, mode ) );
        _helper->renderFrame( painter, option->rect, QColor(), outline, hasFocus );

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
            painter->setPen( _helper->viewFocusBrush().brush( option->palette.currentColorGroup() ).color() );
            painter->drawLine( option->rect.bottomLeft(), option->rect.bottomRight() );
        }

        return true;

    }

    //______________________________________________________________
    bool Style::drawFrameGroupBoxPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionFrame *frameOption = qstyleoption_cast<const QStyleOptionFrame *>( option );
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

    //______________________________________________________________
    bool Style::drawPanelButtonCommandPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        const State& flags( option->state );
        const bool enabled( flags & State_Enabled );
        const bool mouseOver( enabled && ( flags & State_MouseOver ) );
        const bool hasFocus( enabled && ( flags & State_HasFocus ) );
        const bool sunken( flags & ( State_On|State_Sunken ) );
        const QPalette& palette( option->palette );

        // update animation state
        // hover takes precedence over focus
        _animations->widgetStateEngine().updateState( widget, AnimationHover, mouseOver );
        _animations->widgetStateEngine().updateState( widget, AnimationFocus, hasFocus && !mouseOver );

        const AnimationMode mode( _animations->widgetStateEngine().buttonAnimationMode( widget ) );
        const qreal opacity( _animations->widgetStateEngine().buttonOpacity( widget ) );

        const QColor shadow( _helper->alphaColor( palette.color( QPalette::Shadow ), 0.2 ) );
        const QColor outline( _helper->buttonOutlineColor( palette, mouseOver, hasFocus, opacity, mode ) );
        const QColor color( _helper->buttonPanelColor( palette, mouseOver, hasFocus, opacity, mode ) );

        // render
        _helper->renderButtonSlab( painter, option->rect, color, outline, shadow, hasFocus, sunken );

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
        const QColor active( _helper->viewFocusBrush().brush( palette.currentColorGroup() ).color() );
        const QColor hover( _helper->viewHoverBrush().brush( palette.currentColorGroup() ).color() );
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
        const QColor active( _helper->viewFocusBrush().brush( palette.currentColorGroup() ).color() );
        const QColor hover( _helper->viewHoverBrush().brush( palette.currentColorGroup() ).color() );
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
        const QStyleOptionButton* buttonOption = qstyleoption_cast<const QStyleOptionButton*>( option );
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

        const QStyleOptionComboBox* comboBoxOption = qstyleoption_cast<const QStyleOptionComboBox*>( option );
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

        const QStyleOptionProgressBar* progressBarOption = qstyleoption_cast<const QStyleOptionProgressBar*>( option );
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

        const QStyleOptionProgressBar* progressBarOption = qstyleoption_cast<const QStyleOptionProgressBar*>( option );
        if( !progressBarOption ) return true;

        const QStyleOptionProgressBarV2* progressBarOption2 = qstyleoption_cast<const QStyleOptionProgressBarV2*>( option );

        const QRect& rect( option->rect );
        const QPalette& palette( option->palette );

        // check if anything is to be drawn
        qreal progress = progressBarOption->progress - progressBarOption->minimum;
        const bool busyIndicator = ( progressBarOption->minimum == 0 && progressBarOption->maximum == 0 );
        if( busyIndicator ) progress = _animations->busyIndicatorEngine().value();

        if( busyIndicator )
        {

            const bool horizontal = !progressBarOption2 || progressBarOption2->orientation == Qt::Horizontal;
            const QColor first( palette.color( QPalette::Highlight ) );
            const QColor second( KColorUtils::mix( palette.color( QPalette::Highlight ), palette.color( QPalette::Window ), 0.7 ) );
            _helper->renderProgressBarBusyContents( painter, rect, first, second, horizontal, progress );

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
        _helper->renderProgressBarHole( painter, option->rect, color );
        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawProgressBarLabelControl( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {
        const QStyleOptionProgressBar* progressBarOption = qstyleoption_cast<const QStyleOptionProgressBar*>( option );
        if( !progressBarOption ) return true;

        const QRect& rect( option->rect );
        const QPalette& palette( option->palette );
        const State& flags( option->state );
        const bool enabled( flags&State_Enabled );

        const QStyleOptionProgressBarV2* progressBarOption2 = qstyleoption_cast<const QStyleOptionProgressBarV2*>( option );
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
        const QStyleOptionSlider *sliderOption = qstyleoption_cast<const QStyleOptionSlider *>( option );
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
            // it is necessary to retrive the complete widget rect, in order to properly handle overlaps
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

            _helper->renderScrollBarHole( painter, backgroundRect, color );

        }

        {
            // render handle
            // define colors
            QColor color;
            const QPalette& palette( option->palette );

            const QColor base( focus ?
                _helper->viewFocusBrush().brush( palette.currentColorGroup() ).color():
                _helper->alphaColor( palette.color( QPalette::WindowText ), 0.5 ) );

            const QColor highlight( _helper->viewHoverBrush().brush( palette.currentColorGroup() ).color() );
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
        _helper->renderScrollBarHole( painter, backgroundRect, color );

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
        _helper->renderScrollBarHole( painter, backgroundRect, color );

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
                const QPalette palette( option->palette );
                const QRect rect( option->rect );
                const QColor color( KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::WindowText ), 0.25 ) );
                painter->setBrush( Qt::NoBrush );
                painter->setPen( QPen( color, 1 ) );
                painter->drawLine( rect.topLeft(), rect.topRight() );
                return true;
            }

            case QFrame::VLine:
            {
                const QPalette palette( option->palette );
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

    //______________________________________________________________
    bool Style::drawComboBoxComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        // cast option and check
        const QStyleOptionComboBox* comboBoxOption( qstyleoption_cast<const QStyleOptionComboBox *>( option ) );
        if( !comboBoxOption ) return true;

        const State& flags( option->state );
        const QPalette& palette( option->palette );
        const bool enabled( flags & State_Enabled );
        const bool mouseOver( enabled && ( flags & State_MouseOver ) );
        const bool hasFocus( flags & State_HasFocus );
        const bool editable( comboBoxOption->editable );

        // frame
        if( option->subControls & SC_ComboBoxFrame )
        {

            if( editable )
            {

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
                _helper->renderFrame( painter, option->rect, palette.color( QPalette::Base ), outline, hasFocus );

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

                _helper->renderButtonSlab( painter, option->rect, color, outline, shadow, hasFocus, sunken );

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

            const QPolygonF a( genericArrow( ArrowDown, ArrowNormal ) );
            const qreal penThickness( 1.5 );

            painter->save();
            painter->translate( arrowRect.center() );
            painter->setRenderHint( QPainter::Antialiasing );
            painter->setPen( QPen( arrowColor, penThickness ) );
            painter->drawPolyline( a );
            painter->restore();

        }

        return true;

    }

    //______________________________________________________________
    bool Style::drawSpinBoxComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        const QStyleOptionSpinBox *spinBoxOption( qstyleoption_cast<const QStyleOptionSpinBox *>( option ) );
        if( !spinBoxOption ) return true;

        const State& flags( option->state );
        const QPalette& palette( option->palette );
        const bool enabled( flags & State_Enabled );
        const bool mouseOver( enabled && ( flags & State_MouseOver ) );
        const bool hasFocus( flags & State_HasFocus );

        if( option->subControls & SC_SpinBoxFrame )
        {

            // update animation state
            // focus takes precedence over hover
            _animations->lineEditEngine().updateState( widget, AnimationFocus, hasFocus );
            _animations->lineEditEngine().updateState( widget, AnimationHover, mouseOver && !hasFocus );

            // outline color
            const QColor outline( _helper->frameOutlineColor( palette, mouseOver, hasFocus,
                _animations->lineEditEngine().frameOpacity( widget ),
                _animations->lineEditEngine().frameAnimationMode( widget ) ) );

            // render
            _helper->renderFrame( painter, option->rect, palette.color( QPalette::Base ), outline, hasFocus );
        }

        if( option->subControls & SC_SpinBoxUp ) renderSpinBoxArrow( painter, spinBoxOption, widget, SC_SpinBoxUp );
        if( option->subControls & SC_SpinBoxDown ) renderSpinBoxArrow( painter, spinBoxOption, widget, SC_SpinBoxDown );

        return true;

    }

    //______________________________________________________________
    bool Style::drawSliderComplexControl( const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        const QStyleOptionSlider *sliderOption( qstyleoption_cast<const QStyleOptionSlider *>( option ) );
        if( !sliderOption ) return true;

        const QPalette& palette( option->palette );
        const State& flags( option->state );
        const bool enabled( flags & State_Enabled );
        const bool mouseOver( enabled && ( flags & State_MouseOver ) );
        const bool hasFocus( enabled && ( flags & State_HasFocus ) );

        // if( sliderOption->subControls & SC_SliderTickmarks ) { renderSliderTickmarks( painter, sliderOption, widget ); }

        // groove
        if( sliderOption->subControls & SC_SliderGroove )
        {
            // retrieve holeRect
            QRect holeRect( subControlRect( CC_Slider, sliderOption, SC_SliderGroove, widget ) );

            // adjustments
            if( sliderOption->orientation == Qt::Horizontal )
            {

                holeRect = centerRect( holeRect, holeRect.width()-Metrics::Slider_Thickness, Metrics::Slider_Thickness );

            } else {

                holeRect = centerRect( holeRect, Metrics::Slider_Thickness, holeRect.height()-Metrics::Slider_Thickness );

            }

            // base color
            const QColor color( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.3 ) );

            if( !enabled ) _helper->renderSliderHole( painter, holeRect, color );
            else {

                // retrieve slider rect
                QRect sliderRect( subControlRect( CC_Slider, sliderOption, SC_SliderHandle, widget ) );
                sliderRect = centerRect( sliderRect, Metrics::Slider_ControlThickness, Metrics::Slider_ControlThickness );

                // define highlight color
                const QColor highlight( palette.color( QPalette::Highlight ) );

                if( sliderOption->orientation == Qt::Horizontal )
                {

                    const bool reverseLayout( option->direction == Qt::RightToLeft );

                    QRect leftRect( holeRect );
                    leftRect.setRight( sliderRect.right()-1 );
                    _helper->renderSliderHole( painter, leftRect, reverseLayout ? color:highlight );

                    QRect rightRect( holeRect );
                    rightRect.setLeft( sliderRect.left()+1 );
                    _helper->renderSliderHole( painter, rightRect, reverseLayout ? highlight:color );

                } else {

                    QRect topRect( holeRect );
                    topRect.setBottom( sliderRect.bottom()-1 );
                    _helper->renderSliderHole( painter, topRect, highlight );

                    QRect bottomRect( holeRect );
                    bottomRect.setTop( sliderRect.top()+1 );
                    _helper->renderSliderHole( painter, bottomRect, color );

                }

            }

        }

        // handle
        if ( sliderOption->subControls & SC_SliderHandle )
        {

            // get rect and center
            QRect sliderRect( subControlRect( CC_Slider, sliderOption, SC_SliderHandle, widget ) );
            sliderRect = centerRect( sliderRect, Metrics::Slider_ControlThickness, Metrics::Slider_ControlThickness );

            const bool handleActive( sliderOption->activeSubControls & SC_SliderHandle );

            // define colors
            const QColor color( palette.color( QPalette::Button ) );
            const QColor shadow( _helper->alphaColor( palette.color( QPalette::Shadow ), 0.2 ) );
            QColor outline;

            _animations->sliderEngine().updateState( widget, enabled && handleActive );
            const bool animated( _animations->sliderEngine().isAnimated( widget ) );
            const qreal opacity( _animations->sliderEngine().opacity( widget ) );

            const QColor hover( _helper->viewHoverBrush().brush( option->palette.currentColorGroup() ).color() );
            const QColor focus( _helper->viewFocusBrush().brush( option->palette.currentColorGroup() ).color() );
            const QColor defaultOutline( KColorUtils::mix( palette.color( QPalette::Button ), palette.color( QPalette::ButtonText ), 0.4 ) );

            if( animated )
            {

                if( hasFocus ) outline = KColorUtils::mix( focus, hover, opacity );
                else outline = KColorUtils::mix( defaultOutline, hover, opacity );

            } else if( handleActive && mouseOver ) outline = hover;
            else if( hasFocus ) outline = focus;
            else outline = defaultOutline;

            const bool sunken( flags & (State_On|State_Sunken) );
            _helper->renderSliderHandle( painter, sliderRect, color, outline, shadow, hasFocus, sunken );

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
        QPolygonF a( genericArrow( orientation, ArrowNormal ) );

        const QColor base( color );

        painter->save();
        painter->translate( QRectF(rect).center() );

        painter->setPen( QPen( base, penThickness, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );
        painter->drawPolyline( a );
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
//         _animations->spinBoxEngine().updateState( widget, subControl, subControlHover );
//
//         const bool animated( enabled && _animations->spinBoxEngine().isAnimated( widget, subControl ) );
//         const qreal opacity( _animations->spinBoxEngine().opacity( widget, subControl ) );

        QColor color;
//         if( animated )
//         {
//
//             QColor highlight = _helper->viewHoverBrush().brush( palette ).color();
//             color = KColorUtils::mix( palette.color( QPalette::Text ), highlight, opacity );
//
//         } else if( subControlHover ) {
        if( subControlHover ) {

            color = _helper->viewHoverBrush().brush( palette ).color();

        } else if( atLimit ) {

            color = palette.color( QPalette::Disabled, QPalette::Text );

        } else {

            color = palette.color( QPalette::Text );

        }

        const QPolygonF a( genericArrow( ( subControl == SC_SpinBoxUp ) ? ArrowUp:ArrowDown, ArrowNormal ) );
        const QRectF arrowRect( subControlRect( CC_SpinBox, option, subControl, widget ) );

        painter->save();
        painter->translate( arrowRect.center() );
        painter->setRenderHint( QPainter::Antialiasing );

        const qreal penThickness = 1.6;
        painter->setPen( QPen( color, penThickness ) );
        painter->drawPolyline( a );
        painter->restore();

        return;

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

        const bool hover( _animations->scrollBarEngine().isHovered( widget, control ) );
        const bool animated( _animations->scrollBarEngine().isAnimated( widget, control ) );
        const qreal opacity( _animations->scrollBarEngine().opacity( widget, control ) );

        // retrieve mouse position from engine
        QPoint position( hover ? _animations->scrollBarEngine().position( widget ) : QPoint( -1, -1 ) );
        if( hover && rect.contains( position ) )
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

            } else if( hover ) {

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
