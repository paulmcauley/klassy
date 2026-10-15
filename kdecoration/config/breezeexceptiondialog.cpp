//////////////////////////////////////////////////////////////////////////////
// breezeexceptiondialog.cpp
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
// SPDX-FileCopyrightText: 2023-2026 Paul A McAuley <kde@paulmcauley.com>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezeexceptiondialog.h"
#include "breezedetectwidget.h"
#include "presetsmodel.h"

namespace Klassy
{

//___________________________________________
ExceptionDialog::ExceptionDialog(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QWidget *parent)
    : QDialog(parent)
    , m_configuration(config)
    , m_presetsConfiguration(presetsConfig)
{
    m_ui.setupUi(this);

    // disable controls if this is the default exceptions list
    QWidget *parentExceptionListWidget = parentWidget();
    if (parentExceptionListWidget && parentExceptionListWidget->objectName() == "bundledExceptions") {
        m_ui.borderSizeCheckBox->setDisabled(true);
        m_ui.borderSizeComboBox->setDisabled(true);
        m_ui.detectDialogButton->setDisabled(true);
        m_ui.buttonBox->setHidden(true);
        m_ui.exceptionProgramNameEditor->setDisabled(true);
        m_ui.exceptionWindowPropertyEditor->setDisabled(true);
        m_ui.exceptionWindowPropertyType->setDisabled(true);
        m_ui.exceptionHideTitleBarCheckBox->setDisabled(true);
        m_ui.hideTitleBar->setDisabled(true);
        m_ui.exceptionShowInTitle->setDisabled(true);
        m_ui.exceptionShowTitleDocumentName->setDisabled(true);
        m_ui.exceptionShowTitleAppIcon->setDisabled(true);
        m_ui.exceptionShowTitleAppName->setDisabled(true);
        m_ui.opaqueTitleBar->setDisabled(true);
        m_ui.appMenuBarBehaviourCheckBox->setDisabled(true);
        m_ui.appMenuBarBehaviour->setDisabled(true);
        m_ui.exceptionMatchTitleBarToApplicationColor->setDisabled(true);
        m_ui.preventApplyOpacityToHeader->setDisabled(true);
        m_ui.exceptionPresetCheckBox->setDisabled(true);
        m_ui.exceptionPresetComboBox->setDisabled(true);
    }

    connect(m_ui.buttonBox->button(QDialogButtonBox::Cancel), &QAbstractButton::clicked, this, &QWidget::close);

    // populate exceptionPresetComboBox with presets
    QStringList presets = PresetsModel::readPresetsList(m_presetsConfiguration.data());
    if (presets.count()) {
        presets.sort(Qt::CaseInsensitive);
        for (const QString &preset : presets) {
            m_ui.exceptionPresetComboBox->addItem(preset);
        }
    } else
        m_ui.exceptionPresetCheckBox->setEnabled(false);

    // detect window properties
    connect(m_ui.detectDialogButton, &QAbstractButton::clicked, this, &ExceptionDialog::selectWindowProperties);

    // connections
    // direct connections are used in several places so the slot can detect the immediate m_loading status (not available in a queued connection)
    connect(m_ui.exceptionWindowPropertyType, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.exceptionProgramNameEditor, &QLineEdit::textChanged, this, &ExceptionDialog::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.exceptionWindowPropertyEditor, &QLineEdit::textChanged, this, &ExceptionDialog::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.borderSizeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.borderSizeCheckBox, &QAbstractButton::clicked, this, &ExceptionDialog::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.exceptionHideTitleBarCheckBox, &QAbstractButton::clicked, this, &ExceptionDialog::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.hideTitleBar, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);

    connect(m_ui.exceptionShowInTitle, &QAbstractButton::clicked, this, &ExceptionDialog::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.exceptionShowTitleDocumentName, &QAbstractButton::clicked, this, &ExceptionDialog::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.exceptionShowTitleAppIcon, &QAbstractButton::clicked, this, &ExceptionDialog::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.exceptionShowTitleAppName, &QAbstractButton::clicked, this, &ExceptionDialog::updateChanged, Qt::ConnectionType::DirectConnection);

    connect(m_ui.appMenuBarBehaviourCheckBox, &QAbstractButton::clicked, this, &ExceptionDialog::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.appMenuBarBehaviour, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.opaqueTitleBar, &QAbstractButton::clicked, this, &ExceptionDialog::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.exceptionMatchTitleBarToApplicationColor,
            &QAbstractButton::clicked,
            this,
            &ExceptionDialog::updateChanged,
            Qt::ConnectionType::DirectConnection);
    connect(m_ui.preventApplyOpacityToHeader, &QAbstractButton::clicked, this, &ExceptionDialog::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.exceptionPresetCheckBox, &QAbstractButton::clicked, this, &ExceptionDialog::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.exceptionPresetComboBox, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);

    connect(m_ui.opaqueTitleBar, &QAbstractButton::toggled, this, &ExceptionDialog::onOpaqueTitleBarToggled);
}

// loads saved exception options
void ExceptionDialog::setException(InternalSettingsPtr exception)
{
    m_loading = true;
    // store exception internally
    m_exception = exception;

    // type
    m_ui.exceptionWindowPropertyType->setCurrentIndex(m_exception->exceptionWindowPropertyType());
    m_ui.exceptionProgramNameEditor->setText(m_exception->exceptionProgramNamePattern());
    m_ui.exceptionWindowPropertyEditor->setText(m_exception->exceptionWindowPropertyPattern());

    // exception preset controls
    if (m_exception->exceptionPreset().isEmpty()) {
        m_ui.exceptionPresetCheckBox->setChecked(false);
    } else {
        if (PresetsModel::isPresetPresent(m_presetsConfiguration.data(), m_exception->exceptionPreset())) {
            m_ui.exceptionPresetCheckBox->setChecked(true);
            m_ui.exceptionPresetComboBox->setCurrentText(m_exception->exceptionPreset());
        } else
            m_ui.exceptionPresetCheckBox->setChecked(false);
    }

    m_ui.borderSizeComboBox->setCurrentIndex(m_exception->borderSize());
    m_ui.exceptionHideTitleBarCheckBox->setChecked(m_exception->hideTitleBarChecked());
    m_ui.hideTitleBar->setCurrentIndex(m_exception->hideTitleBar());
    m_ui.exceptionShowInTitle->setChecked(m_exception->exceptionShowInTitle());
    m_ui.exceptionShowTitleDocumentName->setChecked(m_exception->exceptionShowTitleDocumentName());
    m_ui.exceptionShowTitleAppIcon->setChecked(m_exception->exceptionShowTitleAppIcon());
    m_ui.exceptionShowTitleAppName->setChecked(m_exception->exceptionShowTitleAppName());
    m_ui.appMenuBarBehaviourCheckBox->setChecked(m_exception->exceptionAppMenuBarBehaviourChecked());
    m_ui.appMenuBarBehaviour->setCurrentIndex(m_exception->exceptionAppMenuBarBehaviour());
    m_ui.opaqueTitleBar->setChecked(m_exception->opaqueTitleBar());
    m_ui.exceptionMatchTitleBarToApplicationColor->setChecked(m_exception->exceptionMatchTitleBarToApplicationColor());
    m_ui.preventApplyOpacityToHeader->setChecked(m_exception->preventApplyOpacityToHeader());
    m_ui.borderSizeCheckBox->setChecked(m_exception->exceptionBorder());

    setChanged(false);
    m_loading = false;
}

//___________________________________________
void ExceptionDialog::save()
{
    m_exception->setExceptionWindowPropertyType(m_ui.exceptionWindowPropertyType->currentIndex());
    m_exception->setExceptionProgramNamePattern(m_ui.exceptionProgramNameEditor->text());
    m_exception->setExceptionWindowPropertyPattern(m_ui.exceptionWindowPropertyEditor->text());

    if (m_ui.exceptionPresetCheckBox->isChecked()) {
        m_exception->setExceptionPreset(m_ui.exceptionPresetComboBox->currentText());
    } else {
        m_exception->setExceptionPreset(QString());
    }

    m_exception->setBorderSize(m_ui.borderSizeComboBox->currentIndex());
    m_exception->setHideTitleBarChecked(m_ui.exceptionHideTitleBarCheckBox->isChecked());
    m_exception->setHideTitleBar(m_ui.hideTitleBar->currentIndex());
    m_exception->setExceptionShowInTitle(m_ui.exceptionShowInTitle->isChecked());
    m_exception->setExceptionShowTitleDocumentName(m_ui.exceptionShowTitleDocumentName->isChecked());
    m_exception->setExceptionShowTitleAppIcon(m_ui.exceptionShowTitleAppIcon->isChecked());
    m_exception->setExceptionShowTitleAppName(m_ui.exceptionShowTitleAppName->isChecked());
    m_exception->setExceptionAppMenuBarBehaviourChecked(m_ui.appMenuBarBehaviourCheckBox->isChecked());
    m_exception->setExceptionAppMenuBarBehaviour(m_ui.appMenuBarBehaviour->currentIndex());
    m_exception->setOpaqueTitleBar(m_ui.opaqueTitleBar->isChecked());
    m_exception->setExceptionMatchTitleBarToApplicationColor(m_ui.exceptionMatchTitleBarToApplicationColor->isChecked());
    m_exception->setPreventApplyOpacityToHeader(m_ui.preventApplyOpacityToHeader->isChecked());
    m_exception->setExceptionBorder(m_ui.borderSizeCheckBox->isChecked());

    setChanged(false);
}

//___________________________________________
void ExceptionDialog::updateChanged()
{
    if (m_loading)
        return; // only check if the user has made a change to the UI, or user has pressed defaults

    bool modified(false);
    if (m_exception->exceptionWindowPropertyType() != m_ui.exceptionWindowPropertyType->currentIndex())
        modified = true;
    else if (m_exception->exceptionProgramNamePattern() != m_ui.exceptionProgramNameEditor->text())
        modified = true;
    else if (m_exception->exceptionWindowPropertyPattern() != m_ui.exceptionWindowPropertyEditor->text())
        modified = true;
    else if (m_exception->borderSize() != m_ui.borderSizeComboBox->currentIndex())
        modified = true;
    else if (m_exception->hideTitleBarChecked() != m_ui.exceptionHideTitleBarCheckBox->isChecked())
        modified = true;
    else if (m_exception->hideTitleBar() != m_ui.hideTitleBar->currentIndex())
        modified = true;
    else if (m_exception->exceptionShowInTitle() != m_ui.exceptionShowInTitle->isChecked())
        modified = true;
    else if (m_exception->exceptionShowTitleDocumentName() != m_ui.exceptionShowTitleDocumentName->isChecked())
        modified = true;
    else if (m_exception->exceptionShowTitleAppIcon() != m_ui.exceptionShowTitleAppIcon->isChecked())
        modified = true;
    else if (m_exception->exceptionShowTitleAppName() != m_ui.exceptionShowTitleAppName->isChecked())
        modified = true;
    else if (m_exception->exceptionAppMenuBarBehaviourChecked() != m_ui.appMenuBarBehaviourCheckBox->isChecked())
        modified = true;
    else if (m_exception->exceptionAppMenuBarBehaviour() != m_ui.appMenuBarBehaviour->currentIndex())
        modified = true;
    else if (m_exception->opaqueTitleBar() != m_ui.opaqueTitleBar->isChecked())
        modified = true;
    else if (m_exception->exceptionMatchTitleBarToApplicationColor() != m_ui.exceptionMatchTitleBarToApplicationColor->isChecked())
        modified = true;
    else if (m_exception->preventApplyOpacityToHeader() != m_ui.preventApplyOpacityToHeader->isChecked())
        modified = true;
    else if (m_exception->exceptionPreset() != m_ui.exceptionPresetComboBox->currentText())
        modified = true;
    else if (!m_exception->exceptionPreset().isEmpty() && !m_ui.exceptionPresetCheckBox->isChecked())
        modified = true;
    else if (m_exception->exceptionBorder() != m_ui.borderSizeCheckBox->isChecked())
        modified = true;

    setChanged(modified);
}

//___________________________________________
void ExceptionDialog::selectWindowProperties()
{
    // create widget
    if (!m_detectDialog) {
        m_detectDialog = new DetectDialog(this);
        connect(m_detectDialog, &DetectDialog::detectionDone, this, &ExceptionDialog::readWindowProperties);
    }

    m_detectDialog->detect();
}

//___________________________________________
void ExceptionDialog::readWindowProperties(bool valid)
{
    Q_CHECK_PTR(m_detectDialog);
    if (valid) {
        // window info
        const QVariantMap properties = m_detectDialog->properties();

        switch (m_ui.exceptionWindowPropertyType->currentIndex()) {
        default:
        case InternalSettings::EnumExceptionWindowPropertyType::ExceptionWindowClassName:
            m_ui.exceptionWindowPropertyEditor->setText(properties.value(QStringLiteral("resourceClass")).toString());
            break;

        case InternalSettings::EnumExceptionWindowPropertyType::ExceptionWindowTitle:
            m_ui.exceptionWindowPropertyEditor->setText(properties.value(QStringLiteral("caption")).toString());
            break;
        }
    }

    delete m_detectDialog;
    m_detectDialog = nullptr;
}
void ExceptionDialog::onOpaqueTitleBarToggled(bool toggled)
{
    if (toggled) {
        m_ui.preventApplyOpacityToHeader->setChecked(true);
    }
}
}
