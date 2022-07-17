//////////////////////////////////////////////////////////////////////////////
// breezeconfigurationui.cpp
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
// SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezeconfigwidget.h"
#include "breezeexceptionlist.h"

#include <KLocalizedString>

#include <KColorScheme>
#include <KConfigGroup>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QIcon>
#include <QRegularExpression>

namespace Breeze
{

//_________________________________________________________
ConfigWidget::ConfigWidget(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_configuration(KSharedConfig::openConfig(QStringLiteral("classikrc")))
    , m_changed(false)
{
    // configuration
    m_ui.setupUi(this);

#if CLASSIK_GIT_MASTER
    // set the long version string if from the git master
    m_ui.version->setText("v" + QString(CLASSIK_VERSION) + ".git");

#else
    // set shortened version string in UI if an official release
    QRegularExpression re("\\d+\\.\\d+");
    QRegularExpressionMatch match = re.match(CLASSIK_VERSION);
    if (match.hasMatch()) {
        QString matched = match.captured(0);
        m_ui.version->setText("v" + matched);
    }
#endif

    QIcon useSystemIconThemeIcon = QIcon::fromTheme(QStringLiteral("preferences-desktop-icons"));
    m_ui.buttonIconStyle->addItem(useSystemIconThemeIcon, "Use system icon theme");

    updateIconsStackedWidgetVisible();
    updateBackgroundShapeStackedWidgetVisible();
    updateCustomColorStackedWidgetVisible();

    // track ui changes
    connect(m_ui.titleAlignment, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui.buttonIconStyle, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui.buttonIconStyle, SIGNAL(currentIndexChanged(int)), SLOT(updateIconsStackedWidgetVisible()));
    connect(m_ui.buttonShape, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui.buttonShape, SIGNAL(currentIndexChanged(int)), SLOT(updateBackgroundShapeStackedWidgetVisible()));
    connect(m_ui.scaleBackgroundPercent, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui.fullHeightButtonWidthMarginLeft, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui.fullHeightButtonWidthMarginRight, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui.backgroundColors, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui.alwaysShow, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui.alwaysShowIconHighlightUsing, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui.iconSize, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui.systemIconSize, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui.buttonSpacingRight, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui.buttonSpacingLeft, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui.titleSidePadding, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui.titlebarTopMargin, SIGNAL(valueChanged(double)), SLOT(updateChanged()));
    connect(m_ui.titlebarBottomMargin, SIGNAL(valueChanged(double)), SLOT(updateChanged()));
    connect(m_ui.percentMaximizedTopBottomMargins, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui.titlebarLeftMargin, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui.titlebarRightMargin, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui.cornerRadius, SIGNAL(valueChanged(double)), SLOT(updateChanged()));
    connect(m_ui.activeTitlebarOpacity, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui.inactiveTitlebarOpacity, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui.boldButtonIcons, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui.redAlwaysShownClose, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);
    connect(m_ui.drawBorderOnMaximizedWindows, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);
    connect(m_ui.drawSizeGrip, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);
    connect(m_ui.drawBackgroundGradient, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);
    connect(m_ui.drawTitleBarSeparator, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);
    connect(m_ui.useTitlebarColorForAllBorders, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);
    connect(m_ui.opaqueMaximizedTitlebars, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);
    connect(m_ui.blurTransparentTitlebars, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);
    connect(m_ui.applyOpacityToHeader, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);
    connect(m_ui.translucentButtonBackgrounds, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);
    connect(m_ui.colorizeSystemIcons, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);
    connect(m_ui.lockTitleBarTopBottomMargins, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);
    connect(m_ui.lockTitleBarLeftRightMargins, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);
    connect(m_ui.lockButtonSpacingLeftRight, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);
    connect(m_ui.lockFullHeightButtonWidthMargins, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);

    // connect dual controls with same values
    connect(m_ui.fullHeightButtonWidthMarginLeft, SIGNAL(valueChanged(int)), SLOT(fullHeightButtonWidthMarginLeftChanged()));
    connect(m_ui.fullHeightButtonWidthMarginRight, SIGNAL(valueChanged(int)), SLOT(fullHeightButtonWidthMarginRightChanged()));
    connect(m_ui.titlebarTopMargin, SIGNAL(valueChanged(double)), SLOT(titlebarTopMarginChanged()));
    connect(m_ui.titlebarBottomMargin, SIGNAL(valueChanged(double)), SLOT(titlebarBottomMarginChanged()));
    connect(m_ui.titlebarLeftMargin, SIGNAL(valueChanged(int)), SLOT(titlebarLeftMarginChanged()));
    connect(m_ui.titlebarRightMargin, SIGNAL(valueChanged(int)), SLOT(titlebarRightMarginChanged()));
    connect(m_ui.buttonSpacingLeft, SIGNAL(valueChanged(int)), SLOT(buttonSpacingLeftChanged()));
    connect(m_ui.buttonSpacingRight, SIGNAL(valueChanged(int)), SLOT(buttonSpacingRightChanged()));
    connect(m_ui.titleSidePadding, SIGNAL(valueChanged(int)), m_ui.titleSidePadding_2, SLOT(setValue(int)));
    connect(m_ui.titleSidePadding_2, SIGNAL(valueChanged(int)), m_ui.titleSidePadding, SLOT(setValue(int)));
    connect(m_ui.cornerRadius, SIGNAL(valueChanged(double)), m_ui.cornerRadius_2, SLOT(setValue(double)));
    connect(m_ui.cornerRadius, SIGNAL(valueChanged(double)), m_ui.cornerRadius_3, SLOT(setValue(double)));
    connect(m_ui.cornerRadius, SIGNAL(valueChanged(double)), m_ui.cornerRadius_4, SLOT(setValue(double)));
    connect(m_ui.cornerRadius_2, SIGNAL(valueChanged(double)), m_ui.cornerRadius, SLOT(setValue(double)));
    connect(m_ui.cornerRadius_2, SIGNAL(valueChanged(double)), m_ui.cornerRadius_3, SLOT(setValue(double)));
    connect(m_ui.cornerRadius_2, SIGNAL(valueChanged(double)), m_ui.cornerRadius_4, SLOT(setValue(double)));
    connect(m_ui.cornerRadius_3, SIGNAL(valueChanged(double)), m_ui.cornerRadius, SLOT(setValue(double)));
    connect(m_ui.cornerRadius_3, SIGNAL(valueChanged(double)), m_ui.cornerRadius_2, SLOT(setValue(double)));
    connect(m_ui.cornerRadius_3, SIGNAL(valueChanged(double)), m_ui.cornerRadius_4, SLOT(setValue(double)));
    connect(m_ui.cornerRadius_4, SIGNAL(valueChanged(double)), m_ui.cornerRadius, SLOT(setValue(double)));
    connect(m_ui.cornerRadius_4, SIGNAL(valueChanged(double)), m_ui.cornerRadius_2, SLOT(setValue(double)));
    connect(m_ui.cornerRadius_4, SIGNAL(valueChanged(double)), m_ui.cornerRadius_3, SLOT(setValue(double)));
    connect(m_ui.activeTitlebarOpacity, SIGNAL(valueChanged(int)), m_ui.activeTitlebarOpacity_2, SLOT(setValue(int)));
    connect(m_ui.activeTitlebarOpacity_2, SIGNAL(valueChanged(int)), m_ui.activeTitlebarOpacity, SLOT(setValue(int)));
    connect(m_ui.inactiveTitlebarOpacity, SIGNAL(valueChanged(int)), m_ui.inactiveTitlebarOpacity_2, SLOT(setValue(int)));
    connect(m_ui.inactiveTitlebarOpacity_2, SIGNAL(valueChanged(int)), m_ui.inactiveTitlebarOpacity, SLOT(setValue(int)));

    // only enable animationsSpeed when animationsEnabled is checked
    connect(m_ui.animationsEnabled, &QAbstractButton::toggled, this, &ConfigWidget::setEnabledAnimationsSpeed);

    // track animations changes
    connect(m_ui.animationsEnabled, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);
    connect(m_ui.animationsSpeedRelativeSystem, SIGNAL(valueChanged(int)), SLOT(updateChanged()));

    // only enable transparency options when transparency is setActiveTitlebarOpacity
    connect(m_ui.activeTitlebarOpacity, SIGNAL(valueChanged(int)), SLOT(setEnabledTransparentTitlebarOptions()));
    connect(m_ui.inactiveTitlebarOpacity, SIGNAL(valueChanged(int)), SLOT(setEnabledTransparentTitlebarOptions()));

    // track shadows changes
    connect(m_ui.shadowSize, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui.shadowStrength, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui.shadowColor, &KColorButton::changed, this, &ConfigWidget::updateChanged);
    connect(m_ui.thinWindowOutlineStyle, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui.thinWindowOutlineStyle, SIGNAL(currentIndexChanged(int)), SLOT(updateCustomColorStackedWidgetVisible()));
    connect(m_ui.thinWindowOutlineCustomColor, &KColorButton::changed, this, &ConfigWidget::updateChanged);
    connect(m_ui.thinWindowOutlineThickness, SIGNAL(valueChanged(double)), SLOT(updateChanged()));
    connect(m_ui.colorizeThinWindowOutlineWithButton, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);

    // track exception changes
    connect(m_ui.exceptions, &ExceptionListWidget::changed, this, &ConfigWidget::updateChanged);
}

//_________________________________________________________
void ConfigWidget::load()
{
    m_loading = true;
    getTitlebarOpacityFromColorScheme();

    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // assign to ui
    m_ui.titleAlignment->setCurrentIndex(m_internalSettings->titleAlignment());
    m_ui.buttonIconStyle->setCurrentIndex(m_internalSettings->buttonIconStyle());
    m_ui.buttonShape->setCurrentIndex(m_internalSettings->buttonShape());
    m_ui.scaleBackgroundPercent->setValue(m_internalSettings->scaleBackgroundPercent());
    m_ui.fullHeightButtonWidthMarginLeft->setValue(m_internalSettings->fullHeightButtonWidthMarginLeft());
    m_ui.fullHeightButtonWidthMarginRight->setValue(m_internalSettings->fullHeightButtonWidthMarginRight());
    m_ui.backgroundColors->setCurrentIndex(m_internalSettings->backgroundColors());
    m_ui.alwaysShow->setCurrentIndex(m_internalSettings->alwaysShow());
    m_ui.alwaysShowIconHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowIconHighlightUsing());
    m_ui.iconSize->setCurrentIndex(m_internalSettings->iconSize());
    m_ui.systemIconSize->setCurrentIndex(m_internalSettings->systemIconSize());
    m_ui.buttonSpacingRight->setValue(m_internalSettings->buttonSpacingRight());
    m_ui.buttonSpacingLeft->setValue(m_internalSettings->buttonSpacingLeft());
    m_ui.titleSidePadding->setValue(m_internalSettings->titleSidePadding());
    m_ui.titlebarTopMargin->setValue(m_internalSettings->titlebarTopMargin());
    m_ui.titlebarBottomMargin->setValue(m_internalSettings->titlebarBottomMargin());
    m_ui.percentMaximizedTopBottomMargins->setValue(m_internalSettings->percentMaximizedTopBottomMargins());
    m_ui.titlebarLeftMargin->setValue(m_internalSettings->titlebarLeftMargin());
    m_ui.titlebarRightMargin->setValue(m_internalSettings->titlebarRightMargin());
    m_ui.cornerRadius->setValue(m_internalSettings->cornerRadius());

    // if there is a non-opaque colour set in the system colour scheme then this overrides the control here and disables it
    if (m_translucentActiveSchemeColor) {
        m_ui.activeTitlebarOpacity->setValue(m_activeSchemeColorAlpha * 100);
        m_ui.activeTitlebarOpacity->setEnabled(false);
        m_ui.activeTitlebarOpacity_2->setEnabled(false);
    } else
        m_ui.activeTitlebarOpacity->setValue(m_internalSettings->activeTitlebarOpacity());
    m_ui.activeTitlebarOpacity_2->setValue(m_ui.activeTitlebarOpacity->value());
    if (m_translucentInactiveSchemeColor) {
        m_ui.inactiveTitlebarOpacity->setValue(m_inactiveSchemeColorAlpha * 100);
        m_ui.inactiveTitlebarOpacity->setEnabled(false);
        m_ui.inactiveTitlebarOpacity_2->setEnabled(false);
    } else
        m_ui.inactiveTitlebarOpacity->setValue(m_internalSettings->inactiveTitlebarOpacity());
    m_ui.inactiveTitlebarOpacity_2->setValue(m_ui.inactiveTitlebarOpacity->value());
    setEnabledTransparentTitlebarOptions();

    m_ui.drawBorderOnMaximizedWindows->setChecked(m_internalSettings->drawBorderOnMaximizedWindows());
    m_ui.boldButtonIcons->setCurrentIndex(m_internalSettings->boldButtonIcons());
    m_ui.redAlwaysShownClose->setChecked(m_internalSettings->redAlwaysShownClose());
    m_ui.drawSizeGrip->setChecked(m_internalSettings->drawSizeGrip());
    m_ui.drawBackgroundGradient->setChecked(m_internalSettings->drawBackgroundGradient());
    m_ui.drawTitleBarSeparator->setChecked(m_internalSettings->drawTitleBarSeparator());
    m_ui.animationsEnabled->setChecked(m_internalSettings->animationsEnabled());
    m_ui.animationsSpeedRelativeSystem->setValue(m_internalSettings->animationsSpeedRelativeSystem());
    m_ui.useTitlebarColorForAllBorders->setChecked(m_internalSettings->useTitlebarColorForAllBorders());
    m_ui.opaqueMaximizedTitlebars->setChecked(m_internalSettings->opaqueMaximizedTitlebars());
    m_ui.blurTransparentTitlebars->setChecked(m_internalSettings->blurTransparentTitlebars());
    m_ui.applyOpacityToHeader->setChecked(m_internalSettings->applyOpacityToHeader());
    m_ui.translucentButtonBackgrounds->setChecked(m_internalSettings->translucentButtonBackgrounds());
    m_ui.colorizeSystemIcons->setChecked(m_internalSettings->colorizeSystemIcons());
    m_ui.lockTitleBarTopBottomMargins->setChecked(m_internalSettings->lockTitleBarTopBottomMargins());
    m_ui.lockTitleBarLeftRightMargins->setChecked(m_internalSettings->lockTitleBarLeftRightMargins());
    m_ui.lockButtonSpacingLeftRight->setChecked(m_internalSettings->lockButtonSpacingLeftRight());
    m_ui.lockFullHeightButtonWidthMargins->setChecked(m_internalSettings->lockFullHeightButtonWidthMargins());

    // load shadows
    if (m_internalSettings->shadowSize() <= InternalSettings::ShadowVeryLarge)
        m_ui.shadowSize->setCurrentIndex(m_internalSettings->shadowSize());
    else
        m_ui.shadowSize->setCurrentIndex(InternalSettings::ShadowLarge);

    m_ui.shadowStrength->setValue(qRound(qreal(m_internalSettings->shadowStrength() * 100) / 255));
    m_ui.shadowColor->setColor(m_internalSettings->shadowColor());
    m_ui.thinWindowOutlineStyle->setCurrentIndex(m_internalSettings->thinWindowOutlineStyle());
    m_ui.thinWindowOutlineCustomColor->setColor(m_internalSettings->thinWindowOutlineCustomColor());
    m_ui.thinWindowOutlineThickness->setValue(m_internalSettings->thinWindowOutlineThickness());
    m_ui.colorizeThinWindowOutlineWithButton->setChecked(m_internalSettings->colorizeThinWindowOutlineWithButton());

    updateIconsStackedWidgetVisible();
    updateBackgroundShapeStackedWidgetVisible();
    updateCustomColorStackedWidgetVisible();
    // load exceptions
    ExceptionList exceptions;
    exceptions.readConfig(m_configuration);
    m_ui.exceptions->setExceptions(exceptions.get());
    setChanged(false);
    m_loading = false;
}

//_________________________________________________________
void ConfigWidget::save()
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    m_internalSettings->setTitleAlignment(m_ui.titleAlignment->currentIndex());
    m_internalSettings->setButtonIconStyle(m_ui.buttonIconStyle->currentIndex());
    m_internalSettings->setButtonShape(m_ui.buttonShape->currentIndex());
    m_internalSettings->setScaleBackgroundPercent(m_ui.scaleBackgroundPercent->value());
    m_internalSettings->setFullHeightButtonWidthMarginLeft(m_ui.fullHeightButtonWidthMarginLeft->value());
    m_internalSettings->setFullHeightButtonWidthMarginRight(m_ui.fullHeightButtonWidthMarginRight->value());
    m_internalSettings->setBackgroundColors(m_ui.backgroundColors->currentIndex());
    m_internalSettings->setAlwaysShow(m_ui.alwaysShow->currentIndex());
    m_internalSettings->setAlwaysShowIconHighlightUsing(m_ui.alwaysShowIconHighlightUsing->currentIndex());
    m_internalSettings->setIconSize(m_ui.iconSize->currentIndex());
    m_internalSettings->setSystemIconSize(m_ui.systemIconSize->currentIndex());
    m_internalSettings->setButtonSpacingRight(m_ui.buttonSpacingRight->value());
    m_internalSettings->setButtonSpacingLeft(m_ui.buttonSpacingLeft->value());
    m_internalSettings->setTitleSidePadding(m_ui.titleSidePadding->value());
    m_internalSettings->setTitlebarTopMargin(m_ui.titlebarTopMargin->value());
    m_internalSettings->setTitlebarBottomMargin(m_ui.titlebarBottomMargin->value());
    m_internalSettings->setPercentMaximizedTopBottomMargins(m_ui.percentMaximizedTopBottomMargins->value());
    m_internalSettings->setTitlebarLeftMargin(m_ui.titlebarLeftMargin->value());
    m_internalSettings->setTitlebarRightMargin(m_ui.titlebarRightMargin->value());
    m_internalSettings->setCornerRadius(m_ui.cornerRadius->value());
    if (!m_translucentActiveSchemeColor || m_defaultsPressed)
        m_internalSettings->setActiveTitlebarOpacity(m_ui.activeTitlebarOpacity->value());
    if (!m_translucentInactiveSchemeColor || m_defaultsPressed)
        m_internalSettings->setInactiveTitlebarOpacity(m_ui.inactiveTitlebarOpacity->value());
    m_internalSettings->setBoldButtonIcons(m_ui.boldButtonIcons->currentIndex());
    m_internalSettings->setRedAlwaysShownClose(m_ui.redAlwaysShownClose->isChecked());
    m_internalSettings->setDrawBorderOnMaximizedWindows(m_ui.drawBorderOnMaximizedWindows->isChecked());
    m_internalSettings->setDrawSizeGrip(m_ui.drawSizeGrip->isChecked());
    m_internalSettings->setDrawBackgroundGradient(m_ui.drawBackgroundGradient->isChecked());
    m_internalSettings->setDrawTitleBarSeparator(m_ui.drawTitleBarSeparator->isChecked());
    m_internalSettings->setAnimationsEnabled(m_ui.animationsEnabled->isChecked());
    m_internalSettings->setAnimationsSpeedRelativeSystem(m_ui.animationsSpeedRelativeSystem->value());
    m_internalSettings->setUseTitlebarColorForAllBorders(m_ui.useTitlebarColorForAllBorders->isChecked());
    m_internalSettings->setOpaqueMaximizedTitlebars(m_ui.opaqueMaximizedTitlebars->isChecked());
    m_internalSettings->setBlurTransparentTitlebars(m_ui.blurTransparentTitlebars->isChecked());
    m_internalSettings->setApplyOpacityToHeader(m_ui.applyOpacityToHeader->isChecked());
    m_internalSettings->setTranslucentButtonBackgrounds(m_ui.translucentButtonBackgrounds->isChecked());
    m_internalSettings->setColorizeSystemIcons(m_ui.colorizeSystemIcons->isChecked());
    m_internalSettings->setLockTitleBarTopBottomMargins(m_ui.lockTitleBarTopBottomMargins->isChecked());
    m_internalSettings->setLockTitleBarLeftRightMargins(m_ui.lockTitleBarLeftRightMargins->isChecked());
    m_internalSettings->setLockButtonSpacingLeftRight(m_ui.lockButtonSpacingLeftRight->isChecked());
    m_internalSettings->setLockFullHeightButtonWidthMargins(m_ui.lockFullHeightButtonWidthMargins->isChecked());

