/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2018 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 * SPDX-FileCopyrightText: 2021-2026 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breezedecoration.h"

#if KLASSY_DECORATION_DEBUG_MODE
#include "setqdebug_logging.h"
#endif

#include "breezeboxshadowrenderer.h"
#include "breezebutton.h"
#include "breezesettingsprovider.h"
#include "dbusupdatenotifier.h"
#include "geometrytools.h"
#include "kdecorationglobals.h"
#include "plasmatools.h"

#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/DecorationButtonGroup>
#include <KDecoration3/DecorationShadow>
#include <KDecoration3/ScaleHelpers>

#include <KColorUtils>
#include <KConfigGroup>
#include <KPluginFactory>
#include <KWindowSystem>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QHoverEvent>
#include <QPainter>
#include <QTextStream>
#include <QTimer>

#include <cmath>
#include <mutex>

K_PLUGIN_FACTORY_WITH_JSON(KlassyDecoFactory, "breeze.json", registerPlugin<Klassy::Decoration>(); registerPlugin<Klassy::Button>();)

namespace
{
struct ShadowParams {
    ShadowParams()
        : offset(QPoint(0, 0))
        , radius(0)
        , opacity(0)
    {
    }

    ShadowParams(const QPoint &offset, int radius, qreal opacity)
        : offset(offset)
        , radius(radius)
        , opacity(opacity)
    {
    }

    QPoint offset;
    int radius;
    qreal opacity;
};

struct CompositeShadowParams {
    CompositeShadowParams() = default;

    CompositeShadowParams(const QPoint &offset, const ShadowParams &shadow1, const ShadowParams &shadow2)
        : offset(offset)
        , shadow1(shadow1)
        , shadow2(shadow2)
    {
    }

    bool isNone() const
    {
        return qMax(shadow1.radius, shadow2.radius) == 0;
    }

    QPoint offset;
    ShadowParams shadow1;
    ShadowParams shadow2;
};

const CompositeShadowParams s_shadowParams[] = {
    // None
    CompositeShadowParams(),
    // Small
    CompositeShadowParams(QPoint(0, 4), ShadowParams(QPoint(0, 0), 16, 1), ShadowParams(QPoint(0, -2), 8, 0.4)),
    // Medium
    CompositeShadowParams(QPoint(0, 8), ShadowParams(QPoint(0, 0), 32, 0.9), ShadowParams(QPoint(0, -4), 16, 0.3)),
    // Large
    CompositeShadowParams(QPoint(0, 12), ShadowParams(QPoint(0, 0), 48, 0.8), ShadowParams(QPoint(0, -6), 24, 0.2)),
    // Very large
    CompositeShadowParams(QPoint(0, 16), ShadowParams(QPoint(0, 0), 64, 0.7), ShadowParams(QPoint(0, -8), 32, 0.1)),
};

inline CompositeShadowParams lookupShadowParams(int size)
{
    switch (size) {
    case Klassy::InternalSettings::EnumShadowSize::None:
        return s_shadowParams[0];
    case Klassy::InternalSettings::EnumShadowSize::Small:
        return s_shadowParams[1];
    case Klassy::InternalSettings::EnumShadowSize::Medium:
        return s_shadowParams[2];
    case Klassy::InternalSettings::EnumShadowSize::Large:
        return s_shadowParams[3];
    case Klassy::InternalSettings::EnumShadowSize::VeryLarge:
        return s_shadowParams[4];
    default:
        // Fallback to the Large size.
        return s_shadowParams[3];
    }
}
}

namespace Klassy
{

KSharedConfig::Ptr Decoration::s_kdeGlobalConfig = KSharedConfig::Ptr();
Side g_taskManagerSide = SideBottom;
TaskManagerType g_taskManagerType = TaskManagerType::IconsAndTextTaskManager;

using KDecoration3::ColorGroup;
using KDecoration3::ColorRole;

static std::mutex g_setGlobalLookAndFeelOptionsMutex;

// cached shadow values
static int g_sDecoCount = 0;
static int g_shadowSizeEnumActive = InternalSettings::EnumShadowSize::Large;
static int g_shadowStrengthActive = 255;
static QColor g_shadowColorActive = Qt::black;
static int g_shadowSizeEnumInactive = InternalSettings::EnumShadowSize::Large;
static int g_shadowStrengthInactive = 128;
static QColor g_shadowColorInactive = Qt::black;
static qreal g_cornerRadius = 3;
static bool g_hasNoBorders = true;
static bool g_roundAllCornersWhenNoBorders = false;
static bool g_hideTitleBar = false;
static std::shared_ptr<KDecoration3::DecorationShadow> g_sShadow;
static std::shared_ptr<KDecoration3::DecorationShadow> g_sShadowInactive;

static QByteArray g_taskManagerTypeAndSideUpdateUuid = QByteArray();

//________________________________________________________________
Decoration::Decoration(QObject *parent, const QVariantList &args)
    : KDecoration3::Decoration(parent, args)
    , m_animation(new QVariantAnimation(this))
    , m_shadowAnimation(new QVariantAnimation(this))
    , m_overrideWindowOutlineFromButtonAnimation(new QVariantAnimation(this))

{
#if KLASSY_DECORATION_DEBUG_MODE
    setDebugOutput(KLASSY_QDEBUG_OUTPUT_PATH_RELATIVE_HOME);
#endif
    if (!s_kdeGlobalConfig) {
        s_kdeGlobalConfig = KSharedConfig::openConfig();
    }
    g_sDecoCount++;
}

//________________________________________________________________
Decoration::~Decoration()
{
    g_sDecoCount--;
    if (g_sDecoCount == 0) {
        // last deco destroyed, clean up shadow
        g_sShadow.reset();
    }
}

//________________________________________________________________
void Decoration::setOpacity(qreal value)
{
    if (m_opacity == value) {
        return;
    }
    m_opacity = value;
    update();
}

//________________________________________________________________
QColor Decoration::titleBarColor(bool returnNonAnimatedColor) const
{
    auto c = window();
    if (hideTitleBar() && !m_internalSettings->useTitleBarColorForAllBorders())
        return c->color(ColorGroup::Inactive, ColorRole::TitleBar);

    QColor activeTitleBarColor = m_decorationColors->active()->titleBarBase;
    QColor inactiveTitlebarColor = m_decorationColors->inactive()->titleBarBase;
    if (m_internalSettings->opaqueTitleBar() || (m_internalSettings->opaqueMaximizedTitleBars() && c->isMaximized())) {
        activeTitleBarColor.setAlpha(255);
        inactiveTitlebarColor.setAlpha(255);
    }

    // do not animate titlebar if there is a tools area/header area as it causes glitches
    if (!m_toolsAreaWillBeDrawn && (m_animation->state() == QAbstractAnimation::Running) && !returnNonAnimatedColor) {
        return KColorUtils::mix(inactiveTitlebarColor, activeTitleBarColor, m_opacity);
    } else {
        return c->isActive() ? activeTitleBarColor : inactiveTitlebarColor;
    }
}

//________________________________________________________________
QColor Decoration::titleBarSeparatorColor() const
{
    auto c = window();
    qreal opacity = 1.0;
    if (m_darkTheme) {
        opacity = 0.5;
    }

    if (c->isActive()) {
        QColor color(m_decorationColors->active()->buttonFocus);
        color.setAlpha(color.alpha() * opacity);
        return color;
    } else
        return QColor();
}

QColor Decoration::overriddenOutlineColorAnimateIn() const
{
    QColor color = m_windowOutlineOverride;
    if (m_overrideWindowOutlineFromButtonAnimation->state() == QAbstractAnimation::Running) {
        auto c = window();
        QColor originalColor;
        c->isActive() ? originalColor = m_originalWindowOutlineActivePreOverride : originalColor = m_originalWindowOutlineInactivePreOverride;

        if (originalColor.isValid())
            return KColorUtils::mix(originalColor, color, m_overrideOutlineAnimationProgress);
        else {
            color.setAlphaF(color.alphaF() * m_overrideOutlineAnimationProgress);
            return color;
        }
    } else
        return color;
}

QColor Decoration::overriddenOutlineColorAnimateOut(const QColor &destinationColor)
{
    if (m_overrideWindowOutlineFromButtonAnimation->state() == QAbstractAnimation::Running) {
        auto c = window();
        QColor originalColor;
        c->isActive() ? originalColor = m_originalWindowOutlineActivePreOverride : originalColor = m_originalWindowOutlineInactivePreOverride;

        if (originalColor.isValid() && destinationColor.isValid()) {
            if (m_overrideOutlineAnimationProgress == 1)
                m_animateOutOverriddenWindowOutline = false;
            return KColorUtils::mix(originalColor, destinationColor, m_overrideOutlineAnimationProgress);
        } else if (originalColor.isValid()) {
            QColor color = originalColor;
            color.setAlphaF(originalColor.alphaF() * (1.0 - m_overrideOutlineAnimationProgress));
            if (m_overrideOutlineAnimationProgress == 1)
                m_animateOutOverriddenWindowOutline = false;
            return color;
        } else {
            if (m_overrideOutlineAnimationProgress == 1)
                m_animateOutOverriddenWindowOutline = false;
            return QColor();
        }
    } else {
        m_animateOutOverriddenWindowOutline = false;
        return destinationColor;
    }
}

//________________________________________________________________
QColor Decoration::fontColor(bool returnNonAnimatedColor) const
{
    auto c = window();

    if (m_animation->state() == QAbstractAnimation::Running && !returnNonAnimatedColor) {
        return KColorUtils::mix(m_decorationColors->inactive()->titleBarText, m_decorationColors->active()->titleBarText, m_animation->currentValue().toReal());
    } else {
        return c->isActive() ? m_decorationColors->active()->titleBarText : m_decorationColors->inactive()->titleBarText;
    }
}

//________________________________________________________________
bool Decoration::init()
{
    auto c = window();

    m_isRightToLeft = (QGuiApplication::layoutDirection() == Qt::LayoutDirection::RightToLeft);

    reconfigureMain(true);
    
    // active state change animation
    // It is important start and end value are of the same type, hence 0.0 and not just 0
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    // Linear to have the same easing as Breeze animations
    m_animation->setEasingCurve(QEasingCurve::Linear);
    connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        setOpacity(value.toReal());
    });

    m_shadowAnimation->setStartValue(0.0);
    m_shadowAnimation->setEndValue(1.0);
    m_shadowAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_shadowAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_shadowOpacity = value.toReal();
        if (m_shadowAnimation->state() == QAbstractAnimation::Running) {
            updateWindowOutline();
            updateShadow();
        }
    });

    m_overrideWindowOutlineFromButtonAnimation->setStartValue(0.0);
    m_overrideWindowOutlineFromButtonAnimation->setEndValue(1.0);
    m_overrideWindowOutlineFromButtonAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    connect(m_overrideWindowOutlineFromButtonAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_overrideOutlineAnimationProgress = value.toReal();
        if (m_overrideWindowOutlineFromButtonAnimation->state() == QAbstractAnimation::Running)
            updateWindowOutline(true);
    });

    // use DBus connection to update on Klassy configuration change
    auto dbus = QDBusConnection::sessionBus();

    dbus.connect(QString(),
                 QStringLiteral("/KGlobalSettings"),
                 QStringLiteral("org.kde.KGlobalSettings"),
                 QStringLiteral("notifyChange"),
                 this,
                 SLOT(reconfigure()));

    updateTitleBar();
    auto s = settings();
    connect(s.get(), &KDecoration3::DecorationSettings::borderSizeChanged, this, &Decoration::recalculateBorders);

    // a change in font might cause the borders to change
    connect(s.get(), &KDecoration3::DecorationSettings::fontChanged, this, &Decoration::recalculateBorders);
    connect(s.get(), &KDecoration3::DecorationSettings::fontChanged, this, &Decoration::updateBlur); // for the case when a border with transparency
    connect(s.get(), &KDecoration3::DecorationSettings::spacingChanged, this, &Decoration::recalculateBorders);
    connect(s.get(), &KDecoration3::DecorationSettings::spacingChanged, this, &Decoration::updateBlur); // for the case when a border with transparency

    // color cache update
    // The slot will only update if the UUID has changed, hence preventing unnecessary multiple colour cache updates
    connect(&g_dBusUpdateNotifier, &DBusUpdateNotifier::decorationSettingsUpdate, this, &Decoration::generateDecorationColorsOnDecorationColorSettingsUpdate);
    connect(&g_dBusUpdateNotifier, &DBusUpdateNotifier::systemColorSchemeUpdate, this, &Decoration::generateDecorationColorsOnSystemColorSettingsUpdate);
    connect(&g_dBusUpdateNotifier, &DBusUpdateNotifier::systemIconsUpdate, this, [this]() {
        if (m_internalSettings->buttonIconStyle() == InternalSettings::EnumButtonIconStyle::SystemIconTheme) {
            Q_EMIT reconfigured(); // this will trigger Button::reconfigure
        }
        update(titleBar());
    });
    connect(c, &KDecoration3::DecoratedWindow::paletteChanged, this, &Decoration::generateDecorationColorsOnClientPaletteUpdate);
    connect(&g_dBusUpdateNotifier, &DBusUpdateNotifier::appletSettingsUpdate, this, &Decoration::updateTaskManagerTypeAndSide);

    // buttons
    connect(s.get(), &KDecoration3::DecorationSettings::spacingChanged, this, &Decoration::updateButtonsGeometryDelayed);
    connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsLeftChanged, this, &Decoration::updateButtonsGeometryDelayed);
    connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsRightChanged, this, &Decoration::updateButtonsGeometryDelayed);
    connect(s.get(), &KDecoration3::DecorationSettings::onAllDesktopsAvailableChanged, this, &Decoration::updateButtonsGeometryDelayed);
