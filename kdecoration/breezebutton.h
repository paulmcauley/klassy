#ifndef BREEZE_BUTTONS_H
#define BREEZE_BUTTONS_H

/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "breezedecoration.h"
#include <KDecoration2/DecorationButton>

#include <QHash>
#include <QImage>

class QVariantAnimation;

namespace Breeze
{

class Button : public KDecoration2::DecorationButton
{
    Q_OBJECT

public:
    //* constructor
    explicit Button(QObject *parent, const QVariantList &args);

    //* destructor
    virtual ~Button() = default;

    //* button creation
    static Button *create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent);

    //* render
    virtual void paint(QPainter *painter, const QRect &repaintRegion) override;

    //* flag
    enum Flag {
        FlagNone,
        FlagStandalone,
        FlagFirstInList,
        FlagLastInList,
    };

    //* flag
    void setFlag(Flag value)
    {
        m_flag = value;
    }

    //* standalone buttons
    bool isStandAlone() const
    {
        return m_flag == FlagStandalone;
    }

    //*set position of buttons which are both enabled and visible
    void setLeftmostLeftVisible(bool value = true)
    {
        m_leftmostLeftVisible = value;
    }

    void setRightmostLeftVisible(bool value = true)
    {
        m_rightmostLeftVisible = value;
    }

    void setLeftmostRightVisible(bool value = true)
    {
        m_leftmostRightVisible = value;
    }

    void setRightmostRightVisible(bool value = true)
    {
        m_rightmostRightVisible = value;
    }

    //* offset for drawing icon
    void setIconOffset(const QPointF &value)
    {
        m_iconOffset = value;
    }

    //* horizontal offset, for rendering icon
    void setHorizontalIconOffset(qreal value)
    {
        m_iconOffset.setX(value);
    }

    //* vertical offset, for rendering icon
    void setVerticalIconOffset(qreal value)
    {
        m_iconOffset.setY(value);
    }

    //* offset for drawing full-sized background -- used for the far left button
    void setFullHeightVisibleBackgroundOffset(const QPointF &value)
    {
        m_fullHeightVisibleBackgroundOffset = value;
    }

    //* geometry for rendering a visible button background- can differ from button geometry
    void setBackgroundVisibleSize(const QSizeF &value)
    {
        m_backgroundVisibleSize = value;
    }

    //* set small button padded size
    void setSmallButtonPaddedSize(const QSize &value)
    {
        m_smallButtonPaddedSize = value;
    }

    //* set icon size
    void setIconSize(const QSize &value)
    {
        m_iconSize = value;
    }

    //*@name active state change animation
    //@{
    void setOpacity(qreal value)
    {
        if (m_opacity == value) {
            return;
        }
        m_opacity = value;
        update();
    }

    qreal opacity() const
    {
        return m_opacity;
    }

    //@}

private Q_SLOTS:

    //* apply configuration changes
    void reconfigure();

    //* animation state
    void updateAnimationState(bool);

    //* get colour and trigger same in thin window outline
    void updateThinWindowOutlineWithButtonColor(bool);

private:
    //* private constructor
    explicit Button(KDecoration2::DecorationButtonType type, Decoration *decoration, QObject *parent = nullptr);

    //* draw button icon
    void drawIcon(QPainter *) const;

    //*@name colors
    //@{
    QColor backgroundColor(bool getNonAnimatedColor = false) const;
    QColor backgroundColor(QColor &foregroundContrastedColor, bool getNonAnimatedColor = false) const;
    QColor foregroundColor(const QColor &backgroundContrastedColor) const;
    QColor outlineColor(bool getNonAnimatedColor = false) const;
    //@}

    bool isSystemIconAvailable();
    void setDevicePixelRatio(QPainter *painter);
    void setShouldDrawBoldButtonIcons();
    void setStandardScaledPenWidth();

    /**
     * @brief Paint the button background for the Full-sized Rectangle button shape;
     * @param painter Current QPainter object.
     */
    void paintFullHeightButtonBackground(QPainter *painter) const;

    /**
     * @brief Paint the button background for the small circle button shape as originally done in Breeze
     *        Also paints the small square or small rounded square shapes
     * @param painter Current QPainter object
     */
    void paintSmallSizedButtonBackground(QPainter *painter) const;

    //* these are the actual colors to be outputted (including for animations)
    QColor m_backgroundColor;
    QColor m_foregroundColor;
    QColor m_outlineColor;

    //* these are the cached non-animated colours used for animations
    QColor m_previousForegroundColor;
    QColor m_previousBackgroundColor;
    QColor m_previousOutlineColor;

    Flag m_flag = FlagNone;

    bool m_leftmostLeftVisible = false;
    bool m_rightmostLeftVisible = false;
    bool m_leftmostRightVisible = false;
    bool m_rightmostRightVisible = false;

    //* active state change animation
    QVariantAnimation *m_animation;

    //* icon offset (for rendering)
    QPointF m_iconOffset;

    //* offset for rendering a large or Full-sized background
    QPointF m_fullHeightVisibleBackgroundOffset = QPointF();

    //* geometry for rendering a Full-sized button
    QSizeF m_backgroundVisibleSize = QSizeF();

    //* small button size (icon + padding)
    QSize m_smallButtonPaddedSize = QSize();

    //* icon size
    QSize m_iconSize = QSize();

    //* active state change opacity
    qreal m_opacity = 0;

    bool m_systemIconIsAvailable;
    bool m_lowContrastBetweenTitleBarAndBackground = false;
    bool m_isGtkCsdButton;
    qreal m_devicePixelRatio = 1.0;
    bool m_boldButtonIcons;
    qreal m_standardScaledPenWidth = 1.0;
};

} // namespace

#endif
