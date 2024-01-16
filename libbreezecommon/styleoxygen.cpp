/*
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "styleoxygen.h"

namespace Breeze
{
void RenderStyleOxygen18By18::renderCloseIcon()
{
    QPen pen = m_painter->pen();

    if ((!m_fromKstyle) && m_boldButtonIcons) {
        pen.setWidthF(pen.widthF() * 1.75);
        m_painter->setPen(pen);
    }

    RenderDecorationButtonIcon18By18::renderCloseIcon();
}

void RenderStyleOxygen18By18::renderMaximizeIcon()
{
    QPen pen = m_painter->pen();

    if ((!m_fromKstyle) && m_boldButtonIcons) {
        pen.setWidthF(pen.widthF() * 1.75);
        m_painter->setPen(pen);
    }

    RenderDecorationButtonIcon18By18::renderMaximizeIcon();
}

void RenderStyleOxygen18By18::renderRestoreIcon()
{
    QPen pen = m_painter->pen();

    if ((!m_fromKstyle) && m_boldButtonIcons) {
        pen.setWidthF(pen.widthF() * 1.75);
        m_painter->setPen(pen);
    }

    RenderDecorationButtonIcon18By18::renderRestoreIcon();
}

void RenderStyleOxygen18By18::renderMinimizeIcon()
{
    QPen pen = m_painter->pen();

    if ((!m_fromKstyle) && m_boldButtonIcons) {
        pen.setWidthF(pen.widthF() * 1.75);
        m_painter->setPen(pen);
    }

    RenderDecorationButtonIcon18By18::renderMinimizeIcon();
}

void RenderStyleOxygen18By18::renderKeepBehindIcon()
{
    QPen pen = m_painter->pen();

    if ((!m_fromKstyle) && m_boldButtonIcons) {
        pen.setWidthF(pen.widthF() * 1.4);
        m_painter->setPen(pen);
    }

    RenderDecorationButtonIcon18By18::renderKeepBehindIcon();
}

void RenderStyleOxygen18By18::renderKeepInFrontIcon()
{
    QPen pen = m_painter->pen();

    if ((!m_fromKstyle) && m_boldButtonIcons) {
        pen.setWidthF(pen.widthF() * 1.4);
        m_painter->setPen(pen);
    }

    RenderDecorationButtonIcon18By18::renderKeepInFrontIcon();
}

void RenderStyleOxygen18By18::renderContextHelpIcon()
{
    renderRounderAndBolderContextHelpIcon();
}

}
