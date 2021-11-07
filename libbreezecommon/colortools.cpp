/*
* SPDX-FileCopyrightText: 2021 Paul McAuley <kde@paulmcauley.com>
* 
* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "colortools.h"

#include <KColorUtils>

namespace Breeze
{
    QColor ColorTools::getDifferentiatedSaturatedColor( const QColor& inputColor )
    {
        int colorHsv[3];
        inputColor.getHsv(&colorHsv[0], &colorHsv[1], &colorHsv[2]);
        if( colorHsv[1] < 240 ) colorHsv[1] = 255; //increase saturation to max if not max
        else colorHsv[1] -= 80; // else reduce saturation if already high to provide differentiation/contrast
        QColor redColorSaturated;
        redColorSaturated.setHsv(colorHsv[0], colorHsv[1], colorHsv[2]);
        return redColorSaturated;
    }
    
    
    QColor ColorTools::getDifferentiatedLessSaturatedColor( const QColor& inputColor )
    {
        int colorHsv[3];
        inputColor.getHsv(&colorHsv[0], &colorHsv[1], &colorHsv[2]);
        if( colorHsv[1] > 125 ) colorHsv[1] -= 80; //decrease saturation if not already low
        else colorHsv[1] += 80; // else increase saturation if very low to provide differentiation/contrast
        QColor outputColor;
        outputColor.setHsv(colorHsv[0], colorHsv[1], colorHsv[2]);
        return outputColor;
    }

    
    QColor ColorTools::getHigherContrastForegroundColor( const QColor& foregroundColor, const QColor& backgroundColor, double blackWhiteContrastThreshold )
    {        
        double contrastRatio = KColorUtils::contrastRatio(foregroundColor, backgroundColor);
        
        //qDebug() << "Contrast ratio: " << contrastRatio;

        if( contrastRatio < blackWhiteContrastThreshold ) return getBlackOrWhiteForegroundForHighContrast(backgroundColor);
        else return foregroundColor;
    }
    
    QColor ColorTools::getBlackOrWhiteForegroundForHighContrast( const QColor& backgroundColor )
    {
        // based on http://www.w3.org/TR/AERT#color-contrast
        
        if ( !backgroundColor.isValid() ) return QColor();
        
        int rgbBackground[3];
        
        backgroundColor.getRgb(&rgbBackground[0], &rgbBackground[1], &rgbBackground[2]);
        
        double brightness = qRound(static_cast<double>(( (rgbBackground[0] * 299) + (rgbBackground[1] *587) + (rgbBackground[2] * 114) ) /1000));
        
        return (brightness > 125) ? QColor(Qt::GlobalColor::black) : QColor(Qt::GlobalColor::white);
    }
}
