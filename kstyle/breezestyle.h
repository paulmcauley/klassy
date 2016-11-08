#ifndef breezestyle_h
#define breezestyle_h

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

#include "breeze.h"
#include "config-breeze.h"

#if BREEZE_HAVE_KSTYLE
#include <KStyle>
#endif

#if BREEZE_USE_KDE4
#include "kstylekde4compat.h"
#endif

#include <QAbstractItemView>
#include <QAbstractScrollArea>

#include <QCommonStyle>
#include <QDockWidget>
#include <QHash>
#include <QIcon>
#include <QMdiSubWindow>
#include <QStyleOption>
#include <QWidget>

#if QT_VERSION >= 0x050000
#include <QCommandLinkButton>
#endif

namespace BreezePrivate
{
    class TabBarData;
}

namespace Breeze
{

    class Animations;
    class FrameShadowFactory;
    class Helper;
    class MdiWindowShadowFactory;
    class Mnemonics;
    class ShadowHelper;
    class SplitterFactory;
    class WidgetExplorer;
    class WindowManager;

    //* convenience typedef for base class
    #if BREEZE_USE_KDE4
    using ParentStyleClass = KStyleKDE4Compat;
    #elif !BREEZE_HAVE_KSTYLE
    using ParentStyleClass = QCommonStyle;
    #else
    using ParentStyleClass = KStyle;
    #endif

    //* base class for breeze style
    /** it is responsible to draw all the primitives to be displayed on screen, on request from Qt paint engine */
    class Style: public ParentStyleClass
    {
        Q_OBJECT

        /* this tells kde applications that custom style elements are supported, using the kstyle mechanism */
        Q_CLASSINFO ("X-KDE-CustomElements", "true")

        public:

        //* constructor
        explicit Style( void );

        //* destructor
        virtual ~Style( void );

        //* needed to avoid warnings at compilation time
        using  ParentStyleClass::polish;
        using  ParentStyleClass::unpolish;

        //* widget polishing
        virtual void polish( QWidget* );

        //* widget unpolishing
        virtual void unpolish( QWidget* );

        //* polish scrollarea
        void polishScrollArea( QAbstractScrollArea* );

        //* pixel metrics
        virtual int pixelMetric(PixelMetric, const QStyleOption* = nullptr, const QWidget* = nullptr) const;

        //* style hints
        virtual int styleHint(StyleHint, const QStyleOption* = nullptr, const QWidget* = nullptr, QStyleHintReturn* = nullptr) const;

        //* returns rect corresponding to one widget's subelement
        virtual QRect subElementRect( SubElement, const QStyleOption*, const QWidget* ) const;

        //* returns rect corresponding to one widget's subcontrol
        virtual QRect subControlRect( ComplexControl, const QStyleOptionComplex*, SubControl, const QWidget* ) const;

        //* returns size matching contents
        QSize sizeFromContents( ContentsType, const QStyleOption*, const QSize&, const QWidget* ) const;

        //* returns which subcontrol given QPoint corresponds to
        SubControl hitTestComplexControl( ComplexControl, const QStyleOptionComplex*, const QPoint&, const QWidget* ) const;

        //* primitives
        void drawPrimitive( PrimitiveElement, const QStyleOption*, QPainter*, const QWidget* ) const;

        //* controls
        void drawControl( ControlElement, const QStyleOption*, QPainter*, const QWidget* ) const;

        //* complex controls
        void drawComplexControl( ComplexControl, const QStyleOptionComplex*, QPainter*, const QWidget* ) const;

        //* generic text rendering
        virtual void drawItemText(
            QPainter*, const QRect&, int alignment, const QPalette&, bool enabled,
            const QString&, QPalette::ColorRole = QPalette::NoRole) const;

        //*@name event filters
        //@{

        virtual bool eventFilter(QObject *, QEvent *);
        bool eventFilterScrollArea( QWidget*, QEvent* );
        bool eventFilterComboBoxContainer( QWidget*, QEvent* );
        bool eventFilterDockWidget( QDockWidget*, QEvent* );
        bool eventFilterMdiSubWindow( QMdiSubWindow*, QEvent* );

        #if QT_VERSION >= 0x050000
        bool eventFilterCommandLinkButton( QCommandLinkButton*, QEvent* );
        #endif

