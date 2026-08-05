#include "appmenu/menustyle.h"

#include <QPainter>
#include <QStyleOption>
#include <QWidget>

namespace Breeze
{

AppMenuMenuStyle::AppMenuMenuStyle(QStyle *style, Decoration *decoration)
    : QProxyStyle(style)
    , m_decoration(decoration)
{
}

void AppMenuMenuStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == PE_PanelMenu && widget && widget->isWindow()) {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(option->palette.brush(QPalette::Window));
        const int titleBarOpacity = m_decoration->internalSettings()->activeTitleBarOpacity();
        if (titleBarOpacity < 100) {
            painter->setCompositionMode(QPainter::CompositionMode_Source);
            painter->setOpacity(titleBarOpacity / 100.0f);
        }
        painter->drawRect(option->rect);
        painter->restore();
    } else {
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
}
}