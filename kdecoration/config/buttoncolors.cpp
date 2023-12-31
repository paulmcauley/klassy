/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "buttoncolors.h"
#include "breezeconfigwidget.h"
#include "presetsmodel.h"
#include <KColorUtils>
#include <KDecoration2/DecorationSettings>
#include <QCheckBox>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QHBoxLayout>
#include <QMutableListIterator>
#include <QPushButton>
#include <QSlider>
#include <QWindow>
#include <memory>

namespace Breeze
{

using KDecoration2::DecorationButtonType;

ButtonColors::ButtonColors(KSharedConfig::Ptr config, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui_ButtonColors)
    , m_configuration(config)
{
    m_ui->setupUi(this);

    getButtonsOrderFromKwinConfig();

    this->resize(755, 400);
    m_ui->activeOverrideGroupBox->setVisible(false);
    connect(m_ui->buttonColorActiveOverrideToggle, &QAbstractButton::toggled, this, &ButtonColors::showActiveOverrideGroupBox);
    connect(m_ui->buttonColorActiveOverrideToggle, &QAbstractButton::clicked, this, &ButtonColors::resizeActiveOverrideGroupBox);

    int numRows = m_overridableButtonColorStatesStrings.count();

    // generate the vertical header
    m_unlockedIcon.addFile(QStringLiteral(":/klassy_config_icons/object-unlocked-symbolic.svg"), QSize(16, 16));
    m_lockedIcon.addFile(QStringLiteral(":/klassy_config_icons/object-locked-symbolic.svg"), QSize(16, 16));
    // m_unlockedIcon = QIcon::fromTheme(QStringLiteral("unlock"));
    // m_lockedIcon = QIcon::fromTheme(QStringLiteral("lock"));
    for (int rowIndex = 0; rowIndex < numRows; rowIndex++) {
        QTableWidgetItem *verticalHeaderItem = new QTableWidgetItem();
        verticalHeaderItem->setText(m_overridableButtonColorStatesStrings[rowIndex]);
        verticalHeaderItem->setIcon(m_unlockedIcon);
        verticalHeaderItem->setToolTip(i18n("Lock to make all colours in this row the same"));
        verticalHeaderItem->setData(Qt::InitialSortOrderRole, rowIndex);
        m_ui->overrideColorTableActive->insertRow(rowIndex);
        m_ui->overrideColorTableActive->setVerticalHeaderItem(rowIndex, verticalHeaderItem);
    }

    connect(m_ui->overrideColorTableActive->verticalHeader(), &QHeaderView::sectionClicked, this, &ButtonColors::activeTableVerticalHeaderSectionClicked);
    connect(m_ui->overrideColorTableActive->verticalHeader(), &QHeaderView::sectionClicked, this, &ButtonColors::updateChanged);

    // gnerate the overrideColorTableActive table UI
    // populate the checkboxes and KColorButtons
    for (int columnIndex = 0; columnIndex < m_colorOverridableButtonTypesStrings.count(); columnIndex++) {
        m_ui->overrideColorTableActive->insertColumn(columnIndex);
        for (int rowIndex = 0; rowIndex < numRows; rowIndex++) {
            QHBoxLayout *hlayout = new QHBoxLayout();
            hlayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
            QCheckBox *checkBox = new QCheckBox();
            checkBox->setProperty("column", columnIndex);
            checkBox->setProperty("row", rowIndex);
            hlayout->addWidget(checkBox);
            KColorButton *colorButton = new KColorButton();
            colorButton->setProperty("column", columnIndex);
            colorButton->setProperty("row", rowIndex);
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

    // populate the horizontal header
    QStringList orderedHorizontalHeaderLabels;
    for (auto buttonType : m_allCustomizableButtonsOrder) { // get the horizontal header labels in the correct user-set order
        orderedHorizontalHeaderLabels.append(m_colorOverridableButtonTypesStrings.value(buttonType));
    }
    m_ui->overrideColorTableActive->setHorizontalHeaderLabels(orderedHorizontalHeaderLabels);
    m_ui->overrideColorTableActive->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    m_ui->overrideColorTableActive->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    // set the header widths to the same as the largest section
    int largestSection = 0;
    for (int columnIndex = 0; columnIndex < m_colorOverridableButtonTypesStrings.count(); columnIndex++) {
        int sectionSize = m_ui->overrideColorTableActive->horizontalHeader()->sectionSize(columnIndex);
        if (sectionSize > largestSection)
            largestSection = sectionSize;
    }
    m_ui->overrideColorTableActive->horizontalHeader()->setMinimumSectionSize(largestSection);

    // track ui changes
    connect(m_ui->buttonIconColors,
            SIGNAL(currentIndexChanged(int)),
            SLOT(refreshCloseButtonIconColorState())); // important that refreshCloseButtonIconColorState is before updateChangeds
    connect(m_ui->buttonIconColors, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->closeButtonIconColor, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->buttonBackgroundColors, SIGNAL(currentIndexChanged(int)), SLOT(setNegativeCloseBackgroundHoverPressState()));
    connect(m_ui->buttonBackgroundColors, SIGNAL(currentIndexChanged(int)), SLOT(refreshCloseButtonIconColorState()));
    connect(m_ui->buttonBackgroundColors, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->translucentButtonBackgrounds, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->translucentButtonBackgrounds, &QAbstractButton::toggled, this, &ButtonColors::showHideTranslucencySettings);
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
    m_ui->buttonBackgroundColors->setCurrentIndex(m_internalSettings->buttonBackgroundColors());
    m_ui->translucentButtonBackgrounds->setChecked(m_internalSettings->translucentButtonBackgrounds());
    m_ui->negativeCloseBackgroundHoverPress->setChecked(m_internalSettings->negativeCloseBackgroundHoverPress());
    m_ui->lockButtonColorsActive->setChecked(m_internalSettings->lockButtonColorsActive());
    m_ui->blackWhiteIconOnPoorContrast->setChecked(m_internalSettings->blackWhiteIconOnPoorContrast());
    m_ui->adjustBackgroundColorOnPoorContrast->setChecked(m_internalSettings->adjustBackgroundColorOnPoorContrast());
    m_ui->translucentButtonBackgroundsOpacity->setValue(m_internalSettings->translucentButtonBackgroundsOpacity() * 100);

    setNegativeCloseBackgroundHoverPressState();
    refreshCloseButtonIconColorState();
    loadCloseButtonIconColor(); // refreshCloseButtonIconColorState must occur before loading closeButtonIconColor
    showHideTranslucencySettings();
    decodeColorOverridableLockStatesAndLoadVerticalHeaderLocks();

    m_overrideColorsLoaded = false;
    bool overrideColorColumnLoaded = false;

    overrideColorColumnLoaded = decodeColorsFlagsAndLoadColumn(m_ui->overrideColorTableActive,
                                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::Close),
                                                               m_internalSettings->buttonOverrideColorsCloseFlags(),
                                                               m_internalSettings->buttonOverrideColorsClose());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorColumnLoaded;

    overrideColorColumnLoaded = decodeColorsFlagsAndLoadColumn(m_ui->overrideColorTableActive,
                                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::Maximize),
                                                               m_internalSettings->buttonOverrideColorsMaximizeFlags(),
                                                               m_internalSettings->buttonOverrideColorsMaximize());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorColumnLoaded;

    overrideColorColumnLoaded = decodeColorsFlagsAndLoadColumn(m_ui->overrideColorTableActive,
                                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::Minimize),
                                                               m_internalSettings->buttonOverrideColorsMinimizeFlags(),
                                                               m_internalSettings->buttonOverrideColorsMinimize());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorColumnLoaded;

