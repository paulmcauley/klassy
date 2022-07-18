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
#include "fullheightrectanglesizing.h"
#include "ui_breezeconfigurationui.h"

#include <KCModule>
#include <KSharedConfig>

#include <QSharedPointer>
#include <QWidget>

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
    virtual ~ConfigWidget() = default;

    //* default
    void defaults() override;

    //* load configuration
    void load() override;

    //* save configuration
    void save() override;

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
    void buttonSpacingLeftChanged();
    void buttonSpacingRightChanged();
    void fullHeightIntegratedRoundedRectangleSizingButtonClicked();
    void fullHeightRectangleSizingButtonClicked();
    void dialogChanged(bool changed);

protected:
    //* set changed state
    void setChanged(bool);

private:
    //* ui
    Ui_BreezeConfigurationUI m_ui;

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
    FullHeightRectangleSizing *m_fullHeightRectangleSizingDialog;

    // system colour scheme alpha settings
    void getTitlebarOpacityFromColorScheme();
    bool m_translucentActiveSchemeColor = false;
    bool m_translucentInactiveSchemeColor = false;
    qreal m_activeSchemeColorAlpha = 1;
    qreal m_inactiveSchemeColorAlpha = 1;
};
}

#endif
