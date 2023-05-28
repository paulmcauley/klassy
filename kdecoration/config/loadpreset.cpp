#include "loadpreset.h"
#include "addpreset.h"
#include "breezeconfigwidget.h"
#include "presetsmodel.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <memory>

namespace Breeze
{
LoadPreset::LoadPreset(KSharedConfig::Ptr config, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui_LoadPreset)
    , m_configuration(config)
    , m_parent(parent)
{
    m_ui->setupUi(this);

    connect(m_ui->addButton, &QAbstractButton::clicked, this, &LoadPreset::addButtonClicked);
    connect(m_ui->loadButton, &QAbstractButton::clicked, this, &LoadPreset::loadButtonClicked);
    connect(m_ui->removeButton, &QAbstractButton::clicked, this, &LoadPreset::removeButtonClicked);
    connect(m_ui->presetsList, &QListWidget::activated, this, &LoadPreset::presetsListActivated);
}

LoadPreset::~LoadPreset()
{
    delete m_ui;
}

void LoadPreset::initPresetsList()
{
    m_ui->loadButton->setEnabled(false);
    m_ui->removeButton->setEnabled(false);
    m_ui->presetsList->clear();
    QRegularExpression re("^Windeco Preset (.+)");
    foreach (auto group, m_configuration->groupList()) {
        QRegularExpressionMatch match = re.match(group);
        if (match.hasMatch()) {
            QString presetName = match.captured(1);
            m_ui->presetsList->addItem(presetName);
        }
    }
}

void LoadPreset::addButtonClicked()
{
    AddPreset *addDialog = new AddPreset();
    addDialog->setWindowTitle(i18n("Add Preset - Klassy Settings"));

    if (!addDialog->exec()) {
        delete addDialog;
        return;
    }

    QRegularExpression re("\\w+");
    while (!re.match(addDialog->m_ui->presetName->text()).hasMatch()) {
        QMessageBox::warning(this, i18n("Warning - Klassy Settings"), i18n("Please provide a name for the Preset"));
        delete addDialog;
        addDialog = new AddPreset();
        addDialog->setWindowTitle(i18n("Add Preset - Klassy Settings"));
        if (addDialog->exec() == QDialog::Rejected) {
            delete addDialog;
            return;
        }
    }

    // if a preset already exists with the same name
    if ((m_ui->presetsList->findItems(addDialog->m_ui->presetName->text(), Qt::MatchExactly)).count()) {
        // confirmation dialog
        QMessageBox messageBox(QMessageBox::Question,
                               i18n("Question - Klassy Settings"),
                               i18n("A preset with the name \"") + addDialog->m_ui->presetName->text() + i18n("\" already exists. Overwrite?"),
                               QMessageBox::Yes | QMessageBox::Cancel);
        messageBox.button(QMessageBox::Yes)->setText(i18n("Overwrite"));
        messageBox.setDefaultButton(QMessageBox::Cancel);
        if (messageBox.exec() == QMessageBox::Cancel) {
            delete addDialog;
            return;
        }
    }

    ConfigWidget *configWidget = qobject_cast<ConfigWidget *>(m_parent);
    if (configWidget)
        configWidget->saveMain(addDialog->m_ui->presetName->text());
    delete addDialog;
    initPresetsList();
}

void LoadPreset::presetsListActivated()
{
    m_ui->loadButton->setEnabled(true);
    m_ui->removeButton->setEnabled(true);
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

}