    overrideColorColumnLoaded = decodeColorsFlagsAndLoadColumn(m_ui->overrideColorTableActive,
                                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::ContextHelp),
                                                               m_internalSettings->buttonOverrideColorsHelpFlags(),
                                                               m_internalSettings->buttonOverrideColorsHelp());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorColumnLoaded;

    overrideColorColumnLoaded = decodeColorsFlagsAndLoadColumn(m_ui->overrideColorTableActive,
                                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::Shade),
                                                               m_internalSettings->buttonOverrideColorsShadeFlags(),
                                                               m_internalSettings->buttonOverrideColorsShade());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorColumnLoaded;

    overrideColorColumnLoaded = decodeColorsFlagsAndLoadColumn(m_ui->overrideColorTableActive,
                                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::OnAllDesktops),
                                                               m_internalSettings->buttonOverrideColorsAllDesktopsFlags(),
                                                               m_internalSettings->buttonOverrideColorsAllDesktops());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorColumnLoaded;

    overrideColorColumnLoaded = decodeColorsFlagsAndLoadColumn(m_ui->overrideColorTableActive,
                                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::KeepBelow),
                                                               m_internalSettings->buttonOverrideColorsKeepBelowFlags(),
                                                               m_internalSettings->buttonOverrideColorsKeepBelow());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorColumnLoaded;

    overrideColorColumnLoaded = decodeColorsFlagsAndLoadColumn(m_ui->overrideColorTableActive,
                                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::KeepAbove),
                                                               m_internalSettings->buttonOverrideColorsKeepAboveFlags(),
                                                               m_internalSettings->buttonOverrideColorsKeepAbove());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorColumnLoaded;

    overrideColorColumnLoaded = decodeColorsFlagsAndLoadColumn(m_ui->overrideColorTableActive,
                                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::ApplicationMenu),
                                                               m_internalSettings->buttonOverrideColorsApplicationMenuFlags(),
                                                               m_internalSettings->buttonOverrideColorsApplicationMenu());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorColumnLoaded;

    overrideColorColumnLoaded = decodeColorsFlagsAndLoadColumn(m_ui->overrideColorTableActive,
                                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::Menu),
                                                               m_internalSettings->buttonOverrideColorsMenuFlags(),
                                                               m_internalSettings->buttonOverrideColorsMenu());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorColumnLoaded;

    m_ui->buttonColorActiveOverrideToggle->setChecked(m_overrideColorsLoaded);

    // add icons
    loadButtonBackgroundColorsIcons();

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
    m_internalSettings->setCloseButtonIconColor(convertCloseButtonIconColorUiToSettingsIndex(m_ui->closeButtonIconColor->currentIndex()));
    m_internalSettings->setButtonBackgroundColors(m_ui->buttonBackgroundColors->currentIndex());
    m_internalSettings->setTranslucentButtonBackgrounds(m_ui->translucentButtonBackgrounds->isChecked());
    m_internalSettings->setNegativeCloseBackgroundHoverPress(m_ui->negativeCloseBackgroundHoverPress->isChecked());
    m_internalSettings->setLockButtonColorsActive(m_ui->lockButtonColorsActive->isChecked());
    m_internalSettings->setBlackWhiteIconOnPoorContrast(m_ui->blackWhiteIconOnPoorContrast->isChecked());
    m_internalSettings->setAdjustBackgroundColorOnPoorContrast(m_ui->adjustBackgroundColorOnPoorContrast->isChecked());
    m_internalSettings->setTranslucentButtonBackgroundsOpacity(m_ui->translucentButtonBackgroundsOpacity->value() / 100.0f);

    QList<int> colorsList;
    uint32_t colorsFlags = 0;

    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        m_internalSettings->setButtonOverrideColorsLockStates(encodeColorOverridableLockStates());
    else
        m_internalSettings->setButtonOverrideColorsLockStates(0);

    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::Close),
                                               colorsFlags,
                                               colorsList);
    m_internalSettings->setButtonOverrideColorsCloseFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsClose(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::Maximize),
                                               colorsFlags,
                                               colorsList);
    m_internalSettings->setButtonOverrideColorsMaximizeFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsMaximize(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::Minimize),
                                               colorsFlags,
                                               colorsList);
    m_internalSettings->setButtonOverrideColorsMinimizeFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsMinimize(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::ContextHelp),
                                               colorsFlags,
                                               colorsList);
    m_internalSettings->setButtonOverrideColorsHelpFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsHelp(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::Shade),
                                               colorsFlags,
                                               colorsList);
    m_internalSettings->setButtonOverrideColorsShadeFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsShade(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::OnAllDesktops),
                                               colorsFlags,
                                               colorsList);
    m_internalSettings->setButtonOverrideColorsAllDesktopsFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsAllDesktops(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::KeepBelow),
                                               colorsFlags,
                                               colorsList);
    m_internalSettings->setButtonOverrideColorsKeepBelowFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsKeepBelow(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::KeepAbove),
                                               colorsFlags,
                                               colorsList);
    m_internalSettings->setButtonOverrideColorsKeepAboveFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsKeepAbove(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::ApplicationMenu),
                                               colorsFlags,
                                               colorsList);
    m_internalSettings->setButtonOverrideColorsApplicationMenuFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsApplicationMenu(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                               m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::Menu),
                                               colorsFlags,
                                               colorsList);
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
    setChanged(!isDefaults());

    m_processingDefaults = false;
    m_defaultsPressed = true;
}

bool ButtonColors::isDefaults()
{
    bool isDefaults = true;

    QString groupName(QStringLiteral("ButtonColors"));
    if (m_configuration->hasGroup(groupName)) {
        KConfigGroup group = m_configuration->group(groupName);
        if (group.keyList().count())
            isDefaults = false;
    }

    return isDefaults;
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
    // update the displayed colours any time the UI settings change
    loadButtonBackgroundColorsIcons();

    // check configuration
    if (!m_internalSettings)
        return;

    if (m_loading)
        return; // only check if the user has made a change to the UI, or user has pressed defaults

    // track modifications
    bool modified(false);
    if (m_ui->buttonIconColors->currentIndex() != m_internalSettings->buttonIconColors())
        modified = true;
    else if (convertCloseButtonIconColorUiToSettingsIndex(m_ui->closeButtonIconColor->currentIndex()) != m_internalSettings->closeButtonIconColor())
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
        if (!modified) {
            if (encodeColorOverridableLockStates() != m_internalSettings->buttonOverrideColorsLockStates())
                modified = true;
        }

        uint32_t colorsFlags;
        QList<int> colorsList;
        if (!modified) {
            encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                                   m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::Close),
                                                   colorsFlags,
                                                   colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsCloseFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsClose());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                                   m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::Maximize),
                                                   colorsFlags,
                                                   colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsMaximizeFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsMaximize());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                                   m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::Minimize),
                                                   colorsFlags,
                                                   colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsMinimizeFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsMinimize());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                                   m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::ContextHelp),
                                                   colorsFlags,
                                                   colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsHelpFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsHelp());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                                   m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::Shade),
                                                   colorsFlags,
                                                   colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsShadeFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsShade());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                                   m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::OnAllDesktops),
                                                   colorsFlags,
                                                   colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsAllDesktopsFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsAllDesktops());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                                   m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::KeepBelow),
                                                   colorsFlags,
                                                   colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsKeepBelowFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsKeepBelow());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                                   m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::KeepAbove),
                                                   colorsFlags,
                                                   colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsKeepAboveFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsKeepAbove());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                                   m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::ApplicationMenu),
                                                   colorsFlags,
                                                   colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsApplicationMenuFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsApplicationMenu());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                                   m_allCustomizableButtonsOrder.indexOf(DecorationButtonType::Menu),
                                                   colorsFlags,
                                                   colorsList);
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

