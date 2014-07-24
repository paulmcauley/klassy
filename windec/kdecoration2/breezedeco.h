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
#ifndef BREEZE_DECORATION_H
#define BREEZE_DECORATION_H

#include <KDecoration2/Decoration>

#include <QColor>
#include <QPalette>
#include <QVariant>

namespace KDecoration2
{
    class DecorationButton;
    class DecorationButtonGroup;
}

namespace Breeze
{

// TODO: move to deco API
class ColorSettings
{
public:
    ColorSettings(const QPalette &pal);

    void update(const QPalette &pal);

    const QColor &titleBarColor(bool active) const {
        return active ? m_activeTitleBarColor : m_inactiveTitleBarColor;
    }
    const QColor &activeTitleBarColor() const {
        return m_activeTitleBarColor;
    }
    const QColor &inactiveTitleBarColor() const {
        return m_inactiveTitleBarColor;
    }
    const QColor &frame(bool active) const {
        return active ? m_activeFrameColor : m_inactiveFrameColor;
    }
    const QColor &activeFrame() const {
        return m_activeFrameColor;
    }
    const QColor &inactiveFrame() const {
        return m_inactiveFrameColor;
    }
    const QColor &font(bool active) const {
        return active ? m_activeFontColor : m_inactiveFontColor;
    }
    const QColor &activeFont() const {
        return m_activeFontColor;
    }
    const QColor &inactiveFont() const {
        return m_inactiveFontColor;
    }
    const QPalette &palette() const {
        return m_palette;
    }
private:
    void init(const QPalette &pal);
    QColor m_activeTitleBarColor;
    QColor m_inactiveTitleBarColor;
    QColor m_activeFrameColor;
    QColor m_inactiveFrameColor;
    QColor m_activeFontColor;
    QColor m_inactiveFontColor;
    QPalette m_palette;
};

class Decoration : public KDecoration2::Decoration
{
    Q_OBJECT
public:
    explicit Decoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    virtual ~Decoration();

    void paint(QPainter *painter) override;

    const ColorSettings &colorSettings() {
        return m_colorSettings;
    }

private Q_SLOTS:
    void recalculateBorders();
    void updateButtonPositions();
    void updateTitleRect();

private:
    QRect captionRect() const;
    void createButtons();
    void paintTitleBar(QPainter *painter);
    void createShadow();
    ColorSettings m_colorSettings;
    QList<KDecoration2::DecorationButton*> m_buttons;
    KDecoration2::DecorationButtonGroup *m_leftButtons;
    KDecoration2::DecorationButtonGroup *m_rightButtons;
};

} // namespace

#endif