    m_internalSettings->setShadowSize(m_ui.shadowSize->currentIndex());
    m_internalSettings->setShadowStrength(qRound(qreal(m_ui.shadowStrength->value() * 255) / 100));
    m_internalSettings->setShadowColor(m_ui.shadowColor->color());
    m_internalSettings->setThinWindowOutlineStyle(m_ui.thinWindowOutlineStyle->currentIndex());
    m_internalSettings->setThinWindowOutlineCustomColor(m_ui.thinWindowOutlineCustomColor->color());
    m_internalSettings->setThinWindowOutlineThickness(m_ui.thinWindowOutlineThickness->value());
    m_internalSettings->setColorizeThinWindowOutlineWithButton(m_ui.colorizeThinWindowOutlineWithButton->isChecked());

    // save configuration
    m_internalSettings->save();

    // get list of exceptions and write
    InternalSettingsList exceptions(m_ui.exceptions->exceptions());
    ExceptionList(exceptions).writeConfig(m_configuration);

    // sync configuration
    m_configuration->sync();
    setChanged(false);

    // needed to tell kwin to reload when running from external kcmshell
    {
        QDBusMessage message = QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
        QDBusConnection::sessionBus().send(message);
    }

    // needed for breeze style to reload shadows
    {
        QDBusMessage message(QDBusMessage::createSignal("/ClassikDecoration", "org.kde.Classik.Style", "reparseConfiguration"));
        QDBusConnection::sessionBus().send(message);
    }
}

