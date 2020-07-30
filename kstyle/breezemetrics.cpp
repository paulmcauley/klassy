#include "breezemetrics.h"

namespace Breeze {
    //BEGIN PenWidth
    constexpr qreal PenWidth::Symbol = 1.01;
    constexpr int PenWidth::Frame = 1;
    constexpr int PenWidth::Shadow = 1;
    constexpr int PenWidth::NoPen = 0;
    //END PenWidth
    
    //BEGIN Metrics
    constexpr int Metrics::Frame_FrameWidth = 2;
    constexpr int Metrics::Frame_FrameRadius = 3;
    constexpr int Metrics::Layout_TopLevelMarginWidth = 10;
    constexpr int Metrics::Layout_ChildMarginWidth = 6;
    constexpr int Metrics::Layout_DefaultSpacing = 6;
    constexpr int Metrics::LineEdit_FrameWidth = 6;
    constexpr int Metrics::Menu_FrameWidth = 0;
    constexpr int Metrics::MenuItem_MarginWidth = 5;
    constexpr int Metrics::MenuItem_MarginHeight = 3;
    constexpr int Metrics::MenuItem_ItemSpacing = 4;
    constexpr int Metrics::MenuItem_AcceleratorSpace = 16;
    constexpr int Metrics::MenuButton_IndicatorWidth = 20;
    constexpr int Metrics::ComboBox_FrameWidth = 6;
    constexpr int Metrics::SpinBox_FrameWidth = LineEdit_FrameWidth;
    constexpr int Metrics::SpinBox_ArrowButtonWidth = 20;
    constexpr int Metrics::GroupBox_TitleMarginWidth = 4;
    constexpr int Metrics::Button_MinWidth = 80;
    constexpr int Metrics::Button_MarginWidth = 6;
    constexpr int Metrics::Button_ItemSpacing = 4;
    constexpr int Metrics::ToolButton_MarginWidth = 6;
    constexpr int Metrics::ToolButton_ItemSpacing = 4;
    constexpr int Metrics::ToolButton_InlineIndicatorWidth = 12;
    constexpr int Metrics::CheckBox_Size = 20;
    constexpr int Metrics::CheckBox_FocusMarginWidth = 2;
    constexpr int Metrics::CheckBox_ItemSpacing = 4;
    constexpr int Metrics::MenuBarItem_MarginWidth = 10;
    constexpr int Metrics::MenuBarItem_MarginHeight = 6;
    constexpr int Metrics::ScrollBar_Extend = 21;
    constexpr int Metrics::ScrollBar_SliderWidth = 6;
    constexpr int Metrics::ScrollBar_MinSliderHeight = 20;
    constexpr int Metrics::ScrollBar_NoButtonHeight = (ScrollBar_Extend-ScrollBar_SliderWidth)/2;
    constexpr int Metrics::ScrollBar_SingleButtonHeight = ScrollBar_Extend;
    constexpr int Metrics::ScrollBar_DoubleButtonHeight = 2*ScrollBar_Extend;
    constexpr int Metrics::ToolBar_FrameWidth = 2;
    constexpr int Metrics::ToolBar_HandleExtent = 10;
    constexpr int Metrics::ToolBar_HandleWidth = 6;
    constexpr int Metrics::ToolBar_SeparatorWidth = 8;
    constexpr int Metrics::ToolBar_ExtensionWidth = 20;
    constexpr int Metrics::ToolBar_ItemSpacing = 0;
    constexpr int Metrics::ProgressBar_BusyIndicatorSize = 14;
    constexpr int Metrics::ProgressBar_Thickness = 6;
    constexpr int Metrics::ProgressBar_ItemSpacing = 4;
    constexpr int Metrics::TitleBar_MarginWidth = 4;
    constexpr int Metrics::Slider_TickLength = 8;
    constexpr int Metrics::Slider_TickMarginWidth = 2;
    constexpr int Metrics::Slider_GrooveThickness = 6;
    constexpr int Metrics::Slider_ControlThickness = 20;
    constexpr int Metrics::TabBar_TabMarginHeight = 4;
    constexpr int Metrics::TabBar_TabMarginWidth = 8;
    constexpr int Metrics::TabBar_TabMinWidth = 80;
    constexpr int Metrics::TabBar_TabMinHeight = 30;
    constexpr int Metrics::TabBar_TabItemSpacing = 8;
    constexpr int Metrics::TabBar_TabOverlap = 1;
    constexpr int Metrics::TabBar_BaseOverlap = 2;
    constexpr int Metrics::TabWidget_MarginWidth = 4;
    constexpr int Metrics::ToolBox_TabMinWidth = 80;
    constexpr int Metrics::ToolBox_TabItemSpacing = 4;
    constexpr int Metrics::ToolBox_TabMarginWidth = 8;
    constexpr int Metrics::ToolTip_FrameWidth = 3;
    constexpr int Metrics::Header_MarginWidth = 6;
    constexpr int Metrics::Header_ItemSpacing = 4;
    constexpr int Metrics::Header_ArrowSize = 10;
    constexpr int Metrics::ItemView_ArrowSize = 10;
    constexpr int Metrics::ItemView_ItemMarginWidth = 3;
    constexpr int Metrics::SidePanel_ItemMarginWidth = 4;
    constexpr int Metrics::Splitter_SplitterWidth = 1;
    constexpr int Metrics::Shadow_Overlap = 2;
    //END Metrics
}