void ButtonColors::refreshCloseButtonIconColorState()
{
    // As selected
    // Negative when hovered/pressed
    // White
    // White when hovered/pressed
    /*
        enum struct CloseButtonIconColorState{
            AsSelected = 1,
            NegativeWhenHoveredPressed = 2,
            White = 4,
            WhiteWhenHoveredPressed = 8
        }

    */

    uint32_t closeButtonIconColorState = static_cast<uint32_t>(CloseButtonIconColorState::AsSelected);
    bool visible = false;
    QString negativeWhenHoveredPressedString;
    QString whiteString;
    QString WhiteWhenHoveredPressedString;

    bool negativeCloseBackground = m_ui->buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
        || m_ui->buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose
        || m_ui->buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights;

    if (negativeCloseBackground) {
        visible = true;
        closeButtonIconColorState = closeButtonIconColorState | static_cast<uint32_t>(CloseButtonIconColorState::White)
            | static_cast<uint32_t>(CloseButtonIconColorState::WhiteWhenHoveredPressed);
        if (m_ui->buttonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
            m_ui->closeButtonIconColorLabel->setText(i18n("Traffic lights icon colours:"));
            whiteString = i18n("White close");
            WhiteWhenHoveredPressedString = i18n("White close when hovered/pressed");
        } else {
            m_ui->closeButtonIconColorLabel->setText(i18n("Close icon colour:"));
            whiteString = i18n("White");
            WhiteWhenHoveredPressedString = i18n("White when hovered/pressed");
        }
    }

    if (m_ui->buttonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::AccentNegativeClose
        || m_ui->buttonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose
        || m_ui->buttonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
        visible = true;
        closeButtonIconColorState = closeButtonIconColorState | static_cast<uint32_t>(CloseButtonIconColorState::NegativeWhenHoveredPressed);

        if (m_ui->buttonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
            negativeWhenHoveredPressedString = i18n("Traffic lights when hovered/pressed");
        } else {
            negativeWhenHoveredPressedString = i18n("Negative when hovered/pressed");
        }
    }

    QString closeIconAsSelectedString;
    switch (m_ui->buttonIconColors->currentIndex()) {
    case InternalSettings::EnumButtonIconColors::TitlebarText:
        closeIconAsSelectedString = i18n("Titlebar text");
        break;
    case InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose:
        closeIconAsSelectedString = i18n("Negative");
        break;
    case InternalSettings::EnumButtonIconColors::Accent:
        closeIconAsSelectedString = i18n("Accent");
        break;
    case InternalSettings::EnumButtonIconColors::AccentNegativeClose:
        closeIconAsSelectedString = i18n("Negative");
        break;
    case InternalSettings::EnumButtonIconColors::AccentTrafficLights:
        closeIconAsSelectedString = i18n("Traffic lights");
        break;
    }

    // remove all items from the closeButtonIconColor combobox
    int count = m_ui->closeButtonIconColor->count();
    for (int i = 0; i < count; i++) {
        m_ui->closeButtonIconColor->removeItem(0);
    }

    // add each item, depending on the state
    if (static_cast<uint32_t>(CloseButtonIconColorState::AsSelected) & closeButtonIconColorState) {
        m_ui->closeButtonIconColor->addItem(closeIconAsSelectedString);
    }

    if (static_cast<uint32_t>(CloseButtonIconColorState::NegativeWhenHoveredPressed) & closeButtonIconColorState) {
        m_ui->closeButtonIconColor->addItem(negativeWhenHoveredPressedString);
    }

    if (static_cast<uint32_t>(CloseButtonIconColorState::White) & closeButtonIconColorState) {
        m_ui->closeButtonIconColor->addItem(whiteString);
    }

    if (static_cast<uint32_t>(CloseButtonIconColorState::WhiteWhenHoveredPressed) & closeButtonIconColorState) {
        m_ui->closeButtonIconColor->addItem(WhiteWhenHoveredPressedString);
    }

    if (visible) {
        m_ui->closeButtonIconColor->setVisible(true);
        m_ui->closeButtonIconColorLabel->setVisible(true);
    } else {
        m_ui->closeButtonIconColor->setVisible(false);
        m_ui->closeButtonIconColorLabel->setVisible(false);
    }
    m_closeButtonIconColorState = closeButtonIconColorState;
}

int ButtonColors::convertCloseButtonIconColorUiToSettingsIndex(const int uiIndex)
{
    /*
    enum struct CloseButtonIconColorState{
        AsSelected = 1,
        NegativeWhenHoveredPressed = 2,
        White = 4,
        WhiteWhenHoveredPressed = 8
    }

*/

    uint32_t state = m_closeButtonIconColorState;
    uint32_t selectedStateBit = 1;
    int uiItemCount = 0;
    int numConfigItems = 4;

    for (int i = 0; i < numConfigItems; i++, selectedStateBit = selectedStateBit << 1) {
        if (state & selectedStateBit) {
            if (uiItemCount == uiIndex) {
                return i;
            }
            uiItemCount++;
        }
    }
    return -1;
}

int ButtonColors::convertCloseButtonIconColorSettingsToUiIndex(const int settingsIndex)
{
    /*
    enum struct CloseButtonIconColorState{
        AsSelected = 1,
        NegativeWhenHoveredPressed = 2,
        White = 4,
        WhiteWhenHoveredPressed = 8
    }

*/
    uint32_t state = m_closeButtonIconColorState;
    uint32_t selectedStateBit = 1;
    int uiItemCount = 0;
    int numConfigItems = 4;

    for (int i = 0; i < numConfigItems; i++, selectedStateBit = selectedStateBit << 1) {
        if (state & selectedStateBit) {
            if (i == settingsIndex) {
                return uiItemCount;
            }
            uiItemCount++;
        }
    }
    return -1;
}

void ButtonColors::loadCloseButtonIconColor()
{
    int uiIndex = convertCloseButtonIconColorSettingsToUiIndex(m_internalSettings->closeButtonIconColor());
    if (uiIndex < 0)
        uiIndex = 0;
    m_ui->closeButtonIconColor->setCurrentIndex(uiIndex);
}

void ButtonColors::setNegativeCloseBackgroundHoverPressState()
{
    bool drawCloseBackgroundNormally = m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::Backgrounds
        || m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::BackgroundsAndOutlines
        || m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::IconsAndBackgrounds
        || m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::IconsAndCloseButtonBackground
        || m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::IconsBackgroundsAndOutlines
        || m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::IconsOutlinesAndCloseButtonBackground;

    bool drawBackgroundNormally = m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::Backgrounds
        || m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::BackgroundsAndOutlines
        || m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::IconsAndBackgrounds
        || m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::IconsBackgroundsAndOutlines;

    if (drawCloseBackgroundNormally
        && (m_ui->buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
            || m_ui->buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose)) {
        m_ui->negativeCloseBackgroundHoverPress->setText(i18n("Negative close on hover/press only"));
        m_ui->negativeCloseBackgroundHoverPress->setVisible(true);
    } else if (drawBackgroundNormally && m_ui->buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
        m_ui->negativeCloseBackgroundHoverPress->setText(i18n("Traffic lights on hover/press only"));
        m_ui->negativeCloseBackgroundHoverPress->setVisible(true);
    } else {
        m_ui->negativeCloseBackgroundHoverPress->setVisible(false);
    }
}

void ButtonColors::showHideTranslucencySettings()
{
    if (m_ui->translucentButtonBackgrounds->isChecked()) {
        m_ui->translucentButtonBackgroundsOpacitySlider->setVisible(true);
        m_ui->opacitySliderLabel_1->setVisible(true);
        m_ui->opacitySliderLabel_2->setVisible(true);
        m_ui->translucentButtonBackgroundsOpacity->setVisible(true);
    } else {
        m_ui->translucentButtonBackgroundsOpacitySlider->setVisible(false);
        m_ui->opacitySliderLabel_1->setVisible(false);
        m_ui->opacitySliderLabel_2->setVisible(false);
        m_ui->translucentButtonBackgroundsOpacity->setVisible(false);
    }
}

void ButtonColors::resizeOverrideColorTable()
{
    m_ui->overrideColorTableActive->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_ui->overrideColorTableActive->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
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
        this->setMaximumSize(755, 400);
        this->showNormal();
    } else {
        this->setMaximumSize(16777215, 16777215);
        this->showMaximized();
    }
}

void ButtonColors::activeTableVerticalHeaderSectionClicked(const int row)
{
    QTableWidgetItem *item = m_ui->overrideColorTableActive->verticalHeaderItem(row);
    if (!item)
        return;
    // toggle checked state
    if (item->checkState() == Qt::CheckState::Checked) {
        setTableVerticalHeaderSectionCheckedState(row, false);
    } else {
        setTableVerticalHeaderSectionCheckedState(row, true);
    }
}