//_________________________________________________________
void ConfigWidget::defaults()
{
    m_processingDefaults = true;
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->setDefaults();

    // assign to ui
    m_ui.titleAlignment->setCurrentIndex(m_internalSettings->titleAlignment());
    m_ui.buttonIconStyle->setCurrentIndex(m_internalSettings->buttonIconStyle());
    m_ui.buttonShape->setCurrentIndex(m_internalSettings->buttonShape());
    m_ui.scaleBackgroundPercent->setValue(m_internalSettings->scaleBackgroundPercent());
    m_ui.fullHeightButtonWidthMarginLeft->setValue(m_internalSettings->fullHeightButtonWidthMarginLeft());
    m_ui.fullHeightButtonWidthMarginRight->setValue(m_internalSettings->fullHeightButtonWidthMarginRight());
    m_ui.backgroundColors->setCurrentIndex(m_internalSettings->backgroundColors());
    m_ui.alwaysShow->setCurrentIndex(m_internalSettings->alwaysShow());
    m_ui.alwaysShowIconHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowIconHighlightUsing());
    m_ui.iconSize->setCurrentIndex(m_internalSettings->iconSize());
    m_ui.systemIconSize->setCurrentIndex(m_internalSettings->systemIconSize());
    m_ui.buttonSpacingRight->setValue(m_internalSettings->buttonSpacingRight());
    m_ui.buttonSpacingLeft->setValue(m_internalSettings->buttonSpacingLeft());
    m_ui.titleSidePadding->setValue(m_internalSettings->titleSidePadding());
    m_ui.titlebarTopMargin->setValue(m_internalSettings->titlebarTopMargin());
    m_ui.titlebarBottomMargin->setValue(m_internalSettings->titlebarBottomMargin());
    m_ui.percentMaximizedTopBottomMargins->setValue(m_internalSettings->percentMaximizedTopBottomMargins());
    m_ui.titlebarLeftMargin->setValue(m_internalSettings->titlebarLeftMargin());
    m_ui.titlebarRightMargin->setValue(m_internalSettings->titlebarRightMargin());
    m_ui.cornerRadius->setValue(m_internalSettings->cornerRadius());
    m_ui.activeTitlebarOpacity->setValue(m_internalSettings->activeTitlebarOpacity());
    m_ui.inactiveTitlebarOpacity->setValue(m_internalSettings->inactiveTitlebarOpacity());
    setEnabledTransparentTitlebarOptions();
    m_ui.boldButtonIcons->setCurrentIndex(m_internalSettings->boldButtonIcons());
    m_ui.redAlwaysShownClose->setChecked(m_internalSettings->redAlwaysShownClose());
    m_ui.drawBorderOnMaximizedWindows->setChecked(m_internalSettings->drawBorderOnMaximizedWindows());
    m_ui.drawSizeGrip->setChecked(m_internalSettings->drawSizeGrip());
    m_ui.drawBackgroundGradient->setChecked(m_internalSettings->drawBackgroundGradient());
    m_ui.animationsEnabled->setChecked(m_internalSettings->animationsEnabled());
    m_ui.animationsSpeedRelativeSystem->setValue(m_internalSettings->animationsSpeedRelativeSystem());
    m_ui.drawTitleBarSeparator->setChecked(m_internalSettings->drawTitleBarSeparator());
    m_ui.useTitlebarColorForAllBorders->setChecked(m_internalSettings->useTitlebarColorForAllBorders());
    m_ui.opaqueMaximizedTitlebars->setChecked(m_internalSettings->opaqueMaximizedTitlebars());
    m_ui.blurTransparentTitlebars->setChecked(m_internalSettings->blurTransparentTitlebars());
    m_ui.applyOpacityToHeader->setChecked(m_internalSettings->applyOpacityToHeader());
    m_ui.translucentButtonBackgrounds->setChecked(m_internalSettings->translucentButtonBackgrounds());
    m_ui.colorizeSystemIcons->setChecked(m_internalSettings->colorizeSystemIcons());
    m_ui.lockTitleBarTopBottomMargins->setChecked(m_internalSettings->lockTitleBarTopBottomMargins());
    m_ui.lockTitleBarLeftRightMargins->setChecked(m_internalSettings->lockTitleBarLeftRightMargins());
    m_ui.lockButtonSpacingLeftRight->setChecked(m_internalSettings->lockButtonSpacingLeftRight());
    m_ui.lockFullHeightButtonWidthMargins->setChecked(m_internalSettings->lockFullHeightButtonWidthMargins());

    m_ui.shadowSize->setCurrentIndex(m_internalSettings->shadowSize());
    m_ui.shadowStrength->setValue(qRound(qreal(m_internalSettings->shadowStrength() * 100) / 255));
    m_ui.shadowColor->setColor(m_internalSettings->shadowColor());
    m_ui.thinWindowOutlineStyle->setCurrentIndex(m_internalSettings->thinWindowOutlineStyle());
    m_ui.thinWindowOutlineCustomColor->setColor(m_internalSettings->thinWindowOutlineCustomColor());
    m_ui.thinWindowOutlineThickness->setValue(m_internalSettings->thinWindowOutlineThickness());
    m_ui.colorizeThinWindowOutlineWithButton->setChecked(m_internalSettings->colorizeThinWindowOutlineWithButton());

    updateIconsStackedWidgetVisible();
    updateBackgroundShapeStackedWidgetVisible();
    updateCustomColorStackedWidgetVisible();

    m_processingDefaults = false;
    m_defaultsPressed = true;
}