#if KDECORATION_VERSION >= KDECORATION_VERSION_CHECK(6, 7, 0) // workaround for strange linking error on Plasma 6.6.2
    connect(c, &KDecoration3::DecoratedWindow::excludeFromCaptureChanged, this, &Decoration::updateButtonsGeometryDelayed);
#endif

    // full reconfiguration
    connect(s.get(), &KDecoration3::DecorationSettings::reconfigured, this, &Decoration::reconfigure);
    connect(s.get(), &KDecoration3::DecorationSettings::reconfigured, this, &Decoration::updateButtonsGeometryDelayed);

    connect(c, &KDecoration3::DecoratedWindow::activeChanged, this, &Decoration::recalculateBorders);
    connect(c, &KDecoration3::DecoratedWindow::adjacentScreenEdgesChanged, this, &Decoration::recalculateBorders);
    connect(c, &KDecoration3::DecoratedWindow::maximizedHorizontallyChanged, this, &Decoration::recalculateBorders);
    connect(c, &KDecoration3::DecoratedWindow::maximizedVerticallyChanged, this, &Decoration::recalculateBorders);
    connect(c, &KDecoration3::DecoratedWindow::shadedChanged, this, &Decoration::recalculateBorders);
    connect(c, &KDecoration3::DecoratedWindow::shadedChanged, this, &Decoration::updateShadowOnChangeNoCache);
    connect(c, &KDecoration3::DecoratedWindow::captionChanged, this, [this]() {
        // update the caption area
        update(titleBar());
    });
    connect(c, &KDecoration3::DecoratedWindow::keepAboveChanged, this, [this]() {
        if (m_internalSettings->colorizeWindowOutlineWithButton()) {
            updateWindowOutline();
        }
    });
    connect(c, &KDecoration3::DecoratedWindow::keepBelowChanged, this, &Decoration::recalculateBorders); // in case EnumHideTitleBar::KeptBehind

    connect(c, &KDecoration3::DecoratedWindow::activeChanged, this, &Decoration::updateAnimationState);
    connect(c, &KDecoration3::DecoratedWindow::activeChanged, this, &Decoration::updateOpaque);
    connect(c, &KDecoration3::DecoratedWindow::activeChanged, this, &Decoration::updateBlur);
    connect(this, &KDecoration3::Decoration::bordersChanged, this, &Decoration::updateTitleBar);
    connect(this, &KDecoration3::Decoration::bordersChanged, this, &Decoration::updateButtonsGeometry);
    connect(this, &KDecoration3::Decoration::bordersChanged, this, &Decoration::updateBlur);
    connect(c, &KDecoration3::DecoratedWindow::adjacentScreenEdgesChanged, this, &Decoration::updateTitleBar);
    connect(c, &KDecoration3::DecoratedWindow::widthChanged, this, &Decoration::updateTitleBar);
    connect(c, &KDecoration3::DecoratedWindow::sizeChanged, this, &Decoration::updateBlur);

    connect(c, &KDecoration3::DecoratedWindow::maximizedChanged, this, &Decoration::updateTitleBar);
    connect(c, &KDecoration3::DecoratedWindow::maximizedChanged, this, &Decoration::updateOpaque);

    connect(c, &KDecoration3::DecoratedWindow::widthChanged, this, &Decoration::updateButtonsGeometry);
    connect(c, &KDecoration3::DecoratedWindow::maximizedChanged, this, &Decoration::updateButtonsGeometry);
    connect(c, &KDecoration3::DecoratedWindow::adjacentScreenEdgesChanged, this, &Decoration::updateButtonsGeometry);
    connect(c, &KDecoration3::DecoratedWindow::shadedChanged, this, &Decoration::updateButtonsGeometry);

    connect(c, &KDecoration3::DecoratedWindow::scaleChanged, this, &Decoration::updateScale);
    connect(c, &KDecoration3::DecoratedWindow::nextScaleChanged, this, &Decoration::updateNextScale);

    createButtons();
    updateShadow();
    return true;
}

//________________________________________________________________
void Decoration::updateTitleBar()
{
    auto c = window();

    qreal width, height, x, y;

    qreal borderTop = this->borderTop();

    // prevents resize handles appearing in button at top window edge for large full-height buttons
    if (m_buttonBackgroundType == ButtonBackgroundType::FullHeight
        && !(m_internalSettings->drawBorderOnMaximizedWindows() && (c->isMaximizedVertically() || c->adjacentScreenEdges().testFlag(Qt::TopEdge)))) {
        width = c->width();
        height = borderTop;
        x = 0;
        y = 0;

    } else {
        // for smaller circular buttons increase the resizable area
        const bool maximizedHorizontally = isMaximizedHorizontally();
        width = maximizedHorizontally ? c->width() : c->width() - this->scaledTitleBarLeftMargin(false) - this->scaledTitleBarRightMargin(false);
        height = (isMaximizedVertically() || isTopEdge()) ? borderTop : borderTop - m_scaledTitleBarTopMargin;
        x = maximizedHorizontally ? 0 : m_scaledTitleBarLeftMargin;
        y = (isMaximizedVertically() || isTopEdge()) ? 0 : m_scaledTitleBarTopMargin;
    }

    setTitleBar(QRectF(x, y, width, height));
}

