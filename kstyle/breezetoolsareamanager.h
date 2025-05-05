#pragma once

#include "breezehelper.h"
#include "breezestyle.h"
#include <KSharedConfig>
#include <QObject>

namespace Breeze
{
class ToolsAreaManager;

//* signal manager for the tools area
class ToolsAreaManager : public QObject
{
    Q_OBJECT

private:
    void doTranslucency(QMainWindow *win, bool on);
    std::shared_ptr<Helper> _helper;
    struct WindowToolBars {
        const QMainWindow *window;
        QVector<QPointer<QToolBar>> toolBars;
    };
    std::vector<WindowToolBars> _windows;
    QPalette _palette = QPalette();
    bool _colorSchemeHasHeaderColor;
    bool _translucent = false;

    void recreateConfigWatcher(const QString &path);
    void appendIfNotAlreadyExists(const QMainWindow *window, const QPointer<QToolBar> &toolBar);
    void removeWindowToolBar(const QMainWindow *window, const QPointer<QToolBar> &toolBar);
    void removeWindow(const QMainWindow *window);

    friend class AppListener;

protected:
    bool tryRegisterToolBar(QPointer<const QMainWindow> window, QPointer<QWidget> widget);
    void tryUnregisterToolBar(QPointer<const QMainWindow> window, QPointer<QWidget> widget);

public:
    explicit ToolsAreaManager(std::shared_ptr<Helper> helper);
    ~ToolsAreaManager();

    void configUpdated();

    bool eventFilter(QObject *watched, QEvent *event) override;

    const QPalette &palette() const
    {
        return _palette;
    }

    void registerWidget(QWidget *widget);
    void unregisterWidget(QWidget *widget);

    QRect toolsAreaRect(const QMainWindow &window) const;

    bool hasHeaderColors();

    // sets the translucency of a window for translucent tools area purposes
    void becomeTransparent();
    void becomeOpaque();
};
}
