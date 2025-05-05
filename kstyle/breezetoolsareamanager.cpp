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

namespace Breeze
{
ToolsAreaManager::ToolsAreaManager(std::shared_ptr<Helper> helper)
    : QObject()
    , _helper(helper)
{
}

ToolsAreaManager::~ToolsAreaManager()
{
}

void ToolsAreaManager::appendIfNotAlreadyExists(const QMainWindow *window, const QPointer<QToolBar> &toolBar)
{
    const auto windowIt = std::find_if(_windows.begin(), _windows.end(), [window](const WindowToolBars &windowToolBars) {
        return window == windowToolBars.window;
    });
    if (windowIt != _windows.end()) {
        if (!windowIt->toolBars.contains(toolBar)) {
            windowIt->toolBars.append(toolBar);
        }
    } else {
        _windows.emplace_back(WindowToolBars{window, {toolBar}});
        connect(window, &QObject::destroyed, this, [this, window] {
            removeWindow(window);
        });
    }
}

void ToolsAreaManager::removeWindowToolBar(const QMainWindow *window, const QPointer<QToolBar> &toolBar)
{
    const auto windowIt = std::find_if(_windows.begin(), _windows.end(), [window](const WindowToolBars &windowToolBars) {
        return window == windowToolBars.window;
    });
    if (windowIt != _windows.end()) {
        windowIt->toolBars.removeAll(toolBar);
    }
}

void ToolsAreaManager::removeWindow(const QMainWindow *window)
{
    std::erase_if(_windows, [window](const WindowToolBars &windowToolBars) {
        return window == windowToolBars.window;
    });
}

void ToolsAreaManager::doTranslucency(QMainWindow *win, bool on)
{
    QVariant wasTranslucent = win->property("_klassy_was_translucent");

    if (on) {
        if (wasTranslucent.isValid()) // if translucency has already been set here then don't set it again
            return;

        win->setProperty("_klassy_was_translucent", win->testAttribute(Qt::WA_TranslucentBackground));
        win->setAttribute(Qt::WA_TranslucentBackground, true);
    } else {
        if (!wasTranslucent.isValid()) // do not turn off translucency if it was initially set by a third party
            return;

        win->setAttribute(Qt::WA_TranslucentBackground, wasTranslucent.toBool()); // set the translucency back to its initial value if altered here
        win->setProperty("_klassy_was_translucent", QVariant());
    }
}

void ToolsAreaManager::registerApplication(QApplication *application)
{
    configUpdated();
}

QRect ToolsAreaManager::toolsAreaRect(const QMainWindow &window) const

{
    int itemHeight = window.menuWidget() ? window.menuWidget()->height() : 0;
    const auto windowIt = std::find_if(_windows.begin(), _windows.end(), [&window](const WindowToolBars &windowToolBars) {
        return &window == windowToolBars.window;
    });
    if (windowIt != _windows.end()) {
        for (auto item : windowIt->toolBars) {
            if (!item.isNull() && item->isVisible() && window.toolBarArea(item) == Qt::TopToolBarArea) {
                itemHeight = qMax(item->mapTo(&window, item->rect().bottomLeft()).y(), itemHeight);
            }
        }
    }
    if (itemHeight > 0) {
        itemHeight += 1;
    }

    return QRect(0, 0, window.width(), itemHeight);
}

bool ToolsAreaManager::tryRegisterToolBar(QPointer<const QMainWindow> window, QPointer<QWidget> widget)
{
    Q_ASSERT(!widget.isNull());
    doTranslucency(const_cast<QMainWindow *>(window.data()), _translucent);

    QPointer<QToolBar> toolbar;
    if (!(toolbar = qobject_cast<QToolBar *>(widget))) {
        return false;
    }

    if (window->toolBarArea(toolbar) == Qt::TopToolBarArea) {
        widget->setPalette(palette());
        appendIfNotAlreadyExists(window, toolbar);
        return true;
    }

    return false;
}

void ToolsAreaManager::tryUnregisterToolBar(QPointer<const QMainWindow> window, QPointer<QWidget> widget)
{
    Q_ASSERT(!widget.isNull());

    doTranslucency(const_cast<QMainWindow *>(window.data()), false);

    QPointer<QToolBar> toolbar;
    if (!(toolbar = qobject_cast<QToolBar *>(widget))) {
        return;
    }

    if (window->toolBarArea(toolbar) != Qt::TopToolBarArea) {
        widget->setPalette(window->palette());
        removeWindowToolBar(window, toolbar);
    }
}

void ToolsAreaManager::configUpdated()
{
    _colorSchemeHasHeaderColor = KColorScheme::isColorSetSupported(_helper->colorSchemeConfig(), KColorScheme::Header);

    bool translucent = false;

    _palette = KColorScheme::createApplicationPalette(_helper->colorSchemeConfig());

    if (_colorSchemeHasHeaderColor) {
        KColorScheme active = KColorScheme(QPalette::Active, KColorScheme::Header, _helper->colorSchemeConfig());
        KColorScheme inactive = KColorScheme(QPalette::Inactive, KColorScheme::Header, _helper->colorSchemeConfig());
        KColorScheme disabled = KColorScheme(QPalette::Disabled, KColorScheme::Header, _helper->colorSchemeConfig());

        _palette.setBrush(QPalette::Active, QPalette::Window, active.background());
        _palette.setBrush(QPalette::Active, QPalette::WindowText, active.foreground());
        _palette.setBrush(QPalette::Disabled, QPalette::Window, disabled.background());
        _palette.setBrush(QPalette::Disabled, QPalette::WindowText, disabled.foreground());
        _palette.setBrush(QPalette::Inactive, QPalette::Window, inactive.background());
        _palette.setBrush(QPalette::Inactive, QPalette::WindowText, inactive.foreground());

        if (_helper->decorationConfig()->applyOpacityToHeader() && !_helper->decorationConfig()->preventApplyOpacityToHeader()) {
            // override active with colour with opacity from decoration if needed
            _palette.setColor(QPalette::Active, QPalette::Window, _helper->decorationColors()->active()->titleBarBase);

            // override inactive with colour with opacity from decoration if needed
            _palette.setColor(QPalette::Inactive, QPalette::Window, _helper->decorationColors()->inactive()->titleBarBase);

            if (_palette.color(QPalette::Active, QPalette::Window).alpha() < 255 || _palette.color(QPalette::Inactive, QPalette::Window).alpha() < 255
                || _palette.color(QPalette::Disabled, QPalette::Window).alpha() < 255) {
                translucent = true;
            }
        }
    }

    if (translucent != _translucent) {
        if (translucent)
            becomeTransparent();
        else
            becomeOpaque();
    }
    _translucent = translucent;

    for (const WindowToolBars &windowToolBars : _windows) {
        for (const auto &toolbar : windowToolBars.toolBars) {
            if (!toolbar.isNull()) {
                toolbar->setPalette(_palette);
            }
        }
        if (QMenuBar *menuBar = windowToolBars.window->menuBar()) {
            menuBar->setPalette(_palette);
        }
    }
}

void ToolsAreaManager::becomeOpaque()
{
    for (auto windowToolBar : _windows) {
        doTranslucency(const_cast<QMainWindow *>(windowToolBar.window), false);
    }
}

void ToolsAreaManager::becomeTransparent()
{
    for (auto windowToolBar : _windows) {
        doTranslucency(const_cast<QMainWindow *>(windowToolBar.window), true);
    }
}

bool ToolsAreaManager::eventFilter(QObject *watched, QEvent *event)
{
    Q_ASSERT(watched);
    Q_ASSERT(event);

    QPointer<QObject> parent = watched;
    QPointer<const QMainWindow> mainWindow = nullptr;
    while (parent != nullptr) {
        if (qobject_cast<QMainWindow *>(parent)) {
            mainWindow = qobject_cast<QMainWindow *>(parent);
            break;
        }
        parent = parent->parent();
    }

    if (const QMainWindow *mw = qobject_cast<QMainWindow *>(watched)) {
        QChildEvent *ev = nullptr;
        if (event->type() == QEvent::ChildAdded || event->type() == QEvent::ChildRemoved) {
            ev = static_cast<QChildEvent *>(event);

            if (event->type() == QEvent::ChildAdded) {
                QChildEvent *childEvent = static_cast<QChildEvent *>(event);
                if (QMenuBar *menuBar = qobject_cast<QMenuBar *>(childEvent->child())) {
                    menuBar->setPalette(_palette);
                }
            }
        } else {
            return false;
        }

        QPointer<QToolBar> tb = qobject_cast<QToolBar *>(ev->child());
        if (tb.isNull()) {
            return false;
        }

        if (ev->added()) {
            if (mw->toolBarArea(tb) == Qt::TopToolBarArea) {
                appendIfNotAlreadyExists(mw, tb);
            }
        } else if (ev->removed()) {
            removeWindowToolBar(mw, tb);
        }
    } else if (qobject_cast<QToolBar *>(watched)) {
        if (!mainWindow.isNull()) {
            tryUnregisterToolBar(mainWindow, qobject_cast<QWidget *>(watched));
        }
    }

    return false;
}

void ToolsAreaManager::registerWidget(QWidget *widget)
{
    Q_ASSERT(widget);
    auto ptr = QPointer<QWidget>(widget);

    QPointer<const QMainWindow> mainWindow = qobject_cast<QMainWindow *>(ptr);

    if (mainWindow && mainWindow.data() == mainWindow->window()) {
        const auto toolBars = mainWindow->findChildren<QToolBar *>(QString(), Qt::FindDirectChildrenOnly);
        for (auto *toolBar : toolBars) {
            tryRegisterToolBar(mainWindow, toolBar);
        }

        if (QMenuBar *menuBar = mainWindow->menuBar()) {
            menuBar->setPalette(_palette);
        }

        return;
    }

    auto parent = ptr;

    while (parent != nullptr) {
        if (qobject_cast<QMdiArea *>(parent) || qobject_cast<QDockWidget *>(parent)) {
            break;
        }
        if (auto window = qobject_cast<QMainWindow *>(parent)) {
            mainWindow = window;
        }
        parent = parent->parentWidget();
    }
    if (mainWindow == nullptr) {
        return;
    }
    if (mainWindow.data() != mainWindow->window()) {
        return;
    }
    tryRegisterToolBar(mainWindow, widget);
}

void ToolsAreaManager::unregisterWidget(QWidget *widget)
{
    Q_ASSERT(widget);
    auto ptr = QPointer<QWidget>(widget);

    if (QPointer<const QMainWindow> window = qobject_cast<QMainWindow *>(ptr)) {
        removeWindow(window);
        return;
    } else if (QPointer<QToolBar> toolbar = qobject_cast<QToolBar *>(ptr)) {
        auto parent = ptr;
        QPointer<const QMainWindow> mainWindow = nullptr;
        while (parent != nullptr) {
            if (qobject_cast<QMainWindow *>(parent)) {
                mainWindow = qobject_cast<QMainWindow *>(parent);
                break;
            }
            parent = parent->parentWidget();
        }
        if (mainWindow == nullptr) {
            return;
        }
        removeWindowToolBar(mainWindow, toolbar);
    }
}

bool Breeze::ToolsAreaManager::hasHeaderColors()
{
    return _colorSchemeHasHeaderColor;
}
}
