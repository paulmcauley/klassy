#ifndef BREEZE_RENDERDECORATIONBUTTONICON18BY18_H
#define BREEZE_RENDERDECORATIONBUTTONICON18BY18_H

/*
 * SPDX-FileCopyrightText: 2021-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

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
                                     const QPointF &deviceOffsetFromZeroReference);

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
};

}
#endif
