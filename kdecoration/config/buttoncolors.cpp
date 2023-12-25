/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "buttoncolors.h"
#include "breezeconfigwidget.h"
#include "presetsmodel.h"
#include <KColorUtils>
#include <QCheckBox>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QWindow>
#include <memory>

namespace Breeze
{

ButtonColors::ButtonColors(KSharedConfig::Ptr config, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui_ButtonColors)
    , m_configuration(config)
{
    m_ui->setupUi(this);

    this->resize(755, 400);
    m_ui->activeOverrideGroupBox->setVisible(false);
    connect(m_ui->buttonColorActiveOverrideToggle, &QAbstractButton::toggled, this, &ButtonColors::showActiveOverrideGroupBox);
    connect(m_ui->buttonColorActiveOverrideToggle, &QAbstractButton::clicked, this, &ButtonColors::resizeActiveOverrideGroupBox);

    int numColumns = m_overridableButtonColorStatesStrings.count();

    // generate the horizontal header
    // m_unlockedIcon.addFile(QStringLiteral(":/klassy_config_icons/object-unlocked-symbolic.svg"),QSize(16,16));
    // m_lockedIcon.addFile(QStringLiteral(":/klassy_config_icons/object-locked-symbolic.svg"),QSize(16,16));
    m_unlockedIcon = QIcon::fromTheme(QStringLiteral("unlock"));
    m_lockedIcon = QIcon::fromTheme(QStringLiteral("lock"));
    for (int columnIndex = 0; columnIndex < numColumns; columnIndex++) {
        QTableWidgetItem *horizontalHeaderItem = new QTableWidgetItem();
        horizontalHeaderItem->setText(m_overridableButtonColorStatesStrings[columnIndex]);
        horizontalHeaderItem->setIcon(m_unlockedIcon);
        horizontalHeaderItem->setToolTip(i18n("Lock to make all colours in this column the same"));
        horizontalHeaderItem->setData(Qt::InitialSortOrderRole, columnIndex);
        m_ui->overrideColorTableActive->setHorizontalHeaderItem(columnIndex, horizontalHeaderItem);
    }

    connect(m_ui->overrideColorTableActive->horizontalHeader(), &QHeaderView::sectionClicked, this, &ButtonColors::activeTableHorizontalHeaderSectionClicked);
    connect(m_ui->overrideColorTableActive->horizontalHeader(), &QHeaderView::sectionClicked, this, &ButtonColors::updateChanged);

    // gnerate the overrideColorTableActive table UI
    // populate the checkboxes and KColorButtons
    for (int rowIndex = 0; rowIndex < m_colorOverridableButtonTypesStrings.count(); rowIndex++) {
        m_ui->overrideColorTableActive->insertRow(rowIndex);
        for (int columnIndex = 0; columnIndex < numColumns; columnIndex++) {
            QHBoxLayout *hlayout = new QHBoxLayout();
            hlayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
            QCheckBox *checkBox = new QCheckBox();
            checkBox->setProperty("row", rowIndex);
            checkBox->setProperty("column", columnIndex);
            hlayout->addWidget(checkBox);
            KColorButton *colorButton = new KColorButton();
            colorButton->setProperty("row", rowIndex);
            colorButton->setProperty("column", columnIndex);
            colorButton->setAlphaChannelEnabled(true);
            connect(colorButton, &KColorButton::changed, this, &ButtonColors::updateChanged);
            colorButton->setVisible(false);
            hlayout->addWidget(colorButton);
            hlayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
            QWidget *w = new QWidget();
            w->setLayout(hlayout);
            m_ui->overrideColorTableActive->setCellWidget(rowIndex, columnIndex, w);
            connect(checkBox, &QAbstractButton::toggled, colorButton, &QWidget::setVisible);
            connect(checkBox, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
            connect(checkBox, &QAbstractButton::toggled, this, &ButtonColors::resizeOverrideColorTable);
            connect(colorButton, &KColorButton::changed, this, &ButtonColors::copyCellDataToOtherCells);
        }
    }

    // populate the vertical header
    m_ui->overrideColorTableActive->setVerticalHeaderLabels(m_colorOverridableButtonTypesStrings);
    m_ui->overrideColorTableActive->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_ui->overrideColorTableActive->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // track ui changes
    connect(m_ui->buttonIconColors, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->closeIconNegativeBackground, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->buttonBackgroundColors, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->buttonBackgroundColors, SIGNAL(currentIndexChanged(int)), SLOT(setNegativeCloseBackgroundHoverPressState()));
    connect(m_ui->buttonBackgroundColors, SIGNAL(currentIndexChanged(int)), SLOT(setCloseIconNegativeBackgroundState()));
    connect(m_ui->translucentButtonBackgrounds, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->negativeCloseBackgroundHoverPress, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->lockButtonColorsActive, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->buttonColorActiveOverrideToggle, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->blackWhiteIconOnPoorContrast, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->adjustBackgroundColorOnPoorContrast, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->translucentButtonBackgroundsOpacity, SIGNAL(valueChanged(int)), SLOT(updateChanged()));

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &ButtonColors::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &ButtonColors::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &ButtonColors::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

ButtonColors::~ButtonColors()
{
    delete m_ui;
}

void ButtonColors::loadMain(const QString loadPreset, const bool assignUiValuesOnly)
{
    if (!assignUiValuesOnly) {
        m_loading = true;

        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr(new InternalSettings());
        if (loadPreset.isEmpty()) { // normal cases
            m_internalSettings->load();
        } else { // loading preset
            PresetsModel::loadPreset(m_internalSettings.data(), m_configuration.data(), loadPreset);
        }
    }

    m_ui->buttonIconColors->setCurrentIndex(m_internalSettings->buttonIconColors());
    m_ui->closeIconNegativeBackground->setCurrentIndex(m_internalSettings->closeIconNegativeBackground());
    m_ui->buttonBackgroundColors->setCurrentIndex(m_internalSettings->buttonBackgroundColors());
    m_ui->translucentButtonBackgrounds->setChecked(m_internalSettings->translucentButtonBackgrounds());
    m_ui->negativeCloseBackgroundHoverPress->setChecked(m_internalSettings->negativeCloseBackgroundHoverPress());
    m_ui->lockButtonColorsActive->setChecked(m_internalSettings->lockButtonColorsActive());
    m_ui->blackWhiteIconOnPoorContrast->setChecked(m_internalSettings->blackWhiteIconOnPoorContrast());
    m_ui->adjustBackgroundColorOnPoorContrast->setChecked(m_internalSettings->adjustBackgroundColorOnPoorContrast());
    m_ui->translucentButtonBackgroundsOpacity->setValue(m_internalSettings->translucentButtonBackgroundsOpacity() * 100);

    decodeColorOverridableLockStatesAndLoadHorizontalHeaderLocks();

    m_overrideColorsLoaded = false;
    bool overrideColorRowLoaded = false;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::Close),
                                                         m_internalSettings->buttonOverrideColorsCloseFlags(),
                                                         m_internalSettings->buttonOverrideColorsClose());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::Maximize),
                                                         m_internalSettings->buttonOverrideColorsMaximizeFlags(),
                                                         m_internalSettings->buttonOverrideColorsMaximize());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::Minimize),
                                                         m_internalSettings->buttonOverrideColorsMinimizeFlags(),
                                                         m_internalSettings->buttonOverrideColorsMinimize());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::Help),
                                                         m_internalSettings->buttonOverrideColorsHelpFlags(),
                                                         m_internalSettings->buttonOverrideColorsHelp());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::Shade),
                                                         m_internalSettings->buttonOverrideColorsShadeFlags(),
                                                         m_internalSettings->buttonOverrideColorsShade());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::AllDesktops),
                                                         m_internalSettings->buttonOverrideColorsAllDesktopsFlags(),
                                                         m_internalSettings->buttonOverrideColorsAllDesktops());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::KeepBelow),
                                                         m_internalSettings->buttonOverrideColorsKeepBelowFlags(),
                                                         m_internalSettings->buttonOverrideColorsKeepBelow());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::KeepAbove),
                                                         m_internalSettings->buttonOverrideColorsKeepAboveFlags(),
                                                         m_internalSettings->buttonOverrideColorsKeepAbove());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::ApplicationMenu),
                                                         m_internalSettings->buttonOverrideColorsApplicationMenuFlags(),
                                                         m_internalSettings->buttonOverrideColorsApplicationMenu());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::Menu),
                                                         m_internalSettings->buttonOverrideColorsMenuFlags(),
                                                         m_internalSettings->buttonOverrideColorsMenu());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    m_ui->buttonColorActiveOverrideToggle->setChecked(m_overrideColorsLoaded);

    // add icons
    setButtonBackgroundColorsIcons();

    if (!assignUiValuesOnly) {
        setChanged(false);

        m_loading = false;
        m_loaded = true;
    }
}