//_______________________________________________
void ConfigWidget::updateChanged()
{
    // check configuration
    if (!m_internalSettings)
        return;

    // track modifications
    bool modified(false);

    if (m_ui.drawTitleBarSeparator->isChecked() != m_internalSettings->drawTitleBarSeparator())
        modified = true;
    else if (m_ui.useTitlebarColorForAllBorders->isChecked() != m_internalSettings->useTitlebarColorForAllBorders())
        modified = true;
    else if (m_ui.opaqueMaximizedTitlebars->isChecked() != m_internalSettings->opaqueMaximizedTitlebars())
        modified = true;
    else if (m_ui.blurTransparentTitlebars->isChecked() != m_internalSettings->blurTransparentTitlebars())
        modified = true;
    else if (m_ui.applyOpacityToHeader->isChecked() != m_internalSettings->applyOpacityToHeader())
        modified = true;
    else if (m_ui.translucentButtonBackgrounds->isChecked() != m_internalSettings->translucentButtonBackgrounds())
        modified = true;
    else if (m_ui.colorizeSystemIcons->isChecked() != m_internalSettings->colorizeSystemIcons())
        modified = true;
    else if (m_ui.lockTitleBarTopBottomMargins->isChecked() != m_internalSettings->lockTitleBarTopBottomMargins())
        modified = true;
    else if (m_ui.lockTitleBarLeftRightMargins->isChecked() != m_internalSettings->lockTitleBarLeftRightMargins())
        modified = true;
    else if (m_ui.lockButtonSpacingLeftRight->isChecked() != m_internalSettings->lockButtonSpacingLeftRight())
        modified = true;
    else if (m_ui.lockFullHeightButtonWidthMargins->isChecked() != m_internalSettings->lockFullHeightButtonWidthMargins())
        modified = true;
    else if (m_ui.titleAlignment->currentIndex() != m_internalSettings->titleAlignment())
        modified = true;
    else if (m_ui.buttonIconStyle->currentIndex() != m_internalSettings->buttonIconStyle())
        modified = true;
    else if (m_ui.buttonShape->currentIndex() != m_internalSettings->buttonShape())
        modified = true;
    else if (m_ui.scaleBackgroundPercent->value() != m_internalSettings->scaleBackgroundPercent())
        modified = true;
    else if (m_ui.fullHeightButtonWidthMarginLeft->value() != m_internalSettings->fullHeightButtonWidthMarginLeft())
        modified = true;
    else if (m_ui.fullHeightButtonWidthMarginRight->value() != m_internalSettings->fullHeightButtonWidthMarginRight())
        modified = true;
    else if (m_ui.backgroundColors->currentIndex() != m_internalSettings->backgroundColors())
        modified = true;
    else if (m_ui.alwaysShow->currentIndex() != m_internalSettings->alwaysShow())
        modified = true;
    else if (m_ui.alwaysShowIconHighlightUsing->currentIndex() != m_internalSettings->alwaysShowIconHighlightUsing())
        modified = true;
    else if (m_ui.iconSize->currentIndex() != m_internalSettings->iconSize())
        modified = true;
    else if (m_ui.systemIconSize->currentIndex() != m_internalSettings->systemIconSize())
        modified = true;
    else if (m_ui.boldButtonIcons->currentIndex() != m_internalSettings->boldButtonIcons())
        modified = true;
    else if (m_ui.redAlwaysShownClose->isChecked() != m_internalSettings->redAlwaysShownClose())
        modified = true;
    else if (m_ui.drawBorderOnMaximizedWindows->isChecked() != m_internalSettings->drawBorderOnMaximizedWindows())
        modified = true;
    else if (m_ui.drawSizeGrip->isChecked() != m_internalSettings->drawSizeGrip())
        modified = true;
    else if (m_ui.drawBackgroundGradient->isChecked() != m_internalSettings->drawBackgroundGradient())
        modified = true;
    else if (m_ui.buttonSpacingRight->value() != m_internalSettings->buttonSpacingRight())
        modified = true;
    else if (m_ui.buttonSpacingLeft->value() != m_internalSettings->buttonSpacingLeft())
        modified = true;
    else if (m_ui.titleSidePadding->value() != m_internalSettings->titleSidePadding())
        modified = true;
    else if (m_ui.titlebarTopMargin->value() != m_internalSettings->titlebarTopMargin())
        modified = true;
    else if (m_ui.titlebarBottomMargin->value() != m_internalSettings->titlebarBottomMargin())
        modified = true;
    else if (m_ui.percentMaximizedTopBottomMargins->value() != m_internalSettings->percentMaximizedTopBottomMargins())
        modified = true;
    else if (m_ui.titlebarLeftMargin->value() != m_internalSettings->titlebarLeftMargin())
        modified = true;
    else if (m_ui.titlebarRightMargin->value() != m_internalSettings->titlebarRightMargin())
        modified = true;
    else if (m_ui.cornerRadius->value() != m_internalSettings->cornerRadius())
        modified = true;
    else if ((!m_translucentActiveSchemeColor) && (m_ui.activeTitlebarOpacity->value() != m_internalSettings->activeTitlebarOpacity()))
        modified = true;
    else if ((!m_translucentInactiveSchemeColor) && (m_ui.inactiveTitlebarOpacity->value() != m_internalSettings->inactiveTitlebarOpacity()))
        modified = true;

    // animations
    else if (m_ui.animationsEnabled->isChecked() != m_internalSettings->animationsEnabled())
        modified = true;
    else if (m_ui.animationsSpeedRelativeSystem->value() != m_internalSettings->animationsSpeedRelativeSystem())
        modified = true;

    // shadows
    else if (m_ui.shadowSize->currentIndex() != m_internalSettings->shadowSize())
        modified = true;
    else if (qRound(qreal(m_ui.shadowStrength->value() * 255) / 100) != m_internalSettings->shadowStrength())
        modified = true;
    else if (m_ui.shadowColor->color() != m_internalSettings->shadowColor())
        modified = true;
    else if (m_ui.thinWindowOutlineStyle->currentIndex() != m_internalSettings->thinWindowOutlineStyle())
        modified = true;
    else if (m_ui.thinWindowOutlineCustomColor->color() != m_internalSettings->thinWindowOutlineCustomColor())
        modified = true;
    else if (m_ui.thinWindowOutlineThickness->value() != m_internalSettings->thinWindowOutlineThickness())
        modified = true;
    else if (m_ui.colorizeThinWindowOutlineWithButton->isChecked() != m_internalSettings->colorizeThinWindowOutlineWithButton())
        modified = true;

    // exceptions
    else if (m_ui.exceptions->isChanged())
        modified = true;

    setChanged(modified);
}

