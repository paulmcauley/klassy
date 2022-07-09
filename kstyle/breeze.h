/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breeze_h
#define breeze_h

#include <QFlags>
#include <QPointer>
#include <QScopedPointer>
#include <QWeakPointer>

namespace Breeze
{

//*@name convenience typedef
//@{

//* scoped pointer convenience typedef
template<typename T>
using WeakPointer = QPointer<T>;

//* scoped pointer convenience typedef
template<typename T>
using ScopedPointer = QScopedPointer<T, QScopedPointerPodDeleter>;

//@}

//* animation mode
enum AnimationMode {
    AnimationNone = 0,
    AnimationHover = 0x1,
    AnimationFocus = 0x2,
    AnimationEnable = 0x4,
    AnimationPressed = 0x8,
};

Q_DECLARE_FLAGS(AnimationModes, AnimationMode)

//* corners
enum Corner {
    CornerTopLeft = 0x1,
    CornerTopRight = 0x2,
    CornerBottomLeft = 0x4,
    CornerBottomRight = 0x8,
    CornersTop = CornerTopLeft | CornerTopRight,
    CornersBottom = CornerBottomLeft | CornerBottomRight,
    CornersLeft = CornerTopLeft | CornerBottomLeft,
    CornersRight = CornerTopRight | CornerBottomRight,
    AllCorners = CornerTopLeft | CornerTopRight | CornerBottomLeft | CornerBottomRight,
};

Q_DECLARE_FLAGS(Corners, Corner)

//* sides
enum Side {
    SideLeft = 0x1,
    SideTop = 0x2,
    SideRight = 0x4,
    SideBottom = 0x8,
    AllSides = SideLeft | SideTop | SideRight | SideBottom,
};

Q_DECLARE_FLAGS(Sides, Side)

//* checkbox state
enum CheckBoxState {
    CheckOff,
    CheckPartial,
    CheckOn,
    CheckAnimated,
};

//* radio button state
enum RadioButtonState {
    RadioOff,
    RadioOn,
    RadioAnimated,
};

//* arrow orientation
enum ArrowOrientation {
    ArrowNone,
    ArrowUp,
    ArrowDown,
    ArrowLeft,
    ArrowRight,
    ArrowDown_Small,
};

//* button type
enum ButtonType {
    ButtonClose,
    ButtonMaximize,
    ButtonMinimize,
    ButtonRestore,
};
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Breeze::AnimationModes)
Q_DECLARE_OPERATORS_FOR_FLAGS(Breeze::Corners)
Q_DECLARE_OPERATORS_FOR_FLAGS(Breeze::Sides)

#endif
