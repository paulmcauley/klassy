#ifndef breezeconfigwidget_h
#define breezeconfigwidget_h
//////////////////////////////////////////////////////////////////////////////
// breezeconfigwidget.h
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
// SPDX-FileCopyrightText: 2021-2023 Paul A McAuley <kde@paulmcauley.com>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breeze.h"
#include "breezeexceptionlistwidget.h"
#include "breezesettings.h"
#include "buttonbehaviour.h"
#include "buttoncolors.h"
#include "buttonsizing.h"
#include "loadpreset.h"
#include "shadowstyle.h"
#include "titlebaropacity.h"
#include "titlebarspacing.h"
#include "ui_breezeconfigurationui.h"
#include "windowoutlinestyle.h"

#include <KCModule>
#include <KSharedConfig>

#include <QSharedPointer>
#include <QTimer>
#include <QWidget>

// needed to display images when qrc is statically linked
// must be in global namespace to work
void initKlassydecorationConfigQrc();
void cleanupKlassydecorationConfigQrc();

namespace Breeze
{

//_____________________________________________
class ConfigWidget : public KCModule
{
    Q_OBJECT

public:
    //* constructor
    explicit ConfigWidget(QWidget *, const QVariantList &);

    //* destructor
    virtual ~ConfigWidget();

    //* default
    void defaults() override;

    //* load configuration
    void load() override;
    void loadMain(QString loadPresetName = QString());

    //* save configuration
    void save() override;
    void saveMain(QString saveAsPresetName = QString());

    static void kwinReloadConfig();
    static void kstyleReloadConfig();

protected Q_SLOTS:

    //* update changed state
    virtual void updateChanged();
    void setEnabledAnimationsSpeed();
    void updateIconsStackedWidgetVisible();
    void updateBackgroundShapeStackedWidgetVisible();
    void integratedRoundedRectangleSizingButtonClicked();
    void fullHeightRectangleSizingButtonClicked();
    void buttonSizingButtonClicked();
    void buttonColorsButtonClicked();
    void buttonBehaviourButtonClicked();
    void titleBarSpacingButtonClicked();
    void titleBarOpacityButtonClicked();
    void shadowStyleButtonClicked();
    void windowOutlineStyleButtonClicked();
    void presetsButtonClicked();
    void dialogChanged(bool changed);

protected:
    //* set changed state
    void setChanged(bool);

private:
    //* ui
    Ui_BreezeConfigurationUI m_ui;

    QPushButton *m_presetsButton;

    //* kconfiguration object
    KSharedConfig::Ptr m_configuration;

    //* internal exception
    InternalSettingsPtr m_internalSettings;

    //* changed state
    bool m_changed;

    //* defaults clicked
    bool m_defaultsPressed = false;

    bool m_processingDefaults = false;
    bool m_loading = false;

    //* dialogs behind button
    ButtonSizing *m_buttonSizingDialog;
    ButtonColors *m_buttonColorsDialog;
    ButtonBehaviour *m_buttonBehaviourDialog;
    TitleBarSpacing *m_titleBarSpacingDialog;
    TitleBarOpacity *m_titleBarOpacityDialog;
    ShadowStyle *m_shadowStyleDialog;
    WindowOutlineStyle *m_windowOutlineStyleDialog;
    LoadPreset *m_loadPresetDialog;

    QString presetGroupName(QString str);
    void writePreset(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &groupName);

    void importBundledPresets();
};
}

#endif