// For Titlebar active state and shadow animations only
void Decoration::updateAnimationState()
{
    if (m_shadowAnimation->duration() > 0) {
        auto c = window();
        m_shadowAnimation->setDirection(c->isActive() ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
        m_shadowAnimation->setEasingCurve(c->isActive() ? QEasingCurve::OutCubic : QEasingCurve::InCubic);
        if (m_shadowAnimation->state() != QAbstractAnimation::Running) {
            m_shadowAnimation->start();
        }

    } else {
        updateWindowOutline();
        updateShadow();
    }

    if (m_animation->duration() > 0) {
        auto c = window();
        m_animation->setDirection(c->isActive() ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
        if (m_animation->state() != QAbstractAnimation::Running) {
            m_animation->start();
        }

    } else {
        update();
    }
}

// For overriding thin window outline with button colour
void Decoration::updateOverrideWindowOutlineFromButtonAnimationState()
{
    if (m_overrideWindowOutlineFromButtonAnimation->duration() > 0) {
        m_overrideWindowOutlineFromButtonAnimation->setDirection(QAbstractAnimation::Forward);
        m_overrideWindowOutlineFromButtonAnimation->setEasingCurve(QEasingCurve::InOutQuad);
        if (m_overrideWindowOutlineFromButtonAnimation->state() != QAbstractAnimation::Running)
            m_overrideWindowOutlineFromButtonAnimation->start();

    } else {
        updateWindowOutline(true);
    }
}

//________________________________________________________________
void Decoration::setScaledBorderSizes(const bool nextScale)
{
    auto c = window();
    qreal scale = nextScale ? c->nextScale() : c->scale();

    qreal &scaledBorderLeftRight = nextScale ? m_scaledBorderLeftRightNext : m_scaledBorderLeftRight;
    qreal &scaledBorderBottom = nextScale ? m_scaledBorderBottomNext : m_scaledBorderBottom;

    const int baseSize = m_smallSpacing;

    if (m_internalSettings && (m_internalSettings->exceptionBorder())) {
        switch (m_internalSettings->borderSize()) {
        case InternalSettings::EnumBorderSize::None:
            scaledBorderLeftRight = 0;
            scaledBorderBottom = scaledBorderLeftRight;
            break;
        case InternalSettings::EnumBorderSize::NoSides:
            scaledBorderLeftRight = 0;
            scaledBorderBottom = qMax(4, baseSize);
            break;
        default:
        case InternalSettings::EnumBorderSize::Tiny:
            scaledBorderLeftRight = baseSize;
            scaledBorderBottom = qMax(4, baseSize);
            break;
        case InternalSettings::EnumBorderSize::Normal:
            scaledBorderLeftRight = baseSize * 2;
            scaledBorderBottom = scaledBorderLeftRight;
            break;
        case InternalSettings::EnumBorderSize::Large:
            scaledBorderLeftRight = baseSize * 3;
            scaledBorderBottom = scaledBorderLeftRight;
            break;
        case InternalSettings::EnumBorderSize::VeryLarge:
            scaledBorderLeftRight = baseSize * 4;
            scaledBorderBottom = scaledBorderLeftRight;
            break;
        case InternalSettings::EnumBorderSize::Huge:
            scaledBorderLeftRight = baseSize * 5;
            scaledBorderBottom = scaledBorderLeftRight;
            break;
        case InternalSettings::EnumBorderSize::VeryHuge:
            scaledBorderLeftRight = baseSize * 6;
            scaledBorderBottom = scaledBorderLeftRight;
            break;
        case InternalSettings::EnumBorderSize::Oversized:
            scaledBorderLeftRight = baseSize * 10;
            scaledBorderBottom = scaledBorderLeftRight;
            break;
        }

    } else {
        switch (settings()->borderSize()) {
        case KDecoration3::BorderSize::None:
            scaledBorderLeftRight = 0;
            scaledBorderBottom = scaledBorderLeftRight;
            break;
        case KDecoration3::BorderSize::NoSides:
            scaledBorderLeftRight = 0;
            scaledBorderBottom = qMax(4, baseSize);
            break;
        default:
        case KDecoration3::BorderSize::Tiny:
            scaledBorderLeftRight = baseSize;
            scaledBorderBottom = qMax(4, baseSize);
            break;
        case KDecoration3::BorderSize::Normal:
            scaledBorderLeftRight = baseSize * 2;
            scaledBorderBottom = scaledBorderLeftRight;
            break;
        case KDecoration3::BorderSize::Large:
            scaledBorderLeftRight = baseSize * 3;
            scaledBorderBottom = scaledBorderLeftRight;
            break;
        case KDecoration3::BorderSize::VeryLarge:
            scaledBorderLeftRight = baseSize * 4;
            scaledBorderBottom = scaledBorderLeftRight;
            break;
        case KDecoration3::BorderSize::Huge:
            scaledBorderLeftRight = baseSize * 5;
            scaledBorderBottom = scaledBorderLeftRight;
            break;
        case KDecoration3::BorderSize::VeryHuge:
            scaledBorderLeftRight = baseSize * 6;
            scaledBorderBottom = scaledBorderLeftRight;
            break;
        case KDecoration3::BorderSize::Oversized:
            scaledBorderLeftRight = baseSize * 10;
            scaledBorderBottom = scaledBorderLeftRight;
            break;
        }
    }

    scaledBorderLeftRight = KDecoration3::snapToPixelGrid(scaledBorderLeftRight, scale);
    scaledBorderBottom = KDecoration3::snapToPixelGrid(scaledBorderBottom, scale);
}

//________________________________________________________________
void Decoration::reconfigureMain(const bool noUpdateShadow)
{
    auto c = window();

    SettingsProvider::self()->reconfigure();
    m_internalSettings = SettingsProvider::self()->internalSettings(this);

    QPalette clientPalette = c->palette();
    updateDecorationColors(clientPalette);
    updateTaskManagerTypeAndSide();

    s_kdeGlobalConfig->reparseConfiguration();
    // settings()->smallSpacing() was used to scale on X11 and usually 2 on Wayland but not always and varies on X11, depending on font size
    // this function sets it to a fixed value of 2, no font-based scaling, so that the appearance between X11 and Wayland is unified
    m_x11Scale = 1.0;
    m_smallSpacing = 2.0;
    m_gridUnit = 10.0;
    if (KWindowSystem::isPlatformX11()) {
        // loads system ScaleFactor from ~/.config/kdeglobals
        const KConfigGroup cgKScreen(s_kdeGlobalConfig, QStringLiteral("KScreen"));
        m_systemScaleFactorX11 = cgKScreen.readEntry(QStringLiteral("ScaleFactor"), 1.0f);
        m_x11Scale *= m_systemScaleFactorX11;
        m_smallSpacing *= m_systemScaleFactorX11;
        m_gridUnit *= m_systemScaleFactorX11;
    }

    setScaledCornerRadius();

    if (m_internalSettings->buttonShape() == InternalSettings::EnumButtonShape::FullHeightRectangle
        || m_internalSettings->buttonShape() == InternalSettings::EnumButtonShape::FullHeightRoundedRectangle
        || m_internalSettings->buttonShape() == InternalSettings::EnumButtonShape::IntegratedRoundedRectangle
        || m_internalSettings->buttonShape() == InternalSettings::EnumButtonShape::IntegratedRoundedRectangleGrouped)
        m_buttonBackgroundType = ButtonBackgroundType::FullHeight;
    else
        m_buttonBackgroundType = ButtonBackgroundType::Small;

    setScaledBorderSizes(false);
    setScaledBorderSizes(true);

    setScaledTitleBarMargins(false);
    setScaledTitleBarMargins(true);

    setScaledIconSizes(false);
    setScaledIconSizes(true);

    setScaledButtonDimensions();

    const KConfigGroup cg(s_kdeGlobalConfig, QStringLiteral("KDE"));
    QString lookAndFeelPackage = cg.readEntry(QStringLiteral("LookAndFeelPackage"));
    setGlobalLookAndFeelOptions(lookAndFeelPackage);

    // animation
    if (m_internalSettings->animationsEnabled()) {
        qreal animationsDurationFactorRelativeSystem = 1;
        if (m_internalSettings->animationsSpeedRelativeSystem() < 0)
            animationsDurationFactorRelativeSystem = (-m_internalSettings->animationsSpeedRelativeSystem() + 2) / 2.0f;
        else if (m_internalSettings->animationsSpeedRelativeSystem() > 0)
            animationsDurationFactorRelativeSystem = 1 / ((m_internalSettings->animationsSpeedRelativeSystem() + 2) / 2.0f);
        m_animation->setDuration(cg.readEntry(QStringLiteral("AnimationDurationFactor"), 1.0f) * 150.0f * animationsDurationFactorRelativeSystem);
        m_shadowAnimation->setDuration(m_animation->duration());
        m_overrideWindowOutlineFromButtonAnimation->setDuration(m_animation->duration());
    } else {
        m_animation->setDuration(0);
        m_shadowAnimation->setDuration(0);
        m_overrideWindowOutlineFromButtonAnimation->setDuration(0);
    }

    // borders
    recalculateBorders();

    updateOpaque();
    updateBlur();

    // shadow
    if (!noUpdateShadow)
        this->updateShadow();

    Q_EMIT reconfigured();
}

void Decoration::updateDecorationColors(const QPalette &clientPalette, QByteArray uuid)
{
    QPalette systemPalette = KColorScheme::createApplicationPalette(s_kdeGlobalConfig);
    bool clientSpecificPalette = false;
    if (clientPalette != systemPalette) { // Some applications can set a Window Colour Scheme, meaning the client palette and system palette differ
        clientSpecificPalette = true;
    }
    QPalette palette = clientSpecificPalette ? clientPalette : systemPalette;

    // this doesn't work for a clientSpecificPalette -- need KWin to provide the  KDE ColorScheme path/hash rather than the QPalette to determine
    m_colorSchemeHasHeaderColor = KColorScheme::isColorSetSupported(s_kdeGlobalConfig, KColorScheme::Header);

    // Commented was how m_toolsAreaWillBeDrawn was determined in Breeze, simplified for Klassy
    // m_toolsAreaWillBeDrawn = ( m_colorSchemeHasHeaderColor && ( settings()->borderSize() == KDecoration3::BorderSize::None || settings()->borderSize() ==
    // KDecoration3::BorderSize::NoSides ) );
    m_toolsAreaWillBeDrawn = (m_colorSchemeHasHeaderColor || m_internalSettings->matchTitleBarToApplicationColor());

    QColor activeApplicationBackground = palette.color(QPalette::ColorGroup::Active, QPalette::ColorRole::Window);

    // determine if a dark colour scheme
    if (activeApplicationBackground.isValid()) {
        if (activeApplicationBackground.lightnessF() < 0.5) {
            m_darkTheme = true;
        } else {
            m_darkTheme = false;
        }
    }

    // The preset exception may modify the decoration colours by having a different translucentButtonBackgroundsOpacity, so in this case we don't want to
    // cache the decoration colours as it may corrupt the colours for normal non-exception decoration windows
    bool noCache = m_internalSettings->property("noCacheException").toBool() || clientSpecificPalette;

    if (noCache) {
        if (!m_decorationColors || m_decorationColors->isCachedPalette()) {
            m_decorationColors = std::make_unique<DecorationColors>(false);
        }
    } else {
        if (!m_decorationColors || !m_decorationColors->isCachedPalette()) {
            m_decorationColors = std::make_unique<DecorationColors>(true);
        }
    }

    bool generateColors = false;

    if (!m_decorationColors->areColorsGenerated()) {
        generateColors = true;
    } else {
        if (!uuid.isEmpty()
            && (noCache
                || (!noCache && uuid != m_decorationColors->settingsUpdateUuid()))) { // case from generateDecorationColorsOnDecorationSettingsPaletteUpdate()
            generateColors = true;
        }

        // TODO: palette may not be a reliable indicator of the entire colour scheme - get an update to KDecoration3::DecoratedWindow to read QString
        // m_colorScheme instead
        if (!generateColors && palette != *m_decorationColors->basePalette()) {
            generateColors = true;
        }
    }

    if (generateColors) {
        auto c = window();

        QColor activeTitleBarBase = c->color(ColorGroup::Active, ColorRole::TitleBar);
        QColor inactiveTitleBarBase = c->color(ColorGroup::Inactive, ColorRole::TitleBar);
        QColor activeTitleBarText = c->color(ColorGroup::Active, ColorRole::Foreground);
        QColor inactiveTitleBarText = c->color(ColorGroup::Inactive, ColorRole::Foreground);

        if (m_internalSettings->matchTitleBarToApplicationColor() && !m_colorSchemeHasHeaderColor) {
            if (activeApplicationBackground.isValid()) {
                activeTitleBarBase = activeApplicationBackground;
            }

            QColor inactiveApplicationBackground = palette.color(QPalette::ColorGroup::Inactive, QPalette::ColorRole::Window);
            if (inactiveApplicationBackground.isValid()) {
                inactiveTitleBarBase = inactiveApplicationBackground;
            }

            QColor activeApplicationText = palette.color(QPalette::ColorGroup::Active, QPalette::ColorRole::WindowText);
            if (activeApplicationText.isValid()) {
                activeTitleBarText = activeApplicationText;
            }

            QColor inactiveApplicationText = palette.color(QPalette::ColorGroup::Inactive, QPalette::ColorRole::WindowText);
            if (inactiveApplicationText.isValid()) {
                inactiveTitleBarText = inactiveApplicationText;
            }
        }

        m_decorationColors->generateDecorationAndButtonColors(palette,
                                                              m_internalSettings,
                                                              activeTitleBarText,
                                                              activeTitleBarBase,
                                                              inactiveTitleBarText,
                                                              inactiveTitleBarBase,
                                                              uuid); // update the decoration colors
    }
}

void Decoration::generateDecorationColorsOnClientPaletteUpdate(const QPalette &clientPalette)
{
    updateDecorationColors(clientPalette);
    update();
}

void Decoration::generateDecorationColorsOnDecorationColorSettingsUpdate(QByteArray uuid)
{
    auto c = window();
    QPalette clientPalette = c->palette();

    SettingsProvider::self()->reconfigure();
    m_internalSettings = SettingsProvider::self()->internalSettings(this);
    s_kdeGlobalConfig->reparseConfiguration();

    updateDecorationColors(clientPalette, uuid);
}

void Decoration::generateDecorationColorsOnSystemColorSettingsUpdate(QByteArray uuid)
{
    auto c = window();
    QPalette clientPalette = c->palette();

    s_kdeGlobalConfig->reparseConfiguration();

    updateDecorationColors(clientPalette, uuid);
    update();
}

void Decoration::setGlobalLookAndFeelOptions(QString lookAndFeelPackageName)
{
    QString lookAndFeelSet = m_internalSettings->lookAndFeelSet();
    if (lookAndFeelPackageName == m_internalSettings->lookAndFeelSet()) {
        return;
    }

    // only allow one thread at a time to set the look-and-feel options
    std::unique_lock<std::mutex> lock(g_setGlobalLookAndFeelOptionsMutex, std::try_to_lock);
    if (lock.owns_lock()) {
        m_internalSettings->setLookAndFeelSet(lookAndFeelPackageName);
        m_internalSettings->save();

        QString presetToLoad;
        QStringList klassyLookAndFeels = {QStringLiteral("org.kde.klassydarkleftpanel.desktop"),
                                          QStringLiteral("org.kde.klassylightleftpanel.desktop"),
                                          QStringLiteral("org.kde.klassydarkbottompanel.desktop"),
                                          QStringLiteral("org.kde.klassylightbottompanel.desktop")};

        bool regenerateIconsOnly = false;
        // load Klassy preset if it is first time applying Klassy Global Theme
        if (klassyLookAndFeels.contains(lookAndFeelPackageName)) {
            if (!klassyLookAndFeels.contains(lookAndFeelSet)) {
                presetToLoad = QStringLiteral("Klassy");
            } else if (lookAndFeelPackageName.contains(QStringLiteral("bottom"))) {
                if (lookAndFeelSet.contains(QStringLiteral("left"))) {
                    regenerateIconsOnly = true;
                }
            } else if (lookAndFeelPackageName.contains(QStringLiteral("left"))) {
                if (lookAndFeelSet.contains(QStringLiteral("bottom"))) {
                    regenerateIconsOnly = true;
                }
            }
        }

        if (!presetToLoad.isEmpty()) { // if switching from a non-Klassy theme, load the associated Klassy window decoration preset
            QTimer::singleShot(3000, [presetToLoad]() {
                system("klassy-settings -w \"" + presetToLoad.toUtf8() + "\" &");
            });
        } else if (regenerateIconsOnly) {
            QTimer::singleShot(3000, []() {
                system("klassy-settings -g &");
            }); // otherwise if Klassy already, and not just switching light/dark, then regenerate the icons only
        }
    }
}

void Decoration::updateTaskManagerTypeAndSide(QByteArray uuid)
{
    if (uuid.isEmpty()) {
        if (g_taskManagerTypeAndSideUpdateUuid.isEmpty()) {
            g_taskManagerTypeAndSideUpdateUuid = "1";
            PlasmaTools::taskManagerTypeAndSide(g_taskManagerType, g_taskManagerSide);
            m_taskManagerType = g_taskManagerType;
            m_taskManagerSide = g_taskManagerSide;
        }
    } else {
        if (g_taskManagerTypeAndSideUpdateUuid != uuid) {
            g_taskManagerTypeAndSideUpdateUuid = uuid;
            PlasmaTools::taskManagerTypeAndSide(g_taskManagerType, g_taskManagerSide);
        }
        if (m_taskManagerSide != g_taskManagerSide || m_taskManagerType != g_taskManagerType) {
            m_taskManagerSide = g_taskManagerSide;
            m_taskManagerType = g_taskManagerType;
            update(titleBar());
        }
    }
}

//________________________________________________________________
void Decoration::recalculateBorders()
{
    auto c = window();
    auto s = settings();
    qreal scale = c->nextScale();

    // left, right and bottom borders
    const qreal left = isLeftEdge() ? 0 : m_scaledBorderLeftRightNext;
    const qreal right = isRightEdge() ? 0 : m_scaledBorderLeftRightNext;
    const qreal bottom = (c->isShaded() || isBottomEdge()) ? 0 : m_scaledBorderBottomNext;

    qreal top = 0;
    if (hideTitleBar()) {
        top = bottom;
    } else {
        QFontMetrics fm(s->font());
        top += KDecoration3::snapToPixelGrid(qMax(qreal(fm.height()), m_scaledSmallButtonPaddedSizeNext), scale);

        // padding below
        top += scaledTitleBarSeparatorHeight(true);
        top += m_scaledTitleBarTopMarginNext + m_scaledTitleBarBottomMarginNext;
    }

    setBorders(QMarginsF(left, top, right, bottom));

    // extended sizes
    const qreal extSize = KDecoration3::snapToPixelGrid(m_smallSpacing * 3, scale);
    qreal extLeft = 0;
    qreal extRight = 0;
    qreal extBottom = 0;
    qreal extTop = 0;

    // Add extended resize handles for Full-sized Rectangle highlight as they cannot overlap with larger full-sized buttons
    if (m_buttonBackgroundType == ButtonBackgroundType::FullHeight || m_scaledTitleBarTopMarginNext < extSize) {
        if (!isMaximizedVertically())
            extTop = extSize;
    }

    if (hasNoBorders()) {
        if (!isMaximizedHorizontally()) {
            extLeft = extSize;
            extRight = extSize;
        }
        if (!isMaximizedVertically()) {
            extBottom = extSize;
        }

    } else if (!isMaximizedHorizontally()) {
        if (hasNoSideBorders()) {
            extLeft = extSize;
            extRight = extSize;
        } else {
            if (scaledTitleBarLeftMargin(true) < extSize)
                extLeft = extSize;
            if (scaledTitleBarRightMargin(true) < extSize)
                extRight = extSize;
        }
    }

    setResizeOnlyBorders(QMarginsF(extLeft, extTop, extRight, extBottom));

    // set clipped corners
    qreal bottomLeftRadius = 0;
    qreal bottomRightRadius = 0;
    qreal topLeftRadius = 0;
    qreal topRightRadius = 0;

    if (!isBottomEdge() && !(hasNoBorders() && !m_internalSettings->roundAllCornersWhenNoBorders())) {
        if (!isLeftEdge()) {
            bottomLeftRadius = m_scaledCornerRadius;
        }
        if (!isRightEdge()) {
            bottomRightRadius = m_scaledCornerRadius;
        }
    }

    if (!isTopEdge() && !(hideTitleBar() && hasNoBorders() && !m_internalSettings->roundAllCornersWhenNoBorders())) {
        if (!isLeftEdge()) {
            topLeftRadius = m_scaledCornerRadius;
        }
        if (!isRightEdge()) {
            topRightRadius = m_scaledCornerRadius;
        }
    }

    setBorderRadius(KDecoration3::BorderRadius(topLeftRadius, topRightRadius, bottomRightRadius, bottomLeftRadius));

    updateWindowOutline();
}

//________________________________________________________________
void Decoration::createButtons()
{
    m_leftButtons = new KDecoration3::DecorationButtonGroup(KDecoration3::DecorationButtonGroup::Position::Left, this, &Button::create);
    m_rightButtons = new KDecoration3::DecorationButtonGroup(KDecoration3::DecorationButtonGroup::Position::Right, this, &Button::create);
    updateButtonsGeometry();
}

//________________________________________________________________
void Decoration::updateButtonsGeometryDelayed()
{
    QTimer::singleShot(0, this, &Decoration::updateButtonsGeometry);
}

//________________________________________________________________
void Decoration::updateButtonsGeometry()
{
    const auto s = settings();

    qreal scale = window()->scale();

    // adjust button position
    qreal bHeightNormal;
    qreal bWidthLeft = 0;
    qreal bWidthRight = 0;
    qreal verticalIconOffsetNormal = 0;
    qreal bHeightMenuGrouped = 0; // used only for the menu button with Integrated rounded rectangle, grouped
    qreal verticalIconOffsetMenuGrouped = 0; // used only for the menu button with Integrated rounded rectangle, grouped
    qreal horizontalIconOffsetLeftButtons = 0;
    qreal horizontalIconOffsetLeftFullHeightClose = 0;
    qreal horizontalIconOffsetRightButtons = 0;
    qreal horizontalIconOffsetRightFullHeightClose = 0;
    qreal scaledTitleBarSeparatorHeight = this->scaledTitleBarSeparatorHeight(false);
    qreal captionHeight = this->captionHeight();

    if (m_buttonBackgroundType == ButtonBackgroundType::FullHeight) {
        bHeightNormal = borderTop();
        bHeightNormal = qMax(bHeightNormal - scaledTitleBarSeparatorHeight, 0.0);
        if (internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::IntegratedRoundedRectangle
            || internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::IntegratedRoundedRectangleGrouped) {
            if (internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::IntegratedRoundedRectangleGrouped) {
                bHeightMenuGrouped = bHeightNormal;
            }
            bHeightNormal = qMax(bHeightNormal - m_scaledIntegratedRoundedRectangleBottomPadding, 0.0);

            qreal shiftUpWithOutline = 0; // how much to shift up the icon to appear more centred - only do when there is a colorizeWindowOutlineWithButton
                                          // or not window outline none/shadow
            if (!window()->isMaximized()
                && (((m_internalSettings->showOutlineOnHover(true) || m_internalSettings->showOutlineOnHover(false))
                     && (m_internalSettings->colorizeWindowOutlineWithButton()
                         || !((m_internalSettings->windowOutlineStyle(true) == InternalSettings::EnumWindowOutlineStyle::None
                               || m_internalSettings->windowOutlineStyle(true) == InternalSettings::EnumWindowOutlineStyle::ShadowColor)
                              && (m_internalSettings->windowOutlineStyle(false) == InternalSettings::EnumWindowOutlineStyle::None
                                  || m_internalSettings->windowOutlineStyle(false) == InternalSettings::EnumWindowOutlineStyle::ShadowColor))))
                    || ((m_internalSettings->showOutlineNormally(true) || m_internalSettings->showOutlineNormally(false))
                        && !((m_internalSettings->windowOutlineStyle(true) == InternalSettings::EnumWindowOutlineStyle::None
                              || m_internalSettings->windowOutlineStyle(true) == InternalSettings::EnumWindowOutlineStyle::ShadowColor)
                             && (m_internalSettings->windowOutlineStyle(false) == InternalSettings::EnumWindowOutlineStyle::None
                                 || m_internalSettings->windowOutlineStyle(false) == InternalSettings::EnumWindowOutlineStyle::ShadowColor))))) {
                shiftUpWithOutline = PenWidth::Symbol;
                if (KWindowSystem::isPlatformX11()) {
                    shiftUpWithOutline *= m_systemScaleFactorX11;
                }
            }
            verticalIconOffsetNormal =
                m_scaledTitleBarTopMargin + qreal(captionHeight - m_scaledIconSize - m_scaledIntegratedRoundedRectangleBottomPadding - shiftUpWithOutline) / 2;
            if (internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::IntegratedRoundedRectangleGrouped) {
                verticalIconOffsetMenuGrouped = m_scaledTitleBarTopMargin + qreal(captionHeight - m_scaledIconSize) / 2;
            }
        } else {
            // do not pixel grid snap icon offsets -- the icon gets snapped at the end anyway, and this keeps icons centred
            verticalIconOffsetNormal = m_scaledTitleBarTopMargin + qreal(captionHeight - m_scaledIconSize) / 2;
        }

        bWidthLeft = m_scaledIconSize + m_scaledButtonWidthMarginLeft * 2;
        bWidthRight = m_scaledIconSize + m_scaledButtonWidthMarginRight * 2;

        // do not pixel grid snap icon offsets -- the icon gets snapped at the end anyway, and this keeps icons centred
        horizontalIconOffsetLeftButtons = m_scaledButtonWidthMarginLeft;
        horizontalIconOffsetRightButtons = m_scaledButtonWidthMarginRight;
    } else {
        qreal bWidthMargin = (m_scaledSmallButtonPaddedSize - m_scaledIconSize) / 2;

        bHeightNormal = captionHeight + (isTopEdge() ? m_scaledTitleBarTopMargin : 0);
        // do not pixel grid snap icon offsets -- the icon gets snapped at the end anyway, and this keeps icons centred
        verticalIconOffsetNormal = (isTopEdge() ? m_scaledTitleBarTopMargin : 0) + captionHeight - m_scaledSmallButtonPaddedSize + bWidthMargin;

        bWidthLeft = m_scaledSmallButtonPaddedSize;
        bWidthRight = m_scaledSmallButtonPaddedSize;

        horizontalIconOffsetLeftButtons = bWidthMargin;
        horizontalIconOffsetRightButtons = bWidthMargin;
    }

    int firstLeftVisibleIndex = -1;
    int lastLeftVisibleIndex = -1;

    int numLeftButtons = m_leftButtons->buttons().count();
    bool menuPresentBefore = false;
    bool spacerPresentBefore = false;

    for (int i = 0; i < numLeftButtons; i++) {
        Button *button = static_cast<Button *>(m_leftButtons->buttons()[i]);

        qreal bHeight = bHeightNormal;
        qreal verticalIconOffset = verticalIconOffsetNormal;
        if (button->type() == KDecoration3::DecorationButtonType::Menu) {
            if (m_internalSettings->buttonShape() == InternalSettings::EnumButtonShape::IntegratedRoundedRectangleGrouped) {
                bHeight = bHeightMenuGrouped;
                verticalIconOffset = verticalIconOffsetMenuGrouped;
            }
        }

        qreal bWidth;
        if (m_buttonBackgroundType == ButtonBackgroundType::FullHeight) {
            if (button->type() == KDecoration3::DecorationButtonType::Close) {
                qreal bWidthMargin =
                    KDecoration3::snapToPixelGrid(m_scaledButtonWidthMarginLeft * m_internalSettings->closeFullHeightButtonWidthMarginRelative() / 100.0f,
                                                  scale);
                bWidth = m_scaledIconSize + bWidthMargin * 2;
                horizontalIconOffsetLeftFullHeightClose = bWidthMargin;
            } else {
                bWidth = bWidthLeft;
            }
            button->setBackgroundVisibleSize(QSizeF(bWidth, bHeight));
        } else {
            bWidth = bWidthLeft;
            button->setBackgroundVisibleSize(QSizeF(m_scaledSmallButtonBackgroundSize, m_scaledSmallButtonBackgroundSize));
        }
        if (button->type() == KDecoration3::DecorationButtonType::Spacer) {
            bWidth = KDecoration3::snapToPixelGrid(bWidth * m_internalSettings->spacerButtonWidthRelative() / 100.0f, scale);
        }

        button->setGeometry(QRectF(QPoint(0, 0), QSizeF(bWidth, bHeight)));
        if (m_buttonBackgroundType == ButtonBackgroundType::FullHeight && button->type() == KDecoration3::DecorationButtonType::Close) {
            button->setIconOffset(QPointF(horizontalIconOffsetLeftFullHeightClose, verticalIconOffset));
        } else {
            button->setIconOffset(QPointF(horizontalIconOffsetLeftButtons, verticalIconOffset));
        }
        button->setScaledSmallButtonPaddedSize(QSizeF(m_scaledSmallButtonPaddedSize, m_scaledSmallButtonPaddedSize));
        button->setScaledIconSize(QSizeF(m_scaledIconSize, m_scaledIconSize));

        button->setLeftButtonVisible(false);
        button->setRightButtonVisible(false);
        m_isRightToLeft ? button->setRightmostLeftVisible(false) : button->setLeftmostLeftVisible(false);
        button->setVisibleAfterMenu(false);
        button->setVisibleBeforeMenu(false);
        m_isRightToLeft ? button->setLeftmostLeftVisible(false) : button->setRightmostLeftVisible(false);
        button->setVisibleAfterSpacer(false);
        button->setVisibleBeforeSpacer(false);
        // determine leftmost left visible and rightmostLeftVisible
        if (button->isVisible() && (button->isEnabled() || button->type() == KDecoration3::DecorationButtonType::Spacer)) {
            button->setLeftButtonVisible(true);

            if (firstLeftVisibleIndex == -1) {
                firstLeftVisibleIndex = i;
                m_isRightToLeft ? button->setRightmostLeftVisible() : button->setLeftmostLeftVisible();
            }

            if (menuPresentBefore) {
                m_isRightToLeft ? button->setVisibleBeforeMenu() : button->setVisibleAfterMenu();
                menuPresentBefore = false;
            }

            if (spacerPresentBefore) {
                m_isRightToLeft ? button->setVisibleBeforeSpacer() : button->setVisibleAfterSpacer();
                spacerPresentBefore = false;
            }

            if (button->type() == KDecoration3::DecorationButtonType::Menu) {
                menuPresentBefore = true;
            } else if (button->type() == KDecoration3::DecorationButtonType::Spacer) {
                spacerPresentBefore = true;
            }

            lastLeftVisibleIndex = i;
        }
    }

    if (lastLeftVisibleIndex != -1) {
        bool menuPresentAfter = false;
        bool spacerPresentAfter = false;

        m_isRightToLeft ? static_cast<Button *>(m_leftButtons->buttons()[lastLeftVisibleIndex])->setLeftmostLeftVisible()
                        : static_cast<Button *>(m_leftButtons->buttons()[lastLeftVisibleIndex])->setRightmostLeftVisible();

        for (int i = numLeftButtons - 2; i >= 0; i--) {
            Button *button = static_cast<Button *>(m_leftButtons->buttons()[i]);
            if (button->isVisible() && (button->isEnabled() || button->type() == KDecoration3::DecorationButtonType::Spacer)) {
                if (menuPresentAfter) {
                    m_isRightToLeft ? button->setVisibleAfterMenu() : button->setVisibleBeforeMenu();
                    menuPresentAfter = false;
                }

                if (spacerPresentAfter) {
                    m_isRightToLeft ? button->setVisibleAfterSpacer() : button->setVisibleBeforeSpacer();
                    spacerPresentAfter = false;
                }

                if (button->type() == KDecoration3::DecorationButtonType::Menu) {
                    menuPresentAfter = true;
                } else if (button->type() == KDecoration3::DecorationButtonType::Spacer) {
                    spacerPresentAfter = true;
                }
            }
        }
    }

    int firstRightVisibleIndex = -1;
    int lastRightVisibleIndex = -1;
    int numRightButtons = m_rightButtons->buttons().count();
    menuPresentBefore = false;
    spacerPresentBefore = false;

    for (int i = 0; i < numRightButtons; i++) {
        Button *button = static_cast<Button *>(m_rightButtons->buttons()[i]);

        qreal bHeight = bHeightNormal;
        qreal verticalIconOffset = verticalIconOffsetNormal;

        if (button->type() == KDecoration3::DecorationButtonType::Menu) {
            if (m_internalSettings->buttonShape() == InternalSettings::EnumButtonShape::IntegratedRoundedRectangleGrouped) {
                bHeight = bHeightMenuGrouped;
                verticalIconOffset = verticalIconOffsetMenuGrouped;
            }
        }

        qreal bWidth;
        if (m_buttonBackgroundType == ButtonBackgroundType::FullHeight) {
            if (button->type() == KDecoration3::DecorationButtonType::Close) {
                qreal bWidthMargin =
                    KDecoration3::snapToPixelGrid(m_scaledButtonWidthMarginRight * m_internalSettings->closeFullHeightButtonWidthMarginRelative() / 100.0f,
                                                  scale);
                bWidth = m_scaledIconSize + bWidthMargin * 2;
                horizontalIconOffsetRightFullHeightClose = bWidthMargin;
            } else {
                bWidth = bWidthRight;
            }
            button->setBackgroundVisibleSize(QSizeF(bWidth, bHeight));
        } else {
            bWidth = bWidthRight;
            button->setBackgroundVisibleSize(QSizeF(m_scaledSmallButtonBackgroundSize, m_scaledSmallButtonBackgroundSize));
        }
        if (button->type() == KDecoration3::DecorationButtonType::Spacer) {
            bWidth = KDecoration3::snapToPixelGrid(bWidth * m_internalSettings->spacerButtonWidthRelative() / 100.0f, scale);
        }

        button->setGeometry(QRectF(QPoint(0, 0), QSizeF(bWidth, bHeight)));
        if (m_buttonBackgroundType == ButtonBackgroundType::FullHeight && button->type() == KDecoration3::DecorationButtonType::Close) {
            button->setIconOffset(QPointF(horizontalIconOffsetRightFullHeightClose, verticalIconOffset));
        } else {
            button->setIconOffset(QPointF(horizontalIconOffsetRightButtons, verticalIconOffset));
        }
        button->setScaledSmallButtonPaddedSize(QSizeF(m_scaledSmallButtonPaddedSize, m_scaledSmallButtonPaddedSize));
        button->setScaledIconSize(QSizeF(m_scaledIconSize, m_scaledIconSize));

        button->setRightButtonVisible(false);
        button->setLeftButtonVisible(false);
        m_isRightToLeft ? button->setRightmostRightVisible(false) : button->setLeftmostRightVisible(false);
        button->setVisibleAfterMenu(false);
        button->setVisibleBeforeMenu(false);
        m_isRightToLeft ? button->setLeftmostRightVisible(false) : button->setRightmostRightVisible(false);
        button->setVisibleAfterSpacer(false);
        button->setVisibleBeforeSpacer(false);
        // determine leftmost right visible and rightmostRightVisible
        if (button->isVisible() && (button->isEnabled() || button->type() == KDecoration3::DecorationButtonType::Spacer)) {
            button->setRightButtonVisible(true);

            if (firstRightVisibleIndex == -1) {
                firstRightVisibleIndex = i;
                m_isRightToLeft ? button->setRightmostRightVisible() : button->setLeftmostRightVisible();
            }

            if (menuPresentBefore) {
                m_isRightToLeft ? button->setVisibleBeforeMenu() : button->setVisibleAfterMenu();
                menuPresentBefore = false;
            }

            if (spacerPresentBefore) {
                m_isRightToLeft ? button->setVisibleBeforeSpacer() : button->setVisibleAfterSpacer();
                spacerPresentBefore = false;
            }

            if (button->type() == KDecoration3::DecorationButtonType::Menu) {
                menuPresentBefore = true;
            } else if (button->type() == KDecoration3::DecorationButtonType::Spacer) {
                spacerPresentBefore = true;
            }

            lastRightVisibleIndex = i;
        }
    }

    if (lastRightVisibleIndex != -1) {
        bool menuPresentAfter = false;
        bool spacerPresentAfter = false;
        m_isRightToLeft ? static_cast<Button *>(m_rightButtons->buttons()[lastRightVisibleIndex])->setLeftmostRightVisible()
                        : static_cast<Button *>(m_rightButtons->buttons()[lastRightVisibleIndex])->setRightmostRightVisible();

        for (int i = numRightButtons - 2; i >= 0; i--) {
            Button *button = static_cast<Button *>(m_rightButtons->buttons()[i]);
            if (button->isVisible() && (button->isEnabled() || button->type() == KDecoration3::DecorationButtonType::Spacer)) {
                if (menuPresentAfter) {
                    m_isRightToLeft ? button->setVisibleAfterMenu() : button->setVisibleBeforeMenu();
                    menuPresentAfter = false;
                }

                if (spacerPresentAfter) {
                    m_isRightToLeft ? button->setVisibleAfterSpacer() : button->setVisibleBeforeSpacer();
                    spacerPresentAfter = false;
                }

                if (button->type() == KDecoration3::DecorationButtonType::Menu) {
                    menuPresentAfter = true;
                } else if (button->type() == KDecoration3::DecorationButtonType::Spacer) {
                    spacerPresentAfter = true;
                }
            }
        }
    }

    // left buttons
    int &leftEdgeButtonIndex = m_isRightToLeft ? lastLeftVisibleIndex : firstLeftVisibleIndex;
    if (!m_leftButtons->buttons().isEmpty() && leftEdgeButtonIndex != -1) {
        // spacing
        m_leftButtons->setSpacing(m_scaledButtonSpacingLeft);

        // padding
        qreal vPadding;
        if (m_buttonBackgroundType == ButtonBackgroundType::FullHeight)
            vPadding = 0;
        else
            vPadding = isTopEdge() ? 0 : m_scaledTitleBarTopMargin;
        const qreal hPadding = scaledTitleBarLeftMargin(false);

        auto leftEdgeButton = static_cast<Button *>(m_leftButtons->buttons()[leftEdgeButtonIndex]);
        if (isLeftEdge()) {
            // add offsets on the side buttons, to preserve padding, but satisfy Fitts law
            leftEdgeButton->setGeometry(QRectF(QPoint(0, 0), QSizeF(leftEdgeButton->geometry().width() + hPadding, leftEdgeButton->geometry().height())));

            if (m_buttonBackgroundType == ButtonBackgroundType::FullHeight && leftEdgeButton->type() == KDecoration3::DecorationButtonType::Close) {
                leftEdgeButton->setHorizontalIconOffset(horizontalIconOffsetLeftFullHeightClose + hPadding);
            } else {
                leftEdgeButton->setHorizontalIconOffset(horizontalIconOffsetLeftButtons + hPadding);
            }
            leftEdgeButton->setFullHeightVisibleBackgroundOffset(QPointF(hPadding, 0));

            m_leftButtons->setPos(QPointF(0, vPadding));

        } else {
            m_leftButtons->setPos(QPointF(hPadding + borderLeft(), vPadding));
            leftEdgeButton->setFullHeightVisibleBackgroundOffset(QPointF(0, 0));
        }
    }

    // right buttons
    int &rightEdgeButtonIndex = m_isRightToLeft ? firstRightVisibleIndex : lastRightVisibleIndex;
    if (!m_rightButtons->buttons().isEmpty() && lastRightVisibleIndex != -1) {
        // spacing
        m_rightButtons->setSpacing(m_scaledButtonSpacingRight);

        // padding
        qreal vPadding;
        if (m_buttonBackgroundType == ButtonBackgroundType::FullHeight)
            vPadding = 0;
        else
            vPadding = isTopEdge() ? 0 : m_scaledTitleBarTopMargin;
        const qreal hPadding = scaledTitleBarRightMargin(false);

        auto rightEdgeButton = static_cast<Button *>(m_rightButtons->buttons()[rightEdgeButtonIndex]);
        if (isRightEdge()) {
            rightEdgeButton->setGeometry(QRectF(QPoint(0, 0), QSizeF(rightEdgeButton->geometry().width() + hPadding, rightEdgeButton->geometry().height())));

            m_rightButtons->setPos(QPointF(size().width() - m_rightButtons->geometry().width(), vPadding));

        } else {
            m_rightButtons->setPos(QPointF(size().width() - m_rightButtons->geometry().width() - hPadding - borderRight(), vPadding));
        }
    }

    update();
}

//________________________________________________________________
void Decoration::paint(QPainter *painter, const QRectF &repaintRegion)
{
    // TODO: optimize based on repaintRegion
    auto c = window();
    auto s = settings();

    calculateWindowShape();

    // paint background
    if (!c->isShaded()) {
        painter->fillRect(rect(), Qt::transparent);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);

        QColor windowBorderColor;
        if (m_internalSettings->useTitleBarColorForAllBorders()) {
            windowBorderColor = titleBarColor();
        } else
            windowBorderColor = c->color(c->isActive() ? ColorGroup::Active : ColorGroup::Inactive, ColorRole::Frame);

        painter->setBrush(windowBorderColor);
        painter->drawPath(m_windowPath);
        //
        painter->restore();
    }

    if (!hideTitleBar()) {
        calculateTitleBarShape();
        paintTitleBar(painter, repaintRegion);
    }

    if (hasBorders() && !s->isAlphaChannelSupported()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(c->isActive() ? c->color(ColorGroup::Active, ColorRole::TitleBar) : c->color(ColorGroup::Inactive, ColorRole::Foreground));

        painter->drawRect(rect().adjusted(0, 0, -1, -1));
        painter->restore();
    }
}

void Decoration::calculateWindowShape()
{
    auto c = window();
    auto s = settings();

    // set windowPath
    QRectF windowRect = rect();
    m_windowPath.clear(); // clear the path for subsequent calls to this function

    if (!c->isShaded()) {
        if (s->isAlphaChannelSupported() && !isMaximized()) {
            Corners windowCorners;

            if (!isBottomEdge() && !(hasNoBorders() && !m_internalSettings->roundAllCornersWhenNoBorders())) {
                if (!isLeftEdge()) {
                    windowCorners |= CornerBottomLeft;
                }
                if (!isRightEdge()) {
                    windowCorners |= CornerBottomRight;
                }
            }

            if (!isTopEdge() && !(hideTitleBar() && hasNoBorders() && !m_internalSettings->roundAllCornersWhenNoBorders())) {
                if (!isLeftEdge()) {
                    windowCorners |= CornerTopLeft;
                }
                if (!isRightEdge()) {
                    windowCorners |= CornerTopRight;
                }
            }
            m_windowPath = GeometryTools::roundedPath(windowRect, windowCorners, m_scaledCornerRadius);

        } else // maximized / no alpha
            m_windowPath.addRect(windowRect);

    } else { // shaded
        m_titleRect = QRectF(QPointF(0, 0), QSizeF(size().width(), borderTop()));

        if (isMaximized() || !s->isAlphaChannelSupported()) {
            m_windowPath.addRect(m_titleRect);
        } else {
            m_windowPath.addRoundedRect(m_titleRect, m_scaledCornerRadius, m_scaledCornerRadius);
        }
    }
}

void Decoration::calculateTitleBarShape()
{
    auto c = window();
    auto s = settings();

    // set titleBar geometry and path
    m_titleRect = QRectF(QPointF(0, 0), QSizeF(size().width(), borderTop()));

    m_titleBarPath.clear(); // clear the path for subsequent calls to this function
    if (isMaximized() || !s->isAlphaChannelSupported()) {
        m_titleBarPath.addRect(m_titleRect);
    } else if (c->isShaded()) {
        m_titleBarPath.addRoundedRect(m_titleRect, m_scaledCornerRadius, m_scaledCornerRadius);
    } else {
        Corners titleBarCorners;
        if (!isTopEdge()) {
            if (!isLeftEdge()) {
                titleBarCorners |= CornerTopLeft;
            }
            if (!isRightEdge()) {
                titleBarCorners |= CornerTopRight;
            }
        }
        m_titleBarPath = GeometryTools::roundedPath(m_titleRect, titleBarCorners, m_scaledCornerRadius);
    }
}

//________________________________________________________________
void Decoration::paintTitleBar(QPainter *painter, const QRectF &repaintRegion)
{
    const auto c = window();

    if (!m_titleRect.intersects(repaintRegion)) {
        return;
    }

    qreal scale = c->scale();

    painter->save();
    painter->setPen(Qt::NoPen);

    QColor titleBarColor(this->titleBarColor());

    if (titleBarColor.alpha() < 255) {
        // on certain fractional scales there is an overlap with the window content
        // this overlap is visible when translucent unless CompositionMode_Source is set
        painter->setCompositionMode(QPainter::CompositionMode_Source);
    }

    // render a linear gradient on title area
    if (c->isActive() && m_internalSettings->drawBackgroundGradient()) {
        QLinearGradient gradient(0, 0, 0, m_titleRect.height());
        gradient.setColorAt(0.0, titleBarColor.lighter(120));
        gradient.setColorAt(0.8, titleBarColor);
        painter->setBrush(gradient);

    } else {
        painter->setBrush(titleBarColor);
    }

    auto s = settings();

    painter->drawPath(m_titleBarPath);

    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    // draw titlebar separator
    qreal separatorHeight;
    if ((separatorHeight = scaledTitleBarSeparatorHeight(false))) {
        const QColor titleBarSeparatorColor(this->titleBarSeparatorColor());

        if (titleBarSeparatorColor.isValid()) {
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setBrush(Qt::NoBrush);
            QPen p(titleBarSeparatorColor);
            p.setWidthF(separatorHeight);
            p.setCapStyle(Qt::FlatCap);
            painter->setPen(p);

            qreal separatorYCoOrd = m_titleRect.bottom() - separatorHeight / 2;
            if (m_internalSettings->useTitleBarColorForAllBorders()) {
                painter->drawLine(QPointF(m_titleRect.bottomLeft().x() + borderLeft(), separatorYCoOrd),
                                  QPointF(m_titleRect.bottomRight().x() - borderRight(), separatorYCoOrd));
            } else {
                painter->drawLine(QPointF(m_titleRect.bottomLeft().x(), separatorYCoOrd), QPointF(m_titleRect.bottomRight().x(), separatorYCoOrd));
            }
        }
    }

    painter->restore();

    // draw caption
    QFont font = s->font();
    if (m_internalSettings->boldTitle() && c->isActive()) {
        QFont::Weight weight = font.weight();
        if (weight < QFont::Black) {
            weight = (weight >= QFont::DemiBold) ? QFont::ExtraBold : QFont::DemiBold;
        }
        font.setWeight(weight);
    }
    painter->setFont(font);
    QColor fontColor = this->fontColor();
    painter->setPen(fontColor);
    const auto [maxCaptionRectangle, alignment] = captionRect();
    const QString caption = painter->fontMetrics().elidedText(c->caption(), Qt::ElideMiddle, maxCaptionRectangle.width());
    QRectF captionBoundingRect;
    painter->drawText(maxCaptionRectangle, alignment | Qt::TextSingleLine, caption, &captionBoundingRect);

    // draw underline
    if (m_internalSettings->underlineTitle() && c->isActive()) {
        QPen underlinePen(titleBarSeparatorColor());
        qreal penWidth = 1;
        if (KWindowSystem::isPlatformX11())
            penWidth *= m_systemScaleFactorX11;
        penWidth = KDecoration3::snapToPixelGrid(penWidth, scale);
        underlinePen.setWidthF(penWidth);
        painter->setPen(underlinePen);
        qreal halfPenWidth = penWidth / 2;
        QLine underline(captionBoundingRect.left(),
                        captionBoundingRect.bottom() + halfPenWidth,
                        captionBoundingRect.right(),
                        captionBoundingRect.bottom() + halfPenWidth);
        painter->drawLine(underline);
    }

    // draw all buttons
    painter->setPen(fontColor);
    m_leftButtons->paint(painter, repaintRegion);
    m_rightButtons->paint(painter, repaintRegion);
}

// outputs the icon size + padding to make a small button, the actual icon size, and the background size to make a small button
void Decoration::setScaledIconSizes(const bool nextScale)
{
    qreal scale = nextScale ? window()->nextScale() : window()->scale();
    qreal baseSize = settings()->gridUnit(); // 10 on Wayland
    qreal basePaddingSize = m_smallSpacing; // 2 on Wayland

    qreal &scaledSmallButtonPaddedSize = nextScale ? m_scaledSmallButtonPaddedSizeNext : m_scaledSmallButtonPaddedSize;
    qreal &scaledIconSize = nextScale ? m_scaledIconSizeNext : m_scaledIconSize;
    qreal &scaledSmallButtonBackgroundSize = nextScale ? m_scaledSmallButtonBackgroundSizeNext : m_scaledSmallButtonBackgroundSize;

    if (m_internalSettings->buttonIconStyle() == InternalSettings::EnumButtonIconStyle::SystemIconTheme) {
        switch (m_internalSettings->systemIconSize()) {
        case InternalSettings::EnumSystemIconSize::SystemIcon8: // 10, 8 on Wayland
            break;
        case InternalSettings::EnumSystemIconSize::SystemIcon12: // 14, 12 on Wayland
            baseSize *= 1.4;
            break;
        case InternalSettings::EnumSystemIconSize::SystemIcon14: // 16, 14 on Wayland
            baseSize *= 1.6;
            break;
        default:
        case InternalSettings::EnumSystemIconSize::SystemIcon16: // 18, 16 on Wayland
            baseSize *= 1.8;
            break;
        case InternalSettings::EnumSystemIconSize::SystemIcon18: // 20, 18 on Wayland
            baseSize *= 2;
            break;
        case InternalSettings::EnumSystemIconSize::SystemIcon20: // 22, 20 on Wayland
            baseSize *= 2.2;
            break;
        case InternalSettings::EnumSystemIconSize::SystemIcon22: // 24, 22 on Wayland
            baseSize *= 2.4;
            break;
        case InternalSettings::EnumSystemIconSize::SystemIcon24: // 26, 24 on Wayland
            baseSize *= 2.6;
            break;
        case InternalSettings::EnumSystemIconSize::SystemIcon32: // 36, 32 on Wayland
            baseSize *= 3.6;
            basePaddingSize *= 2;
            break;
        case InternalSettings::EnumSystemIconSize::SystemIcon48: // 52, 48 on Wayland
            baseSize *= 5.2;
            basePaddingSize *= 2;
            break;
        }
    } else {
        switch (m_internalSettings->iconSize()) {
        case InternalSettings::EnumIconSize::Tiny: // 10, 8 on Wayland
            break;
        case InternalSettings::EnumIconSize::VerySmall: // 14, 12 on Wayland
            baseSize *= 1.4;
            break;
        case InternalSettings::EnumIconSize::Small: // 16, 14 on Wayland
            baseSize *= 1.6;
            break;
        case InternalSettings::EnumIconSize::SmallMedium: // 18, 16 on Wayland
            baseSize *= 1.8;
            break;
        default:
        case InternalSettings::EnumIconSize::Medium: // 20, 18 on Wayland
            baseSize *= 2;
            break;
        case InternalSettings::EnumIconSize::LargeMedium: // 22, 20 on Wayland
            baseSize *= 2.2;
            break;
        case InternalSettings::EnumIconSize::Large: // 24, 22 on Wayland
            baseSize *= 2.4;
            break;
        case InternalSettings::EnumIconSize::VeryLarge: // 26, 24 on Wayland
            baseSize *= 2.6;
            break;
        case InternalSettings::EnumIconSize::Giant: // 36, 32 on Wayland
            baseSize *= 3.6;
            basePaddingSize *= 2;
            break;
        case InternalSettings::EnumIconSize::Humongous: // 52, 48 on Wayland
            baseSize *= 5.2;
            basePaddingSize *= 2;
            break;
        }
    }

    baseSize = KDecoration3::snapToPixelGrid(baseSize, scale);
    basePaddingSize = KDecoration3::snapToPixelGrid(basePaddingSize, scale);

    scaledSmallButtonPaddedSize = baseSize;
    scaledIconSize = baseSize - basePaddingSize;

    if (m_buttonBackgroundType == ButtonBackgroundType::Small) {
        qreal smallBackgroundScaleFactor = qreal(m_internalSettings->scaleBackgroundPercent()) / 100;

        scaledSmallButtonPaddedSize = KDecoration3::snapToPixelGrid(scaledSmallButtonPaddedSize * smallBackgroundScaleFactor, scale);

        scaledSmallButtonBackgroundSize = KDecoration3::snapToPixelGrid(scaledIconSize * smallBackgroundScaleFactor, scale);
    }
}

//________________________________________________________________
qreal Decoration::captionHeight() const
{
    return hideTitleBar() ? borderTop() : borderTop() - m_scaledTitleBarTopMargin - m_scaledTitleBarBottomMargin - scaledTitleBarSeparatorHeight(false);
}

//________________________________________________________________
QPair<QRectF, Qt::Alignment> Decoration::captionRect() const
{
    if (hideTitleBar()) {
        return qMakePair(QRect(), Qt::AlignCenter);
    } else {
        auto c = window();
        qreal scale = c->scale();

        qreal captionHeight = this->captionHeight();

        qreal padding = KDecoration3::snapToPixelGrid(m_internalSettings->titleSidePadding() * m_x11Scale, scale);

        const qreal leftOffset = m_leftButtons->buttons().isEmpty() ? padding : m_leftButtons->geometry().x() + m_leftButtons->geometry().width() + padding;
        const qreal rightOffset = m_rightButtons->buttons().isEmpty() ? padding : size().width() - m_rightButtons->geometry().x() + padding;

        const qreal yOffset = m_scaledTitleBarTopMargin;
        const QRectF maxRect(leftOffset, yOffset, size().width() - leftOffset - rightOffset, captionHeight);

        switch (m_internalSettings->titleAlignment()) {
        case InternalSettings::EnumTitleAlignment::AlignLeft:
            return qMakePair(maxRect, Qt::AlignVCenter | Qt::AlignLeft);

        case InternalSettings::EnumTitleAlignment::AlignRight:
            return qMakePair(maxRect, Qt::AlignVCenter | Qt::AlignRight);

        case InternalSettings::EnumTitleAlignment::AlignCenter:
            return qMakePair(maxRect, Qt::AlignCenter);

        default:
        case InternalSettings::EnumTitleAlignment::AlignCenterFullWidth: {
            // full caption rect
            const QRectF fullRect = QRectF(0, yOffset, size().width(), captionHeight);
            QRectF boundingRect(settings()->fontMetrics().boundingRect(c->caption()).toRect());

            // text bounding rect
            boundingRect.setTop(yOffset);
            boundingRect.setHeight(captionHeight);
            boundingRect.moveLeft((size().width() - boundingRect.width()) / 2);

            if (boundingRect.left() < leftOffset) {
                return qMakePair(maxRect, Qt::AlignVCenter | Qt::AlignLeft);
            } else if (boundingRect.right() > size().width() - rightOffset) {
                return qMakePair(maxRect, Qt::AlignVCenter | Qt::AlignRight);
            } else {
                return qMakePair(fullRect, Qt::AlignCenter);
            }
        }
        }
    }
}

//________________________________________________________________
void Decoration::updateShadow(const bool forceUpdateCache, bool noCache)
{
    auto c = window();

    // The preset exception may modify the shadow, so in this case there is a "noCache" property set - we don't want to cache the exception shadow as it may
    // corrupt the shadow cache for normal non-exception decoration windows. For shaded windows the shadow has a potentially different shape so do not
    // use the shadow cache when shaded
    if (m_internalSettings->property("noCacheException").toBool() || c->isShaded()) {
        noCache = true;
    }
    // Animated case, no cached shadow object
    if ((m_shadowAnimation->state() == QAbstractAnimation::Running) && (m_shadowOpacity != 0.0) && (m_shadowOpacity != 1.0)) {
        QColor shadowColor = KColorUtils::mix(m_decorationColors->inactive()->shadow, m_decorationColors->active()->shadow, m_shadowOpacity);
        setShadow(createShadowObject(shadowColor));
        return;
    }

    // TODO: Potentially make the kdecoration configwidget more intelligent and send a dbus signal which is aware of whether to update the shadow or not, so
    // there is less processing here
    // check if cached settings have changed, if so replace them with new settings values and regenerate the shadow cache
    if (!noCache
        && (forceUpdateCache || g_shadowSizeEnumActive != m_internalSettings->shadowSize(true)
            || g_shadowSizeEnumInactive != m_internalSettings->shadowSize(false) || g_shadowStrengthActive != m_internalSettings->shadowStrength(true)
            || g_shadowStrengthInactive != m_internalSettings->shadowStrength(false) || g_shadowColorActive != m_internalSettings->shadowColor(true)
            || g_shadowColorInactive != m_internalSettings->shadowColor(false) || !(qAbs(g_cornerRadius - m_scaledCornerRadius) < 0.001)
            || g_hasNoBorders != hasNoBorders() || g_roundAllCornersWhenNoBorders != m_internalSettings->roundAllCornersWhenNoBorders()
            || g_hideTitleBar != hideTitleBar())) {
        g_sShadow.reset();
        g_sShadowInactive.reset();
        g_shadowSizeEnumActive = m_internalSettings->shadowSize(true);
        g_shadowStrengthActive = m_internalSettings->shadowStrength(true);
        g_shadowColorActive = m_internalSettings->shadowColor(true);
        g_shadowSizeEnumInactive = m_internalSettings->shadowSize(false);
        g_shadowStrengthInactive = m_internalSettings->shadowStrength(false);
        g_shadowColorInactive = m_internalSettings->shadowColor(false);
        g_cornerRadius = m_scaledCornerRadius;
        g_hasNoBorders = hasNoBorders();
        g_roundAllCornersWhenNoBorders = m_internalSettings->roundAllCornersWhenNoBorders();
        g_hideTitleBar = hideTitleBar();
    }

    std::shared_ptr<KDecoration3::DecorationShadow> nonCachedShadow;
    std::shared_ptr<KDecoration3::DecorationShadow> *shadow = nullptr;

    if (noCache)
        shadow = &nonCachedShadow;
    else // use the already cached shadow
        shadow = (c->isActive()) ? &g_sShadow : &g_sShadowInactive;

    if (!(*shadow)) { // only recreate the shadow if necessary
        QColor shadowColor = c->isActive() ? m_decorationColors->active()->shadow : m_decorationColors->inactive()->shadow;
        *shadow = createShadowObject(shadowColor);
    }

    setShadow(*shadow);
}

//________________________________________________________________
std::shared_ptr<KDecoration3::DecorationShadow> Decoration::createShadowObject(QColor shadowColor)
{
    auto c = window();
    bool active = c->isActive();

    if (active && m_internalSettings->shadowSize(true) == InternalSettings::EnumShadowSize::None) {
        return nullptr;
    }
    if (!active && m_internalSettings->shadowSize(false) == InternalSettings::EnumShadowSize::None) {
        return nullptr;
    }

    const CompositeShadowParams params = lookupShadowParams(active ? m_internalSettings->shadowSize(true) : m_internalSettings->shadowSize(false));

    qreal shadow1Radius = params.shadow1.radius;
    qreal shadow2Radius = params.shadow2.radius;

    QSize boxSize =
        BoxShadowRenderer::calculateMinimumBoxSize(std::round(shadow1Radius)).expandedTo(BoxShadowRenderer::calculateMinimumBoxSize(std::round(shadow2Radius)));

    BoxShadowRenderer shadowRenderer;

    shadowRenderer.setBorderRadius((m_scaledCornerRadius + 0.5));
    shadowRenderer.setBoxSize(boxSize);
    shadowRenderer.addShadow(params.shadow1.offset, shadow1Radius, ColorTools::alphaMix(shadowColor, params.shadow1.opacity));
    shadowRenderer.addShadow(params.shadow2.offset, shadow2Radius, ColorTools::alphaMix(shadowColor, params.shadow2.opacity));

    QImage shadowTexture = shadowRenderer.render();

    QPainter painter(&shadowTexture);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRect outerRect = shadowTexture.rect();

    QRect boxRect(QPoint(0, 0), boxSize);
    boxRect.moveCenter(outerRect.center());

    qreal shadowOverlap = Metrics::Decoration_Shadow_Overlap;
    qreal shadowOffsetX = params.offset.x();
    qreal shadowOffsetY = params.offset.y();

    // Mask out inner rect.
    const QMargins padding = QMargins(boxRect.left() - outerRect.left() - shadowOverlap - shadowOffsetX,
                                      boxRect.top() - outerRect.top() - shadowOverlap - shadowOffsetY,
                                      outerRect.right() - boxRect.right() - shadowOverlap + shadowOffsetX,
                                      outerRect.bottom() - boxRect.bottom() - shadowOverlap + shadowOffsetY);

    const QRectF innerRect = outerRect - padding;

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);

    QPainterPath roundedRectMask;
    if (hasNoBorders() && !m_internalSettings->roundAllCornersWhenNoBorders() && !c->isShaded()) {
        if (hideTitleBar()) {
            roundedRectMask.addRect(innerRect);
        } else {
            roundedRectMask = GeometryTools::roundedPath(innerRect, CornersTop, (m_scaledCornerRadius + 0.5));
        }
    } else {
        roundedRectMask.addRoundedRect(innerRect, (m_scaledCornerRadius + 0.5), (m_scaledCornerRadius + 0.5));
    }

    painter.drawPath(roundedRectMask);
    painter.end();

    auto ret = std::make_shared<KDecoration3::DecorationShadow>();
    ret->setPadding(padding);
    ret->setInnerShadowRect(QRect(outerRect.center(), QSize(1, 1)));
    ret->setShadow(shadowTexture);
    return ret;
}

