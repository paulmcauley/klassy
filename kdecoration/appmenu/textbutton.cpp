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

#include <QApplication>
#include <QPainter>

namespace Breeze
{

AppMenuTextButton::AppMenuTextButton(Decoration *decoration, const int buttonIndex, QObject *parent)
    : AppMenuButton(DecorationButtonType::CustomIntegratedMenuMenu, decoration, buttonIndex, parent)
{
    setVisible(true);
    connect(this, &AppMenuButton::buttonHeightChanged, this, &AppMenuTextButton::updateGeometry);
    reconfigure();
    updateGeometry();
}

AppMenuTextButton::~AppMenuTextButton() = default;

void AppMenuTextButton::drawContent(QPainter *painter, QPointF offsetDecorationTopLeftToContentTopLeft) const
{
    // Font
    painter->setFont(m_font);
    QColor foreground = foregroundColor();
    if (!foreground.isValid() || m_d->internalSettings()->integratedMenuButtonUseTitleColor())
        foreground = m_d->fontColor();
    painter->setPen(foreground);
    painter->setRenderHint(QPainter::TextAntialiasing);

    // TODO: If it becomes possible to listen to alt down/up and alt shortcuts, render the mnemonics.
    // Without notice that alt has been pressed, we can't easily tell KWin the button has an update to render.
    // And without being able to listen for alt shortcuts, we currently can't implement these shortcuts anyways.
    // const bool isAltPressed = (QGuiApplication::keyboardModifiers() & Qt::AltModifier) != 0;
    const bool isAltPressed = false;
    const Qt::TextFlag mnemonicFlag = isAltPressed ? Qt::TextShowMnemonic : Qt::TextHideMnemonic;
    painter->drawText(QRectF(geometry().topLeft() - offsetDecorationTopLeftToContentTopLeft, m_textSize),
                      mnemonicFlag | Qt::AlignCenter | Qt::TextSingleLine,
                      m_text);
}

void AppMenuTextButton::reconfigure()
{
    AppMenuButton::reconfigure();

    m_font = m_d->internalSettings()->integratedMenuButtonUseSystemMenuFont() ? QApplication::font("QMenu") : m_d->settings()->font();
}

QSizeF AppMenuTextButton::getTextSize() const
{
    if (!m_d) {
        return QSizeF(0, 0);
    }

    const qreal textWidth = getTextWidth(false);
    const qreal titleBarHeight = m_d->titleBarHeight();
    return QSizeF(textWidth, titleBarHeight);
}

qreal AppMenuTextButton::getTextWidth(bool showMnemonic) const
{
    const QFontMetricsF fontMetrics(m_font);
    const int flags = showMnemonic ? Qt::TextShowMnemonic : Qt::TextHideMnemonic;
    const QRectF boundingRect = fontMetrics.boundingRect(QRectF(), flags, m_text);
    const qreal scale = m_d->window()->nextScale();
    return qCeil(boundingRect.width() * scale) / scale;
}

void AppMenuTextButton::updateGeometry()
{
    const QSizeF textSize = getTextSize();
    const qreal width = textSize.width() + m_horizontalPadding * 2;
    const QSizeF size = QSizeF(width, buttonHeight());
    setGeometry(QRectF(geometry().topLeft(), size));
    setBackgroundVisibleSize(QSizeF(size.width(), buttonHeight()));
    setTextSize(QSizeF(size.width(), textSize.height()));
}

} // namespace Breeze
