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

namespace Klassy
{

AppMenuTextButton::AppMenuTextButton(Decoration *decoration, const int buttonIndex, AppMenuButtonGroup *parent)
    : AppMenuButton(DecorationButtonType::CustomAppMenuBarMenu, decoration, buttonIndex, parent)
{
    setVisible(true);
    connect(this, &AppMenuButton::buttonHeightChanged, this, &AppMenuTextButton::updateGeometry);
    connect(this, &AppMenuButton::verticalBackgroundOffsetChanged, this, &AppMenuTextButton::updateGeometry);
    reconfigure();
    updateGeometry();
}

AppMenuTextButton::~AppMenuTextButton() = default;

void AppMenuTextButton::drawContent(QPainter *painter, QPointF offsetDecorationTopLeftToContentTopLeft) const
{
    // Font
    painter->setFont(m_font);
    QColor foreground = foregroundColor();
    if (!foreground.isValid())
        foreground = m_d->fontColor();
    painter->setPen(foreground);
    painter->setRenderHint(QPainter::TextAntialiasing);

    // TODO: If it becomes possible to listen to alt down/up and alt shortcuts, render the mnemonics.
    // Without notice that alt has been pressed, we can't easily tell KWin the button has an update to render.
    // And without being able to listen for alt shortcuts, we currently can't implement these shortcuts anyways.
    // const bool isAltPressed = (QGuiApplication::keyboardModifiers() & Qt::AltModifier) != 0;
    const bool isAltPressed = false;
    const Qt::TextFlag mnemonicFlag = isAltPressed ? Qt::TextShowMnemonic : Qt::TextHideMnemonic;
    QString text = m_text;
    QSizeF textSize = m_textSize;

    if (m_hasEllipsis) {
        QString ellipsis = QStringLiteral("...");
        if (m_reversedEllipsisDirection) {
            text = ellipsis + text;
        } else {
            text = text + ellipsis;
        }
    }
    painter->drawText(QRectF(geometry().topLeft() - offsetDecorationTopLeftToContentTopLeft + QPointF(0, verticalBackgroundOffset()), textSize),
                      mnemonicFlag | Qt::AlignCenter | Qt::TextSingleLine,
                      text);
}

void AppMenuTextButton::setFont(QFont font)
{
    m_font = font;
}

QSizeF AppMenuTextButton::getTextSize() const
{
    if (!m_d) {
        return QSizeF(0, 0);
    }

    const qreal textWidth = getTextWidth(false);
    const qreal captionHeight = m_d->captionHeight();
    return QSizeF(textWidth, captionHeight);
}

qreal AppMenuTextButton::getTextWidth(bool showMnemonic) const
{
    return getTextWidth(m_text, m_font, m_d->window()->scale(), showMnemonic);
}

qreal AppMenuTextButton::getTextWidth(const QString text, const QFont font, qreal scale, const bool showMnemonic)
{
    const QFontMetricsF fontMetrics(font);
    const int flags = showMnemonic ? Qt::TextShowMnemonic : Qt::TextHideMnemonic;
    const QRectF boundingRect = fontMetrics.boundingRect(QRectF(), flags, text);
    return qCeil(boundingRect.width() * scale) / scale;
}

qreal AppMenuTextButton::ellipsisWidth(const QFont font, qreal scale)
{
    return getTextWidth(QStringLiteral("..."), font, scale, false);
}

void AppMenuTextButton::updateGeometry()
{
    QSizeF textSize = getTextSize();
    if (m_hasEllipsis) {
        textSize = QSizeF(textSize.width() + ellipsisWidth(m_font, m_d->window()->scale()), textSize.height());
    }
    const qreal width = textSize.width() + m_horizontalMargin * 2;
    const QSizeF size = QSizeF(width, buttonHeight());
    setGeometry(QRectF(geometry().topLeft(), size));
    setBackgroundVisibleSize(QSizeF(size.width(), buttonHeight() - verticalBackgroundOffset() * 2));
    setTextSize(QSizeF(size.width(), textSize.height()));
}

} // namespace Klassy
