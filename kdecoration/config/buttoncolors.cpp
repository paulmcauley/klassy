/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "buttoncolors.h"
#include "breezeconfigwidget.h"
#include "colortools.h"
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
    // gnerate the overrideColorTableActive table UI
    int numColumns = static_cast<int>(OverridableButtonColorStates::Count);
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
            connect(checkBox, &QAbstractButton::toggled, this, &ButtonColors::copyCellDataToInactiveTable);
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
                                                         static_cast<int>(ColorOverridableButtonTypes::Restore),
                                                         m_internalSettings->buttonOverrideColorsRestoreFlags(),
                                                         m_internalSettings->buttonOverrideColorsRestore());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::Minimize),
                                                         m_internalSettings->buttonOverrideColorsMinimizeFlags(),
                                                         m_internalSettings->buttonOverrideColorsMinimize());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::ContextHelp),
                                                         m_internalSettings->buttonOverrideColorsContextHelpFlags(),
                                                         m_internalSettings->buttonOverrideColorsContextHelp());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::Shade),
                                                         m_internalSettings->buttonOverrideColorsShadeFlags(),
                                                         m_internalSettings->buttonOverrideColorsShade());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::Unshade),
                                                         m_internalSettings->buttonOverrideColorsUnshadeFlags(),
                                                         m_internalSettings->buttonOverrideColorsUnshade());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::PinOnAllDesktops),
                                                         m_internalSettings->buttonOverrideColorsPinOnAllDesktopsFlags(),
                                                         m_internalSettings->buttonOverrideColorsPinOnAllDesktops());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::Unpin),
                                                         m_internalSettings->buttonOverrideColorsUnpinFlags(),
                                                         m_internalSettings->buttonOverrideColorsUnpin());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::KeepBelow),
                                                         m_internalSettings->buttonOverrideColorsKeepBelowFlags(),
                                                         m_internalSettings->buttonOverrideColorsKeepBelow());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::KeepBelowChecked),
                                                         m_internalSettings->buttonOverrideColorsKeepBelowCheckedFlags(),
                                                         m_internalSettings->buttonOverrideColorsKeepBelowChecked());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::KeepAbove),
                                                         m_internalSettings->buttonOverrideColorsKeepAboveFlags(),
                                                         m_internalSettings->buttonOverrideColorsKeepAbove());
    m_overrideColorsLoaded = m_overrideColorsLoaded || overrideColorRowLoaded;

    overrideColorRowLoaded = decodeColorsFlagsAndLoadRow(m_ui->overrideColorTableActive,
                                                         static_cast<int>(ColorOverridableButtonTypes::KeepAboveChecked),
                                                         m_internalSettings->buttonOverrideColorsKeepAboveCheckedFlags(),
                                                         m_internalSettings->buttonOverrideColorsKeepAboveChecked());
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
    ;
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::Restore), colorsFlags, colorsList);
    m_internalSettings->setButtonOverrideColorsRestoreFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsRestore(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::Minimize), colorsFlags, colorsList);
    m_internalSettings->setButtonOverrideColorsMinimizeFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsMinimize(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive,
                                            static_cast<int>(ColorOverridableButtonTypes::ContextHelp),
                                            colorsFlags,
                                            colorsList);
    m_internalSettings->setButtonOverrideColorsContextHelpFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsContextHelp(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::Shade), colorsFlags, colorsList);
    m_internalSettings->setButtonOverrideColorsShadeFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsShade(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::Unshade), colorsFlags, colorsList);
    m_internalSettings->setButtonOverrideColorsUnshadeFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsUnshade(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive,
                                            static_cast<int>(ColorOverridableButtonTypes::PinOnAllDesktops),
                                            colorsFlags,
                                            colorsList);
    m_internalSettings->setButtonOverrideColorsPinOnAllDesktopsFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsPinOnAllDesktops(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::Unpin), colorsFlags, colorsList);
    m_internalSettings->setButtonOverrideColorsUnpinFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsUnpin(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::KeepBelow), colorsFlags, colorsList);
    m_internalSettings->setButtonOverrideColorsKeepBelowFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsKeepBelow(colorsList);

    colorsList = QList<int>();
    colorsFlags = 0;
    if (m_ui->buttonColorActiveOverrideToggle->isChecked())
        encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive,
                                            static_cast<int>(ColorOverridableButtonTypes::KeepBelowChecked),
                                            colorsFlags,
                                            colorsList);
    m_internalSettings->setButtonOverrideColorsKeepBelowCheckedFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsKeepBelowChecked(colorsList);

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
                                            static_cast<int>(ColorOverridableButtonTypes::KeepAboveChecked),
                                            colorsFlags,
                                            colorsList);
    m_internalSettings->setButtonOverrideColorsKeepAboveCheckedFlags(colorsFlags);
    m_internalSettings->setButtonOverrideColorsKeepAboveChecked(colorsList);

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
    else if (m_ui->translucentButtonBackgroundsOpacity->value() != (100 * m_internalSettings->translucentButtonBackgroundsOpacity()))
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
                                                static_cast<int>(ColorOverridableButtonTypes::Restore),
                                                colorsFlags,
                                                colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsRestoreFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsRestore());
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
            encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive,
                                                static_cast<int>(ColorOverridableButtonTypes::ContextHelp),
                                                colorsFlags,
                                                colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsContextHelpFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsContextHelp());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::Shade), colorsFlags, colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsShadeFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsShade());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive,
                                                static_cast<int>(ColorOverridableButtonTypes::Unshade),
                                                colorsFlags,
                                                colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsUnshadeFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsUnshade());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive,
                                                static_cast<int>(ColorOverridableButtonTypes::PinOnAllDesktops),
                                                colorsFlags,
                                                colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsPinOnAllDesktopsFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsPinOnAllDesktops());
        }

        if (!modified) {
            encodeColorOverridableButtonTypeRow(m_ui->overrideColorTableActive, static_cast<int>(ColorOverridableButtonTypes::Unpin), colorsFlags, colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsUnpinFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsUnpin());
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
                                                static_cast<int>(ColorOverridableButtonTypes::KeepBelowChecked),
                                                colorsFlags,
                                                colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsKeepBelowCheckedFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsKeepBelowChecked());
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
                                                static_cast<int>(ColorOverridableButtonTypes::KeepAboveChecked),
                                                colorsFlags,
                                                colorsList);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsKeepAboveCheckedFlags());
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColorsKeepAboveChecked());
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
        this->setMaximumSize(755, 400); // report this as a bug
        this->showNormal();
    } else {
        this->setMaximumSize(16777215, 16777215);
        this->showMaximized();
        m_ui->activeOverrideGroupBox->setVisible(value);
    }
}

