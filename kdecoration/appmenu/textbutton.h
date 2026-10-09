/*
 * Copyright (C) 2020 Chris Holland <zrenfire@gmail.com>
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

#pragma once

#include "appmenu/button.h"

#include <QAction>
#include <QPointer>

namespace Klassy
{
class AppMenuButtonGroup;
class Decoration;

class AppMenuTextButton : public AppMenuButton
{
    Q_OBJECT

public:
    AppMenuTextButton(Decoration *decoration, const int buttonIndex, AppMenuButtonGroup *parent);
    ~AppMenuTextButton() override;

    Q_PROPERTY(QAction *action READ action WRITE setAction NOTIFY actionChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

    void drawContent(QPainter *, QPointF) const override;

    void setFont(QFont font);

    static qreal getTextWidth(const QString text, const QFont font, qreal scale, const bool showMnemonic);

    static qreal ellipsisWidth(const QFont font, qreal scale);

    void setAction(QAction *value)
    {
        if (m_action == value)
            return;
        m_action = value;
        Q_EMIT actionChanged();
    }
    QAction *action() const
    {
        return m_action.data();
    }

    void setHorizontalMargin(qreal value)
    {
        if (qFuzzyCompare(m_horizontalMargin, value))
            return;
        m_horizontalMargin = value;
        updateGeometry();
    }
    qreal horizontalMargin() const
    {
        return m_horizontalMargin;
    }

    void setText(const QString &value)
    {
        if (m_text == value)
            return;
        m_text = value;
        Q_EMIT textChanged();
        updateGeometry();
    }
    QString text() const
    {
        return m_text;
    }

    void setTextSize(const QSizeF &value)
    {
        if (m_textSize == value)
            return;
        m_textSize = value;
    }
    QSizeF textSize()
    {
        return m_textSize;
    }

    bool hasEllipsis()
    {
        return m_hasEllipsis;
    }

    void setHasEllipsis(bool value, bool reversedDirection)
    {
        if (m_hasEllipsis != value) {
            m_hasEllipsis = value;
            updateGeometry();
        }
        m_reversedEllipsisDirection = reversedDirection;
    }

signals:
    void actionChanged();
    void textChanged();

private:
    QSizeF getTextSize() const;
    qreal getTextWidth(bool showMnemonic) const;
    void updateGeometry();

    QPointer<QAction> m_action = nullptr;
    QString m_text = QStringLiteral("Menu");
    qreal m_horizontalMargin = 0;
    QSizeF m_textSize;
    QFont m_font;

    bool m_hasEllipsis = false;
    bool m_reversedEllipsisDirection = false;
};

} // namespace Klassy
