/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "loadpreset.h"
#include "addpreset.h"
#include "breezeconfigwidget.h"
#include "presetsmodel.h"
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <memory>

namespace Breeze
{
LoadPreset::LoadPreset(KSharedConfig::Ptr config, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui_LoadPreset)
    , m_addDialog(new AddPreset)
    , m_configuration(config)
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
    QStringList presets(PresetsModel::readPresetsList(m_configuration.data()));
    foreach (const QString presetName, presets) {
        m_ui->presetsList->addItem(presetName);
    }
}

void LoadPreset::addButtonClicked()
{
    m_addDialog->setWindowTitle(i18n("Add Preset - Klassy Settings"));
    m_addDialog->m_ui->presetName->clear();

    if (!m_addDialog->exec()) {
        return;
    }

    QRegularExpression re("\\w+");
    while (!re.match(m_addDialog->m_ui->presetName->text()).hasMatch()) {
        QMessageBox::warning(this, i18n("Warning - Klassy Settings"), i18n("Please provide a name for the Preset"));
        m_addDialog->setWindowTitle(i18n("Add Preset - Klassy Settings"));
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

    ConfigWidget *configWidget = qobject_cast<ConfigWidget *>(m_parent);
    if (configWidget)
        configWidget->saveMain(m_addDialog->m_ui->presetName->text());
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
    if (m_ui->presetsList->selectedItems().count()) {
        ConfigWidget *configWidget = qobject_cast<ConfigWidget *>(m_parent);
        if (configWidget)
            configWidget->loadMain(m_ui->presetsList->selectedItems().first()->text());
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

    PresetsModel::deletePreset(m_configuration.data(), m_ui->presetsList->selectedItems().first()->text());
    initPresetsList();
}

void LoadPreset::importButtonClicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this, i18n("Select Klassy Preset to Import"), "", i18n("Klassy Preset (*.klp)"));
    for (QString fileName : files) {
        KSharedConfig::Ptr importPresetConfig;
        bool validGlobalGroup;
        bool versionValid;
        QString presetName;

        PresetsModel::importPresetValidate(fileName, importPresetConfig, validGlobalGroup, versionValid, presetName);
        if (!validGlobalGroup) {
            QMessageBox msgBox;
            msgBox.setText(i18n("Invalid Klassy Preset file at \"") + fileName + i18n("\"."));
            msgBox.exec();
            continue;
        }

        if (!versionValid) {
            // confirmation dialog
            QMessageBox messageBox(QMessageBox::Question,
                                   i18n("Question - Klassy Settings"),
                                   i18n("The file to import at \"") + fileName
                                       + i18n("\" was created for a different version of Klassy.\n Try to import anyway?"),
                                   QMessageBox::Yes | QMessageBox::Cancel);
            messageBox.button(QMessageBox::Yes)->setText(i18n("Continue Import"));
            messageBox.setDefaultButton(QMessageBox::Cancel);
            if (messageBox.exec() == QMessageBox::Cancel) {
                continue;
            }
        }

        // if a preset already exists with the same name
        if ((m_ui->presetsList->findItems(presetName, Qt::MatchExactly)).count()) {
            // confirmation dialog
            QMessageBox messageBox(QMessageBox::Question,
                                   i18n("Question - Klassy Settings"),
                                   i18n("A preset with the name \"") + presetName + i18n("\" already exists. Overwrite?"),
                                   QMessageBox::Yes | QMessageBox::Cancel);
            messageBox.button(QMessageBox::Yes)->setText(i18n("Overwrite"));
            messageBox.setDefaultButton(QMessageBox::Cancel);
            if (messageBox.exec() == QMessageBox::Cancel) {
                continue;
            } else {
                PresetsModel::deletePreset(m_configuration.data(), presetName);
            }
        }

        PresetsModel::importPreset(m_configuration.data(), importPresetConfig, presetName);
        m_configuration->sync();
        initPresetsList();
    }
}

void LoadPreset::exportButtonClicked()
{
    if (!m_ui->presetsList->selectedItems().count())
        return;

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    i18n("Export Klassy Preset to File"),
                                                    "~/" + m_ui->presetsList->selectedItems().first()->text() + ".klp",
                                                    i18n("Klassy Preset (*.klp)"));
    if (fileName.isEmpty())
        return;
    QDir dir;
    if (dir.exists(fileName))
        dir.remove(fileName);

    PresetsModel::exportPreset(m_configuration.data(), m_ui->presetsList->selectedItems().first()->text(), fileName);
}
}