// given a checkBox sending a toggle signal to this slot, gets the table cell in which the checkBox was located and copies the same data to the equivalent
// inactive window table cell
void ButtonColors::copyCellDataToInactiveTable(const bool on)
{
    QCheckBox *checkBox = qobject_cast<QCheckBox *>(QObject::sender());
    if (checkBox) {
        int row = checkBox->property("row").toInt();
        int column = checkBox->property("column").toInt();
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

void ButtonColors::encodeColorOverridableButtonTypeRow(QTableWidget *table, int row, uint32_t &colorsFlags, QList<int> &colorsList)
{
    uint32_t bitMask = 0x00000001;
    colorsFlags = 0x00000000;
    colorsList.clear();
    for (int columnIndex = 0; columnIndex < static_cast<int>(OverridableButtonColorStates::Count); columnIndex++, bitMask = bitMask << 1) {
        QCheckBox *overrideCheckBox = nullptr;
        KColorButton *overrideColorButton = nullptr;
        if (checkBoxAndColorButtonAtTableCell(table, row, columnIndex, overrideCheckBox, overrideColorButton)) {
            if (overrideCheckBox->isChecked()) { // set the bit of the column by OR-ing the bitmask with the existing value
                colorsFlags = colorsFlags | bitMask;
                colorsList.append(static_cast<int>(overrideColorButton->color().rgba())); // convert the colour to an int and store on the intList
            } else { // clear the bit by AND-ing with the inverse of the bitmask
                colorsFlags = colorsFlags & (~bitMask);
            }
        }
    }
}

bool ButtonColors::decodeColorsFlagsAndLoadRow(QTableWidget *table, int row, uint32_t colorsFlags, const QList<int> &colorsList)
{
    uint32_t bitMask = 0x00000001;
    int colorsListIndex = 0;
    bool rowHasOverrideColorsLoaded = false;
    for (int columnIndex = 0; columnIndex < static_cast<int>(OverridableButtonColorStates::Count); columnIndex++, bitMask = bitMask << 1) {
        QCheckBox *overrideCheckBox = nullptr;
        KColorButton *overrideColorButton = nullptr;
        if (checkBoxAndColorButtonAtTableCell(table, row, columnIndex, overrideCheckBox, overrideColorButton)) {
            if (colorsFlags & bitMask) { // if the current bit in colorsFlags is 1
                rowHasOverrideColorsLoaded = true;
                overrideCheckBox->setChecked(true);
                if (colorsList.count()
                    && colorsListIndex
                        < colorsList.count()) { // this if is to prevent against an unlikely corruption situation when colorsSet and colorsist are out of sync
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
    }
    return rowHasOverrideColorsLoaded;
}

void ButtonColors::setButtonBackgroundColorsIcons() // need to adjust for inactive as well
{
    ColorTools::generateDecorationColors(QApplication::palette(), m_internalSettings, true);

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
    painter->setBrush(g_decorationColors->negativeLessSaturated);
    painter->drawRect(twoByThree10);
    painter->setBrush(g_decorationColors->negativeSaturated);
    painter->drawRect(twoByThree11);
    painter->setBrush(m_internalSettings->negativeCloseBackgroundHoverPress() ? KColorUtils::mix(titlebarBackgroundActive, titlebarTextActive, 0.3)
                                                                              : g_decorationColors->negative);
    painter->drawRect(twoByThree12);
    QIcon backgroundTitlebarTextNegativeClose(pixmap);
    m_ui->buttonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose, backgroundTitlebarTextNegativeClose);

    pixmap.fill(Qt::transparent);
    painter->setBrush(g_decorationColors->buttonFocus);
    painter->drawRect(oneByThree00);
    painter->setBrush(g_decorationColors->buttonHover);
    painter->drawRect(oneByThree01);
    painter->setBrush(KColorUtils::mix(titlebarBackgroundActive, g_decorationColors->buttonHover, 0.8)); // TODO:inactive
    painter->drawRect(oneByThree02);
    QIcon backgroundAccent(pixmap);
    m_ui->buttonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::Accent, backgroundAccent);

    pixmap.fill(Qt::transparent);
    painter->setBrush(g_decorationColors->buttonFocus);
    painter->drawRect(twoByThree00);
    painter->setBrush(g_decorationColors->buttonHover);
    painter->drawRect(twoByThree01);
    painter->setBrush(KColorUtils::mix(titlebarBackgroundActive, g_decorationColors->buttonHover, 0.8)); // TODO:inactive
    painter->drawRect(twoByThree02);
    painter->setBrush(g_decorationColors->negativeLessSaturated);
    painter->drawRect(twoByThree10);
    painter->setBrush(g_decorationColors->negativeSaturated);
    painter->drawRect(twoByThree11);
    painter->setBrush(m_internalSettings->negativeCloseBackgroundHoverPress() ? KColorUtils::mix(titlebarBackgroundActive, g_decorationColors->buttonHover, 0.8)
                                                                              : g_decorationColors->negative);
    painter->drawRect(twoByThree12);
    QIcon backgroundAccentNegativeClose(pixmap);
    m_ui->buttonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose, backgroundAccentNegativeClose);

    pixmap.fill(Qt::transparent);
    painter->setBrush(g_decorationColors->buttonFocus);
    painter->drawRect(fourByThree00);
    painter->setBrush(g_decorationColors->buttonHover);
    painter->drawRect(fourByThree01);
    painter->setBrush(KColorUtils::mix(titlebarBackgroundActive, g_decorationColors->buttonHover, 0.8)); // TODO:inactive
    painter->drawRect(fourByThree02);
    painter->setBrush(g_decorationColors->neutralSaturated);
    painter->drawRect(fourByThree10);
    painter->setBrush(g_decorationColors->neutral);
    painter->drawRect(fourByThree11);
    painter->setBrush(m_internalSettings->negativeCloseBackgroundHoverPress() ? KColorUtils::mix(titlebarBackgroundActive, g_decorationColors->buttonHover, 0.8)
                                                                              : g_decorationColors->neutralLessSaturated);
    painter->drawRect(fourByThree12);
    painter->setBrush(g_decorationColors->positiveSaturated);
    painter->drawRect(fourByThree20);
    painter->setBrush(g_decorationColors->positive);
    painter->drawRect(fourByThree21);
    painter->setBrush(m_internalSettings->negativeCloseBackgroundHoverPress() ? KColorUtils::mix(titlebarBackgroundActive, g_decorationColors->buttonHover, 0.8)
                                                                              : g_decorationColors->positiveLessSaturated);
    painter->drawRect(fourByThree22);
    painter->setBrush(g_decorationColors->negativeLessSaturated);
    painter->drawRect(fourByThree30);
    painter->setBrush(g_decorationColors->negativeSaturated);
    painter->drawRect(fourByThree31);
    painter->setBrush(m_internalSettings->negativeCloseBackgroundHoverPress() ? KColorUtils::mix(titlebarBackgroundActive, g_decorationColors->buttonHover, 0.8)
                                                                              : g_decorationColors->negative);
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
    painter->setBrush(g_decorationColors->negativeSaturated);
    painter->drawRect(RightRect);
    QIcon iconTitlebarTextNegativeClose(pixmap);
    m_ui->buttonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose, iconTitlebarTextNegativeClose);

    pixmap.fill(Qt::transparent);
    painter->setBrush(g_decorationColors->highlight);
    painter->drawRect(wholeRect);
    QIcon iconAccent(pixmap);
    m_ui->buttonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::Accent, iconAccent);

    pixmap.fill(Qt::transparent);
    painter->setBrush(g_decorationColors->highlight);
    painter->drawRect(leftRect);
    painter->setBrush(g_decorationColors->negativeSaturated);
    painter->drawRect(RightRect);
    QIcon iconAccentNegativeClose(pixmap);
    m_ui->buttonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::AccentNegativeClose, iconAccentNegativeClose);

    pixmap.fill(Qt::transparent);
    painter->setBrush(g_decorationColors->highlight);
    painter->drawRect(leftQuarter);
    painter->setBrush(g_decorationColors->neutral);
    painter->drawRect(leftMidQuarter);
    painter->setBrush(g_decorationColors->positiveSaturated);
    painter->drawRect(rightMidQuarter);
    painter->setBrush(g_decorationColors->negativeSaturated);
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
        closeIconColor = g_decorationColors->negativeSaturated;
        othersIconColor = titlebarTextActive;
        break;
    case InternalSettings::EnumButtonIconColors::Accent:
        closeIconColor = g_decorationColors->highlight;
        othersIconColor = g_decorationColors->highlight;
        break;
    case InternalSettings::EnumButtonIconColors::AccentNegativeClose:
        closeIconColor = g_decorationColors->negativeSaturated;
        othersIconColor = g_decorationColors->highlight;
        break;
    case InternalSettings::EnumButtonIconColors::AccentTrafficLights:
        closeIconColor = g_decorationColors->negativeSaturated;
        othersIconColor = g_decorationColors->highlight;
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
