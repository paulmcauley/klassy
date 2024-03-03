/*
 * SPDX-FileCopyrightText: 2021-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "renderdecorationbuttonicon.h"
#include <QGraphicsScene>
#include <memory>

namespace Breeze
{

class BREEZECOMMON_EXPORT RenderDecorationButtonIcon18By18 : public RenderDecorationButtonIcon
{
protected:
    RenderDecorationButtonIcon18By18(QPainter *painter,
                                     const bool fromKstyle,
                                     const bool boldButtonIcons,
                                     const qreal devicePixelRatio,
                                     const QPointF &deviceOffsetFromZeroReference,
                                     const bool forceEvenSquares);

    virtual ~RenderDecorationButtonIcon18By18() = default;

    virtual void renderCloseIcon() override;
    virtual void renderMaximizeIcon() override;
    virtual void renderRestoreIcon() override;
    virtual void renderMinimizeIcon() override;
    virtual void renderPinnedOnAllDesktopsIcon() override;
    virtual void renderPinOnAllDesktopsIcon() override;
    virtual void renderShadeIcon() override;
    virtual void renderUnShadeIcon() override;
    virtual void renderKeepBehindIcon() override;
    virtual void renderKeepInFrontIcon() override;
    virtual void renderApplicationMenuIcon() override;
    virtual void renderContextHelpIcon() override;

    void renderCloseIconAtSquareMaximizeSize();
    std::pair<QRectF, qreal> renderSquareMaximizeIcon(bool returnSizeOnly = false, qreal cornerRelativePercent = 0.025);

    void renderOverlappingWindowsIcon(qreal cornerRelativePercent = -1);
    /**
     * @param shiftOffsetX How much to separate the two squares to prevent blurriness
     * @param shiftOffsetY How much to separate the two squares to prevent blurriness
     * @param overlappingWindowsGroup A pointer to output pointing to a group item representing the overlapping windows
     * @param foregroundSquareItem A pointer to output pointing to an item representing the foreground squares
     * @param backgroundSquareItem A pointer to output pointing to an item representing the background square path
     */
    void calculateBackgroundSquareGeometry(const qreal shiftOffsetX,
                                           const qreal shiftOffsetY,
                                           QGraphicsItemGroup *overlappingWindowsGroup,
                                           QGraphicsPathItem *foregroundSquareItem,
                                           QGraphicsPathItem *&backgroundSquareItem,
                                           qreal halfPenWidthLocal);

    void renderTinySquareMinimizeIcon();
    void renderKeepBehindIconAsFromBreezeIcons();
    void renderKeepInFrontIconAsFromBreezeIcons();
    void renderRounderAndBolderContextHelpIcon();
};

}
