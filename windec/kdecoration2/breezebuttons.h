/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef BREEZE_BUTTONS_H
#define BREEZE_BUTTONS_H
#include <KDecoration2/DecorationButton>
#include "breezedeco.h"

#include <QHash>
#include <QImage>
uint qHash(const QPalette &pal);

namespace Breeze
{

enum class ButtonState : uint {
    Normal,
    Disabled,
    Hovered,
    Pressed,
    Checked,
    CheckedHovered,
    CheckedPressed
};

inline uint qHash(const ButtonState &state) {
    return static_cast<uint>(state);
}

class ImageProvider final
{
public:
    ~ImageProvider();
    static ImageProvider *self();
    const QImage &button(KDecoration2::DecorationButton *decorationButton);
    void clearCache(KDecoration2::DecorationButton *decorationButton);

    void invalidate();

private:
    ImageProvider();
    QImage renderButton(KDecoration2::DecorationButton *decorationButton) const;
    void renderCloseButton(QPainter *p, KDecoration2::DecorationButton *decorationButton) const;
    void renderShadeButton(QPainter *p, KDecoration2::DecorationButton *decorationButton) const;
    void renderMaximizeButton(QPainter *p, KDecoration2::DecorationButton *decorationButton) const;
    void renderOnAllDesktopsButton(QPainter *p, KDecoration2::DecorationButton *decorationButton) const;
    void drawGenericButtonBackground(QPainter *painter, KDecoration2::DecorationButton *decorationButton) const;
    void drawBackground(QPainter *painter, KDecoration2::DecorationButton *decorationButton, const QColor &color) const;
    void drawDownArrow(QPainter *painter, KDecoration2::DecorationButton *decorationButton, const QPointF &offset = QPointF(0.0, 0.0)) const;
    void drawUpArrow(QPainter *painter, KDecoration2::DecorationButton *decorationButton, const QPointF &offset = QPointF(0.0, 0.0)) const;
    QColor foregroundColor(KDecoration2::DecorationButton *decorationButton) const;
    const ColorSettings &colorSettings(KDecoration2::DecorationButton *decorationButton) const;
    const ColorSettings &colorSettings(const QPalette &pal) const;
    static ImageProvider *s_self;
    typedef QHash<ButtonState, QImage> ImagesForButtonState;
    typedef QHash<bool, ImagesForButtonState> ImagesForDecoState;
    typedef QHash<KDecoration2::DecorationButtonType, ImagesForDecoState> ImagesForButton;
    QHash<QPalette, ImagesForButton> m_images;
    QList<ColorSettings> m_colorSettings;
};

class Button : public KDecoration2::DecorationButton
{
    Q_OBJECT
public:

    virtual ~Button();
    void paint(QPainter *painter) override;
    static Button *create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent);

private:
    explicit Button(KDecoration2::DecorationButtonType type, Decoration *decoration, QObject *parent = nullptr);
};

} // namespace

#endif