void Decoration::updateWindowOutline(bool override)
{
    if (isMaximized() || (windowOutlineNone() && !override)) {
        setBorderOutline(KDecoration3::BorderOutline());
    } else {
        setWindowOutlineColor();
        if (m_windowOutline.isValid()) {
            const qreal thickness = std::max(KDecoration3::pixelSize(window()->nextScale()),
                                             KDecoration3::snapToPixelGrid(m_internalSettings->windowOutlineThickness(), window()->nextScale()));

            // set clipped corners
            qreal bottomLeftRadius = 0;
            qreal bottomRightRadius = 0;
            qreal topLeftRadius = 0;
            qreal topRightRadius = 0;

            if (!isBottomEdge() && !(hasNoBorders() && !m_internalSettings->roundAllCornersWhenNoBorders())) {
                if (!isLeftEdge()) {
                    bottomLeftRadius = m_scaledCornerRadius;
                }
                if (!isRightEdge()) {
                    bottomRightRadius = m_scaledCornerRadius;
                }
            }

            if (!isTopEdge() && !(hideTitleBar() && hasNoBorders() && !m_internalSettings->roundAllCornersWhenNoBorders())) {
                if (!isLeftEdge()) {
                    topLeftRadius = m_scaledCornerRadius;
                }
                if (!isRightEdge()) {
                    topRightRadius = m_scaledCornerRadius;
                }
            }

            const auto radius = KDecoration3::BorderRadius(topLeftRadius, topRightRadius, bottomRightRadius, bottomLeftRadius);
            QColor color(qPremultiply(m_windowOutline.rgba()));
            color.setAlpha(m_windowOutline.alpha()); // QColor(QRgb) constructor discards alpha
            setBorderOutline(KDecoration3::BorderOutline(thickness, color, radius));
        } else {
            setBorderOutline(KDecoration3::BorderOutline());
        }
    }
}

