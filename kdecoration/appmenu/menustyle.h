#include "breezedecoration.h"

#include <QProxyStyle>

namespace Breeze
{
class AppMenuMenuStyle : public QProxyStyle
{
public:
    AppMenuMenuStyle(QStyle *style, Decoration *decoration);

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;

private:
    Decoration *m_decoration;
};
}