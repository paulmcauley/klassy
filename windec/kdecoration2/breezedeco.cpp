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
#include "breezedeco.h"
#include "breezebuttons.h"
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationButtonGroup>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationShadow>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KPluginFactory>

#include <QPainter>

K_PLUGIN_FACTORY_WITH_JSON(BreezeDecoFactory,
                           "breeze.json",
                           registerPlugin<Breeze::Decoration>();)

namespace Breeze
{

ColorSettings::ColorSettings(const QPalette &pal)
{
    init(pal);
}

void ColorSettings::update(const QPalette &pal)
{
    init(pal);
}

void ColorSettings::init(const QPalette &pal)
{
    m_palette = pal;
    KConfigGroup wmConfig(KSharedConfig::openConfig(QStringLiteral("kdeglobals")), QStringLiteral("WM"));
    m_activeFrameColor      = wmConfig.readEntry("frame", pal.color(QPalette::Active, QPalette::Background));
    m_inactiveFrameColor    = wmConfig.readEntry("inactiveFrame", m_activeFrameColor);
    m_activeTitleBarColor   = wmConfig.readEntry("activeBackground", pal.color(QPalette::Active, QPalette::Highlight));
    m_inactiveTitleBarColor = wmConfig.readEntry("inactiveBackground", m_inactiveFrameColor);
    m_activeFontColor       = wmConfig.readEntry("activeForeground", pal.color(QPalette::Active, QPalette::HighlightedText));
    m_inactiveFontColor     = wmConfig.readEntry("inactiveForeground", m_activeFontColor.dark());
}

Decoration::Decoration(QObject *parent, const QVariantList &args)
    : KDecoration2::Decoration(parent)
    , m_colorSettings(client()->palette())
    , m_leftButtons(nullptr)
    , m_rightButtons(nullptr)
{
    Q_UNUSED(args)
    recalculateBorders();
    updateTitleRect();
    connect(KDecoration2::DecorationSettings::self(), &KDecoration2::DecorationSettings::borderSizeChanged, this, &Decoration::recalculateBorders);
    connect(client(), &KDecoration2::DecoratedClient::maximizedHorizontallyChanged, this, &Decoration::recalculateBorders);
    connect(client(), &KDecoration2::DecoratedClient::maximizedVerticallyChanged, this, &Decoration::recalculateBorders);
    connect(client(), &KDecoration2::DecoratedClient::captionChanged, this,
        [this]() {
            // update the caption area
            update(captionRect());
        }
    );
    connect(client(), &KDecoration2::DecoratedClient::activeChanged,    this, [this]() { update(); });
    connect(client(), &KDecoration2::DecoratedClient::paletteChanged,   this,
        [this]() {
            m_colorSettings.update(client()->palette());
            update();
        }
    );
    connect(client(), &KDecoration2::DecoratedClient::widthChanged,     this, &Decoration::updateTitleRect);
    connect(client(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::updateTitleRect);
    connect(client(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::setOpaque);

    connect(client(), &KDecoration2::DecoratedClient::widthChanged,     this, &Decoration::updateButtonPositions);
    connect(client(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::updateButtonPositions);
    connect(client(), &KDecoration2::DecoratedClient::shadedChanged,    this, &Decoration::updateButtonPositions);

    createButtons();
    createShadow();
}

Decoration::~Decoration() = default;

void Decoration::updateTitleRect()
{
    const bool maximized = client()->isMaximized();
    const int width = client()->width();
    const int height = maximized ? borderTop() : borderTop() - 4;
    const int x = maximized ? 0 : 4;
    const int y = maximized ? 0 : 2;
    setTitleRect(QRect(x, y, width, height));
}

static int borderSize(bool bottom) {
    const int baseSize = 4;
    switch (KDecoration2::DecorationSettings::self()->borderSize()) {
    case KDecoration2::BorderSize::None:
        return 0;
    case KDecoration2::BorderSize::NoSides:
        return bottom ? baseSize : 0;
    case KDecoration2::BorderSize::Tiny:
        return baseSize / 2;
    case KDecoration2::BorderSize::Normal:
        return baseSize;
    case KDecoration2::BorderSize::Large:
        return baseSize * 1.5;
    case KDecoration2::BorderSize::VeryLarge:
        return baseSize * 2;
    case KDecoration2::BorderSize::Huge:
        return baseSize * 2.5;
    case KDecoration2::BorderSize::VeryHuge:
        return baseSize * 3;
    case KDecoration2::BorderSize::Oversized:
        return baseSize * 5;
    default:
        return baseSize;
    }
}

static int borderSize() {
    return borderSize(false);
}

void Decoration::recalculateBorders()
{
    int left   = client()->isMaximizedHorizontally() ? 0 : borderSize();
    int right  = client()->isMaximizedHorizontally() ? 0 : borderSize();
    int top    = 25;
    int bottom = client()->isMaximizedVertically() ? 0 : borderSize(true);
    setBorders(left, right, top, bottom);
}

void Decoration::createButtons()
{
    m_leftButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Left, this, &Button::create);
    m_rightButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Right, this, &Button::create);
    m_rightButtons->setSpacing(2.0);
    m_leftButtons->setSpacing(2.0);
    updateButtonPositions();
}

void Decoration::updateButtonPositions()
{
    const int padding = client()->isMaximized() ? 0 : 2;
    m_leftButtons->setPos(QPointF(padding, padding));
    m_rightButtons->setPos(QPointF(size().width() - m_rightButtons->geometry().width() - padding, padding));
}

void Decoration::paint(QPainter *painter)
{
    // paint background
    painter->fillRect(rect(), Qt::transparent);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(m_colorSettings.frame(client()->isActive()));
    painter->setBrush(m_colorSettings.frame(client()->isActive()));
    painter->drawRoundedRect(rect(), 5.0, 5.0);

    paintTitleBar(painter);

    painter->restore();
}

void Decoration::paintTitleBar(QPainter *painter)
{
    const bool active = client()->isActive();
    const QRect titleRect(QPoint(0, 0), QSize(size().width(), borderTop()));
    const QColor titleBarColor(m_colorSettings.titleBarColor(client()->isActive()));
    // render a linear gradient on title area
    QLinearGradient gradient;
    gradient.setStart(0.0, 0.0);
    gradient.setFinalStop(0.0, titleRect.height());
    gradient.setColorAt(0.0, titleBarColor.lighter(client()->isActive() ? 120.0 : 100.0));
    gradient.setColorAt(0.8, titleBarColor);
    gradient.setColorAt(1.0, titleBarColor);
    painter->fillRect(titleRect, gradient);
    const int titleBarSpacer = 3;
    if (true) {
        // TODO: should be config option
        painter->fillRect(0, borderTop() - titleBarSpacer - 1,
                          size().width(), titleBarSpacer + 1,
                          client()->palette().color(active ?  QPalette::Highlight: QPalette::Background));
    }
    // draw title bar spacer
    painter->fillRect(0, borderTop() - titleBarSpacer, size().width(), titleBarSpacer, client()->palette().color(QPalette::Background));

    // draw caption
    const QRect cR = captionRect();
    const QString caption = painter->fontMetrics().elidedText(client()->caption(), Qt::ElideMiddle, cR.width());
    painter->drawText(cR, Qt::AlignCenter | Qt::TextSingleLine, caption);

    // draw all buttons
    m_leftButtons->paint(painter);
    m_rightButtons->paint(painter);
}

QRect Decoration::captionRect() const
{
    const int leftOffset = m_leftButtons->geometry().x() + m_leftButtons->geometry().width();
    const int rightOffset = size().width() - m_rightButtons->geometry().x();
    const int offset = qMax(leftOffset, rightOffset);
    return QRect(offset, 0, size().width() - offset * 2, borderTop());
}

void Decoration::createShadow()
{
    KDecoration2::DecorationShadow *decorationShadow = new KDecoration2::DecorationShadow(this);
    decorationShadow->setPaddingBottom(20);
    decorationShadow->setPaddingRight(20);
    decorationShadow->setPaddingTop(10);
    decorationShadow->setPaddingLeft(10);

    decorationShadow->setTopLeft(QSize(20, 20));
    decorationShadow->setTop(QSize(20, 20));
    decorationShadow->setTopRight(QSize(20, 20));
    decorationShadow->setRight(QSize(20, 20));
    decorationShadow->setBottomRight(QSize(20, 20));
    decorationShadow->setBottom(QSize(20, 20));
    decorationShadow->setBottomLeft(QSize(20, 20));
    decorationShadow->setLeft(QSize(20, 20));

    QImage image(60, 60, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    auto gradientStopColor = [](qreal alpha) {
        QColor color(35, 38, 41);
        color.setAlphaF(alpha);
        return color;
    };
    QRadialGradient radialGradient(20, 20, 20);
    radialGradient.setColorAt(0.0,  gradientStopColor(0.35));
    radialGradient.setColorAt(0.25, gradientStopColor(0.25));
    radialGradient.setColorAt(0.5,  gradientStopColor(0.13));
    radialGradient.setColorAt(0.75, gradientStopColor(0.04));
    radialGradient.setColorAt(1.0,  gradientStopColor(0.0));

    QLinearGradient linearGradient;
    linearGradient.setColorAt(0.0,  gradientStopColor(0.35));
    linearGradient.setColorAt(0.25, gradientStopColor(0.25));
    linearGradient.setColorAt(0.5,  gradientStopColor(0.13));
    linearGradient.setColorAt(0.75, gradientStopColor(0.04));
    linearGradient.setColorAt(1.0,  gradientStopColor(0.0));

    QPainter p(&image);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    // topLeft
    p.fillRect(QRect(0, 0, 20, 20), radialGradient);
    // top
    linearGradient.setStart(20, 20);
    linearGradient.setFinalStop(20, 0);
    p.fillRect(QRect(20, 0, 20, 20), linearGradient);
    // topRight
    radialGradient.setCenter(40.0, 20.0);
    radialGradient.setFocalPoint(40.0, 20.0);
    p.fillRect(QRect(40, 0, 20, 20), radialGradient);
    // left
    linearGradient.setStart(20, 20);
    linearGradient.setFinalStop(0, 20);
    p.fillRect(QRect(0, 20, 20, 20), linearGradient);
    // bottom left
    radialGradient.setCenter(20.0, 40.0);
    radialGradient.setFocalPoint(20.0, 40.0);
    p.fillRect(QRect(0, 40, 20, 20), radialGradient);
    // bottom
    linearGradient.setStart(20, 40);
    linearGradient.setFinalStop(20, 60);
    p.fillRect(QRect(20, 40, 20, 20), linearGradient);
    // bottom right
    radialGradient.setCenter(40.0, 40.0);
    radialGradient.setFocalPoint(40.0, 40.0);
    p.fillRect(QRect(40, 40, 20, 20), radialGradient);
    // right
    linearGradient.setStart(40, 20);
    linearGradient.setFinalStop(60, 20);
    p.fillRect(QRect(40, 20, 20, 20), linearGradient);

    decorationShadow->setShadow(image);

    setShadow(decorationShadow);
}

} // namespace

#include "breezedeco.moc"