        //* install event filter to object, in a unique way
        void addEventFilter( QObject* object )
        {
            object->removeEventFilter( this );
            object->installEventFilter( this );
        }

        //@}

        protected Q_SLOTS:

        //* update configuration
        void configurationChanged( void );

        //* standard icons
        virtual QIcon standardIconImplementation( StandardPixmap, const QStyleOption*, const QWidget* ) const;

        protected:

        //* standard icons
        virtual QIcon standardIcon( StandardPixmap pixmap, const QStyleOption* option = nullptr, const QWidget* widget = nullptr) const
        { return standardIconImplementation( pixmap, option, widget ); }

        //* load configuration
        void loadConfiguration();

        //*@name subelementRect specialized functions
        //@{

        //* default implementation. Does not change anything
        QRect defaultSubElementRect( const QStyleOption* option, const QWidget* ) const
        { return option->rect; }

        QRect pushButtonContentsRect( const QStyleOption*, const QWidget* ) const;
        QRect checkBoxContentsRect( const QStyleOption*, const QWidget* ) const;
        QRect lineEditContentsRect( const QStyleOption*, const QWidget* ) const;
        QRect progressBarGrooveRect( const QStyleOption*, const QWidget* ) const;
        QRect progressBarContentsRect( const QStyleOption*, const QWidget* ) const;
        QRect progressBarLabelRect( const QStyleOption*, const QWidget* ) const;
        QRect headerArrowRect( const QStyleOption*, const QWidget* ) const;
        QRect headerLabelRect( const QStyleOption*, const QWidget* ) const;
        QRect tabBarTabLeftButtonRect( const QStyleOption*, const QWidget* ) const;
        QRect tabBarTabRightButtonRect( const QStyleOption*, const QWidget* ) const;
        QRect tabWidgetTabBarRect( const QStyleOption*, const QWidget* ) const;
        QRect tabWidgetTabContentsRect( const QStyleOption*, const QWidget* ) const;
        QRect tabWidgetTabPaneRect( const QStyleOption*, const QWidget* ) const;
        QRect tabWidgetCornerRect( SubElement, const QStyleOption*, const QWidget* widget ) const;
        QRect toolBoxTabContentsRect( const QStyleOption*, const QWidget* ) const;
        QRect genericLayoutItemRect( const QStyleOption*, const QWidget* ) const;

        //@}

        //*@name subcontrol Rect specialized functions
        //@{

        QRect groupBoxSubControlRect( const QStyleOptionComplex*, SubControl, const QWidget* ) const;
        QRect toolButtonSubControlRect( const QStyleOptionComplex*, SubControl, const QWidget* ) const;
        QRect comboBoxSubControlRect( const QStyleOptionComplex*, SubControl, const QWidget* ) const;
        QRect spinBoxSubControlRect( const QStyleOptionComplex*, SubControl, const QWidget* ) const;
        QRect scrollBarInternalSubControlRect( const QStyleOptionComplex*, SubControl ) const;
        QRect scrollBarSubControlRect( const QStyleOptionComplex*, SubControl, const QWidget* ) const;
        QRect dialSubControlRect( const QStyleOptionComplex*, SubControl, const QWidget* ) const;
        QRect sliderSubControlRect( const QStyleOptionComplex*, SubControl, const QWidget* ) const;

        //@}

        //*@name sizeFromContents
        //@{
        QSize defaultSizeFromContents( const QStyleOption*, const QSize& size, const QWidget* ) const
        { return size; }

        QSize checkBoxSizeFromContents( const QStyleOption*, const QSize&, const QWidget* ) const;
        QSize lineEditSizeFromContents( const QStyleOption*, const QSize&, const QWidget* ) const;
        QSize comboBoxSizeFromContents( const QStyleOption*, const QSize&, const QWidget* ) const;
        QSize spinBoxSizeFromContents( const QStyleOption*, const QSize&, const QWidget* ) const;
        QSize sliderSizeFromContents( const QStyleOption*, const QSize&, const QWidget* ) const;
        QSize pushButtonSizeFromContents( const QStyleOption*, const QSize&, const QWidget* ) const;
        QSize toolButtonSizeFromContents( const QStyleOption*, const QSize&, const QWidget* ) const;
        QSize menuBarItemSizeFromContents( const QStyleOption*, const QSize&, const QWidget* ) const;
        QSize menuItemSizeFromContents( const QStyleOption*, const QSize&, const QWidget* ) const;
        QSize progressBarSizeFromContents( const QStyleOption*, const QSize&, const QWidget* ) const;
        QSize tabWidgetSizeFromContents( const QStyleOption*, const QSize&, const QWidget* ) const;
        QSize tabBarTabSizeFromContents( const QStyleOption*, const QSize&, const QWidget* ) const;
        QSize headerSectionSizeFromContents( const QStyleOption*, const QSize&, const QWidget* ) const;
        QSize itemViewItemSizeFromContents( const QStyleOption*, const QSize&, const QWidget* ) const;

