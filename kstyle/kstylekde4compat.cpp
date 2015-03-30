/*************************************************************************
 * Copyright (C) 2015 by David Edmundson <davidedmundson@kde.org>        *
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 *************************************************************************/

#include "kstylekde4compat.h"

#include <KConfigGroup>
#include <QToolBar>
#include <KSharedConfig>

KStyleKDE4Compat::KStyleKDE4Compat()
{
}

KStyleKDE4Compat::~KStyleKDE4Compat()
{
}

int KStyleKDE4Compat::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    switch (hint) {
    case SH_DialogButtonBox_ButtonsHaveIcons: {
        // was KGlobalSettings::showIconsOnPushButtons() :
        KConfigGroup g(KSharedConfig::openConfig(), "KDE");
        return g.readEntry("ShowIconsOnPushButtons", true);
    }

    case SH_ItemView_ArrowKeysNavigateIntoChildren:
        return true;

    case SH_ToolButtonStyle: {
        KConfigGroup g(KSharedConfig::openConfig(), "Toolbar style");

        bool useOthertoolbars = false;
        const QWidget *parent = widget ? widget->parentWidget() : nullptr;

        //If the widget parent is a QToolBar and the magic property is set
        if (parent && qobject_cast< const QToolBar * >(parent)) {
            if (parent->property("otherToolbar").isValid()) {
                useOthertoolbars = true;
            }
        }

        QString buttonStyle;
        if (useOthertoolbars) {
            buttonStyle = g.readEntry("ToolButtonStyleOtherToolbars", "NoText").toLower();
        } else {
            buttonStyle = g.readEntry("ToolButtonStyle", "TextBesideIcon").toLower();
        }

        return buttonStyle == QLatin1String("textbesideicon") ? Qt::ToolButtonTextBesideIcon
               : buttonStyle == QLatin1String("icontextright") ? Qt::ToolButtonTextBesideIcon
               : buttonStyle == QLatin1String("textundericon") ? Qt::ToolButtonTextUnderIcon
               : buttonStyle == QLatin1String("icontextbottom") ? Qt::ToolButtonTextUnderIcon
               : buttonStyle == QLatin1String("textonly") ? Qt::ToolButtonTextOnly
               : Qt::ToolButtonIconOnly;
    }

    default:
        break;
    };

    return QCommonStyle::styleHint(hint, option, widget, returnData);
}