void Decoration::setWindowOutlineOverrideColor(const bool on, const QColor &color)
{
    auto c = window();

    if (on) {
        if (!c->isMaximized()) {
            // draw a thin window outline with this override colour
            m_windowOutlineOverride = color;
            updateOverrideWindowOutlineFromButtonAnimationState();
        }
    } else {
        if (!c->isMaximized()) {
            // reset the thin window outline
            m_windowOutlineOverride = QColor();
            m_animateOutOverriddenWindowOutline = true;
            updateOverrideWindowOutlineFromButtonAnimationState();
        }
    }
}

void Decoration::setWindowOutlineColor()
{
    auto c = window();

    if (m_windowOutlineOverride.isValid()) {
        m_windowOutline = overriddenOutlineColorAnimateIn();
    } else { // normal case, not an override

        QColor windowOutlineActiveFinal = m_decorationColors->active()->windowOutline;
        QColor windowOutlineInactiveFinal = m_decorationColors->inactive()->windowOutline;

        if (m_internalSettings->colorizeWindowOutlineWithButton() && c->isKeepAbove()) { // set a window outline if window keep in front button is checked
            QColor keepAboveOutlinePress = m_decorationColors->buttonPalette(DecorationButtonType::KeepAbove)->active()->outlinePress;
            if (keepAboveOutlinePress.isValid()) {
                qreal alpha = keepAboveOutlinePress.alphaF();
                if (alpha <= 0.6) {
                    keepAboveOutlinePress.setAlphaF(std::min(1.0, alpha * 2.5));
                }
                m_windowOutline = keepAboveOutlinePress;
            } else {
                QColor keepAboveBackgroundPress = m_decorationColors->buttonPalette(DecorationButtonType::KeepAbove)->active()->backgroundPress;
                if (keepAboveBackgroundPress.isValid()) {
                    qreal alpha = keepAboveBackgroundPress.alphaF();
                    if (alpha <= 0.6) {
                        keepAboveBackgroundPress.setAlphaF(std::min(1.0, alpha * 2.5));
                    }
                    m_windowOutline = keepAboveBackgroundPress;
                }
            }
        } else if (m_shadowAnimation->state() == QAbstractAnimation::Running) { // get blended colour if animated
            // deal with animation cases where there is an invalid colour (EnumWindowOutlineStyle::None)
            if (!(windowOutlineActiveFinal.isValid() && windowOutlineInactiveFinal.isValid())) {
                if (!windowOutlineInactiveFinal.isValid() && windowOutlineActiveFinal.isValid()) {
                    m_windowOutline = ColorTools::alphaMix(windowOutlineActiveFinal, m_shadowOpacity);
                } else if (windowOutlineInactiveFinal.isValid() && !windowOutlineActiveFinal.isValid()) {
                    m_windowOutline = ColorTools::alphaMix(windowOutlineInactiveFinal, (1.0 - m_shadowOpacity));
                }
            } else { // standard animated case with both valid colours
                m_windowOutline = KColorUtils::mix(windowOutlineInactiveFinal, windowOutlineActiveFinal, m_shadowOpacity);
            }
        } else { // normal non-animated final colour
            m_windowOutline = c->isActive() ? windowOutlineActiveFinal : windowOutlineInactiveFinal;
        }
    }

    // deal with override colours ("Colourize with highlighted button's colour")
    if (m_animateOutOverriddenWindowOutline)
        m_windowOutline = overriddenOutlineColorAnimateOut(m_windowOutline);

    // the existing thin window outline colour is stored in-case it is overridden in the future and needed by an animation
    if (!m_windowOutlineOverride.isValid()) { // non-override
        c->isActive() ? m_originalWindowOutlineActivePreOverride = m_windowOutline : m_originalWindowOutlineInactivePreOverride = m_windowOutline;
    } else if ((m_overrideWindowOutlineFromButtonAnimation->state() == QAbstractAnimation::Running) && m_overrideOutlineAnimationProgress == 1) {
        // only buffer the override colour once it has finished animating -- used for the override out animation, and when mouse moves from one overrride
        // colour to another
        c->isActive() ? m_originalWindowOutlineActivePreOverride = m_windowOutline : m_originalWindowOutlineInactivePreOverride = m_windowOutline;
    }
}