//_______________________________________________
void ConfigWidget::setChanged(bool value)
{
    emit changed(value);
}

// only enable animationsSpeedRelativeSystem and animationsSpeedLabelx when animationsEnabled is checked
void ConfigWidget::setEnabledAnimationsSpeed()
{
    m_ui.animationsSpeedRelativeSystem->setEnabled(m_ui.animationsEnabled->isChecked());
    m_ui.animationsSpeedLabel1->setEnabled(m_ui.animationsEnabled->isChecked());
    m_ui.animationsSpeedLabel2->setEnabled(m_ui.animationsEnabled->isChecked());
    m_ui.animationsSpeedLabel4->setEnabled(m_ui.animationsEnabled->isChecked());
}

// only enable blurTransparentTitlebars and opaqueMaximizedTitlebars options if transparent titlebars are enabled
void ConfigWidget::setEnabledTransparentTitlebarOptions()
{
    if (m_ui.activeTitlebarOpacity->value() != 100 || m_ui.inactiveTitlebarOpacity->value() != 100) {
        m_ui.opaqueMaximizedTitlebars->setEnabled(true);
        m_ui.blurTransparentTitlebars->setEnabled(true);
        m_ui.applyOpacityToHeader->setEnabled(true);
    } else {
        m_ui.opaqueMaximizedTitlebars->setEnabled(false);
        m_ui.blurTransparentTitlebars->setEnabled(false);
        m_ui.applyOpacityToHeader->setEnabled(false);
    }
}

