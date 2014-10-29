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
}

Decoration::~Decoration() = default;

void Decoration::init()
{
    recalculateBorders();
    updateTitleRect();
    auto s = settings();
    connect(s.data(), &KDecoration2::DecorationSettings::borderSizeChanged, this, &Decoration::recalculateBorders);
    // a change in font might cause the borders to change
    connect(s.data(), &KDecoration2::DecorationSettings::fontChanged, this, &Decoration::recalculateBorders);
    connect(s.data(), &KDecoration2::DecorationSettings::spacingChanged, this, &Decoration::recalculateBorders);
    connect(client().data(), &KDecoration2::DecoratedClient::borderingScreenEdgesChanged, this, &Decoration::recalculateBorders);
    connect(client().data(), &KDecoration2::DecoratedClient::maximizedHorizontallyChanged, this, &Decoration::recalculateBorders);
    connect(client().data(), &KDecoration2::DecoratedClient::maximizedVerticallyChanged, this, &Decoration::recalculateBorders);
    connect(client().data(), &KDecoration2::DecoratedClient::captionChanged, this,
        [this]() {
            // update the caption area
            update(captionRect());
        }
    );
    connect(client().data(), &KDecoration2::DecoratedClient::activeChanged,    this, [this]() { update(); });
    connect(client().data(), &KDecoration2::DecoratedClient::paletteChanged,   this,
        [this]() {
            m_colorSettings.update(client()->palette());
            update();
        }
    );
    connect(client().data(), &KDecoration2::DecoratedClient::widthChanged,     this, &Decoration::updateTitleRect);
    connect(client().data(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::updateTitleRect);
    connect(client().data(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::setOpaque);

    connect(client().data(), &KDecoration2::DecoratedClient::widthChanged,     this, &Decoration::updateButtonPositions);
    connect(client().data(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::updateButtonPositions);
    connect(client().data(), &KDecoration2::DecoratedClient::shadedChanged,    this, &Decoration::updateButtonPositions);

    createButtons();
    createShadow();
}

void Decoration::updateTitleRect()
{
    auto s = settings();
    const bool maximized = client()->isMaximized();
    const int width = client()->width();
    const int height = maximized ? borderTop() : borderTop() - s->smallSpacing();
    const int x = maximized ? 0 : s->largeSpacing() / 2;
    const int y = maximized ? 0 : s->smallSpacing();
    setTitleRect(QRect(x, y, width, height));
}

static int borderSize(const QSharedPointer<KDecoration2::DecorationSettings> &settings, bool bottom) {
    const int baseSize = settings->largeSpacing() / 2;
    switch (settings->borderSize()) {
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

static int borderSize(const QSharedPointer<KDecoration2::DecorationSettings> &settings) {
    return borderSize(settings, false);
}

void Decoration::recalculateBorders()
{
    auto s = settings();
    const Qt::Edges edges = client()->borderingScreenEdges();
    int left   = client()->isMaximizedHorizontally() || edges.testFlag(Qt::LeftEdge) ? 0 : borderSize(s);
    int right  = client()->isMaximizedHorizontally() || edges.testFlag(Qt::RightEdge) ? 0 : borderSize(s);

    QFontMetrics fm(s->font());
    int top = qMax(fm.boundingRect(client()->caption()).height(), s->gridUnit() * 2);
    // padding below
    top += s->smallSpacing() * 2 + 1;
    if (!client()->isMaximized()) {
        // padding above only on maximized
        top += s->smallSpacing();
    }

    int bottom = client()->isMaximizedVertically() || edges.testFlag(Qt::BottomEdge) ? 0 : borderSize(s, true);
    setBorders(left, right, top, bottom);

    const int extSize = s->largeSpacing() / 2;
    int extSides = 0;
    int extBottom = 0;
    if (s->borderSize() == KDecoration2::BorderSize::None) {
        extSides = extSize;
        extBottom = extSize;
    } else if (s->borderSize() == KDecoration2::BorderSize::NoSides) {
        extSides = extSize;
    }
    setExtendedBorders(extSides, extSides, 0, extBottom);
}

void Decoration::createButtons()
{
    m_leftButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Left, this, &Button::create);
    m_rightButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Right, this, &Button::create);
    updateButtonPositions();
}

void Decoration::updateButtonPositions()
{
    auto s = settings();
    const int padding = client()->isMaximized() ? 0 : s->smallSpacing();
    m_rightButtons->setSpacing(s->smallSpacing());
    m_leftButtons->setSpacing(s->smallSpacing());
    m_leftButtons->setPos(QPointF(padding, padding));
    m_rightButtons->setPos(QPointF(size().width() - m_rightButtons->geometry().width() - padding, padding));
}

void Decoration::paint(QPainter *painter)
{
    // paint background
    painter->fillRect(rect(), Qt::transparent);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_colorSettings.frame(client()->isActive()));
    // clip away the top part
    painter->save();
    painter->setClipRect(0, borderTop(), size().width(), size().height() - borderTop(), Qt::IntersectClip);
    painter->drawRoundedRect(rect(), 5.0, 5.0);
    painter->restore();

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
    gradient.setFinalStop(0.0, titleRect.height());
    painter->save();
    painter->setBrush(gradient);
    painter->setPen(Qt::NoPen);
    if (client()->isMaximized()) {
        painter->drawRect(titleRect);
    } else {
        painter->setClipRect(titleRect, Qt::IntersectClip);
        // we make the rect a little bit larger to be able to clip away the rounded corners on bottom
        painter->drawRoundedRect(titleRect.adjusted(0, 0, 0, 5), 5.0, 5.0);
    }
    painter->restore();
    auto s = settings();
    const int titleBarSpacer = s->smallSpacing();
    if (true) {
        // TODO: should be config option
        painter->fillRect(0, borderTop() - titleBarSpacer - 1,
                          size().width(), 1,
                          client()->palette().color(active ?  QPalette::Highlight: QPalette::Background));
    }
    // draw title bar spacer
    painter->fillRect(0, borderTop() - titleBarSpacer, size().width(), titleBarSpacer, client()->palette().color(QPalette::Background));

    // draw caption
    painter->setFont(s->font());
    const QRect cR = captionRect();
    const QString caption = painter->fontMetrics().elidedText(client()->caption(), Qt::ElideMiddle, cR.width());
    painter->setPen(m_colorSettings.font(client()->isActive()));
    painter->drawText(cR, Qt::AlignCenter | Qt::TextSingleLine, caption);

    // draw all buttons
    m_leftButtons->paint(painter);
    m_rightButtons->paint(painter);
}

int Decoration::captionHeight() const
{
    return borderTop() - settings()->smallSpacing() * (client()->isMaximized() ? 2 : 3) - 1;
}

QRect Decoration::captionRect() const
{
    const int leftOffset = m_leftButtons->geometry().x() + m_leftButtons->geometry().width();
    const int rightOffset = size().width() - m_rightButtons->geometry().x();
    const int offset = qMax(leftOffset, rightOffset);
    const int yOffset = client()->isMaximized() ? 0 : settings()->smallSpacing();
    // below is the spacer
    return QRect(offset, yOffset, size().width() - offset * 2, captionHeight());
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