void ButtonColors::setTableVerticalHeaderSectionCheckedState(const int row, const bool checked)
{
    QTableWidgetItem *item = m_ui->overrideColorTableActive->verticalHeaderItem(row);
    if (!item)
        return;
    item->setCheckState(checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    item->setIcon(checked ? m_lockedIcon : m_unlockedIcon);
}

// given a checkBox sending a toggle signal to this slot, gets the table cell in which the checkBox was located and copies the same data to the equivalent
// inactive window table cell and to the rest of the row if the row lock is set
void ButtonColors::copyCellDataToOtherCells()
{
    if (m_loading || m_processingDefaults)
        return; // only do this for user interactions
    KColorButton *colorButton = qobject_cast<KColorButton *>(QObject::sender());
    if (colorButton) {
        bool columnOk, rowOk;
        int column = colorButton->property("column").toInt(&columnOk);
        int row = colorButton->property("row").toInt(&rowOk);

        if (columnOk && rowOk) {
            if (colorButton->parentWidget() && colorButton->parentWidget()->parentWidget() && colorButton->parentWidget()->parentWidget()->parentWidget()) {
                QTableWidget *table = qobject_cast<QTableWidget *>(colorButton->parentWidget()->parentWidget()->parentWidget());
                if (table) {
                    QTableWidgetItem *item = table->verticalHeaderItem(row);
                    if (item) {
                        if (item->checkState() == Qt::CheckState::Checked) { // if the lock icon is locked for the row
                            for (int destinationColumn = 0; destinationColumn < m_colorOverridableButtonTypesStrings.count(); destinationColumn++) {
                                QCheckBox *destinationCheckBox = nullptr;
                                KColorButton *destinationColorButton = nullptr;

                                if (destinationColumn != column) { // copy the values to all other colorBoxes in the row
                                    if (checkBoxAndColorButtonAtTableCell(table, destinationColumn, row, destinationCheckBox, destinationColorButton)) {
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
                                                     const int column,
                                                     const int row,
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
// bit 3 etc. reserved for deactivated state colour
void ButtonColors::encodeColorOverridableButtonTypeColumn(QTableWidget *tableActive, int column, uint32_t &colorsFlags, QList<int> &colorsList)
{
    uint32_t bitMask = 0x00000001;
    colorsFlags = 0x00000000;
    uint32_t validColorsFlagsBits = 0x07770777;
    uint32_t validColorsFlagsBitsActive = 0x0000FFFF;
    QTableWidget **table;

    colorsList.clear();
    for (uint32_t i = 0, rowIndex = 0; i < (sizeof(validColorsFlagsBits) * CHAR_BIT); i++, bitMask = bitMask << 1) {
        if (validColorsFlagsBits & bitMask) {
            QCheckBox *overrideCheckBox = nullptr;
            KColorButton *overrideColorButton = nullptr;
            if (validColorsFlagsBitsActive & bitMask) {
                table = &tableActive;
            } else {
                return;
            }
            if (checkBoxAndColorButtonAtTableCell(*table, column, rowIndex, overrideCheckBox, overrideColorButton)) {
                if (overrideCheckBox->isChecked()) { // set the bit of the row by OR-ing the bitmask with the existing value
                    colorsFlags = colorsFlags | bitMask;
                    colorsList.append(static_cast<int>(overrideColorButton->color().rgba())); // convert the colour to an int and store on the intList
                } else { // clear the bit by AND-ing with the inverse of the bitmask
                    colorsFlags = colorsFlags & (~bitMask);
                }
            }
            rowIndex++;
        }
    }
}

bool ButtonColors::decodeColorsFlagsAndLoadColumn(QTableWidget *tableActive, int column, uint32_t colorsFlags, const QList<int> &colorsList)
{
    uint32_t bitMask = 0x00000001;
    uint32_t validColorsFlagsBits = 0x07770777;
    uint32_t validColorsFlagsBitsActive = 0x0000FFFF;
    QTableWidget **table;
    int colorsListIndex = 0;
    bool columnHasOverrideColorsLoaded = false;
    for (uint32_t i = 0, rowIndex = 0; i < (sizeof(validColorsFlagsBits) * CHAR_BIT); i++, bitMask = bitMask << 1) {
        if (validColorsFlagsBits & bitMask) {
            QCheckBox *overrideCheckBox = nullptr;
            KColorButton *overrideColorButton = nullptr;
            if (validColorsFlagsBitsActive & bitMask) {
                table = &tableActive;
            } else {
                return columnHasOverrideColorsLoaded;
            }
            if (checkBoxAndColorButtonAtTableCell(*table, column, rowIndex, overrideCheckBox, overrideColorButton)) {
                if (colorsFlags & bitMask) { // if the current bit in colorsFlags is 1
                    columnHasOverrideColorsLoaded = true;
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
            rowIndex++;
        }
    }
    return columnHasOverrideColorsLoaded;
}

uint32_t ButtonColors::encodeColorOverridableLockStates()
{
    uint32_t bitMask = 0x00000001;
    uint32_t lockFlags = 0x00000000;
    uint32_t validColorsFlagsBits = 0x07770777;
    uint32_t validColorsFlagsBitsActive = 0x0000FFFF;
    QTableWidget **table;

    for (uint32_t i = 0, rowIndex = 0; i < (sizeof(validColorsFlagsBits) * CHAR_BIT); i++, bitMask = bitMask << 1) {
        if (validColorsFlagsBits & bitMask) {
            if (validColorsFlagsBitsActive & bitMask) {
                table = &m_ui->overrideColorTableActive;
            } else {
                return lockFlags;
            }
            QTableWidgetItem *item = (*table)->verticalHeaderItem(rowIndex);
            if (item) {
                if (item->checkState() == Qt::CheckState::Checked) { // set the bit of the row by OR-ing the bitmask with the existing value
                    lockFlags = lockFlags | bitMask;
                } else { // clear the bit by AND-ing with the inverse of the bitmask
                    lockFlags = lockFlags & (~bitMask);
                }
            }
            rowIndex++;
        }
    }
    return lockFlags;
}

bool ButtonColors::decodeColorOverridableLockStatesAndLoadVerticalHeaderLocks()
{
    uint32_t bitMask = 0x00000001;
    uint32_t validColorsFlagsBits = 0x07770777;
    uint32_t validColorsFlagsBitsActive = 0x0000FFFF;
    QTableWidget *tableActive = m_ui->overrideColorTableActive;
    QTableWidget **table;
    bool columnHasOverrideColorsLoaded = false;
    uint32_t lockFlags = m_internalSettings->buttonOverrideColorsLockStates();
    for (uint32_t i = 0, rowIndex = 0; i < (sizeof(validColorsFlagsBits) * CHAR_BIT); i++, bitMask = bitMask << 1) {
        if (validColorsFlagsBits & bitMask) {
            if (validColorsFlagsBitsActive & bitMask) {
                table = &tableActive;
            } else {
                return columnHasOverrideColorsLoaded;
            }
            QTableWidgetItem *item = (*table)->verticalHeaderItem(rowIndex);
            if (item) {
                if (lockFlags & bitMask) { // if the current bit in lockFlags is 1
                    setTableVerticalHeaderSectionCheckedState(rowIndex, true);
                } else {
                    setTableVerticalHeaderSectionCheckedState(rowIndex, false);
                }
            }
            rowIndex++;
        }
    }
    return columnHasOverrideColorsLoaded;
}

void ButtonColors::loadButtonBackgroundColorsIcons() // need to adjust for inactive as well
{
    InternalSettingsPtr temporaryColorSettings =
        InternalSettingsPtr(new InternalSettings); // temporary settings that reflect the UI, only to instantly update the colours displayed in the UI
    temporaryColorSettings->load();
    // temporaryColorSettings->setButtonIconColors(m_ui->buttonIconColors->currentIndex());
    temporaryColorSettings->setCloseButtonIconColor(convertCloseButtonIconColorUiToSettingsIndex(m_ui->closeButtonIconColor->currentIndex()));
    // temporaryColorSettings->setButtonBackgroundColors(m_ui->buttonBackgroundColors->currentIndex());
    temporaryColorSettings->setTranslucentButtonBackgrounds(m_ui->translucentButtonBackgrounds->isChecked());
    temporaryColorSettings->setNegativeCloseBackgroundHoverPress(m_ui->negativeCloseBackgroundHoverPress->isChecked());
    temporaryColorSettings->setLockButtonColorsActive(m_ui->lockButtonColorsActive->isChecked());
    temporaryColorSettings->setBlackWhiteIconOnPoorContrast(m_ui->blackWhiteIconOnPoorContrast->isChecked());
    temporaryColorSettings->setAdjustBackgroundColorOnPoorContrast(m_ui->adjustBackgroundColorOnPoorContrast->isChecked());
    temporaryColorSettings->setTranslucentButtonBackgroundsOpacity(m_ui->translucentButtonBackgroundsOpacity->value() / 100.0f);

    if (!m_decorationColors)
        m_decorationColors = std::make_shared<DecorationColors>(QApplication::palette(),
                                                                temporaryColorSettings,
                                                                false,
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

    DecorationButtonBehaviour buttonBehaviour;
    buttonBehaviour.reconfigure(temporaryColorSettings);
    DecorationButtonPalette closeButtonPalette(KDecoration2::DecorationButtonType::Close);
    DecorationButtonPalette maximizeButtonPalette(KDecoration2::DecorationButtonType::Maximize);
    DecorationButtonPalette minimizeButtonPalette(KDecoration2::DecorationButtonType::Minimize);
    DecorationButtonPalette otherButtonPalette(KDecoration2::DecorationButtonType::Custom);

    QList<DecorationButtonPalette *> otherCloseButtonList{&otherButtonPalette, &closeButtonPalette};
    otherCloseButtonList = sortButtonsAsPerKwinConfig(otherCloseButtonList);
    QList<DecorationButtonPalette *> otherTrafficLightsButtonList{&otherButtonPalette, &closeButtonPalette, &maximizeButtonPalette, &minimizeButtonPalette};
    otherTrafficLightsButtonList = sortButtonsAsPerKwinConfig(otherTrafficLightsButtonList);
    QList<DecorationButtonPalette *> trafficLightsButtonList{&closeButtonPalette, &maximizeButtonPalette, &minimizeButtonPalette};
    trafficLightsButtonList = sortButtonsAsPerKwinConfig(trafficLightsButtonList);

    qreal size = 32;

    QRectF oneByThree00(QPointF(0, 0), QPointF(size, size / 3));
    QRectF oneByThree01(QPointF(0, size / 3), QPointF(size, size * 2 / 3));
    QRectF oneByThree02(QPointF(0, size * 2 / 3), QPointF(size, size));

    QRectF twoByThree[2][3];
    twoByThree[0][0] = QRectF(QPointF(0, 0), QPointF(size / 2, size / 3));
    twoByThree[0][1] = QRectF(QPointF(0, size / 3), QPointF(size / 2, size * 2 / 3));
    twoByThree[0][2] = QRectF(QPointF(0, size * 2 / 3), QPointF(size / 2, size));
    twoByThree[1][0] = QRectF(QPointF(size / 2, 0), QPointF(size, size / 3));
    twoByThree[1][1] = QRectF(QPointF(size / 2, size / 3), QPointF(size, size * 2 / 3));
    twoByThree[1][2] = QRectF(QPointF(size / 2, size * 2 / 3), QPointF(size, size));

    QRectF threeByThree[3][3];
    threeByThree[0][0] = QRectF(QPointF(0, 0), QPointF(size / 3, size / 3));
    threeByThree[0][1] = QRectF(QPointF(0, size / 3), QPointF(size / 3, size * 2 / 3));
    threeByThree[0][2] = QRectF(QPointF(0, size * 2 / 3), QPointF(size / 3, size));
    threeByThree[1][0] = QRectF(QPointF(size / 3, 0), QPointF(size * 2 / 3, size / 3));
    threeByThree[1][1] = QRectF(QPointF(size / 3, size / 3), QPointF(size * 2 / 3, size * 2 / 3));
    threeByThree[1][2] = QRectF(QPointF(size / 3, size * 2 / 3), QPointF(size * 2 / 3, size));
    threeByThree[2][0] = QRectF(QPointF(size * 2 / 3, 0), QPointF(size, size / 3));
    threeByThree[2][1] = QRectF(QPointF(size * 2 / 3, size / 3), QPointF(size, size * 2 / 3));
    threeByThree[2][2] = QRectF(QPointF(size * 2 / 3, size * 2 / 3), QPointF(size, size));

    QRectF fourByThree[4][3];
    fourByThree[0][0] = QRectF(QPointF(0, 0), QPointF(size / 4, size / 3));
    fourByThree[0][1] = QRectF(QPointF(0, size / 3), QPointF(size / 4, size * 2 / 3));
    fourByThree[0][2] = QRectF(QPointF(0, size * 2 / 3), QPointF(size / 4, size));
    fourByThree[1][0] = QRectF(QPointF(size / 4, 0), QPointF(size / 2, size / 3));
    fourByThree[1][1] = QRectF(QPointF(size / 4, size / 3), QPointF(size / 2, size * 2 / 3));
    fourByThree[1][2] = QRectF(QPointF(size / 4, size * 2 / 3), QPointF(size / 2, size));
    fourByThree[2][0] = QRectF(QPointF(size / 2, 0), QPointF(size * 3 / 4, size / 3));
    fourByThree[2][1] = QRectF(QPointF(size / 2, size / 3), QPointF(size * 3 / 4, size * 2 / 3));
    fourByThree[2][2] = QRectF(QPointF(size / 2, size * 2 / 3), QPointF(size * 3 / 4, size));
    fourByThree[3][0] = QRectF(QPointF(size * 3 / 4, 0), QPointF(size, size / 3));
    fourByThree[3][1] = QRectF(QPointF(size * 3 / 4, size / 3), QPointF(size, size * 2 / 3));
    fourByThree[3][2] = QRectF(QPointF(size * 3 / 4, size * 2 / 3), QPointF(size, size));

    QPixmap pixmap(size, size);
    // pixmap.setDevicePixelRatio(this->window()->devicePixelRatioF());
    std::unique_ptr<QPainter> painter = std::make_unique<QPainter>(&pixmap);
    painter->setPen(Qt::NoPen);

    // background colors
    temporaryColorSettings->setButtonBackgroundColors(InternalSettings::EnumButtonBackgroundColors::TitlebarText);
    m_decorationColors->generateDecorationColors(QApplication::palette(), temporaryColorSettings);
    for (auto &buttonPalette : otherCloseButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);
    };

    m_ui->buttonBackgroundColors->setIconSize(QSize(size, size));
    pixmap.fill(Qt::transparent);
    painter->setBrush(otherButtonPalette.backgroundPress.isValid() ? otherButtonPalette.backgroundPress : Qt::transparent);
    painter->drawRect(oneByThree00);
    painter->setBrush(otherButtonPalette.backgroundHover.isValid() ? otherButtonPalette.backgroundHover : Qt::transparent);
    painter->drawRect(oneByThree01);
    painter->setBrush(otherButtonPalette.backgroundNormal.isValid() ? otherButtonPalette.backgroundNormal : Qt::transparent);
    painter->drawRect(oneByThree02);
    QIcon titlebarText(pixmap);
    m_ui->buttonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::TitlebarText, titlebarText);

    temporaryColorSettings->setButtonBackgroundColors(InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose);
    m_decorationColors->generateDecorationColors(QApplication::palette(), temporaryColorSettings);
    for (auto &buttonPalette : otherCloseButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);
    };

    pixmap.fill(Qt::transparent);
    for (int i = 0; i < 2; i++) {
        if (i < otherTrafficLightsButtonList.count()) {
            painter->setBrush(otherCloseButtonList[i]->backgroundPress.isValid() ? otherCloseButtonList[i]->backgroundPress : Qt::transparent);
            painter->drawRect(twoByThree[i][0]);
            painter->setBrush(otherCloseButtonList[i]->backgroundHover.isValid() ? otherCloseButtonList[i]->backgroundHover : Qt::transparent);
            painter->drawRect(twoByThree[i][1]);
            painter->setBrush(otherCloseButtonList[i]->backgroundNormal.isValid() ? otherCloseButtonList[i]->backgroundNormal : Qt::transparent);
            painter->drawRect(twoByThree[i][2]);
        }
    }
    QIcon backgroundTitlebarTextNegativeClose(pixmap);
    m_ui->buttonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose, backgroundTitlebarTextNegativeClose);

    temporaryColorSettings->setButtonBackgroundColors(InternalSettings::EnumButtonBackgroundColors::Accent);
    m_decorationColors->generateDecorationColors(QApplication::palette(), temporaryColorSettings);
    otherButtonPalette.reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);

    pixmap.fill(Qt::transparent);
    painter->setBrush(otherButtonPalette.backgroundPress.isValid() ? otherButtonPalette.backgroundPress : Qt::transparent);
    painter->drawRect(oneByThree00);
    painter->setBrush(otherButtonPalette.backgroundHover.isValid() ? otherButtonPalette.backgroundHover : Qt::transparent);
    painter->drawRect(oneByThree01);
    painter->setBrush(otherButtonPalette.backgroundNormal.isValid() ? otherButtonPalette.backgroundNormal : Qt::transparent);
    painter->drawRect(oneByThree02);
    QIcon backgroundAccent(pixmap);
    m_ui->buttonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::Accent, backgroundAccent);

    temporaryColorSettings->setButtonBackgroundColors(InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose);
    m_decorationColors->generateDecorationColors(QApplication::palette(), temporaryColorSettings);
    for (auto &buttonPalette : otherCloseButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);
    };

    pixmap.fill(Qt::transparent);
    for (int i = 0; i < 2; i++) {
        if (i < otherTrafficLightsButtonList.count()) {
            painter->setBrush(otherCloseButtonList[i]->backgroundPress.isValid() ? otherCloseButtonList[i]->backgroundPress : Qt::transparent);
            painter->drawRect(twoByThree[i][0]);
            painter->setBrush(otherCloseButtonList[i]->backgroundHover.isValid() ? otherCloseButtonList[i]->backgroundHover : Qt::transparent);
            painter->drawRect(twoByThree[i][1]);
            painter->setBrush(otherCloseButtonList[i]->backgroundNormal.isValid() ? otherCloseButtonList[i]->backgroundNormal : Qt::transparent);
            painter->drawRect(twoByThree[i][2]);
        }
    }
    QIcon backgroundAccentNegativeClose(pixmap);
    m_ui->buttonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose, backgroundAccentNegativeClose);

    temporaryColorSettings->setButtonBackgroundColors(InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights);
    m_decorationColors->generateDecorationColors(QApplication::palette(), temporaryColorSettings);

    for (auto &buttonPalette : otherTrafficLightsButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);
    }

    pixmap.fill(Qt::transparent);
    for (int i = 0; i < 4; i++) {
        if (i < otherTrafficLightsButtonList.count()) {
            painter->setBrush(otherTrafficLightsButtonList[i]->backgroundPress.isValid() ? otherTrafficLightsButtonList[i]->backgroundPress : Qt::transparent);
            painter->drawRect(fourByThree[i][0]);
            painter->setBrush(otherTrafficLightsButtonList[i]->backgroundHover.isValid() ? otherTrafficLightsButtonList[i]->backgroundHover : Qt::transparent);
            painter->drawRect(fourByThree[i][1]);
            painter->setBrush(otherTrafficLightsButtonList[i]->backgroundNormal.isValid() ? otherTrafficLightsButtonList[i]->backgroundNormal
                                                                                          : Qt::transparent);
            painter->drawRect(fourByThree[i][2]);
        }
    }
    QIcon backgroundAccentTrafficLightsClose(pixmap);
    m_ui->buttonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights, backgroundAccentTrafficLightsClose);

    // icon colors ---------------------------------------------------------------------------------
    m_ui->buttonIconColors->setIconSize(QSize(24, 24));
    temporaryColorSettings->setButtonBackgroundColors(m_ui->buttonBackgroundColors->currentIndex());

    temporaryColorSettings->setButtonIconColors(InternalSettings::EnumButtonIconColors::TitlebarText);
    m_decorationColors->generateDecorationColors(QApplication::palette(), temporaryColorSettings);
    for (auto &buttonPalette : otherCloseButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);
    };

    pixmap.fill(Qt::transparent);
    for (int i = 0; i < 2; i++) {
        if (i < otherTrafficLightsButtonList.count()) {
            painter->setBrush(otherCloseButtonList[i]->foregroundPress.isValid() ? otherCloseButtonList[i]->foregroundPress : Qt::transparent);
            painter->drawRect(twoByThree[i][0]);
            painter->setBrush(otherCloseButtonList[i]->foregroundHover.isValid() ? otherCloseButtonList[i]->foregroundHover : Qt::transparent);
            painter->drawRect(twoByThree[i][1]);
            painter->setBrush(otherCloseButtonList[i]->foregroundNormal.isValid() ? otherCloseButtonList[i]->foregroundNormal : Qt::transparent);
            painter->drawRect(twoByThree[i][2]);
        }
    }
    QIcon iconTitlebarText(pixmap);
    m_ui->buttonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::TitlebarText, iconTitlebarText);

    temporaryColorSettings->setButtonIconColors(InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose);
    m_decorationColors->generateDecorationColors(QApplication::palette(), temporaryColorSettings);
    for (auto &buttonPalette : otherCloseButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);
    };

    pixmap.fill(Qt::transparent);
    for (int i = 0; i < 2; i++) {
        if (i < otherTrafficLightsButtonList.count()) {
            painter->setBrush(otherCloseButtonList[i]->foregroundPress.isValid() ? otherCloseButtonList[i]->foregroundPress : Qt::transparent);
            painter->drawRect(twoByThree[i][0]);
            painter->setBrush(otherCloseButtonList[i]->foregroundHover.isValid() ? otherCloseButtonList[i]->foregroundHover : Qt::transparent);
            painter->drawRect(twoByThree[i][1]);
            painter->setBrush(otherCloseButtonList[i]->foregroundNormal.isValid() ? otherCloseButtonList[i]->foregroundNormal : Qt::transparent);
            painter->drawRect(twoByThree[i][2]);
        }
    }
    QIcon iconTitlebarTextNegativeClose(pixmap);
    m_ui->buttonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose, iconTitlebarTextNegativeClose);

    temporaryColorSettings->setButtonIconColors(InternalSettings::EnumButtonIconColors::Accent);
    m_decorationColors->generateDecorationColors(QApplication::palette(), temporaryColorSettings);
    for (auto &buttonPalette : otherCloseButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);
    };

    pixmap.fill(Qt::transparent);
    for (int i = 0; i < 2; i++) {
        if (i < otherTrafficLightsButtonList.count()) {
            painter->setBrush(otherCloseButtonList[i]->foregroundPress.isValid() ? otherCloseButtonList[i]->foregroundPress : Qt::transparent);
            painter->drawRect(twoByThree[i][0]);
            painter->setBrush(otherCloseButtonList[i]->foregroundHover.isValid() ? otherCloseButtonList[i]->foregroundHover : Qt::transparent);
            painter->drawRect(twoByThree[i][1]);
            painter->setBrush(otherCloseButtonList[i]->foregroundNormal.isValid() ? otherCloseButtonList[i]->foregroundNormal : Qt::transparent);
            painter->drawRect(twoByThree[i][2]);
        }
    }
    QIcon iconAccent(pixmap);
    m_ui->buttonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::Accent, iconAccent);

    temporaryColorSettings->setButtonIconColors(InternalSettings::EnumButtonIconColors::AccentNegativeClose);
    m_decorationColors->generateDecorationColors(QApplication::palette(), temporaryColorSettings);
    for (auto &buttonPalette : otherCloseButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);
    };

    pixmap.fill(Qt::transparent);
    for (int i = 0; i < 2; i++) {
        if (i < otherTrafficLightsButtonList.count()) {
            painter->setBrush(otherCloseButtonList[i]->foregroundPress.isValid() ? otherCloseButtonList[i]->foregroundPress : Qt::transparent);
            painter->drawRect(twoByThree[i][0]);
            painter->setBrush(otherCloseButtonList[i]->foregroundHover.isValid() ? otherCloseButtonList[i]->foregroundHover : Qt::transparent);
            painter->drawRect(twoByThree[i][1]);
            painter->setBrush(otherCloseButtonList[i]->foregroundNormal.isValid() ? otherCloseButtonList[i]->foregroundNormal : Qt::transparent);
            painter->drawRect(twoByThree[i][2]);
        }
    }
    QIcon iconAccentNegativeClose(pixmap);
    m_ui->buttonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::AccentNegativeClose, iconAccentNegativeClose);

    temporaryColorSettings->setButtonIconColors(InternalSettings::EnumButtonIconColors::AccentTrafficLights);
    m_decorationColors->generateDecorationColors(QApplication::palette(), temporaryColorSettings);
    for (auto &buttonPalette : otherTrafficLightsButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);
    }

    pixmap.fill(Qt::transparent);
    for (int i = 0; i < 4; i++) {
        if (i < otherTrafficLightsButtonList.count()) {
            painter->setBrush(otherTrafficLightsButtonList[i]->foregroundPress.isValid() ? otherTrafficLightsButtonList[i]->foregroundPress : Qt::transparent);
            painter->drawRect(fourByThree[i][0]);
            painter->setBrush(otherTrafficLightsButtonList[i]->foregroundHover.isValid() ? otherTrafficLightsButtonList[i]->foregroundHover : Qt::transparent);
            painter->drawRect(fourByThree[i][1]);
            painter->setBrush(otherTrafficLightsButtonList[i]->foregroundNormal.isValid() ? otherTrafficLightsButtonList[i]->foregroundNormal
                                                                                          : Qt::transparent);
            painter->drawRect(fourByThree[i][2]);
        }
    }
    QIcon iconAccentTrafficLights(pixmap);
    m_ui->buttonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::AccentTrafficLights, iconAccentTrafficLights);

    // closeButtonIconColor icons ----------------------------------------------------

    m_ui->closeButtonIconColor->setIconSize(QSize(16, 16));
    temporaryColorSettings->setButtonIconColors(m_ui->buttonIconColors->currentIndex());

    int uiItemIndex = convertCloseButtonIconColorSettingsToUiIndex(InternalSettings::EnumCloseButtonIconColor::AsSelected);
    if (uiItemIndex >= 0) {
        temporaryColorSettings->setCloseButtonIconColor(InternalSettings::EnumCloseButtonIconColor::AsSelected);
        m_decorationColors->generateDecorationColors(QApplication::palette(), temporaryColorSettings);
        if (m_ui->buttonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
            for (auto &buttonPalette : trafficLightsButtonList) {
                buttonPalette->reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);
            }
            pixmap.fill(Qt::transparent);
            for (int i = 0; i < 3; i++) {
                if (i < trafficLightsButtonList.count()) {
                    painter->setBrush(trafficLightsButtonList[i]->foregroundPress.isValid() ? trafficLightsButtonList[i]->foregroundPress : Qt::transparent);
                    painter->drawRect(threeByThree[i][0]);
                    painter->setBrush(trafficLightsButtonList[i]->foregroundHover.isValid() ? trafficLightsButtonList[i]->foregroundHover : Qt::transparent);
                    painter->drawRect(threeByThree[i][1]);
                    painter->setBrush(trafficLightsButtonList[i]->foregroundNormal.isValid() ? trafficLightsButtonList[i]->foregroundNormal : Qt::transparent);
                    painter->drawRect(threeByThree[i][2]);
                }
            }
        } else {
            closeButtonPalette.reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);
            pixmap.fill(Qt::transparent);
            painter->setBrush(closeButtonPalette.foregroundPress.isValid() ? closeButtonPalette.foregroundPress : Qt::transparent);
            painter->drawRect(oneByThree00);
            painter->setBrush(closeButtonPalette.foregroundHover.isValid() ? closeButtonPalette.foregroundHover : Qt::transparent);
            painter->drawRect(oneByThree01);
            painter->setBrush(closeButtonPalette.foregroundNormal.isValid() ? closeButtonPalette.foregroundNormal : Qt::transparent);
            painter->drawRect(oneByThree02);
        }
        QIcon icon(pixmap);
        m_ui->closeButtonIconColor->setItemIcon(uiItemIndex, icon);
    }

    uiItemIndex = convertCloseButtonIconColorSettingsToUiIndex(InternalSettings::EnumCloseButtonIconColor::NegativeWhenHoverPress);
    if (uiItemIndex >= 0) {
        temporaryColorSettings->setCloseButtonIconColor(InternalSettings::EnumCloseButtonIconColor::NegativeWhenHoverPress);
        m_decorationColors->generateDecorationColors(QApplication::palette(), temporaryColorSettings);
        if (m_ui->buttonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
            for (auto &buttonPalette : trafficLightsButtonList) {
                buttonPalette->reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);
            }
            pixmap.fill(Qt::transparent);
            for (int i = 0; i < 3; i++) {
                if (i < trafficLightsButtonList.count()) {
                    painter->setBrush(trafficLightsButtonList[i]->foregroundPress.isValid() ? trafficLightsButtonList[i]->foregroundPress : Qt::transparent);
                    painter->drawRect(threeByThree[i][0]);
                    painter->setBrush(trafficLightsButtonList[i]->foregroundHover.isValid() ? trafficLightsButtonList[i]->foregroundHover : Qt::transparent);
                    painter->drawRect(threeByThree[i][1]);
                    painter->setBrush(trafficLightsButtonList[i]->foregroundNormal.isValid() ? trafficLightsButtonList[i]->foregroundNormal : Qt::transparent);
                    painter->drawRect(threeByThree[i][2]);
                }
            }
        } else {
            closeButtonPalette.reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);
            pixmap.fill(Qt::transparent);
            painter->setBrush(closeButtonPalette.foregroundPress.isValid() ? closeButtonPalette.foregroundPress : Qt::transparent);
            painter->drawRect(oneByThree00);
            painter->setBrush(closeButtonPalette.foregroundHover.isValid() ? closeButtonPalette.foregroundHover : Qt::transparent);
            painter->drawRect(oneByThree01);
            painter->setBrush(closeButtonPalette.foregroundNormal.isValid() ? closeButtonPalette.foregroundNormal : Qt::transparent);
            painter->drawRect(oneByThree02);
        }
        QIcon icon(pixmap);
        m_ui->closeButtonIconColor->setItemIcon(uiItemIndex, icon);
    }

    uiItemIndex = convertCloseButtonIconColorSettingsToUiIndex(InternalSettings::EnumCloseButtonIconColor::White);
    if (uiItemIndex >= 0) {
        temporaryColorSettings->setCloseButtonIconColor(InternalSettings::EnumCloseButtonIconColor::White);
        m_decorationColors->generateDecorationColors(QApplication::palette(), temporaryColorSettings);
        if (m_ui->buttonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
            for (auto &buttonPalette : trafficLightsButtonList) {
                buttonPalette->reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);
            }
            pixmap.fill(Qt::transparent);
            for (int i = 0; i < 3; i++) {
                if (i < trafficLightsButtonList.count()) {
                    painter->setBrush(trafficLightsButtonList[i]->foregroundPress.isValid() ? trafficLightsButtonList[i]->foregroundPress : Qt::transparent);
                    painter->drawRect(threeByThree[i][0]);
                    painter->setBrush(trafficLightsButtonList[i]->foregroundHover.isValid() ? trafficLightsButtonList[i]->foregroundHover : Qt::transparent);
                    painter->drawRect(threeByThree[i][1]);
                    painter->setBrush(trafficLightsButtonList[i]->foregroundNormal.isValid() ? trafficLightsButtonList[i]->foregroundNormal : Qt::transparent);
                    painter->drawRect(threeByThree[i][2]);
                }
            }
        } else {
            closeButtonPalette.reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);
            pixmap.fill(Qt::transparent);
            painter->setBrush(closeButtonPalette.foregroundPress.isValid() ? closeButtonPalette.foregroundPress : Qt::transparent);
            painter->drawRect(oneByThree00);
            painter->setBrush(closeButtonPalette.foregroundHover.isValid() ? closeButtonPalette.foregroundHover : Qt::transparent);
            painter->drawRect(oneByThree01);
            painter->setBrush(closeButtonPalette.foregroundNormal.isValid() ? closeButtonPalette.foregroundNormal : Qt::transparent);
            painter->drawRect(oneByThree02);
        }
        QIcon icon(pixmap);
        m_ui->closeButtonIconColor->setItemIcon(uiItemIndex, icon);
    }

    uiItemIndex = convertCloseButtonIconColorSettingsToUiIndex(InternalSettings::EnumCloseButtonIconColor::WhiteWhenHoverPress);
    if (uiItemIndex >= 0) {
        temporaryColorSettings->setCloseButtonIconColor(InternalSettings::EnumCloseButtonIconColor::WhiteWhenHoverPress);
        m_decorationColors->generateDecorationColors(QApplication::palette(), temporaryColorSettings);

        if (m_ui->buttonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
            for (auto &buttonPalette : trafficLightsButtonList) {
                buttonPalette->reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);
            }
            pixmap.fill(Qt::transparent);
            for (int i = 0; i < 3; i++) {
                if (i < trafficLightsButtonList.count()) {
                    painter->setBrush(trafficLightsButtonList[i]->foregroundPress.isValid() ? trafficLightsButtonList[i]->foregroundPress : Qt::transparent);
                    painter->drawRect(threeByThree[i][0]);
                    painter->setBrush(trafficLightsButtonList[i]->foregroundHover.isValid() ? trafficLightsButtonList[i]->foregroundHover : Qt::transparent);
                    painter->drawRect(threeByThree[i][1]);
                    painter->setBrush(trafficLightsButtonList[i]->foregroundNormal.isValid() ? trafficLightsButtonList[i]->foregroundNormal : Qt::transparent);
                    painter->drawRect(threeByThree[i][2]);
                }
            }
        } else {
            closeButtonPalette.reconfigure(temporaryColorSettings, &buttonBehaviour, m_decorationColors.get(), titlebarTextActive, titlebarBackgroundActive);
            pixmap.fill(Qt::transparent);
            painter->setBrush(closeButtonPalette.foregroundPress.isValid() ? closeButtonPalette.foregroundPress : Qt::transparent);
            painter->drawRect(oneByThree00);
            painter->setBrush(closeButtonPalette.foregroundHover.isValid() ? closeButtonPalette.foregroundHover : Qt::transparent);
            painter->drawRect(oneByThree01);
            painter->setBrush(closeButtonPalette.foregroundNormal.isValid() ? closeButtonPalette.foregroundNormal : Qt::transparent);
            painter->drawRect(oneByThree02);
        }
        QIcon icon(pixmap);
        m_ui->closeButtonIconColor->setItemIcon(uiItemIndex, icon);
    }
}

