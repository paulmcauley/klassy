//////////////////////////////////////////////////////////////////////////////
// breezeexceptiondialog.cpp
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezeexceptiondialog.h"
#include "breezedetectwidget.h"

namespace Breeze
{

//___________________________________________
ExceptionDialog::ExceptionDialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    connect(m_ui.buttonBox->button(QDialogButtonBox::Cancel), &QAbstractButton::clicked, this, &QWidget::close);

    // store checkboxes from ui into list
    m_checkboxes.insert(BorderSize, m_ui.borderSizeCheckBox);

    // detect window properties
    connect(m_ui.detectDialogButton, &QAbstractButton::clicked, this, &ExceptionDialog::selectWindowProperties);

    // connections
    connect(m_ui.exceptionWindowPropertyType, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui.exceptionProgramNameEditor, &QLineEdit::textChanged, this, &ExceptionDialog::updateChanged);
    connect(m_ui.exceptionWindowPropertyEditor, &QLineEdit::textChanged, this, &ExceptionDialog::updateChanged);
    connect(m_ui.borderSizeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));

    for (CheckBoxMap::iterator iter = m_checkboxes.begin(); iter != m_checkboxes.end(); ++iter) {
        connect(iter.value(), &QAbstractButton::clicked, this, &ExceptionDialog::updateChanged);
    }

    connect(m_ui.hideTitleBar, &QAbstractButton::clicked, this, &ExceptionDialog::updateChanged);
    connect(m_ui.opaqueTitleBar, &QAbstractButton::clicked, this, &ExceptionDialog::updateChanged);
    connect(m_ui.preventApplyOpacityToHeader, &QAbstractButton::clicked, this, &ExceptionDialog::updateChanged);

    connect(m_ui.opaqueTitleBar, &QAbstractButton::toggled, this, &ExceptionDialog::onOpaqueTitleBarToggled);
}

//___________________________________________
void ExceptionDialog::setException(InternalSettingsPtr exception)
{
    // store exception internally
    m_exception = exception;

    // type
    m_ui.exceptionWindowPropertyType->setCurrentIndex(m_exception->exceptionWindowPropertyType());
    m_ui.exceptionProgramNameEditor->setText(m_exception->exceptionProgramNamePattern());
    m_ui.exceptionWindowPropertyEditor->setText(m_exception->exceptionWindowPropertyPattern());
    m_ui.borderSizeComboBox->setCurrentIndex(m_exception->borderSize());
    m_ui.hideTitleBar->setChecked(m_exception->hideTitleBar());
    m_ui.opaqueTitleBar->setChecked(m_exception->opaqueTitleBar());
    m_ui.preventApplyOpacityToHeader->setChecked(m_exception->preventApplyOpacityToHeader());

    // mask
    for (CheckBoxMap::iterator iter = m_checkboxes.begin(); iter != m_checkboxes.end(); ++iter) {
        iter.value()->setChecked(m_exception->mask() & iter.key());
    }

    setChanged(false);
}

//___________________________________________
void ExceptionDialog::save()
{
    m_exception->setExceptionWindowPropertyType(m_ui.exceptionWindowPropertyType->currentIndex());
    m_exception->setExceptionProgramNamePattern(m_ui.exceptionProgramNameEditor->text());
    m_exception->setExceptionWindowPropertyPattern(m_ui.exceptionWindowPropertyEditor->text());
    m_exception->setBorderSize(m_ui.borderSizeComboBox->currentIndex());
    m_exception->setHideTitleBar(m_ui.hideTitleBar->isChecked());
    m_exception->setOpaqueTitleBar(m_ui.opaqueTitleBar->isChecked());
    m_exception->setPreventApplyOpacityToHeader(m_ui.preventApplyOpacityToHeader->isChecked());

    // mask
    unsigned int mask = None;
    for (CheckBoxMap::iterator iter = m_checkboxes.begin(); iter != m_checkboxes.end(); ++iter) {
        if (iter.value()->isChecked())
            mask |= iter.key();
    }

    m_exception->setMask(mask);

    setChanged(false);
}

//___________________________________________
void ExceptionDialog::updateChanged()
{
    bool modified(false);
    if (m_exception->exceptionWindowPropertyType() != m_ui.exceptionWindowPropertyType->currentIndex())
        modified = true;
    else if (m_exception->exceptionProgramNamePattern() != m_ui.exceptionProgramNameEditor->text())
        modified = true;
    else if (m_exception->exceptionWindowPropertyPattern() != m_ui.exceptionWindowPropertyEditor->text())
        modified = true;
    else if (m_exception->borderSize() != m_ui.borderSizeComboBox->currentIndex())
        modified = true;
    else if (m_exception->hideTitleBar() != m_ui.hideTitleBar->isChecked())
        modified = true;
    else if (m_exception->opaqueTitleBar() != m_ui.opaqueTitleBar->isChecked())
        modified = true;
    else if (m_exception->preventApplyOpacityToHeader() != m_ui.preventApplyOpacityToHeader->isChecked())
        modified = true;
    else {
        // check mask
        for (CheckBoxMap::iterator iter = m_checkboxes.begin(); iter != m_checkboxes.end(); ++iter) {
            if (iter.value()->isChecked() != (bool)(m_exception->mask() & iter.key())) {
                modified = true;
                break;
            }
        }
    }

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
        case InternalSettings::ExceptionWindowClassName:
            m_ui.exceptionWindowPropertyEditor->setText(properties.value(QStringLiteral("resourceClass")).toString());
            break;

        case InternalSettings::ExceptionWindowTitle:
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