void ConfigWidget::updateIconsStackedWidgetVisible()
{
    if (m_ui.buttonIconStyle->currentIndex() == InternalSettings::EnumButtonIconStyle::StyleSystemIconTheme)
        m_ui.iconsStackedWidget->setCurrentIndex(1);
    else
        m_ui.iconsStackedWidget->setCurrentIndex(0);
}

void ConfigWidget::updateBackgroundShapeStackedWidgetVisible()
{
    if (m_ui.buttonShape->currentIndex() == InternalSettings::EnumButtonShape::ShapeFullHeightRectangle
        || m_ui.buttonShape->currentIndex() == InternalSettings::EnumButtonShape::ShapeFullHeightRoundedRectangle
        || m_ui.buttonShape->currentIndex() == InternalSettings::EnumButtonShape::ShapeFullHeightIntegratedRoundedRectangle)
        m_ui.backgroundShapeStackedWidget->setCurrentIndex(1);
    else
        m_ui.backgroundShapeStackedWidget->setCurrentIndex(0);
}

void ConfigWidget::updateCustomColorStackedWidgetVisible()
{
    if (m_ui.thinWindowOutlineStyle->currentIndex() == InternalSettings::EnumThinWindowOutlineStyle::WindowOutlineCustomColor
        || m_ui.thinWindowOutlineStyle->currentIndex() == InternalSettings::EnumThinWindowOutlineStyle::WindowOutlineCustomWithContrast)
        m_ui.customColorStackedWidget->setCurrentIndex(1);
    else
        m_ui.customColorStackedWidget->setCurrentIndex(0);
}

