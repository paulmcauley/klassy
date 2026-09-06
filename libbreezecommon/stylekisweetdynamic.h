/*
 * SPDX-FileCopyrightText: 2026 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "renderdecorationbuttonicon18by18.h"

#include <QGraphicsScene>
#include <QPainter>
#include <memory>

namespace Breeze
{

class RenderStyleKisweetDynamic18By18 : public RenderDecorationButtonIcon18By18
{
public:
    RenderStyleKisweetDynamic18By18(QPainter *painter)
        : RenderDecorationButtonIcon18By18(painter) { };

    void renderCloseIcon() override;
    void renderMaximizeIcon() override;
    void renderFloatIcon() override;
    void renderMinimizeIcon() override;
    void renderShadeIcon() override;
    void renderUnShadeIcon() override;

private:
};

}