void ButtonColors::getButtonsOrderFromKwinConfig()
{
    QMap<KDecoration2::DecorationButtonType, QChar> buttonNames;
    // list modified from https://invent.kde.org/plasma/kwin/-/blob/master/src/decorations/settings.cpp
    buttonNames[KDecoration2::DecorationButtonType::Menu] = QChar('M');
    buttonNames[KDecoration2::DecorationButtonType::ApplicationMenu] = QChar('N');
    buttonNames[KDecoration2::DecorationButtonType::OnAllDesktops] = QChar('S');
    buttonNames[KDecoration2::DecorationButtonType::KeepAbove] = QChar('F');
    buttonNames[KDecoration2::DecorationButtonType::KeepBelow] = QChar('B');
    buttonNames[KDecoration2::DecorationButtonType::Shade] = QChar('L');
    buttonNames[KDecoration2::DecorationButtonType::ContextHelp] = QChar('H');
    buttonNames[KDecoration2::DecorationButtonType::Minimize] = QChar('I');
    buttonNames[KDecoration2::DecorationButtonType::Maximize] = QChar('A');
    buttonNames[KDecoration2::DecorationButtonType::Close] = QChar('X');

    QString buttonsOnLeft;
    QString buttonsOnRight;

    // very hacky way to do this -- better would be to find a way to get the settings from <KDecoration2/DecorationSettings>
    //  read kwin button border setting
    KSharedConfig::Ptr kwinConfig = KSharedConfig::openConfig(QStringLiteral("kwinrc"));
    if (kwinConfig && kwinConfig->hasGroup(QStringLiteral("org.kde.kdecoration2"))) {
        KConfigGroup kdecoration2Group = kwinConfig->group(QStringLiteral("org.kde.kdecoration2"));

        if (!kdecoration2Group.hasKey(QStringLiteral("ButtonsOnLeft"))) {
            buttonsOnLeft = QStringLiteral("MS");
        } else {
            buttonsOnLeft = kdecoration2Group.readEntry(QStringLiteral("ButtonsOnLeft"));
        }

        if (!kdecoration2Group.hasKey(QStringLiteral("ButtonsOnRight"))) {
            buttonsOnRight = QStringLiteral("HIAX");
        } else {
            buttonsOnRight = kdecoration2Group.readEntry(QStringLiteral("ButtonsOnRight"));
        }

    } else {
        buttonsOnLeft = QStringLiteral("MS");
        buttonsOnRight = QStringLiteral("HIAX");
    }

    QString visibleButtons = buttonsOnLeft + buttonsOnRight;

    m_visibleButtonsOrder.clear();
    for (QChar *it = visibleButtons.begin(); it != visibleButtons.end(); it++) {
        auto key = buttonNames.key(*it, KDecoration2::DecorationButtonType::Custom);
        if (key != KDecoration2::DecorationButtonType::Custom)
            m_visibleButtonsOrder.append(key);
    }

    m_hiddenButtons.clear();
    // add hidden buttons to m_hiddenButtons
    for (auto it = buttonNames.begin(); it != buttonNames.end(); it++) {
        if (!visibleButtons.contains(*it)) {
            m_hiddenButtons.append(buttonNames.key(*it));
        }
    }

    // Place a custom button type in the average position of these "other" button types
    QList<int> otherButtonIndexes{
        m_visibleButtonsOrder.indexOf(KDecoration2::DecorationButtonType::Menu),
        m_visibleButtonsOrder.indexOf(KDecoration2::DecorationButtonType::ApplicationMenu),
        m_visibleButtonsOrder.indexOf(KDecoration2::DecorationButtonType::OnAllDesktops),
        m_visibleButtonsOrder.indexOf(KDecoration2::DecorationButtonType::ContextHelp),
        m_visibleButtonsOrder.indexOf(KDecoration2::DecorationButtonType::KeepAbove),
        m_visibleButtonsOrder.indexOf(KDecoration2::DecorationButtonType::KeepBelow),
        m_visibleButtonsOrder.indexOf(KDecoration2::DecorationButtonType::Shade),
    };

    // remove the -1s (index not found)
    QMutableListIterator<int> i(otherButtonIndexes);
    while (i.hasNext()) {
        if (i.next() == -1)
            i.remove();
    }

    int indexOfCustom;
    if (otherButtonIndexes.count()) {
        int sum = 0;
        for (int i = 0; i < otherButtonIndexes.count(); i++) {
            sum += otherButtonIndexes[i] + 1;
        }
        indexOfCustom = (sum / otherButtonIndexes.count()) - 1; // indexOfCustom is now at the median index position of otherButtonIndexes
    } else {
        indexOfCustom = 0;
    }

    // Want to give Close/Maximize/Minimize buttons priority over the custom button to be at either the left or right edges
    QMap<int, KDecoration2::DecorationButtonType> leftEdgePriorityButtons; // a list of Close/Maximize/Minimize if at left edge
    QMap<int, KDecoration2::DecorationButtonType> rightEdgePriorityButtons; // a list of Close/Maximize/Minimize if at right edge

    // find leftEdgePriorityButtons
    for (int i = 0; i < 3; i++) {
        if (buttonsOnLeft.indexOf(QChar('X')) == i) {
            leftEdgePriorityButtons.insert(i, KDecoration2::DecorationButtonType::Close);
        } else if (buttonsOnLeft.indexOf(QChar('A')) == i) {
            leftEdgePriorityButtons.insert(i, KDecoration2::DecorationButtonType::Maximize);
        } else if (buttonsOnLeft.indexOf(QChar('I')) == i) {
            leftEdgePriorityButtons.insert(i, KDecoration2::DecorationButtonType::Minimize);
        }
    }

    // find rightEdgePrioritybuttons
    for (int i = m_visibleButtonsOrder.count() - 1; i >= m_visibleButtonsOrder.count() - 3; i--) {
        if (buttonsOnRight.lastIndexOf(QChar('X')) == i) { // lastIndexOf in-case a weirdo adds more than one button of the same type
            rightEdgePriorityButtons.insert(i, KDecoration2::DecorationButtonType::Close);
        } else if (buttonsOnRight.lastIndexOf(QChar('A')) == i) {
            rightEdgePriorityButtons.insert(i, KDecoration2::DecorationButtonType::Maximize);
        } else if (buttonsOnRight.lastIndexOf(QChar('I')) == i) {
            rightEdgePriorityButtons.insert(i, KDecoration2::DecorationButtonType::Minimize);
        }
    }

    // if custom is at an edge, make sure priority (min/max/close) button has a priority over custom button for the edge
    if (indexOfCustom >= 0 && indexOfCustom <= (leftEdgePriorityButtons.count() - 1)
        && leftEdgePriorityButtons.count()) { // if custom is to go at start but a leftEdgePriority button is there
        indexOfCustom = m_visibleButtonsOrder.indexOf(leftEdgePriorityButtons.value(leftEdgePriorityButtons.count() - 1)) + 1;
    } else if (indexOfCustom <= (m_visibleButtonsOrder.count() - 1) && indexOfCustom >= (m_visibleButtonsOrder.count() - rightEdgePriorityButtons.count())
               && rightEdgePriorityButtons.count()) { // if custom is to go at end but a right EdgePriority button is there
        indexOfCustom = m_visibleButtonsOrder.indexOf(rightEdgePriorityButtons.value(0));
    }

    m_allCustomizableButtonsOrder = m_visibleButtonsOrder + m_hiddenButtons;

    m_visibleButtonsOrder.insert(indexOfCustom,
                                 KDecoration2::DecorationButtonType::Custom); // dummy Custom button inserted for illustrating colour palettes in icons
}

QList<DecorationButtonPalette *> ButtonColors::sortButtonsAsPerKwinConfig(QList<DecorationButtonPalette *> inputlist)
{
    QList<DecorationButtonPalette *> outputlist;

    QMutableListIterator<DecorationButtonPalette *> j(inputlist);
    for (int i = 0; i < m_visibleButtonsOrder.count(); i++) {
        for (int j = inputlist.count() - 1; j >= 0; j--) { // iterate loop in reverse order as want to delete elements
            if (m_visibleButtonsOrder[i] == (inputlist[j])->buttonType()) {
                outputlist.append(inputlist[j]);
                inputlist.removeAt(j);
            }
        }
    }
    return outputlist;
}

bool ButtonColors::event(QEvent *ev)
{
    if (ev->type() == QEvent::ApplicationPaletteChange) {
        // overwrite handling of palette change
        m_decorationColors->generateDecorationColors(QApplication::palette(), m_internalSettings);

        if (!m_loaded)
            load();
        else
            loadButtonBackgroundColorsIcons();
        return QWidget::event(ev);
    }
    // Make sure the rest of events are handled
    return QWidget::event(ev);
}
}
