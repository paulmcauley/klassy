/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezestyleconfig.h"

#include "../config-breeze.h"
#include "breezestyleconfigdata.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDialog>
#include <QRegularExpression>

extern "C" {
Q_DECL_EXPORT QWidget *allocate_kstyle_config(QWidget *parent)
{
    return new Breeze::StyleConfig(parent);
}
}

namespace Breeze
{

//__________________________________________________________________
StyleConfig::StyleConfig(QWidget *parent)
    : QWidget(parent)
    , _configuration(KSharedConfig::openConfig(QStringLiteral("klassyrc")))
{
    // this is a hack to get an Apply button
    QDialog *parentDialog = qobject_cast<QDialog *>(parent);
    if (parentDialog && QCoreApplication::applicationName() == QStringLiteral("systemsettings")) {
        system("kcmshell5 klassystyleconfig &");
        parentDialog->close();
    }

    setupUi(this);

    // hide the title if a kcmshell dialog
    if (this->window()) {
        KPageWidget *kPageWidget = this->window()->findChild<KPageWidget *>();
        if (kPageWidget) {
            KPageWidgetItem *currentPage = kPageWidget->currentPage();
            if (currentPage)
                kPageWidgetChanged(currentPage, currentPage); // this line usually is false but currentPage is valid on change
            connect(kPageWidget, &KPageWidget::currentPageChanged, this, &StyleConfig::kPageWidgetChanged);
        }
    }

#if KLASSY_GIT_MASTER
    // set the long version string if from the git master
    _version->setText("v" + QString(KLASSY_VERSION) + ".git");

#else
    // set shortened version string in UI if an official release
    QRegularExpression re("\\d+\\.\\d+");
    QRegularExpressionMatch match = re.match(KLASSY_VERSION);
    if (match.hasMatch()) {
        QString matched = match.captured(0);
        _version->setText("v" + matched);
    }
#endif

    // load setup from configData
    load();

    connect(_tabBarDrawCenteredTabs, &QAbstractButton::toggled, this, &StyleConfig::updateChanged);
    connect(_buttonGradient, &QAbstractButton::toggled, this, &StyleConfig::updateChanged);
    connect(_toolBarDrawItemSeparator, &QAbstractButton::toggled, this, &StyleConfig::updateChanged);
    connect(_viewDrawFocusIndicator, &QAbstractButton::toggled, this, &StyleConfig::updateChanged);
    connect(_dockWidgetDrawFrame, &QAbstractButton::toggled, this, &StyleConfig::updateChanged);
    connect(_sidePanelDrawFrame, &QAbstractButton::toggled, this, &StyleConfig::updateChanged);
    connect(_menuItemDrawThinFocus, &QAbstractButton::toggled, this, &StyleConfig::updateChanged);
    connect(_sliderDrawTickMarks, &QAbstractButton::toggled, this, &StyleConfig::updateChanged);
    connect(_splitterProxyEnabled, &QAbstractButton::toggled, this, &StyleConfig::updateChanged);
    connect(_mnemonicsMode, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(_scrollBarAddLineButtons, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(_scrollBarSubLineButtons, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(_windowDragMode, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(_menuOpacity, &QAbstractSlider::valueChanged, this, &StyleConfig::updateChanged);
}

//__________________________________________________________________
void StyleConfig::save()
{
    StyleConfigData::setTabBarDrawCenteredTabs(_tabBarDrawCenteredTabs->isChecked());
    StyleConfigData::setButtonGradient(_buttonGradient->isChecked());
    StyleConfigData::setToolBarDrawItemSeparator(_toolBarDrawItemSeparator->isChecked());
    StyleConfigData::setViewDrawFocusIndicator(_viewDrawFocusIndicator->isChecked());
    StyleConfigData::setDockWidgetDrawFrame(_dockWidgetDrawFrame->isChecked());
    StyleConfigData::setSidePanelDrawFrame(_sidePanelDrawFrame->isChecked());
    StyleConfigData::setMenuItemDrawStrongFocus(!_menuItemDrawThinFocus->isChecked());
    StyleConfigData::setSliderDrawTickMarks(_sliderDrawTickMarks->isChecked());
    StyleConfigData::setSplitterProxyEnabled(_splitterProxyEnabled->isChecked());
    StyleConfigData::setMnemonicsMode(_mnemonicsMode->currentIndex());
    StyleConfigData::setScrollBarAddLineButtons(_scrollBarAddLineButtons->currentIndex());
    StyleConfigData::setScrollBarSubLineButtons(_scrollBarSubLineButtons->currentIndex());
    StyleConfigData::setWindowDragMode(_windowDragMode->currentIndex());
    StyleConfigData::setMenuOpacity(_menuOpacity->value());

    StyleConfigData::self()->save();
    _configuration->sync();

    // emit dbus signal
    QDBusMessage message(
        QDBusMessage::createSignal(QStringLiteral("/KlassyStyle"), QStringLiteral("org.kde.Klassy.Style"), QStringLiteral("reparseConfiguration")));
    QDBusConnection::sessionBus().send(message);
}

//__________________________________________________________________
void StyleConfig::defaults()
{
    StyleConfigData::self()->setDefaults();
    load();

    emit changed(!isDefaults());
}

bool StyleConfig::isDefaults()
{
    bool isDefaults = true;

    QString groupName(QStringLiteral("Style"));
    if (_configuration->hasGroup(groupName)) {
        KConfigGroup group = _configuration->group(groupName);
        if (group.keyList().count())
            isDefaults = false;
    }

    return isDefaults;
}

//__________________________________________________________________
void StyleConfig::reset()
{
    // reparse configuration
    StyleConfigData::self()->load();

    load();
}

//__________________________________________________________________
void StyleConfig::updateChanged()
{
    bool modified(false);

    // check if any value was modified
    if (_tabBarDrawCenteredTabs->isChecked() != StyleConfigData::tabBarDrawCenteredTabs())
        modified = true;
    else if (_buttonGradient->isChecked() != StyleConfigData::buttonGradient())
        modified = true;
    else if (_toolBarDrawItemSeparator->isChecked() != StyleConfigData::toolBarDrawItemSeparator())
        modified = true;
    else if (_viewDrawFocusIndicator->isChecked() != StyleConfigData::viewDrawFocusIndicator())
        modified = true;
    else if (_dockWidgetDrawFrame->isChecked() != StyleConfigData::dockWidgetDrawFrame())
        modified = true;
    else if (_sidePanelDrawFrame->isChecked() != StyleConfigData::sidePanelDrawFrame())
        modified = true;
    else if (_menuItemDrawThinFocus->isChecked() == StyleConfigData::menuItemDrawStrongFocus())
        modified = true;
    else if (_sliderDrawTickMarks->isChecked() != StyleConfigData::sliderDrawTickMarks())
        modified = true;
    else if (_mnemonicsMode->currentIndex() != StyleConfigData::mnemonicsMode())
        modified = true;
    else if (_scrollBarAddLineButtons->currentIndex() != StyleConfigData::scrollBarAddLineButtons())
        modified = true;
    else if (_scrollBarSubLineButtons->currentIndex() != StyleConfigData::scrollBarSubLineButtons())
        modified = true;
    else if (_splitterProxyEnabled->isChecked() != StyleConfigData::splitterProxyEnabled())
        modified = true;
    else if (_windowDragMode->currentIndex() != StyleConfigData::windowDragMode())
        modified = true;
    else if (_menuOpacity->value() != StyleConfigData::menuOpacity())
        modified = true;

    emit changed(modified);
}

//__________________________________________________________________
void StyleConfig::load()
{
    _tabBarDrawCenteredTabs->setChecked(StyleConfigData::tabBarDrawCenteredTabs());
    _buttonGradient->setChecked(StyleConfigData::buttonGradient());
    _toolBarDrawItemSeparator->setChecked(StyleConfigData::toolBarDrawItemSeparator());
    _viewDrawFocusIndicator->setChecked(StyleConfigData::viewDrawFocusIndicator());
    _dockWidgetDrawFrame->setChecked(StyleConfigData::dockWidgetDrawFrame());
    _sidePanelDrawFrame->setChecked(StyleConfigData::sidePanelDrawFrame());
    _menuItemDrawThinFocus->setChecked(!StyleConfigData::menuItemDrawStrongFocus());
    _sliderDrawTickMarks->setChecked(StyleConfigData::sliderDrawTickMarks());
    _mnemonicsMode->setCurrentIndex(StyleConfigData::mnemonicsMode());
    _splitterProxyEnabled->setChecked(StyleConfigData::splitterProxyEnabled());
    _scrollBarAddLineButtons->setCurrentIndex(StyleConfigData::scrollBarAddLineButtons());
    _scrollBarSubLineButtons->setCurrentIndex(StyleConfigData::scrollBarSubLineButtons());
    _windowDragMode->setCurrentIndex(StyleConfigData::windowDragMode());
    _menuOpacity->setValue(StyleConfigData::menuOpacity());
}

void StyleConfig::kPageWidgetChanged(KPageWidgetItem *current, KPageWidgetItem *before)
{
    Q_UNUSED(before)
    if (current) {
        current->setHeaderVisible(false);
    }
}
}
