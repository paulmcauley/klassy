#include "breezetoolsareamanager.h"
#include "breezepropertynames.h"

#include <QMainWindow>
#include <QMdiArea>
#include <QMenuBar>
#include <QObject>
#include <QToolBar>
#include <QWidget>
#include <QWindow>

#include <KColorUtils>

const char* colorProperty = "KDE_COLOR_SCHEME_PATH";

namespace Breeze {
    ToolsAreaManager::ToolsAreaManager(Helper *helper, QObject *parent) : QObject(parent), _helper(helper)
    {
        if (qApp && qApp->property(colorProperty).isValid()) {
            auto path = qApp->property(colorProperty).toString();
            _config = KSharedConfig::openConfig(path);
        } else {
            _config = KSharedConfig::openConfig();
        }
        _watcher = KConfigWatcher::create(_config);
        connect(_watcher.data(), &KConfigWatcher::configChanged, this, &ToolsAreaManager::configUpdated);
        configUpdated();
    }

    ToolsAreaManager::~ToolsAreaManager() {}

    template<class T1, class T2>
    void appendIfNotAlreadyExists(T1* list, T2 item) {
        for (auto listItem : *list) {
            if (listItem == item) {
                return;
            }
        }
        list->append(item);
    }

    void ToolsAreaManager::registerApplication(QApplication *application)
    {
        _listener = new AppListener(this);
        _listener->manager = this;
        if (application->property(colorProperty).isValid()) {
            auto path = application->property(colorProperty).toString();
            _config = KSharedConfig::openConfig(path);
            _watcher = KConfigWatcher::create(_config);
            connect(_watcher.data(), &KConfigWatcher::configChanged, this, &ToolsAreaManager::configUpdated);
        }
        application->installEventFilter(_listener);
        configUpdated();
    }

    QRect ToolsAreaManager::toolsAreaRect(const QMainWindow *window)
    {
        Q_ASSERT(window);

        int itemHeight = window->menuWidget() ? window->menuWidget()->height() : 0;
        for (auto item : _windows[window]) {
            if (!item.isNull() && item->isVisible() && window->toolBarArea(item) == Qt::TopToolBarArea) {
                itemHeight = qMax(item->mapTo(window, item->rect().bottomLeft()).y(), itemHeight);
            }
        }
        if (itemHeight > 0) {
            itemHeight += window->devicePixelRatio();
        }

        return QRect(0, 0, window->width(), itemHeight);
    }

    bool ToolsAreaManager::tryRegisterToolBar(QPointer<QMainWindow> window, QPointer<QWidget> widget)
    {
        Q_ASSERT(!widget.isNull());

        QPointer<QToolBar> toolbar;
        if (!(toolbar = qobject_cast<QToolBar*>(widget))) return false;

        if (window->toolBarArea(toolbar) == Qt::TopToolBarArea) {
            widget->setPalette(palette());
            appendIfNotAlreadyExists(&_windows[window], toolbar);
            return true;
        }

        return false;
    }

    void ToolsAreaManager::tryUnregisterToolBar(QPointer<QMainWindow> window, QPointer<QWidget> widget)
    {
        Q_ASSERT(!widget.isNull());

        QPointer<QToolBar> toolbar;
        if (!(toolbar = qobject_cast<QToolBar*>(widget))) return;

        if (window->toolBarArea(toolbar) != Qt::TopToolBarArea) {
            widget->setPalette(window->palette());
            _windows[window].removeAll(toolbar);
        }
    }

    void ToolsAreaManager::configUpdated()
    {
        auto active = KColorScheme(QPalette::Active, KColorScheme::Header, _config);
        auto inactive = KColorScheme(QPalette::Inactive, KColorScheme::Header, _config);
        auto disabled = KColorScheme(QPalette::Disabled, KColorScheme::Header, _config);

        _palette = KColorScheme::createApplicationPalette(_config);

        _palette.setBrush(QPalette::Active, QPalette::Window, active.background());
        _palette.setBrush(QPalette::Active, QPalette::WindowText, active.foreground());
        _palette.setBrush(QPalette::Disabled, QPalette::Window, disabled.background());
        _palette.setBrush(QPalette::Disabled, QPalette::WindowText, disabled.foreground());
        _palette.setBrush(QPalette::Inactive, QPalette::Window, inactive.background());
        _palette.setBrush(QPalette::Inactive, QPalette::WindowText, inactive.foreground());

        for (auto window : _windows) {
            for (auto toolbar : window) {
                if (!toolbar.isNull()) {
                    toolbar->setPalette(_palette);
                }
            }
        }

        _colorSchemeHasHeaderColor =  KColorScheme::isColorSetSupported(_config, KColorScheme::Header);
    }

