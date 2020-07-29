#ifndef breezemetrics_h
#define breezemetrics_h

#include <QtCore>

/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>    *
 * Copyright (C) 2020 by Noah Davis <noahadvs@gmail.com>                 *
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

namespace Breeze
{
    //* standard pen widths
    struct PenWidth
    {
        /* Using 1 instead of slightly more than 1 causes symbols drawn with
         * pen strokes to look skewed. The exact amount added does not matter
         * as long as it isn't too visible.
         */
        // The standard pen stroke width for symbols.
        static const qreal Symbol;

        // The standard pen stroke width for frames.
        static const int Frame;

        // The standard pen stroke width for shadows.
        static const int Shadow;
        
        // A value for pen width arguments to make it clear that there is no pen stroke
        static const int NoPen;
    };

    //* metrics
    struct Metrics
    {
        // frames
        static const int Frame_FrameWidth;
        static const int Frame_FrameRadius;

        // layout
        static const int Layout_TopLevelMarginWidth;
        static const int Layout_ChildMarginWidth;
        static const int Layout_DefaultSpacing;

        // line editors
        static const int LineEdit_FrameWidth;

        // menu items
        static const int Menu_FrameWidth;
        static const int MenuItem_MarginWidth;
        static const int MenuItem_MarginHeight;
        static const int MenuItem_ItemSpacing;
        static const int MenuItem_AcceleratorSpace;
        static const int MenuButton_IndicatorWidth;

        // combobox
        static const int ComboBox_FrameWidth;

        // spinbox
        static const int SpinBox_FrameWidth;
        static const int SpinBox_ArrowButtonWidth;

        // groupbox title margin
        static const int GroupBox_TitleMarginWidth;

        // buttons
        static const int Button_MinWidth;
        static const int Button_MarginWidth;
        static const int Button_ItemSpacing;

        // tool buttons
        static const int ToolButton_MarginWidth;
        static const int ToolButton_ItemSpacing;
        static const int ToolButton_InlineIndicatorWidth;

        // checkboxes and radio buttons
        static const int CheckBox_Size;
        static const int CheckBox_FocusMarginWidth;
        static const int CheckBox_ItemSpacing;

        // menubar items
        static const int MenuBarItem_MarginWidth;
        static const int MenuBarItem_MarginHeight;

        // scrollbars
        static const int ScrollBar_Extend;
        static const int ScrollBar_SliderWidth;
        static const int ScrollBar_MinSliderHeight;
        static const int ScrollBar_NoButtonHeight;
        static const int ScrollBar_SingleButtonHeight;
        static const int ScrollBar_DoubleButtonHeight;

        // toolbars
        static const int ToolBar_FrameWidth;
        static const int ToolBar_HandleExtent;
        static const int ToolBar_HandleWidth;
        static const int ToolBar_SeparatorWidth;
        static const int ToolBar_ExtensionWidth;
        static const int ToolBar_ItemSpacing;

        // progressbars
        static const int ProgressBar_BusyIndicatorSize;
        static const int ProgressBar_Thickness;
        static const int ProgressBar_ItemSpacing;

        // mdi title bar
        static const int TitleBar_MarginWidth;

        // sliders
        static const int Slider_TickLength;
        static const int Slider_TickMarginWidth;
        static const int Slider_GrooveThickness;
        static const int Slider_ControlThickness;

        // tabbar
        static const int TabBar_TabMarginHeight;
        static const int TabBar_TabMarginWidth;
        static const int TabBar_TabMinWidth;
        static const int TabBar_TabMinHeight;
        static const int TabBar_TabItemSpacing;
        static const int TabBar_TabOverlap;
        static const int TabBar_BaseOverlap;

        // tab widget
        static const int TabWidget_MarginWidth;

        // toolbox
        static const int ToolBox_TabMinWidth;
        static const int ToolBox_TabItemSpacing;
        static const int ToolBox_TabMarginWidth;

        // tooltips
        static const int ToolTip_FrameWidth;

        // list headers
        static const int Header_MarginWidth;
        static const int Header_ItemSpacing;
        static const int Header_ArrowSize;

        // tree view
        static const int ItemView_ArrowSize;
        static const int ItemView_ItemMarginWidth;
        static const int SidePanel_ItemMarginWidth;

        // splitter
        static const int Splitter_SplitterWidth;

        // shadow dimensions
        static const int Shadow_Overlap;
    };
}
#endif