void ButtonColors::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    m_internalSettings->setButtonIconColors(m_ui->buttonIconColors->currentIndex());
    m_internalSettings->setCloseIconNegativeBackground(m_ui->closeIconNegativeBackground->currentIndex());
    m_internalSettings->setButtonBackgroundColors(m_ui->buttonBackgroundColors->currentIndex());
    m_internalSettings->setTranslucentButtonBackgrounds(m_ui->translucentButtonBackgrounds->isChecked());
    m_internalSettings->setNegativeCloseBackgroundHoverPress(m_ui->negativeCloseBackgroundHoverPress->isChecked());
    m_internalSettings->setLockButtonColorsActive(m_ui->lockButtonColorsActive->isChecked());
    m_internalSettings->setBlackWhiteIconOnPoorContrast(m_ui->blackWhiteIconOnPoorContrast->isChecked());
    m_internalSettings->setAdjustBackgroundColorOnPoorContrast(m_ui->adjustBackgroundColorOnPoorContrast->isChecked());
    m_internalSettings->setTranslucentButtonBackgroundsOpacity(m_ui->translucentButtonBackgroundsOpacity->value() / 100.0f);

    m_internalSettings->setButtonOverrideColorsLockStates(encodeColorOverridableLockStates());

    QList<int> colorsList;
    uint32_t colorsFlags = 0;

    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::Close), colorsFlags, colorsList);
    m_internalSettings->setButtonOverrideColorsCloseFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsClose(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::Maximize), colorsFlags, colorsList);
    m_internalSettings->setButtonOverrideColorsMaximizeFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsMaximize(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::Minimize), colorsFlags, colorsList);
    m_internalSettings->setButtonOverrideColorsMinimizeFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsMinimize(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::Help), colorsFlags, colorsList);
    m_internalSettings->setButtonOverrideColorsHelpFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsHelp(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::Shade), colorsFlags, colorsList);
    m_internalSettings->setButtonOverrideColorsShadeFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsShade(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive,
                                            static_cast<int>(ColorOverridableButtonTypes::AllDesktops),
                                            colorsFlags,
                                            colorsList);
    m_internalSettings->setButtonOverrideColorsAllDesktopsFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsAllDesktops(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::KeepBelow), colorsFlags, colorsList);
    m_internalSettings->setButtonOverrideColorsKeepBelowFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsKeepBelow(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::KeepAbove), colorsFlags, colorsList);
    m_internalSettings->setButtonOverrideColorsKeepAboveFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsKeepAbove(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive,
                                            static_cast<int>(ColorOverridableButtonTypes::ApplicationMenu),
                                            colorsFlags,
                                            colorsList);
    m_internalSettings->setButtonOverrideColorsApplicationMenuFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsApplicationMenu(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::Menu), colorsFlags, colorsList);
    m_internalSettings->setButtonOverrideColorsMenuFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsMenu(colorsList);

    m_internalSettings->save();

    load(); // need to re-load in the case where m_ui->buttonColorActiveOverrideToggle is unchecked

    setChanged(false);

    if (reloadKwinConfig) {
        ConfigWidget::kwinReloadConfig();
        ConfigWidget::kstyleReloadConfig();
    }
}