void Decoration::setScaledCornerRadius()
{
    m_scaledCornerRadius = m_internalSettings->windowCornerRadius() * m_x11Scale;
}

void Decoration::setScaledTitleBarMargins(const bool nextScale)
{
    auto c = window();
    qreal scale = nextScale ? c->nextScale() : c->scale();
    qreal &scaledIntegratedRoundedRectangleBottomPadding =
        nextScale ? m_scaledIntegratedRoundedRectangleBottomPaddingNext : m_scaledIntegratedRoundedRectangleBottomPadding;
    qreal &scaledTitleBarTopMargin = nextScale ? m_scaledTitleBarTopMarginNext : m_scaledTitleBarTopMargin;
    qreal &scaledTitleBarBottomMargin = nextScale ? m_scaledTitleBarBottomMarginNext : m_scaledTitleBarBottomMargin;
    qreal &scaledTitleBarSeparatorHeight = nextScale ? m_scaledTitleBarSeparatorHeightNext : m_scaledTitleBarSeparatorHeight;
    qreal &scaledTitleBarLeftMarginMaximizedHorizontally =
        nextScale ? m_scaledTitleBarLeftMarginMaximizedHorizontallyNext : m_scaledTitleBarLeftMarginMaximizedHorizontally;
    qreal &scaledTitleBarRightMarginMaximizedHorizontally =
        nextScale ? m_scaledTitleBarRightMarginMaximizedHorizontallyNext : m_scaledTitleBarRightMarginMaximizedHorizontally;
    qreal &scaledTitleBarLeftMargin = nextScale ? m_scaledTitleBarLeftMarginNext : m_scaledTitleBarLeftMargin;
    qreal &scaledTitleBarRightMargin = nextScale ? m_scaledTitleBarRightMarginNext : m_scaledTitleBarRightMargin;

    qreal topMargin = m_internalSettings->titleBarTopMargin();
    qreal bottomMargin = m_internalSettings->titleBarBottomMargin();
    if (m_internalSettings->buttonShape() == InternalSettings::EnumButtonShape::IntegratedRoundedRectangle
        || m_internalSettings->buttonShape() == InternalSettings::EnumButtonShape::IntegratedRoundedRectangleGrouped) {
        scaledIntegratedRoundedRectangleBottomPadding =
            KDecoration3::snapToPixelGrid(m_internalSettings->integratedRoundedRectangleBottomPadding() * m_x11Scale, scale);
    } else {
        scaledIntegratedRoundedRectangleBottomPadding = 0;
    }
    if (c->isMaximized()) {
        qreal maximizedScaleFactor = qreal(m_internalSettings->percentMaximizedTopBottomMargins()) / 100;
        topMargin *= maximizedScaleFactor;
        bottomMargin *= maximizedScaleFactor;
    }

    scaledTitleBarTopMargin = KDecoration3::snapToPixelGrid(m_x11Scale * topMargin, scale);
    scaledTitleBarBottomMargin = KDecoration3::snapToPixelGrid(m_x11Scale * bottomMargin, scale);

    scaledTitleBarSeparatorHeight = KDecoration3::snapToPixelGrid(1 * m_x11Scale, scale);

    scaledTitleBarLeftMarginMaximizedHorizontally = KDecoration3::snapToPixelGrid(qreal(m_internalSettings->titleBarLeftMargin()) * qreal(m_x11Scale), scale);
    scaledTitleBarRightMarginMaximizedHorizontally = KDecoration3::snapToPixelGrid(qreal(m_internalSettings->titleBarRightMargin()) * qreal(m_x11Scale), scale);

    // for non-maximized subtract any added borders from the side margin so the user doesn't need to adjust the side margins when changing border size
    // this makes the side margin relative to the border edge rather than the titlebar edge
    qreal &borderSizeLeftRight = nextScale ? m_scaledBorderLeftRightNext : m_scaledBorderLeftRight;
    scaledTitleBarLeftMargin = scaledTitleBarLeftMarginMaximizedHorizontally - borderSizeLeftRight;
    scaledTitleBarRightMargin = scaledTitleBarRightMarginMaximizedHorizontally - borderSizeLeftRight;
}

