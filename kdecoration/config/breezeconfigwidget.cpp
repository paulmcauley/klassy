//////////////////////////////////////////////////////////////////////////////
// breezeconfigwidget.cpp
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
// SPDX-FileCopyrightText: 2021-2024 Paul A McAuley <kde@paulmcauley.com>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezeconfigwidget.h"
#include "dbusmessages.h"
#include "decorationexceptionlist.h"
#include "presetsmodel.h"

#include <KLocalizedString>

#include <QIcon>
#include <QRegularExpression>
#include <QStackedLayout>
#include <QStackedWidget>
#include <QWindow>

void initKlassydecorationConfigQrc()
{
    // needed to display images when qrc is statically linked
    // must be in global namespace to work
    Q_INIT_RESOURCE(klassydecoration_config);
}

void cleanupKlassydecorationConfigQrc()
{
    // needed to free qrc resources
    // must be in global namespace to work
    Q_CLEANUP_RESOURCE(klassydecoration_config);
}

namespace Breeze
{

//_________________________________________________________
ConfigWidget::ConfigWidget(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_configuration(KSharedConfig::openConfig(QStringLiteral("klassy/klassyrc")))
    , m_presetsConfiguration(KSharedConfig::openConfig(QStringLiteral("klassy/windecopresetsrc")))
    , m_changed(false)
{
    QDialog *parentDialog = qobject_cast<QDialog *>(parent);

    // this is a hack to get an Apply button
    if (parentDialog && QCoreApplication::applicationName() == QStringLiteral("systemsettings")) {
        system("kcmshell5 plasma/kcms/klassy/kcm_klassydecoration &");
        parentDialog->close();
    }
    setButtons(KCModule::Default | KCModule::Apply);

    initKlassydecorationConfigQrc();

    // configuration
    m_ui.setupUi(this);

    m_ui.defaultExceptions->setKConfig(m_configuration, m_presetsConfiguration);
    m_ui.exceptions->setKConfig(m_configuration, m_presetsConfiguration);

    // add the "Presets..." button
    QVBoxLayout *presetsButtonVLayout = new QVBoxLayout();
    m_presetsButton = new QPushButton(i18n("&Presets..."));
    presetsButtonVLayout->addWidget(m_presetsButton);
    m_presetsButton->setMinimumWidth(125);
    presetsButtonVLayout->setContentsMargins(0, 0, 0, 0);
    m_ui.gridLayout_9->addLayout(presetsButtonVLayout, 0, 0, Qt::AlignRight | Qt::AlignTop);
    connect(m_presetsButton, &QAbstractButton::clicked, this, &ConfigWidget::presetsButtonClicked);
    this->setTabOrder(m_presetsButton, m_ui.tabWidget);

    // hide the title if a kcmshell dialog
    if (this->window()) {
        window()->setMinimumWidth(775);
        m_kPageWidget = this->window()->findChild<KPageWidget *>();
        if (m_kPageWidget) {
            KPageWidgetItem *currentPage = m_kPageWidget->currentPage();
            if (currentPage)
                kPageWidgetChanged(currentPage, currentPage); // this line usually is false but currentPage is valid on change
            connect(m_kPageWidget, &KPageWidget::currentPageChanged, this, &ConfigWidget::kPageWidgetChanged);
        }
    }

    // hide the push buttons for default exceptions
    QList<QPushButton *> defaultPushButtons = m_ui.defaultExceptions->findChildren<QPushButton *>();
    for (QPushButton *defaultPushButton : defaultPushButtons) {
        QSizePolicy spRetain = defaultPushButton->sizePolicy();
        spRetain.setRetainSizeWhenHidden(true);
        defaultPushButton->setSizePolicy(spRetain);

        defaultPushButton->hide();
    }

    m_unlockedIcon.addFile(QStringLiteral(":/klassy_config_icons/object-unlocked-symbolic.svg"), QSize(16, 16));
    m_lockedIcon.addFile(QStringLiteral(":/klassy_config_icons/object-locked-symbolic.svg"), QSize(16, 16));
    // add corner icon
    m_ui.cornerRadiusIcon->setPixmap(QIcon::fromTheme(QStringLiteral("tool_curve")).pixmap(16, 16));

    m_loadPresetDialog = new LoadPreset(m_configuration, m_presetsConfiguration, this);
    m_buttonSizingDialog = new ButtonSizing(m_configuration, m_presetsConfiguration, this);
    m_buttonColorsDialog = new ButtonColors(m_configuration, m_presetsConfiguration, this);
    m_buttonBehaviourDialog = new ButtonBehaviour(m_configuration, m_presetsConfiguration, this);
    m_titleBarSpacingDialog = new TitleBarSpacing(m_configuration, m_presetsConfiguration, this);
    m_titleBarOpacityDialog = new TitleBarOpacity(m_configuration, m_presetsConfiguration, this);
    m_windowOutlineStyleDialog = new WindowOutlineStyle(m_configuration, m_presetsConfiguration, this);
    m_shadowStyleDialog = new ShadowStyle(m_configuration, m_presetsConfiguration, this);

    // the dispayed colours in the ButtonColors UI depend upon ButtonBehaviour
    connect(m_buttonBehaviourDialog, &ButtonBehaviour::saved, m_buttonColorsDialog, &ButtonColors::loadButtonPaletteColorsIcons);

    // update the horizontal header icons in-case the icon style has changed
    connect(this, &ConfigWidget::saved, m_buttonColorsDialog, &ButtonColors::load);

    // this is necessary because when you reload the kwin config in a sub-dialog it prevents this main dialog from saving (this happens when run from
    // systemsettings only)
    if (parentDialog && QCoreApplication::applicationName() == QStringLiteral("systemsettings"))
        connect(parentDialog, &QDialog::accepted, this, &ConfigWidget::save);

#if KLASSY_GIT_MASTER
    // set the long version string if from the git master
    m_ui.version->setText("v" + klassyLongVersion());

#else
    // set shortened version string in UI if an official release
    QRegularExpression re("\\d+\\.\\d+");
    QRegularExpressionMatch match = re.match(KLASSY_VERSION);
    if (match.hasMatch()) {
        QString matched = match.captured(0);
        m_ui.version->setText("v" + matched);
    }
#endif

    connect(m_ui.integratedRoundedRectangleSizingButton, &QAbstractButton::clicked, this, &ConfigWidget::integratedRoundedRectangleSizingButtonClicked);
    connect(m_ui.fullHeightRectangleSizingButton, &QAbstractButton::clicked, this, &ConfigWidget::fullHeightRectangleSizingButtonClicked);
    connect(m_ui.buttonSizingButton, &QAbstractButton::clicked, this, &ConfigWidget::buttonSizingButtonClicked);
    connect(m_ui.buttonColorsButton, &QAbstractButton::clicked, this, &ConfigWidget::buttonColorsButtonClicked);
    connect(m_ui.buttonBehaviourButton, &QAbstractButton::clicked, this, &ConfigWidget::buttonBehaviourButtonClicked);
    connect(m_ui.titleBarSpacingButton, &QAbstractButton::clicked, this, &ConfigWidget::titleBarSpacingButtonClicked);
    connect(m_ui.titleBarOpacityButton, &QAbstractButton::clicked, this, &ConfigWidget::titleBarOpacityButtonClicked);
    connect(m_ui.thinWindowOutlineStyleButton, &QAbstractButton::clicked, this, &ConfigWidget::windowOutlineStyleButtonClicked);
    connect(m_ui.shadowStyleButton, &QAbstractButton::clicked, this, &ConfigWidget::shadowStyleButtonClicked);

    updateIconsStackedWidgetVisible();
    updateBackgroundShapeStackedWidgetVisible();

    // track ui changes
    // direct connections are used in several places so the slot can detect the immediate m_loading status (not available in a queued connection)
    connect(m_ui.buttonIconStyle, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.buttonIconStyle, SIGNAL(currentIndexChanged(int)), SLOT(updateIconsStackedWidgetVisible()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.buttonShape, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.buttonShape, SIGNAL(currentIndexChanged(int)), SLOT(updateBackgroundShapeStackedWidgetVisible()));
    connect(m_ui.iconSize, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.systemIconSize, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.cornerRadius, SIGNAL(valueChanged(double)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.boldButtonIcons, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.drawBorderOnMaximizedWindows, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.drawBackgroundGradient, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.drawTitleBarSeparator, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.useTitlebarColorForAllBorders, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.roundBottomCornersWhenNoBorders, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.forceColorizeSystemIcons, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged, Qt::ConnectionType::DirectConnection);

    // only enable animationsSpeed when animationsEnabled is checked
    connect(m_ui.animationsEnabled, &QAbstractButton::toggled, this, &ConfigWidget::setEnabledAnimationsSpeed);

    // track animations changes
    connect(m_ui.animationsEnabled, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.animationsSpeedRelativeSystem, SIGNAL(valueChanged(int)), SLOT(updateChanged()));

    connect(m_ui.colorizeThinWindowOutlineWithButton, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged, Qt::ConnectionType::DirectConnection);

    // track exception changes
    connect(m_ui.defaultExceptions, &ExceptionListWidget::changed, this, &ConfigWidget::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.exceptions, &ExceptionListWidget::changed, this, &ConfigWidget::updateChanged, Qt::ConnectionType::DirectConnection);

    QApplication::instance()->installEventFilter(this); // to monitor palette changes in the app (can't get them from this)
}

ConfigWidget::~ConfigWidget()
{
    cleanupKlassydecorationConfigQrc();
}

//_________________________________________________________
void ConfigWidget::load()
{
    m_loading = true;

    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();
    m_buttonSizingDialog->load();
    m_buttonColorsDialog->load();
    m_buttonBehaviourDialog->load();
    m_titleBarSpacingDialog->load();
    m_titleBarOpacityDialog->load();
    m_windowOutlineStyleDialog->load();
    m_shadowStyleDialog->load();
    PresetsModel::importBundledPresets(m_presetsConfiguration.data());
    updateLockIcons();

    // assign to ui
    m_ui.buttonIconStyle->setCurrentIndex(m_internalSettings->buttonIconStyle());
    m_ui.buttonShape->setCurrentIndex(m_internalSettings->buttonShape());
    m_ui.iconSize->setCurrentIndex(m_internalSettings->iconSize());
    m_ui.systemIconSize->setCurrentIndex(m_internalSettings->systemIconSize());
    m_ui.cornerRadius->setValue(m_internalSettings->cornerRadius());

    m_ui.drawBorderOnMaximizedWindows->setChecked(m_internalSettings->drawBorderOnMaximizedWindows());
    m_ui.boldButtonIcons->setCurrentIndex(m_internalSettings->boldButtonIcons());
    m_ui.drawBackgroundGradient->setChecked(m_internalSettings->drawBackgroundGradient());
    m_ui.drawTitleBarSeparator->setChecked(m_internalSettings->drawTitleBarSeparator());
    m_ui.animationsEnabled->setChecked(m_internalSettings->animationsEnabled());
    m_ui.animationsSpeedRelativeSystem->setValue(m_internalSettings->animationsSpeedRelativeSystem());
    m_ui.useTitlebarColorForAllBorders->setChecked(m_internalSettings->useTitlebarColorForAllBorders());
    m_ui.roundBottomCornersWhenNoBorders->setChecked(m_internalSettings->roundBottomCornersWhenNoBorders());
    m_ui.forceColorizeSystemIcons->setChecked(m_internalSettings->forceColorizeSystemIcons());

    m_ui.colorizeThinWindowOutlineWithButton->setChecked(m_internalSettings->colorizeThinWindowOutlineWithButton());

    updateIconsStackedWidgetVisible();
    updateBackgroundShapeStackedWidgetVisible();
    // load exceptions
    DecorationExceptionList exceptions;
    exceptions.readConfig(m_configuration);
    if (exceptions.numberDefaults()) {
        m_ui.defaultExceptions->setExceptions(exceptions.getDefault());
    } else {
        m_ui.defaultExceptions->hide();
        m_ui.defaultExceptionsLabel->hide();
        m_ui.defaultExceptionsSpacer->setGeometry(QRect());
    }
    m_ui.exceptions->setExceptions(exceptions.get());
    setChanged(false);
    m_loading = false;
}

//_________________________________________________________
void ConfigWidget::save()
{
    saveMain();
}

void ConfigWidget::saveMain(QString saveAsPresetName)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    m_internalSettings->setButtonIconStyle(m_ui.buttonIconStyle->currentIndex());
    m_internalSettings->setButtonShape(m_ui.buttonShape->currentIndex());
    m_internalSettings->setIconSize(m_ui.iconSize->currentIndex());
    m_internalSettings->setSystemIconSize(m_ui.systemIconSize->currentIndex());
    m_internalSettings->setCornerRadius(m_ui.cornerRadius->value());
    m_internalSettings->setBoldButtonIcons(m_ui.boldButtonIcons->currentIndex());
    m_internalSettings->setDrawBorderOnMaximizedWindows(m_ui.drawBorderOnMaximizedWindows->isChecked());
    m_internalSettings->setDrawBackgroundGradient(m_ui.drawBackgroundGradient->isChecked());
    m_internalSettings->setDrawTitleBarSeparator(m_ui.drawTitleBarSeparator->isChecked());
    m_internalSettings->setAnimationsEnabled(m_ui.animationsEnabled->isChecked());
    m_internalSettings->setAnimationsSpeedRelativeSystem(m_ui.animationsSpeedRelativeSystem->value());
    m_internalSettings->setUseTitlebarColorForAllBorders(m_ui.useTitlebarColorForAllBorders->isChecked());
    m_internalSettings->setRoundBottomCornersWhenNoBorders(m_ui.roundBottomCornersWhenNoBorders->isChecked());
    m_internalSettings->setForceColorizeSystemIcons(m_ui.forceColorizeSystemIcons->isChecked());
    m_internalSettings->setColorizeThinWindowOutlineWithButton(m_ui.colorizeThinWindowOutlineWithButton->isChecked());

    if (saveAsPresetName.isEmpty()) { // normal case
        m_buttonSizingDialog->save(false);
        m_buttonColorsDialog->save(false);
        m_buttonBehaviourDialog->save(false);
        m_titleBarSpacingDialog->save(false);
        m_titleBarOpacityDialog->save(false);
        m_shadowStyleDialog->save(false);
        m_windowOutlineStyleDialog->save(false);

        // save configuration
        m_internalSettings->save();

        // get list of exceptions and write
        InternalSettingsList exceptions(m_ui.exceptions->exceptions());
        InternalSettingsList defaultExceptions(m_ui.defaultExceptions->exceptions());
        DecorationExceptionList(exceptions, defaultExceptions).writeConfig(m_configuration);

        // sync configuration for exceptions
        m_configuration->sync();

        setChanged(false);
        Q_EMIT saved();

        bool updateColorCachePresetPresent = false;
        for (auto &exception : (exceptions + defaultExceptions)) {
            if ((!exception->exceptionPreset().isEmpty() && PresetsModel::isPresetPresent(m_presetsConfiguration.data(), exception->exceptionPreset()))
                || exception->opaqueTitleBar()) {
                updateColorCachePresetPresent = true;
                break;
            }
        }
        if (updateColorCachePresetPresent)
            DBusMessages::updateDecorationColorCache();
        // needed to tell kwin to reload when running from external kcmshell
        DBusMessages::kwinReloadConfig();
        DBusMessages::kstyleReloadDecorationConfig();

    } else { // set the preset
        // delete the preset if one of that name already exists
        PresetsModel::deletePreset(m_presetsConfiguration.data(), saveAsPresetName);

        // write the new internalSettings value as a new preset
        PresetsModel::writePreset(m_internalSettings.data(), m_presetsConfiguration.data(), saveAsPresetName);
        // sync configuration for presets
        m_presetsConfiguration->sync();
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
    m_ui.buttonIconStyle->setCurrentIndex(m_internalSettings->buttonIconStyle());
    m_ui.buttonShape->setCurrentIndex(m_internalSettings->buttonShape());
    m_ui.iconSize->setCurrentIndex(m_internalSettings->iconSize());
    m_ui.systemIconSize->setCurrentIndex(m_internalSettings->systemIconSize());
    m_ui.cornerRadius->setValue(m_internalSettings->cornerRadius());
    m_ui.boldButtonIcons->setCurrentIndex(m_internalSettings->boldButtonIcons());
    m_ui.drawBorderOnMaximizedWindows->setChecked(m_internalSettings->drawBorderOnMaximizedWindows());
    m_ui.drawBackgroundGradient->setChecked(m_internalSettings->drawBackgroundGradient());
    m_ui.animationsEnabled->setChecked(m_internalSettings->animationsEnabled());
    m_ui.animationsSpeedRelativeSystem->setValue(m_internalSettings->animationsSpeedRelativeSystem());
    m_ui.drawTitleBarSeparator->setChecked(m_internalSettings->drawTitleBarSeparator());
    m_ui.useTitlebarColorForAllBorders->setChecked(m_internalSettings->useTitlebarColorForAllBorders());
    m_ui.roundBottomCornersWhenNoBorders->setChecked(m_internalSettings->roundBottomCornersWhenNoBorders());
    m_ui.forceColorizeSystemIcons->setChecked(m_internalSettings->forceColorizeSystemIcons());
    m_ui.colorizeThinWindowOutlineWithButton->setChecked(m_internalSettings->colorizeThinWindowOutlineWithButton());

    // set defaults in dialogs
    m_buttonSizingDialog->defaults();
    m_buttonColorsDialog->defaults();
    m_buttonBehaviourDialog->defaults();
    m_titleBarSpacingDialog->defaults();
    m_titleBarOpacityDialog->defaults();
    m_windowOutlineStyleDialog->defaults();
    m_shadowStyleDialog->defaults();

    // load default exceptions and refresh (leave user-set exceptions alone)
    DecorationExceptionList exceptions;
    exceptions.readConfig(m_configuration, true);
    if (exceptions.numberDefaults()) {
        m_ui.defaultExceptions->setExceptions(exceptions.getDefault());
    } else {
        m_ui.defaultExceptions->hide();
    }

    updateIconsStackedWidgetVisible();
    updateBackgroundShapeStackedWidgetVisible();
    setChanged(!isDefaults());

    m_processingDefaults = false;
    m_defaultsPressed = true;
}

bool ConfigWidget::isDefaults()
{
    bool isDefaults = true;

    QString groupName(QStringLiteral("Windeco"));
    if (m_configuration->hasGroup(groupName)) {
        KConfigGroup group = m_configuration->group(groupName);
        if (group.keyList().count())
            return false;
    }

    if (!m_buttonSizingDialog->isDefaults()) {
        return false;
    }

    if (!m_buttonColorsDialog->isDefaults()) {
        return false;
    }

    if (!m_buttonBehaviourDialog->isDefaults()) {
        return false;
    }

    if (!m_titleBarSpacingDialog->isDefaults()) {
        return false;
    }

    if (!m_titleBarOpacityDialog->isDefaults()) {
        return false;
    }

    if (!m_windowOutlineStyleDialog->isDefaults()) {
        return false;
    }

    if (!m_shadowStyleDialog->isDefaults()) {
        return false;
    }

    return isDefaults;
}

//_______________________________________________
void ConfigWidget::updateChanged()
{
    // check configuration
    if (!m_internalSettings) {
        return;
    }

    if (m_loading)
        return; // only check if the user has made a change to the UI, or user has pressed defaults

    // track modifications
    bool modified(false);

    if (m_ui.drawTitleBarSeparator->isChecked() != m_internalSettings->drawTitleBarSeparator())
        modified = true;
    else if (m_ui.useTitlebarColorForAllBorders->isChecked() != m_internalSettings->useTitlebarColorForAllBorders())
        modified = true;
    else if (m_ui.roundBottomCornersWhenNoBorders->isChecked() != m_internalSettings->roundBottomCornersWhenNoBorders())
        modified = true;
    else if (m_ui.forceColorizeSystemIcons->isChecked() != m_internalSettings->forceColorizeSystemIcons())
        modified = true;
    else if (m_ui.buttonIconStyle->currentIndex() != m_internalSettings->buttonIconStyle())
        modified = true;
    else if (m_ui.buttonShape->currentIndex() != m_internalSettings->buttonShape())
        modified = true;
    else if (m_ui.iconSize->currentIndex() != m_internalSettings->iconSize())
        modified = true;
    else if (m_ui.systemIconSize->currentIndex() != m_internalSettings->systemIconSize())
        modified = true;
    else if (m_ui.boldButtonIcons->currentIndex() != m_internalSettings->boldButtonIcons())
        modified = true;
    else if (m_ui.drawBorderOnMaximizedWindows->isChecked() != m_internalSettings->drawBorderOnMaximizedWindows())
        modified = true;
    else if (m_ui.drawBackgroundGradient->isChecked() != m_internalSettings->drawBackgroundGradient())
        modified = true;
    else if (m_ui.cornerRadius->value() != m_internalSettings->cornerRadius())
        modified = true;
    else if (m_ui.colorizeThinWindowOutlineWithButton->isChecked() != m_internalSettings->colorizeThinWindowOutlineWithButton())
        modified = true;

    // animations
    else if (m_ui.animationsEnabled->isChecked() != m_internalSettings->animationsEnabled())
        modified = true;
    else if (m_ui.animationsSpeedRelativeSystem->value() != m_internalSettings->animationsSpeedRelativeSystem())
        modified = true;

    // dialogs
    else if (m_buttonSizingDialog->m_changed)
        modified = true;
    else if (m_windowOutlineStyleDialog->m_changed)
        modified = true;

    // exceptions
    else if (m_ui.defaultExceptions->isChanged())
        modified = true;
    else if (m_ui.exceptions->isChanged())
        modified = true;

    setChanged(modified);
}

//_______________________________________________
void ConfigWidget::setChanged(bool value)
{
    Q_EMIT changed(value);
}

void ConfigWidget::kPageWidgetChanged(KPageWidgetItem *current, KPageWidgetItem *before)
{
    Q_UNUSED(before)
    if (current) {
        current->setHeaderVisible(false);
    }
}

// only enable animationsSpeedRelativeSystem and animationsSpeedLabelx when animationsEnabled is checked
void ConfigWidget::setEnabledAnimationsSpeed()
{
    m_ui.animationsSpeedRelativeSystem->setEnabled(m_ui.animationsEnabled->isChecked());
    m_ui.animationsSpeedLabel1->setEnabled(m_ui.animationsEnabled->isChecked());
    m_ui.animationsSpeedLabel2->setEnabled(m_ui.animationsEnabled->isChecked());
    m_ui.animationsSpeedLabel4->setEnabled(m_ui.animationsEnabled->isChecked());
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
        || m_ui.buttonShape->currentIndex() == InternalSettings::EnumButtonShape::ShapeFullHeightRoundedRectangle)
        m_ui.backgroundShapeStackedWidget->setCurrentIndex(1);
    else if (m_ui.buttonShape->currentIndex() == InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangle)
        m_ui.backgroundShapeStackedWidget->setCurrentIndex(2);
    else
        m_ui.backgroundShapeStackedWidget->setCurrentIndex(0);
}

void ConfigWidget::dialogChanged(bool changed)
{
    setChanged(changed);
}

// these 3 functions would be better rewritten with a stacked widget (like for windowOutlineButtonClicked) for simplicity
void ConfigWidget::integratedRoundedRectangleSizingButtonClicked()
{
    m_buttonSizingDialog->setGeometry(0, 0, m_buttonSizingDialog->geometry().width(), 400);
    m_buttonSizingDialog->setWindowTitle(i18n("Button Width & Spacing - Klassy Settings"));
    m_buttonSizingDialog->m_ui.groupBox->setTitle(i18n("Integrated Rounded Rectangle Width && Spacing"));

    m_buttonSizingDialog->m_ui.scaleBackgroundPercentLabel->setVisible(false);
    m_buttonSizingDialog->m_ui.scaleBackgroundPercent->setVisible(false);

    m_buttonSizingDialog->m_ui.fullHeightButtonWidthMarginLeftLabel->setVisible(true);
    m_buttonSizingDialog->m_ui.fullHeightButtonWidthMarginLeft->setVisible(true);
    m_buttonSizingDialog->m_ui.line->setVisible(true);
    m_buttonSizingDialog->m_ui.lockFullHeightButtonWidthMargins->setVisible(true);
    m_buttonSizingDialog->m_ui.line_2->setVisible(true);
    m_buttonSizingDialog->m_ui.fullHeightButtonWidthMarginRightLabel->setVisible(true);
    m_buttonSizingDialog->m_ui.fullHeightButtonWidthMarginRight->setVisible(true);

    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingLeftLabel->setVisible(true);
    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingLeft->setVisible(true);
    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingLeftLine->setVisible(true);
    m_buttonSizingDialog->m_ui.lockFullHeightButtonSpacingLeftRight->setVisible(true);
    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingRightLabel->setVisible(true);
    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingRight->setVisible(true);
    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingRightLine->setVisible(true);

    m_buttonSizingDialog->m_ui.buttonSpacingLeftLabel->setVisible(false);
    m_buttonSizingDialog->m_ui.buttonSpacingLeft->setVisible(false);
    m_buttonSizingDialog->m_ui.buttonSpacingLeftLine->setVisible(false);
    m_buttonSizingDialog->m_ui.lockButtonSpacingLeftRight->setVisible(false);
    m_buttonSizingDialog->m_ui.buttonSpacingRightLabel->setVisible(false);
    m_buttonSizingDialog->m_ui.buttonSpacingRight->setVisible(false);
    m_buttonSizingDialog->m_ui.buttonSpacingRightLine->setVisible(false);

    m_buttonSizingDialog->m_ui.integratedRoundedRectangleBottomPadding->setVisible(true);
    m_buttonSizingDialog->m_ui.integratedRoundedRectangleBottomPaddingLabel->setVisible(true);

    m_buttonSizingDialog->m_ui.verticalSpacer_2->changeSize(20, 40, QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_buttonSizingDialog->m_ui.verticalSpacer_3->changeSize(20, 40, QSizePolicy::Fixed, QSizePolicy::Expanding);

    if (!m_buttonSizingDialog->m_loaded)
        m_buttonSizingDialog->load();
    m_buttonSizingDialog->show();
}

void ConfigWidget::fullHeightRectangleSizingButtonClicked()
{
    m_buttonSizingDialog->setGeometry(0, 0, m_buttonSizingDialog->geometry().width(), 300);
    m_buttonSizingDialog->setWindowTitle(i18n("Button Width & Spacing - Klassy Settings"));
    m_buttonSizingDialog->m_ui.groupBox->setTitle(i18n("Full-height Rectangle Width && Spacing"));

    m_buttonSizingDialog->m_ui.scaleBackgroundPercentLabel->setVisible(false);
    m_buttonSizingDialog->m_ui.scaleBackgroundPercent->setVisible(false);

    m_buttonSizingDialog->m_ui.fullHeightButtonWidthMarginLeftLabel->setVisible(true);
    m_buttonSizingDialog->m_ui.fullHeightButtonWidthMarginLeft->setVisible(true);
    m_buttonSizingDialog->m_ui.line->setVisible(true);
    m_buttonSizingDialog->m_ui.lockFullHeightButtonWidthMargins->setVisible(true);
    m_buttonSizingDialog->m_ui.line_2->setVisible(true);
    m_buttonSizingDialog->m_ui.fullHeightButtonWidthMarginRightLabel->setVisible(true);
    m_buttonSizingDialog->m_ui.fullHeightButtonWidthMarginRight->setVisible(true);

    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingLeftLabel->setVisible(true);
    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingLeft->setVisible(true);
    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingLeftLine->setVisible(true);
    m_buttonSizingDialog->m_ui.lockFullHeightButtonSpacingLeftRight->setVisible(true);
    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingRightLabel->setVisible(true);
    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingRight->setVisible(true);
    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingRightLine->setVisible(true);

    m_buttonSizingDialog->m_ui.buttonSpacingLeftLabel->setVisible(false);
    m_buttonSizingDialog->m_ui.buttonSpacingLeft->setVisible(false);
    m_buttonSizingDialog->m_ui.buttonSpacingLeftLine->setVisible(false);
    m_buttonSizingDialog->m_ui.lockButtonSpacingLeftRight->setVisible(false);
    m_buttonSizingDialog->m_ui.buttonSpacingRightLabel->setVisible(false);
    m_buttonSizingDialog->m_ui.buttonSpacingRight->setVisible(false);
    m_buttonSizingDialog->m_ui.buttonSpacingRightLine->setVisible(false);

    m_buttonSizingDialog->m_ui.integratedRoundedRectangleBottomPadding->setVisible(false);
    m_buttonSizingDialog->m_ui.integratedRoundedRectangleBottomPaddingLabel->setVisible(false);

    m_buttonSizingDialog->m_ui.verticalSpacer_2->changeSize(20, 40, QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_buttonSizingDialog->m_ui.verticalSpacer_3->changeSize(20, 40, QSizePolicy::Fixed, QSizePolicy::Expanding);

    m_buttonSizingDialog->show();
}

void ConfigWidget::buttonSizingButtonClicked()
{
    m_buttonSizingDialog->setGeometry(0, 0, m_buttonSizingDialog->geometry().width(), 275);
    m_buttonSizingDialog->setWindowTitle(i18n("Button Size & Spacing - Klassy Settings"));
    m_buttonSizingDialog->m_ui.groupBox->setTitle(i18n("Button Size && Spacing"));

    m_buttonSizingDialog->m_ui.scaleBackgroundPercentLabel->setVisible(true);
    m_buttonSizingDialog->m_ui.scaleBackgroundPercent->setVisible(true);

    m_buttonSizingDialog->m_ui.fullHeightButtonWidthMarginLeftLabel->setVisible(false);
    m_buttonSizingDialog->m_ui.fullHeightButtonWidthMarginLeft->setVisible(false);
    m_buttonSizingDialog->m_ui.line->setVisible(false);
    m_buttonSizingDialog->m_ui.lockFullHeightButtonWidthMargins->setVisible(false);
    m_buttonSizingDialog->m_ui.line_2->setVisible(false);
    m_buttonSizingDialog->m_ui.fullHeightButtonWidthMarginRightLabel->setVisible(false);
    m_buttonSizingDialog->m_ui.fullHeightButtonWidthMarginRight->setVisible(false);

    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingLeftLabel->setVisible(false);
    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingLeft->setVisible(false);
    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingLeftLine->setVisible(false);
    m_buttonSizingDialog->m_ui.lockFullHeightButtonSpacingLeftRight->setVisible(false);
    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingRightLabel->setVisible(false);
    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingRight->setVisible(false);
    m_buttonSizingDialog->m_ui.fullHeightButtonSpacingRightLine->setVisible(false);

    m_buttonSizingDialog->m_ui.buttonSpacingLeftLabel->setVisible(true);
    m_buttonSizingDialog->m_ui.buttonSpacingLeft->setVisible(true);
    m_buttonSizingDialog->m_ui.buttonSpacingLeftLine->setVisible(true);
    m_buttonSizingDialog->m_ui.lockButtonSpacingLeftRight->setVisible(true);
    m_buttonSizingDialog->m_ui.buttonSpacingRightLabel->setVisible(true);
    m_buttonSizingDialog->m_ui.buttonSpacingRight->setVisible(true);
    m_buttonSizingDialog->m_ui.buttonSpacingRightLine->setVisible(true);

    m_buttonSizingDialog->m_ui.integratedRoundedRectangleBottomPadding->setVisible(false);
    m_buttonSizingDialog->m_ui.integratedRoundedRectangleBottomPaddingLabel->setVisible(false);

    m_buttonSizingDialog->m_ui.verticalSpacer_2->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_buttonSizingDialog->m_ui.verticalSpacer_3->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_buttonSizingDialog->show();
}

void ConfigWidget::buttonColorsButtonClicked()
{
    m_buttonColorsDialog->setWindowTitle(i18n("Button Colours - Klassy Settings"));
    m_buttonColorsDialog->setWindowIcon(QIcon::fromTheme(QStringLiteral("color-management")));
    m_buttonColorsDialog->resizeDialog();
    m_buttonColorsDialog->show();
}

void ConfigWidget::buttonBehaviourButtonClicked()
{
    m_buttonBehaviourDialog->setWindowTitle(i18n("Button Behaviour - Klassy Settings"));
    m_buttonBehaviourDialog->show();
}

void ConfigWidget::titleBarSpacingButtonClicked()
{
    m_titleBarSpacingDialog->setWindowTitle(i18n("Titlebar Spacing - Klassy Settings"));
    m_titleBarSpacingDialog->show();
}

void ConfigWidget::titleBarOpacityButtonClicked()
{
    m_titleBarOpacityDialog->setWindowTitle(i18n("Titlebar Opacity - Klassy Settings"));
    m_titleBarOpacityDialog->show();
}

void ConfigWidget::shadowStyleButtonClicked()
{
    m_shadowStyleDialog->setWindowTitle(i18n("Shadow Style - Klassy Settings"));
    m_shadowStyleDialog->show();
}

void ConfigWidget::windowOutlineStyleButtonClicked()
{
    m_windowOutlineStyleDialog->setWindowTitle(i18n("Window Outline Style - Klassy Settings"));
    m_windowOutlineStyleDialog->show();
}

void ConfigWidget::presetsButtonClicked()
{
    m_loadPresetDialog->setWindowTitle(i18n("Presets - Klassy Settings"));
    m_loadPresetDialog->initPresetsList();
    m_loadPresetDialog->show();
}

void ConfigWidget::updateLockIcons()
{
    ColorTools::convertAlphaToColor(m_unlockedIcon, QSize(16, 16), QApplication::palette().windowText().color());
    ColorTools::convertAlphaToColor(m_lockedIcon, QSize(16, 16), QApplication::palette().windowText().color());
    m_lockIcon.addPixmap(m_unlockedIcon.pixmap(QSize(16, 16)), QIcon::Mode::Normal, QIcon::State::Off);
    m_lockIcon.addPixmap(m_lockedIcon.pixmap(QSize(16, 16)), QIcon::Mode::Normal, QIcon::State::On);

    m_buttonSizingDialog->updateLockIcons();
    m_buttonColorsDialog->updateLockIcons();
    m_buttonBehaviourDialog->updateLockIcons();
    m_titleBarSpacingDialog->updateLockIcons();
    m_windowOutlineStyleDialog->updateLockIcons();
}

bool ConfigWidget::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() == QEvent::ApplicationPaletteChange) {
        // overwrite handling of palette change
        updateLockIcons();
        return QObject::eventFilter(obj, ev);
    }
    // Make sure the rest of events are handled
    return QObject::eventFilter(obj, ev);
}
}