void ButtonColors::defaults()
{
    m_processingDefaults = true;

    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->setDefaults();

    // assign to ui
    loadMain(QString(), true);
    setChanged(true); // TODO:add this everywhere else

    m_processingDefaults = false;
    m_defaultsPressed = true;
}

void ButtonColors::setChanged(bool value)
{
    m_changed = value;
    setApplyButtonState(value);
    // Q_EMIT changed(value);
}

void ButtonColors::accept()
{
    save();
    QDialog::accept();
}

void ButtonColors::updateChanged()
{
    // check configuration
    if (!m_internalSettings)
        return;

    // track modifications
    bool modified(false);
    if (m_ui->buttonIconColors->currentIndex() != m_internalSettings->buttonIconColors())
        modified = true;
    else if (m_ui->closeIconNegativeBackground->currentIndex() != m_internalSettings->closeIconNegativeBackground())
        modified = true;
    else if (m_ui->buttonBackgroundColors->currentIndex() != m_internalSettings->buttonBackgroundColors())
        modified = true;
    else if (m_ui->translucentButtonBackgrounds->isChecked() != m_internalSettings->translucentButtonBackgrounds())
        modified = true;
    else if (m_ui->negativeCloseBackgroundHoverPress->isChecked() != m_internalSettings->negativeCloseBackgroundHoverPress())
        modified = true;
    else if (m_ui->lockButtonColorsActive->isChecked() != m_internalSettings->lockButtonColorsActive())
        modified = true;
    else if (m_ui->blackWhiteIconOnPoorContrast->isChecked() != m_internalSettings->blackWhiteIconOnPoorContrast())
        modified = true;
    else if (m_ui->adjustBackgroundColorOnPoorContrast->isChecked() != m_internalSettings->adjustBackgroundColorOnPoorContrast())
        modified = true;
    else if (!(qAbs(m_ui->translucentButtonBackgroundsOpacity->value() - (100 * m_internalSettings->translucentButtonBackgroundsOpacity())) < 0.001))
        modified = true;
    else if (encodeColorOverridableLockStates() != m_internalSettings->buttonOverrideColorsLockStates())
        modified = true;

    if (m_ui->buttonColorActiveOverrideToggle->isChecked()) {
        uint32_t colorsFlags;
        QList<int> colorsList;
        if (!modified) {
            encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::Close), colorsFlags, colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsCloseFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsClose());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive,
                                                static_cast<int>(ColorOverridableButtonTypes::Maximize),
                                                colorsFlags,
                                                colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsMaximizeFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsMaximize());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive,
                                                static_cast<int>(ColorOverridableButtonTypes::Minimize),
                                                colorsFlags,
                                                colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsMinimizeFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsMinimize());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::Help), colorsFlags, colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsHelpFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsHelp());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::Shade), colorsFlags, colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsShadeFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsShade());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive,
                                                static_cast<int>(ColorOverridableButtonTypes::AllDesktops),
                                                colorsFlags,
                                                colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsAllDesktopsFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsAllDesktops());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive,
                                                static_cast<int>(ColorOverridableButtonTypes::KeepBelow),
                                                colorsFlags,
                                                colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsKeepBelowFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsKeepBelow());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive,
                                                static_cast<int>(ColorOverridableButtonTypes::KeepAbove),
                                                colorsFlags,
                                                colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsKeepAboveFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsKeepAbove());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive,
                                                static_cast<int>(ColorOverridableButtonTypes::ApplicationMenu),
                                                colorsFlags,
                                                colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsApplicationMenuFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsApplicationMenu());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::Menu), colorsFlags, colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsMenuFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsMenu());
        }
    } else {
        if (!modified && m_overrideColorsLoaded && !m_ui->buttonColorActiveOverrideToggle->isChecked())
            modified = true;
    }

    setChanged(modified);
}

