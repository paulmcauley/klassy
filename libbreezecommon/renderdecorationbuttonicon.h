#ifndef BREEZE_RENDERDECORATIONBUTTONICON_H
#define BREEZE_RENDERDECORATIONBUTTONICON_H

/*
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breezecommon_export.h"
#include "breezesettings.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QPainterPath>
#include <memory>

namespace Breeze
{

/**
 * @brief Base Class to render decoration button icons in style set by EnumButtonIconStyle.
 *        Rendering is to be performed on a QPainter object with an 18x18 reference window.
 *        Co-ordinates relative to top-left.
 *        To be used as common code base across both kdecoration and kstyle.
 */
class BREEZECOMMON_EXPORT RenderDecorationButtonIcon18By18
{
public:
    /**
     * @brief Factory to return a pointer to a new inherited object to render in the specified style.
     * @param internalSettings An InternalSettingsPtr from the Window decoration config
     * @param painter A QPainter object already initialised with an 18x18 reference window and pen.
     * @param fromKstyle Indicates that button is not to be drawn in the title bar, but somewhere else in the UI -- ususally means will be smaller
     * @param boldButtonIcons When in titlebar this will draw bolder button icons if true
     * @param iconWidth the unscaled icon width -- used only when the system icon theme is used
     * @param devicePixelRatio the device pixel ratio (set also for X11 from system scale factor)
     * @param iconScaleFactor the amount this icon will be sized (excluding dpr/smallSpacing due to screen scaling)
     * @return std::unique_ptr< Breeze::RenderDecorationButtonIcon18By18, std::default_delete< Breeze::RenderDecorationButtonIcon18By18 > > Pointer to a new
     * sub-style object.
     */
    static std::unique_ptr<RenderDecorationButtonIcon18By18> factory(const QSharedPointer<InternalSettings> internalSettings,
                                                                     QPainter *painter,
                                                                     const bool fromKstyle = false,
                                                                     const bool boldButtonIcons = false,
                                                                     const qreal iconWidth = 18,
                                                                     qreal devicePixelRatio = 1);

    virtual ~RenderDecorationButtonIcon18By18();

    virtual void renderCloseIcon();
    virtual void renderMaximizeIcon();
    virtual void renderRestoreIcon();
    virtual void renderMinimizeIcon();
    virtual void renderPinnedOnAllDesktopsIcon();
    virtual void renderPinOnAllDesktopsIcon();
    virtual void renderShadeIcon();
    virtual void renderUnShadeIcon();
    virtual void renderKeepBehindIcon();
    virtual void renderKeepInFrontIcon();
    virtual void renderApplicationMenuIcon();
    virtual void renderContextHelpIcon();

protected:
    /**
     * @brief Constructor
     *
     * @param internalSettings An InternalSettingsPtr from the Window decoration config
     * @param painter A QPainter object already initialised with an 18x18 reference window and pen.
     * @param fromKstyle Indicates that button is not to be drawn in the title bar, but somewhere else in the UI -- usually means will be smaller
     * @param boldButtonIcons When in titlebar this will draw bolder button icons if true
     * @param iconWidth the unscaled icon width -- used only when the system icon theme is used
     * @param devicePixelRatio the device pixel ratio (set also for X11 from system scale factor)
     * @param iconScaleFactor the amount this icon will be sized (excluding dpr/smallSpacing due to screen scaling)
     */
    RenderDecorationButtonIcon18By18(QPainter *painter, const bool fromKstyle, const bool boldButtonIcons, const qreal devicePixelRatio);

    /**
     * @brief Initialises pen to standardise cap and join styles.
     * No brush is normal for Breeze's simple outline style.
     */
    virtual void initPainter();

    void renderCloseIconAtSquareMaximizeSize();
    qreal renderSquareMaximizeIcon(bool returnSizeOnly = false);