void Decoration::setScaledButtonDimensions()
{
    qreal scale = window()->scale();

    qreal buttonSpacingLeft;
    qreal buttonSpacingRight;
    if (m_buttonBackgroundType == ButtonBackgroundType::FullHeight) {
        buttonSpacingLeft = m_isRightToLeft ? m_internalSettings->fullHeightButtonSpacingRight() : m_internalSettings->fullHeightButtonSpacingLeft();
        buttonSpacingRight = m_isRightToLeft ? m_internalSettings->fullHeightButtonSpacingLeft() : m_internalSettings->fullHeightButtonSpacingRight();

        m_scaledButtonWidthMarginLeft = KDecoration3::snapToPixelGrid(
            m_x11Scale * (m_isRightToLeft ? m_internalSettings->fullHeightButtonWidthMarginRight() : m_internalSettings->fullHeightButtonWidthMarginLeft()),
            scale);
        m_scaledButtonWidthMarginRight = KDecoration3::snapToPixelGrid(
            m_x11Scale * (m_isRightToLeft ? m_internalSettings->fullHeightButtonWidthMarginLeft() : m_internalSettings->fullHeightButtonWidthMarginRight()),
            scale);
    } else {
        buttonSpacingLeft = m_isRightToLeft ? m_internalSettings->buttonSpacingRight() : m_internalSettings->buttonSpacingLeft();
        buttonSpacingRight = m_isRightToLeft ? m_internalSettings->buttonSpacingLeft() : m_internalSettings->buttonSpacingRight();
    }

    m_scaledButtonSpacingLeft = KDecoration3::snapToPixelGrid(m_x11Scale * buttonSpacingLeft, scale);
    m_scaledButtonSpacingRight = KDecoration3::snapToPixelGrid(m_x11Scale * buttonSpacingRight, scale);
}