void ConfigWidget::fullHeightButtonWidthMarginLeftChanged()
{
    if (m_ui.lockFullHeightButtonWidthMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.fullHeightButtonWidthMarginRight->setValue(m_ui.fullHeightButtonWidthMarginLeft->value());
}

void ConfigWidget::fullHeightButtonWidthMarginRightChanged()
{
    if (m_ui.lockFullHeightButtonWidthMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.fullHeightButtonWidthMarginLeft->setValue(m_ui.fullHeightButtonWidthMarginRight->value());
}

void ConfigWidget::titlebarTopMarginChanged()
{
    if (m_ui.lockTitleBarTopBottomMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.titlebarBottomMargin->setValue(m_ui.titlebarTopMargin->value());
}

void ConfigWidget::titlebarBottomMarginChanged()
{
    if (m_ui.lockTitleBarTopBottomMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.titlebarTopMargin->setValue(m_ui.titlebarBottomMargin->value());
}

void ConfigWidget::titlebarLeftMarginChanged()
{
    if (m_ui.lockTitleBarLeftRightMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.titlebarRightMargin->setValue(m_ui.titlebarLeftMargin->value());
}

void ConfigWidget::titlebarRightMarginChanged()
{
    if (m_ui.lockTitleBarLeftRightMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.titlebarLeftMargin->setValue(m_ui.titlebarRightMargin->value());
}

void ConfigWidget::buttonSpacingLeftChanged()
{
    if (m_ui.lockButtonSpacingLeftRight->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.buttonSpacingRight->setValue(m_ui.buttonSpacingLeft->value());
}

void ConfigWidget::buttonSpacingRightChanged()
{
    if (m_ui.lockButtonSpacingLeftRight->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.buttonSpacingLeft->setValue(m_ui.buttonSpacingRight->value());
}

void ConfigWidget::getTitlebarOpacityFromColorScheme()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    bool colorSchemeHasHeaderColor = KColorScheme::isColorSetSupported(config, KColorScheme::Header);

    // get the alpha values from the system colour scheme
    if (colorSchemeHasHeaderColor) {
        KColorScheme activeHeader = KColorScheme(QPalette::Active, KColorScheme::Header, config);
        m_translucentActiveSchemeColor = !activeHeader.background().isOpaque();
        m_activeSchemeColorAlpha = activeHeader.background().color().alphaF();

        KColorScheme inactiveHeader = KColorScheme(QPalette::Inactive, KColorScheme::Header, config);
        m_translucentInactiveSchemeColor = !inactiveHeader.background().isOpaque();
        m_inactiveSchemeColorAlpha = inactiveHeader.background().color().alphaF();
    } else {
        KConfigGroup wmConfig(config, QStringLiteral("WM"));
        if (wmConfig.exists()) {
            QColor activeTitlebarColor;
            QColor inactiveTitlebarColor;
            activeTitlebarColor = wmConfig.readEntry("activeBackground", QColorConstants::Black);
            inactiveTitlebarColor = wmConfig.readEntry("inactiveBackground", QColorConstants::Black);

            m_translucentActiveSchemeColor = (activeTitlebarColor.alpha() != 255);
            m_translucentInactiveSchemeColor = (inactiveTitlebarColor.alpha() != 255);
            m_activeSchemeColorAlpha = activeTitlebarColor.alphaF();
            m_inactiveSchemeColorAlpha = inactiveTitlebarColor.alphaF();
        }
    }
}
}
