
#include "appmenu/searchbutton.h"
#include "appmenu/buttongroup.h"
#include "breezedecoration.h"

#include <KLocalizedString>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QKeyEvent>

namespace Breeze
{
static constexpr int MAX_SEARCH_RESULTS = 100;

AppMenuSearchButton::AppMenuSearchButton(Decoration *decoration, const int buttonIndex, AppMenuButtonGroup *parent)
    : AppMenuIconButton(DecorationButtonType::CustomAppMenuBarSearch, decoration, buttonIndex, parent)
    , m_appMenuBar(parent)
    , m_menu(new NavigableMenu(nullptr, decoration))
    , m_lineEdit(new QLineEdit(m_menu.get()))
    , m_debounceTimer(new QTimer(this))
{
    m_lineEdit->setMinimumWidth(200);

    auto *searchAction = new QWidgetAction(m_menu.get());
    searchAction->setDefaultWidget(m_lineEdit);
    m_menu->addAction(searchAction);
    m_menu->addSeparator();
    m_menu->installEventFilter(this);
    connect(m_menu.get(), &QMenu::aboutToHide, this, &AppMenuSearchButton::onMenuHide);

    m_lineEdit->installEventFilter(this);
    m_lineEdit->setFocusPolicy(Qt::StrongFocus);
    m_lineEdit->setPlaceholderText(i18nd("plasma_applet_org.kde.plasma.appmenu", "Search") + QStringLiteral("…"));
    m_lineEdit->setClearButtonEnabled(false);
    connect(m_lineEdit, &QLineEdit::textChanged, m_debounceTimer, qOverload<>(&QTimer::start));

    m_debounceTimer->setInterval(150);
    m_debounceTimer->setSingleShot(true);
    connect(m_debounceTimer, &QTimer::timeout, this, &AppMenuSearchButton::onTimerTimeout);
};

void AppMenuSearchButton::reconfigure()
{
    AppMenuIconButton::reconfigure();
    auto settings = m_d->internalSettings();
    this->setVisible(settings->appMenuBarSearchEnabled());
}

void AppMenuSearchButton::filter(const QString &text)
{
    m_lastSearchQuery = text;

    // Clear results if search text is too short
    if (text.length() < 3) {
        const auto actions = m_menu->actions();
        if (actions.count() > 2) {
            for (int i = actions.count() - 1; i >= 2; --i) {
                QAction *action = actions.at(i);
                m_menu->removeAction(action);
                action->deleteLater();
            }
        }
        m_lastResults.clear();
        reposition();

        if (text.isEmpty()) {
            m_lineEdit->setClearButtonEnabled(false);
            m_lineEdit->setPlaceholderText(i18nd("plasma_applet_org.kde.plasma.appmenu", "Search") + QStringLiteral("…"));
            return;
        }
        m_lineEdit->setClearButtonEnabled(true);
        return;
    } else {
        m_lineEdit->setClearButtonEnabled(true);
    }

    if (!m_appMenuBar->model()) {
        return;
    }

    // Find results
    QList<SearchResult> results;
    QMenu *rootMenu = m_appMenuBar->model()->menu();
    if (rootMenu) {
        QSet<QMenu *> visited;
        const auto *deco = qobject_cast<const Decoration *>(decoration());
        const bool ignoreTopLevel = deco && deco->internalSettings()->appMenuBarSearchIgnoreTopLevel();
        const bool ignoreSubMenus = deco && deco->internalSettings()->appMenuBarSearchIgnoreSubMenus();
        QStringList currentPath;
        QStringMatcher matcher(text, Qt::CaseInsensitive);
        searchMenu(rootMenu, matcher, results, visited, ignoreTopLevel, ignoreSubMenus, currentPath);
    }

    // If results are the same as last time, do nothing to prevent the freeze.
    if (m_lastResults == results) {
        return;
    }

    m_menu->setUpdatesEnabled(false);

    m_lastResults = results;

    // Clear previous results
    const auto actions = m_menu->actions();
    for (int i = actions.count() - 1; i >= 2; --i) {
        QAction *action = actions.at(i);
        m_menu->removeAction(action);
        action->deleteLater();
    }

    // Add new results
    const auto *deco = qobject_cast<const Decoration *>(decoration());
    if (!deco) {
        m_menu->setUpdatesEnabled(true);
        return;
    }

    int resultCount = 0;
    for (const SearchResult &result : std::as_const(results)) {
        if (resultCount >= MAX_SEARCH_RESULTS) { // stop after 100 results
            break;
        }

        const ActionInfo &info = result.info;
        QAction *action = result.action;
        if (!info.isEffectivelyEnabled && deco->internalSettings()->appMenuBarSearchIgnoreDisabled()) {
            continue;
        }
        QAction *newAction = new QAction(action->icon(), info.path, m_menu.get());
        newAction->setEnabled(info.isEffectivelyEnabled);
        newAction->setCheckable(action->isCheckable());
        newAction->setChecked(action->isChecked());

        if (action->actionGroup() && action->actionGroup()->isExclusive()) {
            auto *group = new QActionGroup(newAction);
            group->setExclusive(true);
            group->addAction(newAction);
        }

        QPointer<QAction> safeAction = action;
        connect(newAction, &QAction::triggered, this, [safeAction, searchMenu = m_menu.get()]() {
            if (safeAction) {
                safeAction->trigger();
            }
            if (searchMenu) {
                searchMenu->hide();
            }
        });
        m_menu->addAction(newAction);
        resultCount++;
    }

    reposition();
    m_menu->setUpdatesEnabled(true);
    // qCDebug(category) << "[AppMenuButtonGroup] filterMenu(" << text << ") ended";
}

void AppMenuSearchButton::filterToLastSearch()
{
    if (!m_lastSearchQuery.isEmpty())
        filter(m_lastSearchQuery);
}

void AppMenuSearchButton::startDebounceIfHasLastQuery()
{
    if (!m_lastSearchQuery.isEmpty() && !m_debounceTimer->isActive())
        m_debounceTimer->start();
}

void AppMenuSearchButton::clearCache()
{
    m_actionTextCache.clear();
}

void AppMenuSearchButton::setLineFocus()
{
    m_lineEdit->setFocus();
    m_uiVisible = true;
}

void AppMenuSearchButton::reposition()
{
    if (!m_menu || !m_menu->isVisible()) {
        return;
    }

    KDecoration3::Positioner positioner;
    positioner.setAnchorRect(this->geometry());
    m_d->popup(positioner, m_menu.get());
    m_menu->popup(m_menu->pos()); // HACK without this the scrollbar remain even if not necessary
}

void AppMenuSearchButton::searchMenu(QMenu *menu,
                                     const QStringMatcher &matcher,
                                     QList<SearchResult> &results,
                                     QSet<QMenu *> &visited,
                                     bool ignoreTopLevel,
                                     bool ignoreSubMenus,
                                     QStringList &currentPath,
                                     bool isParentEnabled,
                                     bool parentMatched)
{
    if (results.size() >= MAX_SEARCH_RESULTS || !menu || visited.contains(menu)) {
        return;
    }
    visited.insert(menu);

    QAction *menuAction = menu->menuAction();
    bool isCurrentEnabled = isParentEnabled;
    bool addedToPath = false;
    bool currentMatched = parentMatched;

    if (menuAction) {
        if (!menuAction->isEnabled()) {
            isCurrentEnabled = false;
        }
        const QString menuText = getActionText(menuAction);
        if (!menuText.isEmpty()) {
            currentPath.append(menuText);
            addedToPath = true;

            if (!currentMatched && (!ignoreTopLevel || currentPath.size() > 1)) {
                if (matcher.indexIn(menuText) != -1) {
                    currentMatched = true;
                }
            }
        }
    }

    for (QAction *action : menu->actions()) {
        if (results.size() >= MAX_SEARCH_RESULTS) {
            break;
        }
        if (action->isSeparator()) {
            continue;
        }
        if (action->menu()) {
            searchMenu(action->menu(), matcher, results, visited, ignoreTopLevel, ignoreSubMenus, currentPath, isCurrentEnabled, currentMatched);
        } else {
            const QString itemText = getActionText(action);
            bool match = currentMatched;
            if (ignoreSubMenus) {
                match = matcher.indexIn(itemText) != -1;
            } else {
                // Check the text of the action
                if (!match && (!ignoreTopLevel || !currentPath.isEmpty())) {
                    if (matcher.indexIn(itemText) != -1) {
                        match = true;
                    }
                }
            }

            if (match) {
                ActionInfo info;
                info.label = itemText;
                info.isEffectivelyEnabled = isCurrentEnabled && action->isEnabled();

                currentPath.append(itemText);
                info.path = currentPath.join(QStringLiteral(" » "));
                info.searchablePath = (currentPath.size() > 1) ? currentPath.mid(1).join(QStringLiteral(" » ")) : itemText;
                currentPath.removeLast();

                results.append({action, info});
            }
        }
    }

    if (addedToPath) {
        currentPath.removeLast();
    }
}

QString AppMenuSearchButton::getActionText(QAction *action) const
{
    if (!action) {
        return QString();
    }
    const QString rawText = action->text();
    auto it = m_actionTextCache.find(rawText);
    if (it != m_actionTextCache.end()) {
        return it.value();
    }
    const QString cleanedText = KLocalizedString::removeAcceleratorMarker(rawText.trimmed());
    m_actionTextCache.insert(rawText, cleanedText);
    return cleanedText;
}

bool AppMenuSearchButton::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_lineEdit || event->type() != QEvent::KeyPress)
        return AppMenuIconButton::eventFilter(watched, event);
    auto *keyEvent = static_cast<QKeyEvent *>(event);

    // Forward Left/Right key events to m_searchMenu when at the line boundaries
    if ((keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right) && keyEvent->modifiers() == Qt::NoModifier) {
        bool atBoundary = (keyEvent->key() == Qt::Key_Left && m_lineEdit->cursorPosition() == 0)
            || (keyEvent->key() == Qt::Key_Right && m_lineEdit->cursorPosition() == m_lineEdit->text().length());

        if (atBoundary) {
            QApplication::sendEvent(m_menu.get(), keyEvent);
            return true;
        }
    }
    return false;
}

void AppMenuSearchButton::onMenuHide()
{
    m_uiVisible = false;
    m_lineEdit->clear();
    m_lastResults.clear();
    m_lastSearchQuery.clear();
    clearCache();
}

void AppMenuSearchButton::onTimerTimeout()
{
    if (m_lineEdit) {
        filter(m_lineEdit->text());
    }
}
}