        //@}

        //*@name primitives specialized functions
        //@{

        bool emptyPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const
        { return true; }

        bool drawFramePrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawFrameLineEditPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawFrameFocusRectPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawFrameMenuPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawFrameGroupBoxPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawFrameTabWidgetPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawFrameTabBarBasePrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawFrameWindowPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;

        bool drawIndicatorArrowUpPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
        { return drawIndicatorArrowPrimitive( ArrowUp, option, painter, widget ); }

        bool drawIndicatorArrowDownPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
        { return drawIndicatorArrowPrimitive( ArrowDown, option, painter, widget ); }

        bool drawIndicatorArrowLeftPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
        { return drawIndicatorArrowPrimitive( ArrowLeft, option, painter, widget ); }

        bool drawIndicatorArrowRightPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
        { return drawIndicatorArrowPrimitive( ArrowRight, option, painter, widget ); }

        bool drawIndicatorArrowPrimitive( ArrowOrientation, const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawIndicatorHeaderArrowPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawPanelButtonCommandPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawPanelButtonToolPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawTabBarPanelButtonToolPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawPanelScrollAreaCornerPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawPanelMenuPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawPanelTipLabelPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawPanelItemViewItemPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawIndicatorCheckBoxPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawIndicatorRadioButtonPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawIndicatorButtonDropDownPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawIndicatorTabClosePrimitive( const QStyleOption*, QPainter* painter, const QWidget* widget ) const;
        bool drawIndicatorTabTearPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawIndicatorToolBarHandlePrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawIndicatorToolBarSeparatorPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
        bool drawIndicatorBranchPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;

        //@}

        //*@name controls specialized functions
        //@{

        bool emptyControl( const QStyleOption*, QPainter*, const QWidget* ) const
        { return true; }

        virtual bool drawPushButtonLabelControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawToolButtonLabelControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawCheckBoxLabelControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawComboBoxLabelControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawMenuBarItemControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawMenuItemControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawProgressBarControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawProgressBarContentsControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawProgressBarGrooveControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawProgressBarLabelControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawScrollBarSliderControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawScrollBarAddLineControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawScrollBarSubLineControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawShapedFrameControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawRubberBandControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawHeaderSectionControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawHeaderEmptyAreaControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawTabBarTabLabelControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawTabBarTabShapeControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawToolBoxTabLabelControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawToolBoxTabShapeControl( const QStyleOption*, QPainter*, const QWidget* ) const;
        virtual bool drawDockWidgetTitleControl( const QStyleOption*, QPainter*, const QWidget* ) const;

        //*@}

        //*@name complex ontrols specialized functions
        //@{
        bool drawGroupBoxComplexControl( const QStyleOptionComplex*, QPainter*, const QWidget* ) const;
        bool drawToolButtonComplexControl( const QStyleOptionComplex*, QPainter*, const QWidget* ) const;
        bool drawComboBoxComplexControl( const QStyleOptionComplex*, QPainter*, const QWidget* ) const;
        bool drawSpinBoxComplexControl( const QStyleOptionComplex*, QPainter*, const QWidget* ) const;
        bool drawSliderComplexControl( const QStyleOptionComplex*, QPainter*, const QWidget* ) const;
        bool drawDialComplexControl( const QStyleOptionComplex*, QPainter*, const QWidget* ) const;
        bool drawScrollBarComplexControl( const QStyleOptionComplex*, QPainter*, const QWidget* ) const;
        bool drawTitleBarComplexControl( const QStyleOptionComplex*, QPainter*, const QWidget* ) const;
        //@}

        //!*@name various utilty functions
        //@{

        //* spinbox arrows
        void renderSpinBoxArrow( const SubControl&, const QStyleOptionSpinBox*, QPainter*, const QWidget*) const;

        //* menu title
        void renderMenuTitle( const QStyleOptionToolButton*, QPainter*, const QWidget* ) const;

        //* return dial angle based on option and value
        qreal dialAngle( const QStyleOptionSlider*, int ) const;

        //* returns relevant scrollbar parent
        /** needed to detect parent focus */
        const QWidget* scrollBarParent( const QWidget* ) const;

        //* returns true if given scrollbar arrow is animated
        QColor scrollBarArrowColor( const QStyleOptionSlider*, const SubControl&, const QWidget* ) const;

        //* scrollbar buttons
        enum ScrollBarButtonType
        {
            NoButton,
            SingleButton,
            DoubleButton
        };

        //* returns height for scrollbar buttons depending of button types
        int scrollBarButtonHeight( const ScrollBarButtonType& type ) const
        {
            switch( type )
            {
                case NoButton: return Metrics::ScrollBar_NoButtonHeight;
                case SingleButton: return Metrics::ScrollBar_SingleButtonHeight;
                case DoubleButton: return Metrics::ScrollBar_DoubleButtonHeight;
                default: return 0;
            }
        }

        //@}

        //* translucent background
        void setTranslucentBackground( QWidget* ) const;

        /**
        separator can have a title and an icon
        in that case they are rendered as sunken flat toolbuttons
        return toolbutton option that matches named separator menu items
        */
        QStyleOptionToolButton separatorMenuItemOption( const QStyleOptionMenuItem*, const QWidget* ) const;

        //* create toolbar extension icon
        QIcon toolBarExtensionIcon( StandardPixmap, const QStyleOption*, const QWidget* ) const;

        //* create title bar button icon
        QIcon titleBarButtonIcon( StandardPixmap, const QStyleOption*, const QWidget* ) const;

        //* returns item view parent if any
        /** needed to have correct color on focused checkboxes and radiobuttons */
        const QAbstractItemView* itemViewParent( const QWidget* ) const;

        //* returns true if a given widget is a selected item in a focused list
        /**
        This is necessary to have the correct colors used for e.g. checkboxes and radiobuttons in lists
        @param widget The widget to be checked
        @param position Used to find the relevant QModelIndex
        */
        bool isSelectedItem( const QWidget*, const QPoint& ) const;

        //* return true if option corresponds to QtQuick control
        bool isQtQuickControl( const QStyleOption*, const QWidget* ) const;

        //@}

        //* adjust rect based on provided margins
        QRect insideMargin( const QRect& r, int margin ) const
        { return insideMargin( r, margin, margin ); }

        //* adjust rect based on provided margins
        QRect insideMargin( const QRect& r, int marginWidth, int marginHeight ) const
        { return r.adjusted( marginWidth, marginHeight, -marginWidth, -marginHeight ); }

        //* expand size based on margins
        QSize expandSize( const QSize& size, int margin ) const
        { return expandSize( size, margin, margin ); }

        //* expand size based on margins
        QSize expandSize( const QSize& size, int marginWidth, int marginHeight ) const
        { return size + 2*QSize( marginWidth, marginHeight ); }

        //* returns true for vertical tabs
        bool isVerticalTab( const QStyleOptionTab* option ) const
        { return isVerticalTab( option->shape ); }

        bool isVerticalTab( const QTabBar::Shape& shape ) const
        {
            return shape == QTabBar::RoundedEast
                || shape == QTabBar::RoundedWest
                || shape == QTabBar::TriangularEast
                || shape == QTabBar::TriangularWest;

        }

        //* right to left alignment handling
        using ParentStyleClass::visualRect;
        QRect visualRect(const QStyleOption* opt, const QRect& subRect) const
        { return ParentStyleClass::visualRect(opt->direction, opt->rect, subRect); }

        //* centering
        QRect centerRect(const QRect &rect, const QSize& size ) const
        { return centerRect( rect, size.width(), size.height() ); }

        QRect centerRect(const QRect &rect, int width, int height) const
        { return QRect(rect.left() + (rect.width() - width)/2, rect.top() + (rect.height() - height)/2, width, height); }

        /*
        Checks whether the point is before the bound rect for bound of given orientation.
        This is needed to implement custom number of buttons in scrollbars,
        as well as proper mouse-hover
        */
        inline bool preceeds( const QPoint&, const QRect&, const QStyleOption* ) const;

        //* return which arrow button is hit by point for scrollbar double buttons
        inline QStyle::SubControl scrollBarHitTest( const QRect&, const QPoint&, const QStyleOption* ) const;

        //! return true if one of the widget's parent inherits requested type
        inline bool hasParent( const QWidget*, const char* ) const;

        //* return true if one of the widget's parent inherits requested type
        template<typename T> bool hasParent( const QWidget* ) const;

        //* return true if icons should be shown in menus
        bool showIconsInMenuItems( void ) const;

        //* return true if icons should be shown on buttons
        bool showIconsOnPushButtons( void ) const;

        //* return true if passed widget is a menu title (KMenu::addTitle)
        bool isMenuTitle( const QWidget* ) const;

        //* return true if passed widget is a menu title (KMenu::addTitle)
        bool hasAlteredBackground( const QWidget* ) const;

        private:

        //*@name scrollbar button types (for addLine and subLine )
        //@{
        ScrollBarButtonType _addLineButtons;
        ScrollBarButtonType _subLineButtons;
        //@}

        //* helper
        Helper* _helper;

        //* shadow helper
        ShadowHelper* _shadowHelper;

        //* animations
        Animations* _animations;

        //* keyboard accelerators
        Mnemonics* _mnemonics;

        //* window manager
        WindowManager* _windowManager;

        //* frame shadows
        FrameShadowFactory* _frameShadowFactory;

        //* mdi window shadows
        MdiWindowShadowFactory* _mdiWindowShadowFactory;

        //* splitter Factory, to extend splitters hit area
        SplitterFactory* _splitterFactory;

        //* widget explorer
        WidgetExplorer* _widgetExplorer;

        //* tabbar data
        BreezePrivate::TabBarData* _tabBarData;

        //* icon hash
        using IconCache = QHash<StandardPixmap, QIcon>;
        IconCache _iconCache;

        //* pointer to primitive specialized function
        using StylePrimitive = bool(Style::*)(const QStyleOption*, QPainter*, const QWidget* ) const;
        StylePrimitive _frameFocusPrimitive = nullptr;

        //* pointer to control specialized function
        using StyleControl = bool (Style::*)( const QStyleOption*, QPainter*, const QWidget* ) const;

        //* pointer to control specialized function
        using StyleComplexControl = bool (Style::*)( const QStyleOptionComplex*, QPainter*, const QWidget* ) const;

        //*@name custom elements
        //@{

        //* use Argb Drag and Drop Window
        QStyle::StyleHint SH_ArgbDndWindow;

        //! styled painting for KCapacityBar
        QStyle::ControlElement CE_CapacityBar;

        //@}

    };

