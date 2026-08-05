/*
 * SPDX-FileCopyrightText: 2021-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "renderdecorationbuttonicon.h"
#include <QGraphicsScene>
#include <memory>

namespace Klassy
{

class KLASSYCOMMON_EXPORT RenderDecorationButtonIcon18By18 : public RenderDecorationButtonIcon
{
protected:
    RenderDecorationButtonIcon18By18(QPainter *painter);

    virtual ~RenderDecorationButtonIcon18By18() = default;

    virtual void renderCloseIcon() override;
    virtual void renderMaximizeIcon() override;
    virtual void renderFloatIcon() override;
    virtual void renderMinimizeIcon() override;
    virtual void renderPinnedOnAllDesktopsIcon() override;
    virtual void renderPinOnAllDesktopsIcon() override;
    virtual void renderShadeIcon() override;
    virtual void renderUnShadeIcon() override;
    virtual void renderKeepBehindIcon() override;
    virtual void renderKeepInFrontIcon() override;
    virtual void renderApplicationMenuIcon() override;
    virtual void renderContextHelpIcon() override;
    virtual void renderExcludeFromCaptureIcon() override;
    virtual void renderIntegratedMenuOverflowIcon() override;
    virtual void renderIntegratedMenuSearchIcon() override;

    void renderCloseIconAtSquareMaximizeSize();
    std::pair<QRectF, qreal> renderSquareMaximizeIcon(bool returnSizeOnly = false, qreal cornerRelativePercent = 0.025);
    void renderRectangleMaximizeIcon(const bool dynamicMinimize);

    void renderTinySquareMinimizeIcon(bool showArrows = false);
    void renderCenteredLineMinimizeIcon(bool smaller = false);
    void renderDynamicMinimizeIcon(bool dynamicMinimize = true);
    void renderOnAllDesktopsIcon();
};

}
