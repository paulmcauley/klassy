#ifndef breeze_h
#define breeze_h

/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#define KLASSY_DECORATION_DEBUG_MODE 1
#define KLASSY_STYLE_DEBUG_MODE 1
#define KLASSY_QDEBUG_OUTPUT_PATH_RELATIVE_HOME "/Desktop/klassy_debug.txt"

#include "breezecommon_export.h"
#include "breezesettings.h"

#include <QFlags>
#include <QList>
#include <QPointer>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QWeakPointer>

namespace Breeze
{

// COMMON-------------------------------------------------------------------

QString BREEZECOMMON_EXPORT klassyLongVersion();

//* standard pen widths
struct BREEZECOMMON_EXPORT PenWidth {
    /* Using 1 instead of slightly more than 1 causes symbols drawn with
     * pen strokes to look skewed. The exact amount added does not matter
     * as long as it isn't too visible. Even with QPen::setCosmetic(true),
     * 1px pen widths still look slightly worse.
     */
    // The standard pen stroke width for symbols.
    static constexpr qreal Symbol = 1.001;

    // The standard pen stroke width for frames.
    static constexpr qreal Frame = 1.001;

    // The standard pen stroke width for shadows.
    static constexpr qreal Shadow = 1.001;

    // A value for pen width arguments to make it clear that there is no pen stroke
    static constexpr int NoPen = 0;
};

// list of keys used for window decoration exceptions
static QStringList windecoExceptionKeys = {
    "Enabled",
    "ExceptionProgramNamePattern",
    "ExceptionWindowPropertyPattern",
    "ExceptionWindowPropertyType",
    "HideTitleBar",
    "OpaqueTitleBar",
    "PreventApplyOpacityToHeader",
    "ExceptionBorder",
    "BorderSize",
    "ExceptionPreset",
};

enum struct BREEZECOMMON_EXPORT ColorOverridableButtonTypes {
    Close,
    Maximize,
    Minimize,
    Help,
    Shade,
    AllDesktops,
    KeepBelow,
    KeepAbove,
    ApplicationMenu,
    Menu,
    Count,
};

enum struct BREEZECOMMON_EXPORT OverridableButtonColorStates {
    IconNormal,
    IconHover,
    IconPressed,
    BackgroundNormal,
    BackgroundHover,
    BackgroundPress,
    OutlineNormal,
    OutlineHover,
    OutlinePress,
    Count,
};

// KDECORATION-------------------------------------------------------------

//* convenience typedefs
using InternalSettingsPtr = QSharedPointer<InternalSettings>;
using InternalSettingsList = QList<InternalSettingsPtr>;
using InternalSettingsListIterator = QListIterator<InternalSettingsPtr>;

//* metrics
namespace Metrics
{

//* corner radius (pixels)
// Paul A McAuley: No longer used - now InternalSettings::cornerRadius()
// Frame_FrameRadius = 3,

//* titlebar metrics, in units of small spacing

// Paul A McAuley: No longer used - now InternalSettings::titlebarTopMargin(), InternalSettings::titlebarBottomMargin()
// TitleBar_TopMargin = 2,
// TitleBar_BottomMargin = 2,

// Paul A McAuley: No longer used - now InternalSettings::titlebarLeftMargin(), InternalSettings::titlebarRightMargin()
// TitleBar_SideMargin = 2,

// Paul A McAuley: No longer used - now InternalSettings::buttonSpacingLeft() and InternalSettings::buttonSpacingRight()
// TitleBar_ButtonSpacing = 2,

// shadow dimensions (pixels)
static constexpr int Decoration_Shadow_Overlap = 3;

};

// KSTYLE------------------------------------------------------------------

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
enum BREEZECOMMON_EXPORT AnimationMode {
    AnimationNone = 0,
    AnimationHover = 0x1,
    AnimationFocus = 0x2,
    AnimationEnable = 0x4,
    AnimationPressed = 0x8,
};

Q_DECLARE_FLAGS(AnimationModes, AnimationMode)

//* corners
enum BREEZECOMMON_EXPORT Corner {
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
enum BREEZECOMMON_EXPORT Side {
    SideLeft = 0x1,
    SideTop = 0x2,
    SideRight = 0x4,
    SideBottom = 0x8,
    AllSides = SideLeft | SideTop | SideRight | SideBottom,
};

Q_DECLARE_FLAGS(Sides, Side)

//* checkbox state
enum BREEZECOMMON_EXPORT CheckBoxState {
    CheckOff,
    CheckPartial,
    CheckOn,
    CheckAnimated,
};

//* radio button state
enum BREEZECOMMON_EXPORT RadioButtonState {
    RadioOff,
    RadioOn,
    RadioAnimated,
};

//* arrow orientation
enum BREEZECOMMON_EXPORT ArrowOrientation {
    ArrowNone,
    ArrowUp,
    ArrowDown,
    ArrowLeft,
    ArrowRight,
};

//* button type
enum BREEZECOMMON_EXPORT ButtonType {
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