    void renderOverlappingWindowsIcon();
    /**
     * @param isOddPenWidth Whether the pen width, after rounding, is an odd or even number of pixels
     * @param shiftOffset How much to separate the two squares to prevent blurriness
     * @param overlappingWindowsGroup A pointer to output pointing to a group item representing the overlapping windows
     * @param foregroundSquareItem A pointer to output pointing to an item representing the foreground squares
     * @param backgroundSquareItem A pointer to output pointing to an item representing the background square path
     * @return Returns an 18x18 std::unique_ptr<QGraphicsScene> of the overlapping windows icon
     */
    std::unique_ptr<QGraphicsScene> calculateOverlappingWindowsScene(const bool isOddPenWidth,
                                                                     const qreal shiftOffset,
                                                                     QGraphicsItemGroup *&overlappingWindowsGroup,
                                                                     QGraphicsRectItem *&foregroundSquareItem,
                                                                     QGraphicsPathItem *&backgroundSquareItem);

    void renderTinySquareMinimizeIcon();
    void renderKeepBehindIconAsFromBreezeIcons();
    void renderKeepInFrontIconAsFromBreezeIcons();
    void renderRounderAndBolderContextHelpIcon();

    /**
     *@brief Multiplies the pen width by the bolding factor, and rounds it. Also returns whether the integer-rounded bold pen with is an even or odd number of
     *pixels This is useful for pixel alignment
     *
     *@param penWidth The input pen width
     *@param outputRoundedPenWidth The output pen width, factored by boldingFactor, and rounded
     *@param boldingFactor Optional bolding factor. Set to 1 for no bolding
     */
    bool roundedPenWidthIsOdd(const qreal &penWidth, int &outputRoundedPenWidth, const qreal boldingFactor);

    /**
     * @brief Converts between actual device pixels and the number of pixels in this 18x18 reference grid (accounting for all possible scaling)
     *
     * @param devicePixels The input number of actual pixels on the screen
     * @return The equivalent number of pixels in this 18x18 reference grid
     */
    qreal convertDevicePixelsTo18By18(const qreal devicePixels);

    /**
     * @brief Translates painter so antialiased painting co-ordinates will be painted in the same position if aliased
     * @param penWidthOdd Whether the integer pen width is an odd number
     */
    void translatePainterForAliasedPainting(const bool penWidthOdd);

    enum class ThresholdRound { Up, Down };
    /**
     * @brief rounds the given number to the nearest half
     * @param coord The number to round
     * @param roundAtZero whether or not to round up or down at zero
     * @return the rounded number
     */
    qreal roundCoordToHalf(qreal coord, const ThresholdRound roundAtZero);

    /**
     * @brief rounds the given number to the nearest whole
     * @param coord The number to round
     * @param roundAtZero whether or not to round up or down at half
     * @return the rounded number
     */
    qreal roundCoordToWhole(qreal coord, const ThresholdRound roundAtHalf);

    enum class SnapPixel { ToHalf, ToWhole };

    /**
     * @brief Given a logical 18x18 point, snaps it to the nearest half or whole pixel boundary in device pixels, and returns an adjusted equivalent 18x18
     * logical value
     * @param SnapX Whether to snap the X co-ordinate to a half or whole pixel
     * @param SnapY Whether to snap the Y co-ordinate to a half or whole pixel
     * @param roundAtThresholdX Whether to round up or down the X co-ordinate if it is at the threshold
     * @param roundAtThresholdY Whether to round up or down the Y co-ordinate if it is at the threshold
     * @return snapped equivalent in 18x18 logical coordinates
     */
    QPointF snapToNearestPixel(QPointF point18By18,
                               const SnapPixel snapX,
                               const SnapPixel snapY,
                               const ThresholdRound roundAtThresholdX = ThresholdRound::Up,
                               const ThresholdRound roundAtThresholdY = ThresholdRound::Up);

    qreal penWidthTo18By18(const QPen &pen);

    QPainter *m_painter;
    QPen m_pen;
    bool m_fromKstyle;
    bool m_boldButtonIcons;
    qreal m_devicePixelRatio; // unlike getting it directly from the paint device, this DPR is also set for X11, i.e. not just 1 on X11
    qreal m_totalScalingFactor;
    std::unique_ptr<QTransform> m_inverseDeviceTransform;

    //* how much to factor the pen width for a bold restore button
    static constexpr qreal m_overlappingWindowsBoldPenWidthFactor = 1.5;

    //* how much to factor the pen width for a bold square maximize button
    static constexpr qreal m_squareMaximizeBoldPenWidthFactor = 1.5;
};
}

#endif
