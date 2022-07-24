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
    if ((!m_fromKstyle) && m_boldButtonIcons) {
        m_pen.setWidthF(m_pen.widthF() * 1.75);
        m_painter->setPen(m_pen);
    }

    RenderDecorationButtonIcon18By18::renderCloseIcon();
}

void RenderStyleOxygen18By18::renderMaximizeIcon()
{
    if ((!m_fromKstyle) && m_boldButtonIcons) {
        m_pen.setWidthF(m_pen.widthF() * 1.75);
        m_painter->setPen(m_pen);
    }

    RenderDecorationButtonIcon18By18::renderMaximizeIcon();
}

void RenderStyleOxygen18By18::renderRestoreIcon()
{
    if ((!m_fromKstyle) && m_boldButtonIcons) {
        m_pen.setWidthF(m_pen.widthF() * 1.75);
        m_painter->setPen(m_pen);
    }

    RenderDecorationButtonIcon18By18::renderRestoreIcon();
}

void RenderStyleOxygen18By18::renderMinimizeIcon()
{
    if ((!m_fromKstyle) && m_boldButtonIcons) {
        m_pen.setWidthF(m_pen.widthF() * 1.75);
        m_painter->setPen(m_pen);
    }

    RenderDecorationButtonIcon18By18::renderMinimizeIcon();
}

void RenderStyleOxygen18By18::renderKeepBehindIcon()
{
    if ((!m_fromKstyle) && m_boldButtonIcons) {
        m_pen.setWidthF(m_pen.widthF() * 1.4);
        m_painter->setPen(m_pen);
    }

    RenderDecorationButtonIcon18By18::renderKeepBehindIcon();
}

void RenderStyleOxygen18By18::renderKeepInFrontIcon()
{
    if ((!m_fromKstyle) && m_boldButtonIcons) {
        m_pen.setWidthF(m_pen.widthF() * 1.4);
        m_painter->setPen(m_pen);
    }

    RenderDecorationButtonIcon18By18::renderKeepInFrontIcon();
}

void RenderStyleOxygen18By18::renderContextHelpIcon()
{
    renderRounderAndBolderContextHelpIcon();
}

}
