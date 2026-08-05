/*
 * Copyright (C) 2020 Chris Holland <zrenfire@gmail.com>
 * Copyright (C) 2016 Kai Uwe Broulik <kde@privat.broulik.de>
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "appmenu/textbutton.h"
#include "breezedecoration.h"

#include <QPainter>

namespace Breeze
{

AppMenuTextButton::AppMenuTextButton(Decoration *decoration, const int buttonIndex, QObject *parent)
    : AppMenuButton(decoration, buttonIndex, parent)
    , m_action(nullptr)
    , m_text(QStringLiteral("Menu"))
{
    setVisible(true);
    updateGeometry();
}

AppMenuTextButton::~AppMenuTextButton()
{
}

void AppMenuTextButton::paintIcon(QPainter *painter, const QRectF &iconRect, const qreal)
{
    const auto *deco = qobject_cast<Decoration *>(decoration());

    // Font
    painter->setFont(deco->menuFont());
    painter->setPen(deco->fontColor());
    painter->setRenderHint(QPainter::TextAntialiasing);

    // TODO: Use Qt::TextShowMnemonic when Alt is pressed
    const bool isAltPressed = false;
    const Qt::TextFlag mnemonicFlag = isAltPressed ? Qt::TextShowMnemonic : Qt::TextHideMnemonic;
    // translating 1 pixel seems the only way to perfect centering the text.
    painter->drawText(iconRect.translated(1.0, 0.0), mnemonicFlag | Qt::AlignCenter | Qt::TextSingleLine, m_text);
}

QSizeF AppMenuTextButton::getTextSize() const
{
    const auto *deco = qobject_cast<const Decoration *>(decoration());
    if (!deco) {
        return QSizeF(0, 0);
    }

    const qreal textWidth = deco->getMenuTextWidth(m_text);
    const qreal titleBarHeight = deco->titleBarHeight();
    return QSizeF(textWidth, titleBarHeight);
}

QAction *AppMenuTextButton::action() const
{
    return m_action.data();
}

void AppMenuTextButton::setAction(QAction *set)
{
    if (m_action != set) {
        m_action = set;
        Q_EMIT actionChanged();
    }
}

QString AppMenuTextButton::text() const
{
    return m_text;
}

void AppMenuTextButton::setText(const QString &set)
{
    if (m_text != set) {
        m_text = set;
        Q_EMIT textChanged();

        updateGeometry();
    }
}

void AppMenuTextButton::setHeight(qreal buttonHeight)
{
    Q_UNUSED(buttonHeight)

    updateGeometry();
}

void AppMenuTextButton::setHorzPadding(qreal value)
{
    if (!qFuzzyCompare(m_horzPadding, value)) {
        m_horzPadding = value;
        updateGeometry();
    }
}

qreal AppMenuTextButton::horzPadding() const
{
    return m_horzPadding;
}

void AppMenuTextButton::updateGeometry()
{
    const QSizeF textSize = getTextSize();
    const qreal width = textSize.width() + m_horzPadding * 2;
    setGeometry(QRectF(geometry().topLeft(), QSizeF(width, textSize.height())));
}

} // namespace Breeze