void ButtonColors::setApplyButtonState(const bool on)
{
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

void ButtonColors::setNegativeCloseBackgroundHoverPressState()
{
    if (m_ui->buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
        || m_ui->buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose) {
        m_ui->negativeCloseBackgroundHoverPress->setText(i18n("Negative close on hover/press only"));
        m_ui->negativeCloseBackgroundHoverPress->setVisible(true);
    } else if (m_ui->buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
        m_ui->negativeCloseBackgroundHoverPress->setText(i18n("Traffic lights on hover/press only"));
        m_ui->negativeCloseBackgroundHoverPress->setVisible(true);
    } else {
        m_ui->negativeCloseBackgroundHoverPress->setVisible(false);
    }
}

void ButtonColors::setCloseIconNegativeBackgroundState()
{
    // TODO: need behavioural logic here too
    if (m_ui->buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
        || m_ui->buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose
        || m_ui->buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
        m_ui->closeIconNegativeBackground->setVisible(true);
        m_ui->closeIconNegativeBackgroundLabel->setVisible(true);
    } else {
        m_ui->closeIconNegativeBackground->setVisible(false);
        m_ui->closeIconNegativeBackgroundLabel->setVisible(false);
    }
}

void ButtonColors::resizeOverrideColorTable()
{
    m_ui->overrideColorTableActive->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_ui->overrideColorTableActive->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void ButtonColors::showActiveOverrideGroupBox(const bool value)
{
    if (!value) {
        m_ui->activeOverrideGroupBox->setVisible(value);
    } else {
        m_ui->activeOverrideGroupBox->setVisible(value);
    }
}

void ButtonColors::resizeActiveOverrideGroupBox(const bool value)
{
    if (!value) {
        this->setMaximumSize(755, 400); // TODO: report the window corruption that occurs here as a kwin bug
        this->showNormal();
    } else {
        this->setMaximumSize(16777215, 16777215);
        this->showMaximized();
    }
}

void ButtonColors::activeTableHorizontalHeaderSectionClicked(const int column)
{
    QTableWidgetItem *item = m_ui->overrideColorTableActive->horizontalHeaderItem(column);
    if (!item)
        return;
    // toggle checked state
    if (item->checkState() == Qt::CheckState::Checked) {
        setTableHorizontalHeaderSectionCheckedState(column, false);
    } else {
        setTableHorizontalHeaderSectionCheckedState(column, true);
    }
}

void ButtonColors::setTableHorizontalHeaderSectionCheckedState(const int column, const bool checked)
{
    QTableWidgetItem *item = m_ui->overrideColorTableActive->horizontalHeaderItem(column);
    if (!item)
        return;
    item->setCheckState(checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    item->setIcon(checked ? m_lockedIcon : m_unlockedIcon);
}

// given a checkBox sending a toggle signal to this slot, gets the table cell in which the checkBox was located and copies the same data to the equivalent
// inactive window table cell and to the rest of the column if the column lock is set
void ButtonColors::copyCellDataToOtherCells()
{
    if (m_loading || m_processingDefaults)
        return; // only do this for user interactions
    KColorButton *colorButton = qobject_cast<KColorButton *>(QObject::sender());
    if (colorButton) {
        bool rowOk, columnOk;
        int row = colorButton->property("row").toInt(&rowOk);
        int column = colorButton->property("column").toInt(&columnOk);

        if (rowOk && columnOk) {
            if (colorButton->parentWidget() && colorButton->parentWidget()->parentWidget() && colorButton->parentWidget()->parentWidget()->parentWidget()) {
                QTableWidget *table = qobject_cast<QTableWidget *>(colorButton->parentWidget()->parentWidget()->parentWidget());
                if (table) {
                    QTableWidgetItem *item = table->horizontalHeaderItem(column);
                    if (item) {
                        if (item->checkState() == Qt::CheckState::Checked) { // if the lock icon is locked for the column
                            for (int destinationRow = 0; destinationRow < m_colorOverridableButtonTypesStrings.count(); destinationRow++) {
                                QCheckBox *destinationCheckBox = nullptr;
                                KColorButton *destinationColorButton = nullptr;

                                if (destinationRow != row) { // copy the values to all other colorBoxes in the column
                                    if (checkBoxAndColorButtonAtTableCell(table, destinationRow, column, destinationCheckBox, destinationColorButton)) {
                                        // disconnect(destinationColorButton, &KColorButton::changed, this, &ButtonColors::copyCellDataToOtherCells);
                                        // //disconnect the signals that call this function from the other cells to prevent this function being called many
                                        // times
                                        destinationColorButton->setColor(colorButton->color());
                                        destinationCheckBox->setChecked(true);
                                        // connect(destinationColorButton, &KColorButton::changed, this, &ButtonColors::copyCellDataToOtherCells); //re-enable
                                        // the connection
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

bool ButtonColors::checkBoxAndColorButtonAtTableCell(QTableWidget *table,
                                                     const int row,
                                                     const int column,
                                                     QCheckBox *&outputCheckBox,
                                                     KColorButton *&outputColorButton)
{
    outputCheckBox = nullptr;
    outputColorButton = nullptr;

    if (!table)
        return false;
    QWidget *widget = table->cellWidget(row, column);
    if (!widget)
        return false;
    if (!widget->children().count())
        return false;

    QCheckBox *checkBox = nullptr;
    for (const auto &child : widget->children()) {
        checkBox = qobject_cast<QCheckBox *>(child);
        if (checkBox) {
            outputCheckBox = checkBox;
            break;
        }
    }
    if (!outputCheckBox)
        return false;

    KColorButton *colorButton = nullptr;
    for (const auto &child : widget->children()) {
        colorButton = qobject_cast<KColorButton *>(child);
        if (colorButton) {
            outputColorButton = colorButton;
            break;
        }
    }

    if (outputCheckBox && outputColorButton) {
        return true;
    }
    return false;
}

// colorsFlags storage bits: Active Window colours: bits 0-15, Inactive Window colours bits 16-31
// active icon bits 0-3, active background bits 4-7, active outline bits 8-11
// inactive icon bits 16-19, inactive background bits 20-23, inactive outline bits 24-27
// bit 0 etc. reserved for deactivated state colour
void ButtonColors::encodeColorOverridableButtonTypeRow(QTableWidget *tableActive, int row, uint32_t &colorsFlags, QList<int> &colorsList)
{
    uint32_t bitMask = 0x00000001;
    colorsFlags = 0x00000000;
    uint32_t validColorsFlagsBits = 0x0EEE0EEE;
    uint32_t validColorsFlagsBitsActive = 0x0000FFFF;
    QTableWidget **table;

    colorsList.clear();
    for (uint32_t i = 0, columnIndex = 0; i < (sizeof(validColorsFlagsBits) * CHAR_BIT); i++, bitMask = bitMask << 1) {
        if (validColorsFlagsBits & bitMask) {
            QCheckBox *overrideCheckBox = nullptr;
            KColorButton *overrideColorButton = nullptr;
            if (validColorsFlagsBitsActive & bitMask) {
                table = &tableActive;
            } else {
                return;
            }
            if (checkBoxAndColorButtonAtTableCell(*table, row, columnIndex, overrideCheckBox, overrideColorButton)) {
                if (overrideCheckBox->isChecked()) { // set the bit of the column by OR-ing the bitmask with the existing value
                    colorsFlags = colorsFlags | bitMask;
                    colorsList.append(static_cast<int>(overrideColorButton->color().rgba())); // convert the colour to an int and store on the intList
                } else { // clear the bit by AND-ing with the inverse of the bitmask
                    colorsFlags = colorsFlags & (~bitMask);
                }
            }
            columnIndex++;
        }
    }
}

bool ButtonColors::decodeColorsFlagsAndLoadRow(QTableWidget *tableActive, int row, uint32_t colorsFlags, const QList<int> &colorsList)
{
    uint32_t bitMask = 0x00000001;
    uint32_t validColorsFlagsBits = 0x0EEE0EEE;
    uint32_t validColorsFlagsBitsActive = 0x0000FFFF;
    QTableWidget **table;
    int colorsListIndex = 0;
    bool rowHasOverrideColorsLoaded = false;
    for (uint32_t i = 0, columnIndex = 0; i < (sizeof(validColorsFlagsBits) * CHAR_BIT); i++, bitMask = bitMask << 1) {
        if (validColorsFlagsBits & bitMask) {
            QCheckBox *overrideCheckBox = nullptr;
            KColorButton *overrideColorButton = nullptr;
            if (validColorsFlagsBitsActive & bitMask) {
                table = &tableActive;
            } else {
                return rowHasOverrideColorsLoaded;
            }
            if (checkBoxAndColorButtonAtTableCell(*table, row, columnIndex, overrideCheckBox, overrideColorButton)) {
                if (colorsFlags & bitMask) { // if the current bit in colorsFlags is 1
                    rowHasOverrideColorsLoaded = true;
                    overrideCheckBox->setChecked(true);
                    if (colorsList.count() && colorsListIndex < colorsList.count()) { // this if is to prevent against an unlikely corruption situation when
                                                                                      // colorsSet and colorsist are out of sync
                        QRgb color = QRgb(static_cast<QRgb>(colorsList[colorsListIndex++]));
                        QColor qcolor(color);
                        qcolor.setAlpha(qAlpha(color));
                        overrideColorButton->setColor(qcolor);
                    }
                } else {
                    overrideCheckBox->setChecked(false);
                    overrideColorButton->setColor(QColor());
                }
            }
            columnIndex++;
        }
    }
    return rowHasOverrideColorsLoaded;
}

uint32_t ButtonColors::encodeColorOverridableLockStates()
{
    uint32_t bitMask = 0x00000001;
    uint32_t lockFlags = 0x00000000;
    uint32_t validColorsFlagsBits = 0x0EEE0EEE;
    uint32_t validColorsFlagsBitsActive = 0x0000FFFF;
    QTableWidget **table;

    for (uint32_t i = 0, columnIndex = 0; i < (sizeof(validColorsFlagsBits) * CHAR_BIT); i++, bitMask = bitMask << 1) {
        if (validColorsFlagsBits & bitMask) {
            if (validColorsFlagsBitsActive & bitMask) {
                table = &m_ui->overrideColorTableActive;
            } else {
                return lockFlags;
            }
            QTableWidgetItem *item = (*table)->horizontalHeaderItem(columnIndex);
            if (item) {
                if (item->checkState() == Qt::CheckState::Checked) { // set the bit of the column by OR-ing the bitmask with the existing value
                    lockFlags = lockFlags | bitMask;
                } else { // clear the bit by AND-ing with the inverse of the bitmask
                    lockFlags = lockFlags & (~bitMask);
                }
            }
            columnIndex++;
        }
    }
    return lockFlags;
}

bool ButtonColors::decodeColorOverridableLockStatesAndLoadHorizontalHeaderLocks()
{
    uint32_t bitMask = 0x00000001;
    uint32_t validColorsFlagsBits = 0x0EEE0EEE;
    uint32_t validColorsFlagsBitsActive = 0x0000FFFF;
    QTableWidget *tableActive = m_ui->overrideColorTableActive;
    QTableWidget **table;
    bool rowHasOverrideColorsLoaded = false;
    uint32_t lockFlags = m_internalSettings->buttonOverrideColorsLockStates();
    for (uint32_t i = 0, columnIndex = 0; i < (sizeof(validColorsFlagsBits) * CHAR_BIT); i++, bitMask = bitMask << 1) {
        if (validColorsFlagsBits & bitMask) {
            if (validColorsFlagsBitsActive & bitMask) {
                table = &tableActive;
            } else {
                return rowHasOverrideColorsLoaded;
            }
            QTableWidgetItem *item = (*table)->horizontalHeaderItem(columnIndex);
            if (item) {
                if (lockFlags & bitMask) { // if the current bit in lockFlags is 1
                    setTableHorizontalHeaderSectionCheckedState(columnIndex, true);
                } else {
                    setTableHorizontalHeaderSectionCheckedState(columnIndex, false);
                }
            }
            columnIndex++;
        }
    }
    return rowHasOverrideColorsLoaded;
}

void ButtonColors::setButtonBackgroundColorsIcons() // need to adjust for inactive as well
{
    if (!m_decorationColors)
        m_decorationColors = std::make_shared<DecorationColors>(QApplication::palette(),
                                                                m_internalSettings,
                                                                true,
                                                                true); // get decoration colors and cache globally - may need to move this up to
                                                                       // breezeconfigwidget.cpp later if want colours in other areas of the UI

    QColor titlebarTextActive;
    QColor titlebarTextInactive;
    QColor titlebarBackgroundActive;
    QColor titlebarBackgroundInactive;
    // get the titlebar text colour
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    bool colorSchemeHasHeaderColor = KColorScheme::isColorSetSupported(config, KColorScheme::Header);

    // get the alpha values from the system colour scheme
    if (colorSchemeHasHeaderColor) {
        KColorScheme activeHeader = KColorScheme(QPalette::Active, KColorScheme::Header, config);
        titlebarTextActive = activeHeader.foreground().color();
        titlebarBackgroundActive = activeHeader.background().color();

        KColorScheme inactiveHeader = KColorScheme(QPalette::Inactive, KColorScheme::Header, config);
        titlebarTextInactive = inactiveHeader.foreground().color();
        titlebarBackgroundInactive = inactiveHeader.background().color();
    } else {
        KConfigGroup wmConfig(config, QStringLiteral("WM"));
        if (wmConfig.exists()) {
            titlebarTextActive = wmConfig.readEntry("activeForeground", QColorConstants::Black);
            titlebarTextInactive = wmConfig.readEntry("inactiveForeground", QColorConstants::Black);
            titlebarBackgroundActive = wmConfig.readEntry("activeBackground", QColorConstants::Black);
            titlebarBackgroundInactive = wmConfig.readEntry("inactiveBackground", QColorConstants::Black);
        }
    }

    qreal size = 32;

    QRectF wholeRect(QPointF(0, 0), QPointF(size, size));

    QRectF topRect(QPointF(0, 0), QPointF(size, size / 2));
    QRectF bottomRect(QPointF(0, size / 2), QPointF(size, size));

    QRectF leftRect(QPointF(0, 0), QPointF(size / 2, size));
    QRectF RightRect(QPointF(size / 2, 0), QPointF(size, size));

    QRectF topLeftRect(QPointF(0, 0), QPointF(size / 2, size / 2));
    QRectF topRightRect(QPointF(size / 2, 0), QPointF(size, size / 2));
    QRectF bottomRightRect(QPointF(size / 2, size / 2), QPointF(size, size));
    QRectF bottomLeftRect(QPointF(0, size / 2), QPointF(size / 2, size));

    QRectF leftQuarter(QPointF(0, 0), QPointF(size / 4, size));
    QRectF leftMidQuarter(QPointF(size / 4, 0), QPointF(size / 2, size));
    QRectF rightMidQuarter(QPointF(size / 2, 0), QPointF(size * 3 / 4, size));
    QRectF rightQuarter(QPointF(size * 3 / 4, 0), QPointF(size, size));

    QRectF oneByThree00(QPointF(0, 0), QPointF(size, size / 3));
    QRectF oneByThree01(QPointF(0, size / 3), QPointF(size, size * 2 / 3));
    QRectF oneByThree02(QPointF(0, size * 2 / 3), QPointF(size, size));

    QRectF twoByThree00(QPointF(0, 0), QPointF(size / 2, size / 3));
    QRectF twoByThree01(QPointF(0, size / 3), QPointF(size / 2, size * 2 / 3));
    QRectF twoByThree02(QPointF(0, size * 2 / 3), QPointF(size / 2, size));
    QRectF twoByThree10(QPointF(size / 2, 0), QPointF(size, size / 3));
    QRectF twoByThree11(QPointF(size / 2, size / 3), QPointF(size, size * 2 / 3));
    QRectF twoByThree12(QPointF(size / 2, size * 2 / 3), QPointF(size, size));

    QRectF fourByThree00(QPointF(0, 0), QPointF(size / 4, size / 3));
    QRectF fourByThree01(QPointF(0, size / 3), QPointF(size / 4, size * 2 / 3));
    QRectF fourByThree02(QPointF(0, size * 2 / 3), QPointF(size / 4, size));
    QRectF fourByThree10(QPointF(size / 4, 0), QPointF(size / 2, size / 3));
    QRectF fourByThree11(QPointF(size / 4, size / 3), QPointF(size / 2, size * 2 / 3));
    QRectF fourByThree12(QPointF(size / 4, size * 2 / 3), QPointF(size / 2, size));
    QRectF fourByThree20(QPointF(size / 2, 0), QPointF(size * 3 / 4, size / 3));
    QRectF fourByThree21(QPointF(size / 2, size / 3), QPointF(size * 3 / 4, size * 2 / 3));
    QRectF fourByThree22(QPointF(size / 2, size * 2 / 3), QPointF(size * 3 / 4, size));
    QRectF fourByThree30(QPointF(size * 3 / 4, 0), QPointF(size, size / 3));
    QRectF fourByThree31(QPointF(size * 3 / 4, size / 3), QPointF(size, size * 2 / 3));
    QRectF fourByThree32(QPointF(size * 3 / 4, size * 2 / 3), QPointF(size, size));

    QRectF fourByTwo00(QPointF(0, 0), QPointF(size / 4, size / 2));
    QRectF fourByTwo01(QPointF(size / 4, 0), QPointF(size / 2, size / 2));
    QRectF fourByTwo02(QPointF(size / 2, 0), QPointF(size * 3 / 4, size / 2));
    QRectF fourByTwo03(QPointF(size * 3 / 4, 0), QPointF(size, size / 2));
    QRectF fourByTwo10(QPointF(0, size / 2), QPointF(size / 4, size));
    QRectF fourByTwo11(QPointF(size / 4, size / 2), QPointF(size / 2, size));
    QRectF fourByTwo12(QPointF(size / 2, size / 2), QPointF(size * 3 / 4, size));
    QRectF fourByTwo13(QPointF(size * 3 / 4, size / 2), QPointF(size, size));

    QPixmap pixmap(size, size);
    // pixmap.setDevicePixelRatio(this->window()->devicePixelRatioF());
    std::unique_ptr<QPainter> painter = std::make_unique<QPainter>(&pixmap);
    painter->setPen(Qt::NoPen);

    // background colors
    m_ui->buttonBackgroundColors->setIconSize(QSize(size, size));

    painter->setBrush(titlebarTextActive);
    painter->drawRect(oneByThree00);
    painter->setBrush(KColorUtils::mix(titlebarBackgroundActive, titlebarTextActive, 0.6));
    painter->drawRect(oneByThree01);
    painter->setBrush(KColorUtils::mix(titlebarBackgroundActive, titlebarTextActive, 0.3));
    painter->drawRect(oneByThree02);
    QIcon titlebarText(pixmap);
    m_ui->buttonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::TitlebarText, titlebarText);

    pixmap.fill(Qt::transparent);
    painter->setBrush(titlebarTextActive);
    painter->drawRect(twoByThree00);
    painter->setBrush(KColorUtils::mix(titlebarBackgroundActive, titlebarTextActive, 0.6));
    painter->drawRect(twoByThree01);
    painter->setBrush(KColorUtils::mix(titlebarBackgroundActive, titlebarTextActive, 0.3));
    painter->drawRect(twoByThree02);
    painter->setBrush(m_decorationColors->negativeLessSaturated());
    painter->drawRect(twoByThree10);
    painter->setBrush(m_decorationColors->negativeSaturated());
    painter->drawRect(twoByThree11);
    painter->setBrush(m_internalSettings->negativeCloseBackgroundHoverPress() ? KColorUtils::mix(titlebarBackgroundActive, titlebarTextActive, 0.3)
                                                                              : m_decorationColors->negative());
    painter->drawRect(twoByThree12);
    QIcon backgroundTitlebarTextNegativeClose(pixmap);
    m_ui->buttonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose, backgroundTitlebarTextNegativeClose);

    pixmap.fill(Qt::transparent);
    painter->setBrush(m_decorationColors->buttonFocus());
    painter->drawRect(oneByThree00);
    painter->setBrush(m_decorationColors->buttonHover());
    painter->drawRect(oneByThree01);
    painter->setBrush(KColorUtils::mix(titlebarBackgroundActive, m_decorationColors->buttonHover(), 0.8)); // TODO:inactive
    painter->drawRect(oneByThree02);
    QIcon backgroundAccent(pixmap);
    m_ui->buttonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::Accent, backgroundAccent);

    pixmap.fill(Qt::transparent);
    painter->setBrush(m_decorationColors->buttonFocus());
    painter->drawRect(twoByThree00);
    painter->setBrush(m_decorationColors->buttonHover());
    painter->drawRect(twoByThree01);
    painter->setBrush(KColorUtils::mix(titlebarBackgroundActive, m_decorationColors->buttonHover(), 0.8)); // TODO:inactive
    painter->drawRect(twoByThree02);
    painter->setBrush(m_decorationColors->negativeLessSaturated());
    painter->drawRect(twoByThree10);
    painter->setBrush(m_decorationColors->negativeSaturated());
    painter->drawRect(twoByThree11);
    painter->setBrush(m_internalSettings->negativeCloseBackgroundHoverPress()
                          ? KColorUtils::mix(titlebarBackgroundActive, m_decorationColors->buttonHover(), 0.8)
                          : m_decorationColors->negative());
    painter->drawRect(twoByThree12);
    QIcon backgroundAccentNegativeClose(pixmap);
    m_ui->buttonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose, backgroundAccentNegativeClose);

    pixmap.fill(Qt::transparent);
    painter->setBrush(m_decorationColors->buttonFocus());
    painter->drawRect(fourByThree00);
    painter->setBrush(m_decorationColors->buttonHover());
    painter->drawRect(fourByThree01);
    painter->setBrush(KColorUtils::mix(titlebarBackgroundActive, m_decorationColors->buttonHover(), 0.8)); // TODO:inactive
    painter->drawRect(fourByThree02);
    painter->setBrush(m_decorationColors->neutralSaturated());
    painter->drawRect(fourByThree10);
    painter->setBrush(m_decorationColors->neutral());
    painter->drawRect(fourByThree11);
    painter->setBrush(m_internalSettings->negativeCloseBackgroundHoverPress()
                          ? KColorUtils::mix(titlebarBackgroundActive, m_decorationColors->buttonHover(), 0.8)
                          : m_decorationColors->neutralLessSaturated());
    painter->drawRect(fourByThree12);
    painter->setBrush(m_decorationColors->positiveSaturated());
    painter->drawRect(fourByThree20);
    painter->setBrush(m_decorationColors->positive());
    painter->drawRect(fourByThree21);
    painter->setBrush(m_internalSettings->negativeCloseBackgroundHoverPress()
                          ? KColorUtils::mix(titlebarBackgroundActive, m_decorationColors->buttonHover(), 0.8)
                          : m_decorationColors->positiveLessSaturated());
    painter->drawRect(fourByThree22);
    painter->setBrush(m_decorationColors->negativeLessSaturated());
    painter->drawRect(fourByThree30);
    painter->setBrush(m_decorationColors->negativeSaturated());
    painter->drawRect(fourByThree31);
    painter->setBrush(m_internalSettings->negativeCloseBackgroundHoverPress()
                          ? KColorUtils::mix(titlebarBackgroundActive, m_decorationColors->buttonHover(), 0.8)
                          : m_decorationColors->negative());
    painter->drawRect(fourByThree32);
    QIcon backgroundAccentTrafficLightsClose(pixmap);
    m_ui->buttonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights, backgroundAccentTrafficLightsClose);

    // icon colors
    m_ui->buttonIconColors->setIconSize(QSize(24, 24));
    pixmap.fill(Qt::transparent);
    painter->setBrush(titlebarTextActive); // TODO:add inactive
    painter->drawRect(wholeRect);
    QIcon iconTitlebarText(pixmap);
    m_ui->buttonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::TitlebarText, iconTitlebarText);

    pixmap.fill(Qt::transparent);
    painter->setBrush(titlebarTextActive); // TODO:add inactive
    painter->drawRect(leftRect);
    painter->setBrush(m_decorationColors->negativeSaturated());
    painter->drawRect(RightRect);
    QIcon iconTitlebarTextNegativeClose(pixmap);
    m_ui->buttonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose, iconTitlebarTextNegativeClose);

    pixmap.fill(Qt::transparent);
    painter->setBrush(m_decorationColors->highlight());
    painter->drawRect(wholeRect);
    QIcon iconAccent(pixmap);
    m_ui->buttonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::Accent, iconAccent);

    pixmap.fill(Qt::transparent);
    painter->setBrush(m_decorationColors->highlight());
    painter->drawRect(leftRect);
    painter->setBrush(m_decorationColors->negativeSaturated());
    painter->drawRect(RightRect);
    QIcon iconAccentNegativeClose(pixmap);
    m_ui->buttonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::AccentNegativeClose, iconAccentNegativeClose);

    pixmap.fill(Qt::transparent);
    painter->setBrush(m_decorationColors->highlight());
    painter->drawRect(leftQuarter);
    painter->setBrush(m_decorationColors->neutral());
    painter->drawRect(leftMidQuarter);
    painter->setBrush(m_decorationColors->positiveSaturated());
    painter->drawRect(rightMidQuarter);
    painter->setBrush(m_decorationColors->negativeSaturated());
    painter->drawRect(rightQuarter);
    QIcon iconAccentTrafficLights(pixmap);
    m_ui->buttonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::AccentTrafficLights, iconAccentTrafficLights);

    // closeIconNegativeBackground
    QColor closeIconColor;
    QColor othersIconColor;
    switch (m_internalSettings->buttonIconColors()) {
    case InternalSettings::EnumButtonIconColors::TitlebarText:
        closeIconColor = titlebarTextActive; // TODO:inactive
        othersIconColor = titlebarTextActive;
        break;
    case InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose:
        closeIconColor = m_decorationColors->negativeSaturated();
        othersIconColor = titlebarTextActive;
        break;
    case InternalSettings::EnumButtonIconColors::Accent:
        closeIconColor = m_decorationColors->highlight();
        othersIconColor = m_decorationColors->highlight();
        break;
    case InternalSettings::EnumButtonIconColors::AccentNegativeClose:
        closeIconColor = m_decorationColors->negativeSaturated();
        othersIconColor = m_decorationColors->highlight();
        break;
    case InternalSettings::EnumButtonIconColors::AccentTrafficLights:
        closeIconColor = m_decorationColors->negativeSaturated();
        othersIconColor = m_decorationColors->highlight();
        break;
    }

    m_ui->closeIconNegativeBackground->setIconSize(QSize(16, 16));

    pixmap.fill(Qt::transparent);
    painter->setBrush(othersIconColor);
    painter->drawRect(wholeRect);
    QIcon closeIconNegativeBackgroundSame(pixmap);
    m_ui->closeIconNegativeBackground->setItemIcon(InternalSettings::EnumCloseIconNegativeBackground::Same, closeIconNegativeBackgroundSame);

    pixmap.fill(Qt::transparent);
    painter->setBrush(Qt::GlobalColor::white);
    painter->drawRect(wholeRect);
    QIcon closeIconNegativeBackgroundWhite(pixmap);
    m_ui->closeIconNegativeBackground->setItemIcon(InternalSettings::EnumCloseIconNegativeBackground::White, closeIconNegativeBackgroundWhite);

    pixmap.fill(Qt::transparent);
    painter->setBrush(Qt::GlobalColor::white);
    painter->drawRect(topRect);
    painter->setBrush(closeIconColor);
    painter->drawRect(bottomRect);
    QIcon closeIconNegativeBackgroundWhiteWhenHoverPress(pixmap);
    m_ui->closeIconNegativeBackground->setItemIcon(InternalSettings::EnumCloseIconNegativeBackground::WhiteWhenHoverPress,
                                                   closeIconNegativeBackgroundWhiteWhenHoverPress);
}

bool ButtonColors::event(QEvent *ev)
{
    if (ev->type() == QEvent::ApplicationPaletteChange) {
        // overwrite handling of palette change
        m_decorationColors->generateDecorationColors(QApplication::palette(), m_internalSettings);

        if (!m_loaded)
            load();
        else
            setButtonBackgroundColorsIcons();
        return QWidget::event(ev);
    }
    // Make sure the rest of events are handled
    return QWidget::event(ev);
}
}
