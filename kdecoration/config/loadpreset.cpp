/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "loadpreset.h"
#include "addpreset.h"
#include "breezeconfigwidget.h"
#include "dbusmessages.h"
#include "presetsmodel.h"
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <memory>

namespace Breeze
{
LoadPreset::LoadPreset(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QObject *parent)
    : QDialog(static_cast<ConfigWidget *>(parent)->widget())
    , m_ui(new Ui_LoadPreset)
    , m_addDialog(new AddPreset)
    , m_configuration(config)
    , m_presetsConfiguration(presetsConfig)
    , m_parent(parent)
{
    m_ui->setupUi(this);

    connect(m_ui->addButton, &QAbstractButton::clicked, this, &LoadPreset::addButtonClicked);
    connect(m_ui->loadButton, &QAbstractButton::clicked, this, &LoadPreset::loadButtonClicked);
    connect(m_ui->removeButton, &QAbstractButton::clicked, this, &LoadPreset::removeButtonClicked);
    connect(m_ui->presetsList, &QListWidget::itemSelectionChanged, this, &LoadPreset::presetsListActivated);

    connect(m_ui->exportButton, &QAbstractButton::clicked, this, &LoadPreset::exportButtonClicked);
    connect(m_addDialog, &AddPreset::importClicked, this, &LoadPreset::importButtonClicked);
}

LoadPreset::~LoadPreset()
{
    delete m_ui;
    delete m_addDialog;
}

void LoadPreset::initPresetsList()
{
    m_ui->loadButton->setEnabled(false);
    m_ui->removeButton->setEnabled(false);
    m_ui->exportButton->setEnabled(false);
    m_ui->presetsList->clear();
    QStringList presets(PresetsModel::readPresetsList(m_presetsConfiguration.data()));
    for (const QString &presetName : presets) {
        m_ui->presetsList->addItem(presetName);
    }
    m_ui->presetsList->setFocus();
}

void LoadPreset::addButtonClicked()
{
    m_addDialog->setWindowTitle(i18n("Add Preset - Klassy Settings"));
    m_addDialog->m_ui->presetName->clear();
    m_addDialog->m_ui->presetName->setFocus();

    if (!m_addDialog->exec()) {
        return;
    }

    QRegularExpression re("\\w+");
    while (!re.match(m_addDialog->m_ui->presetName->text()).hasMatch()) {
        QMessageBox::warning(this, i18n("Warning - Klassy Settings"), i18n("Please provide a name for the Preset"));
        m_addDialog->setWindowTitle(i18n("Add Preset - Klassy Settings"));
        m_addDialog->m_ui->presetName->setFocus();
        if (m_addDialog->exec() == QDialog::Rejected) {
            return;
        }
    }

    // if a preset already exists with the same name
    if ((m_ui->presetsList->findItems(m_addDialog->m_ui->presetName->text(), Qt::MatchExactly)).count()) {
        // confirmation dialog
        QMessageBox messageBox(QMessageBox::Question,
                               i18n("Question - Klassy Settings"),
                               i18n("A preset with the name \"") + m_addDialog->m_ui->presetName->text() + i18n("\" already exists. Overwrite?"),
                               QMessageBox::Yes | QMessageBox::Cancel);
        messageBox.button(QMessageBox::Yes)->setText(i18n("Overwrite"));
        messageBox.setDefaultButton(QMessageBox::Cancel);
        if (messageBox.exec() == QMessageBox::Cancel) {
            return;
        }
    }

    static_cast<ConfigWidget *>(m_parent)->saveMain(m_addDialog->m_ui->presetName->text());
    initPresetsList();
}

void LoadPreset::presetsListActivated()
{
    if (!m_ui->presetsList->selectedItems().count())
        return;
    m_ui->loadButton->setEnabled(true);
    m_ui->removeButton->setEnabled(true);
    m_ui->exportButton->setEnabled(true);
}

void LoadPreset::loadButtonClicked()
{
    InternalSettingsPtr internalSettings = InternalSettingsPtr(new InternalSettings());

    if (m_ui->presetsList->selectedItems().count()) {
        PresetsModel::loadPresetAndSave(internalSettings.data(),
                                        m_configuration.data(),
                                        m_presetsConfiguration.data(),
                                        m_ui->presetsList->selectedItems().first()->text(),
                                        true);

        ConfigWidget *configWidget = static_cast<ConfigWidget *>(m_parent);
        configWidget->load();
        DBusMessages::updateDecorationColorCache();
        DBusMessages::kwinReloadConfig();
        configWidget->generateSystemIcons();
    }
}

void LoadPreset::removeButtonClicked()
{
    if (!m_ui->presetsList->selectedItems().count())
        return;

    // confirmation dialog
    QMessageBox messageBox(QMessageBox::Question,
                           i18n("Question - Klassy Settings"),
                           i18n("Remove \"") + m_ui->presetsList->selectedItems().first()->text() + i18n("\" preset?"),
                           QMessageBox::Yes | QMessageBox::Cancel);
    messageBox.button(QMessageBox::Yes)->setText(i18n("Remove"));
    messageBox.setDefaultButton(QMessageBox::Cancel);
    if (messageBox.exec() == QMessageBox::Cancel) {
        return;
    }

    PresetsModel::deletePreset(m_presetsConfiguration.data(), m_ui->presetsList->selectedItems().first()->text());
    m_presetsConfiguration->sync();
    initPresetsList();
}

void LoadPreset::importButtonClicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this, i18n("Select Klassy Preset to Import"), "", i18n("Klassy Preset (*.klpw)"));
    for (QString filePath : files) {
        QString presetName;
        QString error;

        // if a preset already exists with the same name
        if (PresetsModel::isPresetFromFilePresent(m_presetsConfiguration.data(), filePath, presetName)) {
            // confirmation dialog
            QMessageBox messageBox(QMessageBox::Question,
                                   i18n("Question - Klassy Settings"),
                                   i18n("A preset with the name \"") + presetName + i18n("\" already exists. Overwrite?"),
                                   QMessageBox::Yes | QMessageBox::Cancel);
            messageBox.button(QMessageBox::Yes)->setText(i18n("Overwrite"));
            messageBox.setDefaultButton(QMessageBox::Cancel);
            if (messageBox.exec() == QMessageBox::Cancel) {
                continue;
            }
            presetName = QString();
        }

        PresetsErrorFlag importErrors = PresetsModel::importPreset(m_presetsConfiguration.data(), filePath, presetName, error, false);

        if (importErrors == PresetsErrorFlag::InvalidGlobalGroup) {
            QMessageBox msgBox;
            msgBox.setText(i18n("Invalid Klassy Preset file at \"") + filePath + i18n("\"."));
            msgBox.exec();
            continue;
        }

        if (importErrors == PresetsErrorFlag::InvalidVersion) {
            // confirmation dialog
            QMessageBox messageBox(QMessageBox::Question,
                                   i18n("Question - Klassy Settings"),
                                   i18n("The file to import at \"") + filePath
                                       + i18n("\" was created for a different version of Klassy.\n Try to import anyway?"),
                                   QMessageBox::Yes | QMessageBox::Cancel);
            messageBox.button(QMessageBox::Yes)->setText(i18n("Continue Import"));
            messageBox.setDefaultButton(QMessageBox::Cancel);
            if (messageBox.exec() == QMessageBox::Cancel) {
                continue;
            }

            // reset and try again
            presetName = QString();
            error = QString();
            importErrors = PresetsModel::importPreset(m_presetsConfiguration.data(), filePath, presetName, error, true);
        }

        if (importErrors == PresetsErrorFlag::InvalidGroup) {
            QMessageBox msgBox;
            msgBox.setText(i18n("No preset group found in Klassy Preset file at \"") + filePath + i18n("\"."));
            msgBox.exec();
            continue;
        }

        if (importErrors == PresetsErrorFlag::InvalidKey) {
            QMessageBox msgBox;
            msgBox.setText(i18n("Invalid key \"") + error + i18n("\" in Klassy Preset file at \"") + filePath + i18n("\"."));
            msgBox.exec();
            continue;
        }

        m_presetsConfiguration->sync();
        initPresetsList();
    }
}

void LoadPreset::exportButtonClicked()
{
    if (!m_ui->presetsList->selectedItems().count())
        return;

    QString fileBaseName = m_ui->presetsList->selectedItems().first()->text();
    fileBaseName = fileBaseName.simplified(); // replace whitespace with spaces
    fileBaseName.replace(" ", "_"); // replace spaces with underscores

    QString filePath = QFileDialog::getSaveFileName(this, i18n("Export Klassy Preset to File"), "~/" + fileBaseName + ".klpw", i18n("Klassy Preset (*.klpw)"));
    if (filePath.isEmpty())
        return;
    QDir dir;
    if (dir.exists(filePath))
        dir.remove(filePath);

    PresetsModel::exportPreset(m_presetsConfiguration.data(), m_ui->presetsList->selectedItems().first()->text(), filePath);
}
}