    bool AppListener::eventFilter(QObject *watched, QEvent *event)
    {
        Q_ASSERT(watched);
        Q_ASSERT(event);

        if (watched != qApp) {
            return false;
        }

        if (event->type() == QEvent::DynamicPropertyChange) {
            if (watched != qApp) {
                return false;
            }
            auto ev = static_cast<QDynamicPropertyChangeEvent*>(event);
            if (ev->propertyName() == colorProperty) {
                if (qApp && qApp->property(colorProperty).isValid()) {
                    auto path = qApp->property(colorProperty).toString();
                    manager->_config = KSharedConfig::openConfig(path);
                } else {
                    manager->_config = KSharedConfig::openConfig();
                }
                manager->_watcher = KConfigWatcher::create(manager->_config);
                connect(manager->_watcher.data(), &KConfigWatcher::configChanged, manager, &ToolsAreaManager::configUpdated);
                manager->configUpdated();
            }
        }

        return false;
    }

    bool ToolsAreaManager::eventFilter(QObject *watched, QEvent *event)
    {
        Q_ASSERT(watched);
        Q_ASSERT(event);

        QPointer<QObject> parent = watched;
        QPointer<QMainWindow> mainWindow = nullptr;
        while (parent != nullptr) {
            if (qobject_cast<QMainWindow*>(parent)) {
                mainWindow = qobject_cast<QMainWindow*>(parent);
                break;
            }
            parent = parent->parent();
        }

        if (QPointer<QMainWindow> mw = qobject_cast<QMainWindow*>(watched)) {
            QChildEvent *ev = nullptr;
            if (event->type() == QEvent::ChildAdded || event->type() == QEvent::ChildRemoved)
                ev = static_cast<QChildEvent*>(event);

            QPointer<QToolBar> tb = qobject_cast<QToolBar*>(ev->child());
            if (tb.isNull())
                return false;

            if (ev->added()) {
                if (mw->toolBarArea(tb) == Qt::TopToolBarArea)
                    appendIfNotAlreadyExists(&_windows[mw], tb);
            } else if (ev->removed()) {
                _windows[mw].removeAll(tb);
            }
        } else if (qobject_cast<QToolBar*>(watched)) {
            if (!mainWindow.isNull()) {
                tryUnregisterToolBar(mainWindow, qobject_cast<QWidget*>(watched));
            }
        }

        return false;
    }

    void ToolsAreaManager::registerWidget(QWidget *widget)
    {
        Q_ASSERT(widget);
        auto ptr = QPointer<QWidget>(widget);

        auto parent = ptr;
        QPointer<QMainWindow> mainWindow = nullptr;
        while (parent != nullptr) {
            if (qobject_cast<QMdiArea*>(parent) || qobject_cast<QDockWidget*>(parent)) {
                break;
            }
            if (qobject_cast<QMainWindow*>(parent)) {
                mainWindow = qobject_cast<QMainWindow*>(parent);
            }
            parent = parent->parentWidget();
        } if (mainWindow == nullptr) {
            return;
        }
        if (mainWindow != mainWindow->window()) {
            return;
        }
        tryRegisterToolBar(mainWindow, widget);
    }

    void ToolsAreaManager::unregisterWidget(QWidget *widget)
    {
        Q_ASSERT(widget);
        auto ptr = QPointer<QWidget>(widget);

        if (QPointer<QMainWindow> window = qobject_cast<QMainWindow*>(ptr)) {
            _windows.remove(window);
            return;
        } else if (QPointer<QToolBar> toolbar = qobject_cast<QToolBar*>(ptr)) {
            auto parent = ptr;
            QPointer<QMainWindow> mainWindow = nullptr;
            while (parent != nullptr) {
                if (qobject_cast<QMainWindow*>(parent)) {
                    mainWindow = qobject_cast<QMainWindow*>(parent);
                    break;
                }
                parent = parent->parentWidget();
            } if (mainWindow == nullptr) {
                return;
            }
            _windows[mainWindow].removeAll(toolbar);
        }
    }

    bool Breeze::ToolsAreaManager::hasHeaderColors()
    {
        return _colorSchemeHasHeaderColor;
    }
}
