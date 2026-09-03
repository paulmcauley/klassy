/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-FileCopyrightText: 2021-2025 Paul A McAuley <kde@paulmcauley.com>
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
extern qreal Frame_FrameRadius; // set in Helper::loadConfig

// layout
static constexpr int Layout_TopLevelMarginWidth = 10;
static constexpr int Layout_ChildMarginWidth = 6;
static constexpr int Layout_DefaultSpacing = 6;

// line editors
static constexpr int LineEdit_FrameWidth = 6;

// menu items
static constexpr int Menu_FrameWidth = 0;
static constexpr int MenuItem_MarginWidth = 4;
static constexpr int MenuItem_HighlightGap = 4;
static constexpr int MenuItem_ExtraLeftMargin = 4;
static constexpr int MenuItem_MarginHeight = 4;
static constexpr int MenuItem_ItemSpacing = 2;
static constexpr int MenuItem_AcceleratorSpace = 16;
static constexpr int MenuItem_TextLeftMargin = 8;

// combobox
static constexpr int ComboBox_FrameWidth = 6;
static constexpr int ComboBox_ItemMarginWidth = 3;

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
extern qreal CheckBox_Radius; // set in Helper::loadConfig

// menubar items
static constexpr int MenuBarItem_MarginWidth = 10;
static constexpr int MenuBarItem_MarginHeight = 6;

// scrollbars
static bool const LibreOfficeScrollBarException =
    (qAppName() == QStringLiteral("soffice.bin")); // LibreOffice Qt renders scrollbars very badly if they are not limited to certain dimensions
static int const &ScrollBar_TopBottomMargins = LibreOfficeScrollBarException ? 0 : StyleConfigData::scrollBarTopBottomMargins();
static int const &ScrollBar_SliderWidthMouseOver = StyleConfigData::scrollBarSliderThicknessMouseOver();
static int const &ScrollBar_SliderWidthMouseNotOver =
    int(std::max(1.0,
                 qreal(ScrollBar_SliderWidthMouseOver)
                     * (LibreOfficeScrollBarException ? 1.0 : qreal(StyleConfigData::scrollBarSliderThicknessMouseNotOverPercent()) / 100.0f)));
static int const &ScrollBar_MinSliderHeight = StyleConfigData::scrollBarMinSliderHeight();
static int const &ScrollBarSliderPadding = StyleConfigData::scrollBarSliderPadding();
static int const &ScrollBar_Extent = int(ScrollBar_SliderWidthMouseOver + (ScrollBarSliderPadding * 2) + 1);
static int ScrollBar_NoButtonHeight = (ScrollBar_Extent - ScrollBar_SliderWidthMouseOver) / 2 + ScrollBar_TopBottomMargins;
static int const &ScrollBar_SingleButtonHeight = ScrollBar_Extent + ScrollBar_TopBottomMargins;
static int const &ScrollBar_DoubleButtonHeight = 2 * ScrollBar_Extent + ScrollBar_TopBottomMargins;
static int const &ScrollBarTopOneButtonSpacing = LibreOfficeScrollBarException ? 0 : StyleConfigData::scrollBarTopOneButtonSpacing();
static int const &ScrollBarBottomOneButtonSpacing = LibreOfficeScrollBarException ? 0 : StyleConfigData::scrollBarBottomOneButtonSpacing();
static int const &ScrollBarTopTwoButtonSpacing = StyleConfigData::scrollBarTopTwoButtonSpacing();
static int const &ScrollBarBottomTwoButtonSpacing = StyleConfigData::scrollBarBottomTwoButtonSpacing();

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
static constexpr int ItemView_ItemMarginLeft = 2;
static constexpr int ItemView_ItemMarginRight = 2;
// Normal margin height is 2 to have rows spaced 4
// First and last items will have a bigger top/bottom margin for consistent borders with the outside frame
static constexpr int ItemView_FirstItemTopMarginHeight = 2;
static constexpr int ItemView_ItemMarginTop = 1;
static constexpr int ItemView_ItemMarginBottom = 1;
static constexpr int ItemView_ItemPaddingWidth = 2;
static constexpr int ItemView_ItemPaddingHeight = 2;
static constexpr int SidePanel_ItemMarginWidth = 4;

// splitter
static constexpr int Splitter_SplitterWidth = 1;

// shadow dimensions
static constexpr int Shadow_Overlap = 2;

// Value used to blend background and foreground,
// used for hover effects and similar.
static constexpr qreal Blend_Value = 0.3;

static constexpr qreal Focus_LightenColorValue = 110;
}

}