    //_________________________________________________________________________
    bool Style::preceeds( const QPoint& point, const QRect& bound, const QStyleOption* option ) const
    {
        if( option->state&QStyle::State_Horizontal)
        {

            if( option->direction == Qt::LeftToRight) return point.x() < bound.right();
            else return point.x() > bound.x();

        } else return point.y() < bound.y();

    }

    //_________________________________________________________________________
    QStyle::SubControl Style::scrollBarHitTest( const QRect& rect, const QPoint& point, const QStyleOption* option ) const
    {
        if( option->state & QStyle::State_Horizontal)
        {

            if( option->direction == Qt::LeftToRight ) return point.x() < rect.center().x() ? QStyle::SC_ScrollBarSubLine : QStyle::SC_ScrollBarAddLine;
            else return point.x() > rect.center().x() ? QStyle::SC_ScrollBarSubLine : QStyle::SC_ScrollBarAddLine;

        } else return point.y() < rect.center().y() ? QStyle::SC_ScrollBarSubLine : QStyle::SC_ScrollBarAddLine;

    }

    //_________________________________________________________________________
    bool Style::hasParent( const QWidget* widget, const char* className ) const
    {

        if( !widget ) return false;

        while( (widget = widget->parentWidget()) )
        { if( widget->inherits( className ) ) return true; }

        return false;

    }

    //_________________________________________________________________________
    template< typename T > bool Style::hasParent( const QWidget* widget ) const
    {

        if( !widget ) return false;

        while( (widget = widget->parentWidget()) )
        { if( qobject_cast<const T*>( widget ) ) return true; }

        return false;

    }


}

#endif
