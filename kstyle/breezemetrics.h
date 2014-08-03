#ifndef breezemetrics_h
#define breezemetrics_h

/*
* this file is part of the breeze gtk engine
* Copyright (c) 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
* Copyright (c) 2010 Ruslan Kabatsayev <b7.10110111@gmail.com>
*
* This  library is free  software; you can  redistribute it and/or
* modify it  under  the terms  of the  GNU Lesser  General  Public
* License  as published  by the Free  Software  Foundation; either
* version 2 of the License, or( at your option ) any later version.
*
* This library is distributed  in the hope that it will be useful,
* but  WITHOUT ANY WARRANTY; without even  the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License  along  with  this library;  if not,  write to  the Free
* Software Foundation, Inc., 51  Franklin St, Fifth Floor, Boston,
* MA 02110-1301, USA.
*/

namespace Breeze
{

    //! metrics
    /*! these are copied from the old KStyle WidgetProperties */
    enum Metrics
    {

        // frames
        Frame_FrameWidth = 2,
        Frame_FrameRadius = 3,

        // line editors
        LineEdit_MarginWidth = 6,

        // combobox
        ComboBox_MarginWidth = 4,
        ComboBox_ButtonWidth = 20,
        ComboBox_BoxTextSpace = 8,

        // spinbox
        SpinBox_ButtonWidth = 20,

        // groupbox title margin
        GroupBox_TitleMarginWidth = 4,

        // buttons
        Button_MarginWidth = 6,

        // checkboxes and radio buttons
        RadioButton_Size = 22,
        CheckBox_Size = 22,
        CheckBox_FocusMarginWidth = 2,
        CheckBox_BoxTextSpace = 8,

        // scrollbar
        ScrollBar_Extend = 20,
        ScrollBar_Width = 10,
        ScrollBar_SliderWidth = ScrollBar_Width,
        ScrollBar_MinSliderHeight = ScrollBar_Width,
        ScrollBar_NoButtonHeight = 0,
        ScrollBar_SingleButtonHeight = ScrollBar_Extend,
        ScrollBar_DoubleButtonHeight = 2*ScrollBar_Extend,

        // progress bar
        ProgressBar_BusyIndicatorSize = 14,
        ProgressBar_Thickness = 6,
        ProgressBar_BoxTextSpace = 4,

        // sliders
        Slider_Thickness = 6,
        Slider_ControlThickness = 20,

        // tabbar
        TabBar_TabMarginHeight = 4,
        TabBar_TabMarginWidth = 8,
        TabBar_TabMinWidth = 80,
        TabBar_TabMinHeight = 28,
        TabBar_TabRadius = 3,
        TabBar_TabOverlap = 1,
        TabBar_BaseOverlap = 2,

        // tab widget
        TabWidget_MarginWidth = 4,

        // list headers
        Header_MarginWidth = 8,
        Header_BoxTextSpace = 4,
        Header_MarkSize = 10,

        // dock widget
        DockWidget_TitleMarginWidth = Frame_FrameWidth
    };

}

#endif
