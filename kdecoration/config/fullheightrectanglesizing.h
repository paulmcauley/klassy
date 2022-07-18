#ifndef FULLHEIGHTRECTANGLESIZING_H
#define FULLHEIGHTRECTANGLESIZING_H

//////////////////////////////////////////////////////////////////////////////
// fullheightrectanglesizing.h
// -------------------
//
// SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breeze.h"
#include "breezesettings.h"
#include "ui_fullheightrectanglesizing.h"
#include <QDialog>

namespace Breeze
{

class FullHeightRectangleSizing : public QDialog
{
    Q_OBJECT

    friend class ConfigWidget;

public:
    explicit FullHeightRectangleSizing(QWidget *parent = nullptr);
    ~FullHeightRectangleSizing();

    void load();
    void save(const bool reloadKwinConfig = true);
    void defaults();

private slots:
    void accept() override;
    void updateChanged();
    void fullHeightButtonWidthMarginLeftChanged();
    void fullHeightButtonWidthMarginRightChanged();
    void saveAndReloadKWinConfig()
    {
        save(true);
    }
    void setApplyButtonState(const bool on);

signals:
    void changed(bool);

private:
    void setChanged(bool value);

    Ui_FullHeightRectangleSizing m_ui;

    InternalSettingsPtr m_internalSettings;

    //* changed state
    bool m_changed;

    //* defaults clicked
    bool m_defaultsPressed = false;

    bool m_loading;
    bool m_processingDefaults;
};

}

#endif // FULLHEIGHTRECTANGLESIZING_H