void Decoration::updateOpaque()
{
    // access client
    auto c = window();

    if (isOpaqueTitleBar()) { // opaque titlebar colours
        if (c->isMaximized())
            setOpaque(true);
        else
            setOpaque(false);
    } else { // transparent titlebar colours
        setOpaque(false);
    }
}

void Decoration::updateBlur()
{
    // disable blur if the titlebar is opaque
    if (isOpaqueTitleBar()) { // opaque titlebar colours
        setBlurRegion(QRegion());
    } else { // transparent titlebar colours
        if (m_internalSettings->blurTransparentTitleBars()) { // enable blur
            QRectF rectEnlarged = rect().adjusted(-1, -1, 1, 1);
            setBlurRegion(QRegion(rectEnlarged.toRect()));
        } else
            setBlurRegion(QRegion());
    }
}

bool Decoration::isOpaqueTitleBar()
{
    QColor activeTitleBarColor = m_decorationColors->active()->titleBarBase;
    QColor inactiveTitlebarColor = m_decorationColors->inactive()->titleBarBase;

    if (m_internalSettings->opaqueTitleBar()) {
        activeTitleBarColor.setAlpha(255);
        inactiveTitlebarColor.setAlpha(255);
    }

    return ((activeTitleBarColor.alpha() == 255) && (inactiveTitlebarColor.alpha() == 255));
}

qreal Decoration::scaledTitleBarSeparatorHeight(const bool nextScale) const
{
    // access client
    auto c = window();

    if (m_internalSettings->drawTitleBarSeparator() && !c->isShaded() && !m_toolsAreaWillBeDrawn) {
        return nextScale ? m_scaledTitleBarSeparatorHeightNext : m_scaledTitleBarSeparatorHeight;
    } else {
        return 0;
    }
}

void Decoration::updateScale()
{
    setScaledBorderSizes(false);
    setScaledTitleBarMargins(false);
    setScaledIconSizes(false);
    setScaledButtonDimensions();
    updateButtonsGeometry();
}

void Decoration::updateNextScale()
{
    setScaledBorderSizes(true);
    setScaledTitleBarMargins(true);
    setScaledIconSizes(true);
    recalculateBorders();
}

// for unison hovering
void Decoration::setButtonUnisonHovered(bool value)
{
    if (m_buttonUnisonHovered == value) {
        return;
    }
    m_buttonUnisonHovered = value;
    emit buttonUnisonHoveredChanged(value);
}

// for unison hovering
void Decoration::hoverMoveEvent(QHoverEvent *event)
{
    if (m_internalSettings->unisonHovering()) {
        const bool groupContains = m_leftButtons->geometry().contains(event->position()) || m_rightButtons->geometry().contains(event->position());
        setButtonUnisonHovered(groupContains);
    }

    KDecoration3::Decoration::hoverMoveEvent(event);
}

void Decoration::hoverLeaveEvent(QHoverEvent *event)
{
    if (m_internalSettings->unisonHovering()) {
        setButtonUnisonHovered(false);
    }
    KDecoration3::Decoration::hoverLeaveEvent(event);
}

} // namespace

#include "breezedecoration.moc"
