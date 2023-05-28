#ifndef breezeconfigwidget_h
#define breezeconfigwidget_h
//////////////////////////////////////////////////////////////////////////////
// breezeconfigwidget.h
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breeze.h"
#include "breezeexceptionlistwidget.h"
#include "breezesettings.h"
#include "buttonsizing.h"
#include "loadpreset.h"
#include "ui_breezeconfigurationui.h"
#include "windowoutlineopacity.h"

#include <KCModule>
#include <KSharedConfig>

#include <QSharedPointer>
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

protected Q_SLOTS:

    //* update changed state
    virtual void updateChanged();
    void setEnabledAnimationsSpeed();
    void setEnabledTransparentTitlebarOptions();
    void updateIconsStackedWidgetVisible();
    void updateBackgroundShapeStackedWidgetVisible();
    void updateCustomColorStackedWidgetVisible();
    void titlebarTopMarginChanged();
    void titlebarBottomMarginChanged();
    void titlebarLeftMarginChanged();
    void titlebarRightMarginChanged();
    void integratedRoundedRectangleSizingButtonClicked();
    void fullHeightRectangleSizingButtonClicked();
    void buttonSizingButtonClicked();
    void windowOutlineShadowColorOpacityButtonClicked()
    {
        windowOutlineButtonClicked(0);
    }
    void windowOutlineContrastOpacityButtonClicked()
    {
        windowOutlineButtonClicked(1);
    }
    void windowOutlineAccentColorOpacityButtonClicked()
    {
        windowOutlineButtonClicked(2);
    }
    void windowOutlineAccentWithContrastOpacityButtonClicked()
    {
        windowOutlineButtonClicked(3);
    }
    void windowOutlineCustomColorOpacityButtonClicked()
    {
        windowOutlineButtonClicked(4);
    }
    void windowOutlineCustomWithContrastOpacityButtonClicked()
    {
        windowOutlineButtonClicked(5);
    }
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
    WindowOutlineOpacity *m_windowOutlineOpacityDialog;
    LoadPreset *m_loadPresetDialog;

    void windowOutlineButtonClicked(int index);
    QString presetGroupName(QString str);
    void writePreset(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &groupName);

    // system colour scheme alpha settings
    void getTitlebarOpacityFromColorScheme();
    bool m_translucentActiveSchemeColor = false;
    bool m_translucentInactiveSchemeColor = false;
    qreal m_activeSchemeColorAlpha = 1;
    qreal m_inactiveSchemeColorAlpha = 1;
};
}

#endif
