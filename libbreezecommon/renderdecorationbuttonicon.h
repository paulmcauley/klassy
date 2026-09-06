/*
 * SPDX-FileCopyrightText: 2021-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "breeze.h"
#include "breezesettings.h"
#include "klassycommon_export.h"

#include <QPainter>
#include <QPainterPath>
#include <memory>

namespace Klassy
{

/**
 * @brief Base Class to render decoration button icons in style set by EnumButtonIconStyle.
 *        To be used as common code base across both kdecoration and kstyle.
 */
class KLASSYCOMMON_EXPORT RenderDecorationButtonIcon
{
public:
    /**
     * @brief Factory to return a pointer to a new inherited object to render in the specified style.
     * @param internalSettings An InternalSettingsPtr from the Window decoration config
     * @param painter A QPainter object already initialised with an 18x18 reference window and pen.
     * @return static std::pair<std::unique_ptr<RenderDecorationButtonIcon>,int> Pointer to a new sub-style object, icon rendering width
     */
    static std::pair<std::unique_ptr<RenderDecorationButtonIcon>, int> factory(const QSharedPointer<InternalSettings> internalSettings, QPainter *painter);

    virtual ~RenderDecorationButtonIcon();

    /**
     * @brief Indicates that button is not to be drawn in the title bar, but somewhere else in the UI -- usually means will be smaller
     */
    void setFromKstyle(bool v)
    {
        m_fromKstyle = v;
    }

    /**
     * @brief boldButtonIcons When in titlebar this will draw bolder button icons if true
     */
    void setBoldButtonIcons(bool v)
    {
        m_boldButtonIcons = v;
    }

    /**
     * @brief systemScale the system scale factor (set also for X11 from system scale factor)
     */
    void setSystemScale(qreal v)
    {
        m_systemScale = v;
    }

    /**
     * @brief The offset of the top-left of this icon from a zero whole-pixel reference point (in device pixels)
     */
    void setDeviceOffsetFromZeroReference(QPointF v)
    {
        m_deviceOffsetFromZeroReference = v;
    }

    /**
     *  @brief When set, instructs the renderer to try to draw squares at an even device size - can help with centring with small button sizes
     * which are also forced even
     */
    void setForceEvenSquares(bool v)
    {
        m_forceEvenSquares = v;
    }

    /**
     * @brief When outputting icons for GTK and the system, closed pens get filled -- this flag is to convert pen strokes to filled paths to fix this
     */
    void setStrokeToFilledPath(bool v)
    {
        m_strokeToFilledPath = v;
    }

    /**
     * @brief indicates the side of the screen on which a panel has a task manager (primary display)
     */
    void setTaskManagerSide(Side taskManagerSide)
    {
        m_taskManagerSide = taskManagerSide;
    }

    /**
     * @brief indicates the type of task manager used on a panel on the primary display
     */
    void setTaskManagerType(TaskManagerType taskManagerType)
    {
        m_taskManagerType = taskManagerType;
    }

    /**
     * @brief After configuring the RenderDecorationButtonIcon object, call this function to paint with QPainter
     */
    void renderIcon(DecorationButtonType type, bool checked);

protected:
    /**
     * @brief Constructor
     *
     * @param painter A QPainter object already initialised with a correctly sized (e.g. 18x18) reference window and pen.
     */
    RenderDecorationButtonIcon(QPainter *painter);

    /**
     * @brief Initialises pen to standardise cap and join styles.
     * No brush is normal for Breeze's simple outline style.
     */
    virtual void initPainter();

    virtual void renderCloseIcon() = 0;
    virtual void renderMaximizeIcon() = 0;
    virtual void renderFloatIcon() = 0;
    virtual void renderMinimizeIcon() = 0;
    virtual void renderPinnedOnAllDesktopsIcon() = 0;
    virtual void renderPinOnAllDesktopsIcon() = 0;
    virtual void renderShadeIcon() = 0;
    virtual void renderUnShadeIcon() = 0;
    virtual void renderKeepBehindIcon() = 0;
    virtual void renderKeepInFrontIcon() = 0;
    virtual void renderApplicationMenuIcon() = 0;
    virtual void renderContextHelpIcon() = 0;
    virtual void renderExcludeFromCaptureIcon() = 0;

    /**
     * @brief Determines whether a rounded version of a given Pen's width is an odd number. Accounts for cosmetic/non-cosmetic pens.
     *
     *@param pen The input QPen
     *@param outputRoundedPenWidth The output pen width, factored by boldingFactor, and rounded
     *@param boldingFactor Optional bolding factor. Set to 1 for no bolding
     */
    bool roundedPenWidthIsOdd(const QPen &pen);

    /**
     *@brief Multiplies the pen width by the bolding factor, and rounds it. Also returns whether the integer-rounded bold pen with is an even or odd number of
     *pixels This is useful for pixel alignment.  Accounts for cosmetic/non-cosmetic pens.
     *
     *@param pen The input QPen
     *@param outputRoundedPenWidth The output pen width, factored by boldingFactor, and rounded
     *@param boldingFactor Optional bolding factor. Set to 1 for no bolding
     */
    bool roundedPenWidthIsOdd(const QPen &pen, qreal &outputRoundedPenWidth, const qreal boldingFactor);

