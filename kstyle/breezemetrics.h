/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "breezestyleconfigdata.h"
#include <QtGlobal>

namespace Breeze
{

//* metrics
namespace Metrics
{
// general
static constexpr int ArrowSize = 10;
static constexpr int SmallArrowSize = 5;

// frames
static constexpr int Frame_FrameWidth = 2;
static qreal Frame_FrameRadius = 3; // set in Helper::loadConfig

// layout
static constexpr int Layout_TopLevelMarginWidth = 10;
static constexpr int Layout_ChildMarginWidth = 6;
static constexpr int Layout_DefaultSpacing = 6;

// line editors
static constexpr int LineEdit_FrameWidth = 6;

// menu items
static constexpr int Menu_FrameWidth = 0;
static constexpr int MenuItem_MarginWidth = 5;
static constexpr int MenuItem_HighlightGap = 4;
static constexpr int MenuItem_ExtraLeftMargin = 4;
static constexpr int MenuItem_MarginHeight = 3;
static constexpr int MenuItem_ItemSpacing = 4;
static constexpr int MenuItem_AcceleratorSpace = 16;

// combobox
static constexpr int ComboBox_FrameWidth = 6;

// spinbox
static constexpr int SpinBox_FrameWidth = LineEdit_FrameWidth;
static constexpr int SpinBox_ArrowButtonWidth = 20;

// groupbox title margin
static constexpr int GroupBox_TitleMarginWidth = 4;

// buttons
static constexpr int Button_MinWidth = 80;
static constexpr int Button_MarginWidth = 6;
static constexpr int Button_ItemSpacing = 4;

// tool buttons
static constexpr int ToolButton_MarginWidth = 6;
static constexpr int ToolButton_ItemSpacing = 4;
static constexpr int ToolButton_InlineIndicatorWidth = 12;

// menu button indicator
static constexpr int MenuButton_IndicatorWidth = 20;

// checkboxes and radio buttons
static constexpr int CheckBox_Size = 20;
static constexpr int CheckBox_FocusMarginWidth = 2;
static constexpr int CheckBox_ItemSpacing = 4;
static qreal CheckBox_Radius = qMax(0.0, Frame_FrameRadius - 1); // set in Helper::loadConfig

// menubar items
static constexpr int MenuBarItem_MarginWidth = 10;
static constexpr int MenuBarItem_MarginHeight = 6;

// scrollbars
static int const &ScrollBar_TopBottomMargins = StyleConfigData::scrollBarTopBottomMargins();
static int const &ScrollBar_SliderWidthMouseOver = StyleConfigData::scrollBarSliderThicknessMouseOver();
static int const &ScrollBar_SliderWidthMouseNotOver =
    int(std::max(1.0, qreal(ScrollBar_SliderWidthMouseOver) * (qreal(StyleConfigData::scrollBarSliderThicknessMouseNotOverPercent()) / 100.0f)));
static int const &ScrollBar_MinSliderHeight = StyleConfigData::scrollBarMinSliderHeight();
static int const &ScrollBarSliderPadding = StyleConfigData::scrollBarSliderPadding();
static int const &ScrollBar_Extend = int(ScrollBar_SliderWidthMouseOver + (ScrollBarSliderPadding * 2) + 1);
static int ScrollBar_NoButtonHeight = (ScrollBar_Extend - ScrollBar_SliderWidthMouseOver) / 2 + ScrollBar_TopBottomMargins;
static int const &ScrollBar_SingleButtonHeight = ScrollBar_Extend + ScrollBar_TopBottomMargins;
static int const &ScrollBar_DoubleButtonHeight = 2 * ScrollBar_Extend + ScrollBar_TopBottomMargins;

// toolbars
static constexpr int ToolBar_FrameWidth = 0;
static constexpr int ToolBar_HandleExtent = 10;
static constexpr int ToolBar_HandleWidth = 6;
static constexpr int ToolBar_SeparatorWidth = 8;
static constexpr int ToolBar_ExtensionWidth = 20;
static constexpr int ToolBar_ItemMargin = 6;
static constexpr int ToolBar_ItemSpacing = 0;
static constexpr int ToolBar_SeparatorVerticalMargin = 2;

// progressbars
static constexpr int ProgressBar_BusyIndicatorSize = 14;
static constexpr int ProgressBar_Thickness = 6;
static constexpr int ProgressBar_ItemSpacing = 4;

// mdi title bar
static constexpr int TitleBar_MarginWidth = 4;

// sliders
static constexpr int Slider_TickLength = 8;
static constexpr int Slider_TickMarginWidth = 2;
static constexpr int Slider_GrooveThickness = 6;
static constexpr int Slider_ControlThickness = 20;

// tabbar
static constexpr int TabBar_TabMarginHeight = 4;
static constexpr int TabBar_TabMarginWidth = 8;
static constexpr int TabBar_TabMinWidth = 80;
static constexpr int TabBar_TabMinHeight = 30;
static constexpr int TabBar_TabItemSpacing = 8;
static constexpr int TabBar_TabOverlap = 1;
static constexpr int TabBar_BaseOverlap = 2;

// tab widget
static constexpr int TabWidget_MarginWidth = 3;

// toolbox
static constexpr int ToolBox_TabMinWidth = 80;
static constexpr int ToolBox_TabItemSpacing = 4;
static constexpr int ToolBox_TabMarginWidth = 8;

// tooltips
static constexpr int ToolTip_FrameWidth = 3;

// list headers
static constexpr int Header_MarginWidth = 6;
static constexpr int Header_ItemSpacing = 4;
static constexpr int Header_ArrowSize = ArrowSize;

// tree view
static constexpr int ItemView_ArrowSize = ArrowSize;
static constexpr int ItemView_ItemMarginWidth = 3;
static constexpr int SidePanel_ItemMarginWidth = 4;

// splitter
static constexpr int Splitter_SplitterWidth = 1;

// shadow dimensions
static constexpr int Shadow_Overlap = 2;

// frame intensities (called bias in KColorUtilities::Mix)
// Keep this value in sync with Kirigami  PlatformTheme::frameContrast()
// https://invent.kde.org/frameworks/kirigami/-/blob/master/src/platform/platformtheme.cpp?ref_type=heads#L701
static constexpr qreal Bias_Default = 0.20;
}

}
