#pragma once

#include "appmenu/iconbutton.h"
#include "appmenu/navigablemenu.h"

#include <QLineEdit>
#include <QStyle>
#include <QTimer>
#include <QWidgetAction>

namespace Breeze
{
class AppMenuButtonGroup;

class AppMenuSearchButton : public AppMenuIconButton
{
public:
    AppMenuSearchButton(QStyle *menuStyle, Decoration *decoration, const int buttonIndex, AppMenuButtonGroup *parent);

    void reconfigure() override;

    void filter(const QString &text);
    void filterToLastSearch();
    void startDebounceIfHasLastQuery();
    void clearCache();
    void setLineFocus();

    QMenu *menu()
    {
        return m_menu.get();
    }
    bool isUiVisible() const
    {
        return m_uiVisible;
    }

private:
    struct ActionInfo {
        QString path;
        QString searchablePath;
        QString label;
        bool isEffectivelyEnabled;
    };

    struct SearchResult {
        QAction *action;
        ActionInfo info;

        bool operator==(const SearchResult &other) const
        {
            return action == other.action && info.isEffectivelyEnabled == other.info.isEffectivelyEnabled && info.path == other.info.path
                && (!action || (action->isChecked() == other.action->isChecked() && action->isCheckable() == other.action->isCheckable()));
        }
    };

    void reposition();

    void searchMenu(QMenu *menu,
                    const QStringMatcher &matcher,
                    QList<SearchResult> &results,
                    QSet<QMenu *> &visited,
                    bool ignoreTopLevel,
                    bool ignoreSubMenus,
                    QStringList &currentPath,
                    bool isParentEnabled = true,
                    bool parentMatched = false);
    QString getActionText(QAction *action) const;

    bool eventFilter(QObject *watched, QEvent *event) override;
    void onTimerTimeout();
    void onMenuHide();

    AppMenuButtonGroup *m_integratedMenu;
    std::unique_ptr<NavigableMenu> m_menu;
    QPointer<QLineEdit> m_lineEdit;
    QTimer *m_debounceTimer;
    bool m_uiVisible = false;
    QString m_lastSearchQuery;
    QList<SearchResult> m_lastResults;
    mutable QHash<QString, QString> m_actionTextCache;
};
};