    /**
     * @brief Converts between actual device pixels and the number of pixels in the local reference grid (accounting for all possible scaling)
     *
     * @param devicePixels The input number of actual pixels on the screen
     * @return The equivalent number of pixels in this local (e.g. 18x18) reference grid
     */
    qreal convertDevicePixelsToLocal(const qreal devicePixels);

    /**
     * @brief Converts from pixels in the local reference grid to device pixels (accounting for all possible scaling)
     *
     * @param localPixels The input number of pixels on the current local reference grid
     * @return The equivalent number of device pixels
     */
    qreal convertLocalPixelsToDevice(const qreal localPixels);

    /**
     * @brief Translates painter so antialiased painting co-ordinates will be painted in the same position if aliased
     * @param penWidthOdd Whether the integer pen width is an odd number
     */
    void translatePainterForAliasedPainting(const bool penWidthOdd);

    enum class ThresholdRound {
        Up,
        Down
    };
    /**
     * @brief rounds the given number to the nearest half
     * @param coord The number to round
     * @param roundAtZero whether or not to round up or down at zero
     * @return the rounded number
     */
    qreal roundCoordToHalf(qreal coord, const ThresholdRound roundAtZero = ThresholdRound::Up);

    /**
     * @brief rounds the given number to the nearest whole
     * @param coord The number to round
     * @param roundAtHalf whether or not to round up or down at half
     * @return the rounded number
     */
    qreal roundCoordToWhole(qreal coord, const ThresholdRound roundAtHalf = ThresholdRound::Up);

    enum class SnapPixel {
        ToHalf,
        ToWhole
    };

    /**
     * @brief Given a local point, snaps it to the nearest half or whole pixel boundary in device pixels, and returns an adjusted equivalent local point
     * @param pointLocal input point in local coordinates to snap
     * @param SnapX Whether to snap the X co-ordinate to a half or whole pixel
     * @param SnapY Whether to snap the Y co-ordinate to a half or whole pixel
     * @param roundAtThresholdX Whether to round up or down the X co-ordinate if it is at the threshold
     * @param roundAtThresholdY Whether to round up or down the Y co-ordinate if it is at the threshold
     * @return pixel-snapped equivalent in local logical coordinates
     */
    QPointF snapToNearestPixel(QPointF pointLocal,
                               const SnapPixel snapX,
                               const SnapPixel snapY,
                               const ThresholdRound roundAtThresholdX = ThresholdRound::Up,
                               const ThresholdRound roundAtThresholdY = ThresholdRound::Up);
    /**
     * @brief Overrloaded function. Given a local point, snaps it to the nearest half or whole pixel boundary in device pixels, and returns an adjusted
     * equivalent local point This overloaded version automatically determines whether to snap to a half (if input is 0.5), or otherwise whole, pixel.
     * @param pointLocal input point in local coordinates to snap
     * @return pixel-snapped equivalent in local logical coordinates
     *
     */
    QPointF snapToNearestPixel(const QPointF pointLocal);

    /**
     * @brief given a pen, returns it in local co-ordinates, accounting for whether the pen is cosmetic or not
     * @param pen the input pen
     * @return the pen width in local coordinates
     */
    qreal penWidthToLocal(const QPen &pen);

    /**
     * @brief given a pen, returns it in device co-ordinates, accounting for whether the pen is cosmetic or not
     * @param pen the input pen
     * @return the pen width in device coordinates
     */
    qreal penWidthToDevice(const QPen &pen);

    /**
     * @brief Sometimes the diagonals of a close button look fainter than a straight line, so reduce the opacity of the straight lines to compensate
     * @return Returns the opacity for a straight line
     */
    qreal straightLineOpacity();

    QPainter *m_painter;
    bool m_fromKstyle = false;
    bool m_boldButtonIcons = false;
    qreal m_systemScale = 1.0; // unlike getting it directly from the paint device, this scale is also set for X11, i.e. not just 1 on X11
    qreal m_totalScalingFactor;
    QPointF m_deviceOffsetFromZeroReference = QPointF(0, 0);
    bool m_forceEvenSquares = false;
    bool m_strokeToFilledPath =
        false; // When outputting icons for GTK and the system, closed pens get filled -- this flag is to convert pen strokes to filled paths to fix this
    Side m_taskManagerSide = SideBottom;
    TaskManagerType m_taskManagerType = TaskManagerType::IconsAndTextTaskManager;

    //* how much to factor the pen width for a bold restore button
    static constexpr qreal m_overlappingWindowsBoldPenWidthFactor = 1.5;

    //* how much to factor the pen width for a bold square maximize button
    static constexpr qreal m_squareMaximizeBoldPenWidthFactor = 1.5;
};
}
