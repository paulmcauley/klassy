//////////////////////////////////////////////////////////////////////////////
// breezeconfigurationui.cpp
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
// SPDX-FileCopyrightText: 2021 Paul McAuley <kde@paulmcauley.com>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezeconfigwidget.h"
#include "breezeexceptionlist.h"

#include <KLocalizedString>

#include <QDBusConnection>
#include <QDBusMessage>

namespace Breeze
{

    //_________________________________________________________
    ConfigWidget::ConfigWidget( QWidget* parent, const QVariantList &args ):
        KCModule(parent, args),
        m_configuration( KSharedConfig::openConfig( QStringLiteral( "classikstylesrc" ) ) ),
        m_changed( false )
    {

        // configuration
        m_ui.setupUi( this );

        // track ui changes
        connect( m_ui.titleAlignment, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.buttonIconStyle, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.buttonShape, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.backgroundColors, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.alwaysShow, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.alwaysShowIconHighlightUsing, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.iconSize, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.buttonSpacingRight, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.buttonSpacingLeft, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.titlebarTopBottomMargins, SIGNAL(valueChanged(double)), SLOT(updateChanged()) );
        connect( m_ui.percentMaximizedTopBottomMargins, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.titlebarSideMargins, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.cornerRadius, SIGNAL(valueChanged(double)), SLOT(updateChanged()) );
        connect( m_ui.activeTitlebarOpacity, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.inactiveTitlebarOpacity, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.boldButtonIcons, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.redOutline, &QAbstractButton::clicked, this, &ConfigWidget::updateChanged );
        connect( m_ui.drawBorderOnMaximizedWindows, &QAbstractButton::clicked, this, &ConfigWidget::updateChanged );
        connect( m_ui.drawSizeGrip, &QAbstractButton::clicked, this, &ConfigWidget::updateChanged );
        connect( m_ui.drawBackgroundGradient, &QAbstractButton::clicked, this, &ConfigWidget::updateChanged );
        connect( m_ui.drawTitleBarSeparator, &QAbstractButton::clicked, this, &ConfigWidget::updateChanged );
        connect( m_ui.useTitlebarColorForAllBorders, &QAbstractButton::clicked, this, &ConfigWidget::updateChanged );
        connect( m_ui.opaqueMaximizedTitlebars, &QAbstractButton::clicked, this, &ConfigWidget::updateChanged );
        connect( m_ui.blurTransparentTitlebars, &QAbstractButton::clicked, this, &ConfigWidget::updateChanged );
        connect( m_ui.translucentButtonBackgrounds, &QAbstractButton::clicked, this, &ConfigWidget::updateChanged );
        
        //connect dual controls with same values
        connect( m_ui.titlebarTopBottomMargins, SIGNAL(valueChanged(double)), m_ui.titlebarTopBottomMargins_2, SLOT(setValue(double)) );
        connect( m_ui.titlebarTopBottomMargins_2, SIGNAL(valueChanged(double)), m_ui.titlebarTopBottomMargins, SLOT(setValue(double)) );
        connect( m_ui.titlebarSideMargins, SIGNAL(valueChanged(int)), m_ui.titlebarSideMargins_2, SLOT(setValue(int)) );
        connect( m_ui.titlebarSideMargins_2, SIGNAL(valueChanged(int)), m_ui.titlebarSideMargins, SLOT(setValue(int)) );
        connect( m_ui.cornerRadius, SIGNAL(valueChanged(double)), m_ui.cornerRadius_2, SLOT(setValue(double)) );
        connect( m_ui.cornerRadius, SIGNAL(valueChanged(double)), m_ui.cornerRadius_3, SLOT(setValue(double)) );
        connect( m_ui.cornerRadius, SIGNAL(valueChanged(double)), m_ui.cornerRadius_4, SLOT(setValue(double)) );
        connect( m_ui.cornerRadius_2, SIGNAL(valueChanged(double)), m_ui.cornerRadius, SLOT(setValue(double)) );
        connect( m_ui.cornerRadius_2, SIGNAL(valueChanged(double)), m_ui.cornerRadius_3, SLOT(setValue(double)) );
        connect( m_ui.cornerRadius_2, SIGNAL(valueChanged(double)), m_ui.cornerRadius_4, SLOT(setValue(double)) );
        connect( m_ui.cornerRadius_3, SIGNAL(valueChanged(double)), m_ui.cornerRadius, SLOT(setValue(double)) );
        connect( m_ui.cornerRadius_3, SIGNAL(valueChanged(double)), m_ui.cornerRadius_2, SLOT(setValue(double)) );
        connect( m_ui.cornerRadius_3, SIGNAL(valueChanged(double)), m_ui.cornerRadius_4, SLOT(setValue(double)) );
        connect( m_ui.cornerRadius_4, SIGNAL(valueChanged(double)), m_ui.cornerRadius, SLOT(setValue(double)) );
        connect( m_ui.cornerRadius_4, SIGNAL(valueChanged(double)), m_ui.cornerRadius_2, SLOT(setValue(double)) );
        connect( m_ui.cornerRadius_4, SIGNAL(valueChanged(double)), m_ui.cornerRadius_3, SLOT(setValue(double)) );
        connect( m_ui.activeTitlebarOpacity, SIGNAL(valueChanged(int)), m_ui.activeTitlebarOpacity_2, SLOT(setValue(int)) );
        connect( m_ui.activeTitlebarOpacity_2, SIGNAL(valueChanged(int)), m_ui.activeTitlebarOpacity, SLOT(setValue(int)) );
        connect( m_ui.inactiveTitlebarOpacity, SIGNAL(valueChanged(int)), m_ui.inactiveTitlebarOpacity_2, SLOT(setValue(int)) );
        connect( m_ui.inactiveTitlebarOpacity_2, SIGNAL(valueChanged(int)), m_ui.inactiveTitlebarOpacity, SLOT(setValue(int)) );

        //only enable animationsSpeed when animationsEnabled is checked
        connect( m_ui.animationsEnabled, &QAbstractButton::clicked, this, &ConfigWidget::setEnabledAnimationsSpeed );

        // track animations changes
        connect( m_ui.animationsEnabled, &QAbstractButton::clicked, this, &ConfigWidget::updateChanged );
        connect( m_ui.animationsSpeedRelativeSystem, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );
        
        /*
        //only enable transparency options when transparency is setActiveTitlebarOpacity
        connect( m_ui.activeTitlebarOpacity, SIGNAL(valueChanged(int)), SLOT(setEnabledTransparentTitlebarOptions()) );
        connect( m_ui.inactiveTitlebarOpacity, SIGNAL(valueChanged(int)), SLOT(setEnabledTransparentTitlebarOptions()) );
        */
        
        // track shadows changes
        connect( m_ui.shadowSize, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.shadowStrength, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.shadowColor, &KColorButton::changed, this, &ConfigWidget::updateChanged );
        connect( m_ui.thinWindowOutlineStyle, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        
        // track exception changes
        connect( m_ui.exceptions, &ExceptionListWidget::changed, this, &ConfigWidget::updateChanged );
        
    }

    //_________________________________________________________
    void ConfigWidget::load()
    {

        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr( new InternalSettings() );
        m_internalSettings->load();

        // assign to ui
        m_ui.titleAlignment->setCurrentIndex( m_internalSettings->titleAlignment() );
        m_ui.buttonIconStyle->setCurrentIndex( m_internalSettings->buttonIconStyle() );
        m_ui.buttonShape->setCurrentIndex( m_internalSettings->buttonShape() );
        m_ui.backgroundColors->setCurrentIndex( m_internalSettings->backgroundColors() );
        m_ui.alwaysShow->setCurrentIndex( m_internalSettings->alwaysShow() );
        m_ui.alwaysShowIconHighlightUsing->setCurrentIndex( m_internalSettings->alwaysShowIconHighlightUsing() );
        m_ui.iconSize->setCurrentIndex( m_internalSettings->iconSize() );
        m_ui.buttonSpacingRight->setValue( m_internalSettings->buttonSpacingRight() );
        m_ui.buttonSpacingLeft->setValue( m_internalSettings->buttonSpacingLeft() );
        m_ui.titlebarTopBottomMargins->setValue( m_internalSettings->titlebarTopBottomMargins() );
        m_ui.titlebarTopBottomMargins_2->setValue( m_internalSettings->titlebarTopBottomMargins() );
        m_ui.percentMaximizedTopBottomMargins->setValue( m_internalSettings->percentMaximizedTopBottomMargins() );
        m_ui.titlebarSideMargins->setValue( m_internalSettings->titlebarSideMargins() );
        m_ui.titlebarSideMargins_2->setValue( m_internalSettings->titlebarSideMargins() );
        m_ui.cornerRadius->setValue( m_internalSettings->cornerRadius() );
        m_ui.activeTitlebarOpacity->setValue( m_internalSettings->activeTitlebarOpacity() );
        m_ui.activeTitlebarOpacity_2->setValue( m_internalSettings->activeTitlebarOpacity() );
        m_ui.inactiveTitlebarOpacity->setValue( m_internalSettings->inactiveTitlebarOpacity() );
        m_ui.inactiveTitlebarOpacity_2->setValue( m_internalSettings->inactiveTitlebarOpacity() );
        /*setEnabledTransparentTitlebarOptions();*/
        m_ui.drawBorderOnMaximizedWindows->setChecked( m_internalSettings->drawBorderOnMaximizedWindows() );
        m_ui.boldButtonIcons->setCurrentIndex( m_internalSettings->boldButtonIcons() );
        m_ui.redOutline->setChecked( m_internalSettings->redOutline() );
        m_ui.drawSizeGrip->setChecked( m_internalSettings->drawSizeGrip() );
        m_ui.drawBackgroundGradient->setChecked( m_internalSettings->drawBackgroundGradient() );
        m_ui.drawTitleBarSeparator->setChecked( m_internalSettings->drawTitleBarSeparator() );
        m_ui.animationsEnabled->setChecked( m_internalSettings->animationsEnabled() );
        m_ui.animationsSpeedRelativeSystem->setValue( m_internalSettings->animationsSpeedRelativeSystem() );
        setEnabledAnimationsSpeed();
        m_ui.useTitlebarColorForAllBorders->setChecked( m_internalSettings->useTitlebarColorForAllBorders() );
        m_ui.opaqueMaximizedTitlebars->setChecked( m_internalSettings->opaqueMaximizedTitlebars() );
        m_ui.blurTransparentTitlebars->setChecked( m_internalSettings->blurTransparentTitlebars() );
        m_ui.translucentButtonBackgrounds->setChecked( m_internalSettings->translucentButtonBackgrounds() );
        
        // load shadows
        if( m_internalSettings->shadowSize() <= InternalSettings::ShadowVeryLarge ) m_ui.shadowSize->setCurrentIndex( m_internalSettings->shadowSize() );
        else m_ui.shadowSize->setCurrentIndex( InternalSettings::ShadowLarge );

        m_ui.shadowStrength->setValue( qRound(qreal(m_internalSettings->shadowStrength()*100)/255 ) );
        m_ui.shadowColor->setColor( m_internalSettings->shadowColor() );
        m_ui.thinWindowOutlineStyle->setCurrentIndex( m_internalSettings->thinWindowOutlineStyle() );
        
        // load exceptions
        ExceptionList exceptions;
        exceptions.readConfig( m_configuration );
        m_ui.exceptions->setExceptions( exceptions.get() );
        setChanged( false );

    }

    //_________________________________________________________
    void ConfigWidget::save()
    {

        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr( new InternalSettings() );
        m_internalSettings->load();

        // apply modifications from ui
        m_internalSettings->setTitleAlignment( m_ui.titleAlignment->currentIndex() );
        m_internalSettings->setButtonIconStyle( m_ui.buttonIconStyle->currentIndex() );
        m_internalSettings->setButtonShape( m_ui.buttonShape->currentIndex() );
        m_internalSettings->setBackgroundColors( m_ui.backgroundColors->currentIndex() );
        m_internalSettings->setAlwaysShow( m_ui.alwaysShow->currentIndex() );
        m_internalSettings->setAlwaysShowIconHighlightUsing( m_ui.alwaysShowIconHighlightUsing->currentIndex() );
        m_internalSettings->setIconSize( m_ui.iconSize->currentIndex() );
        m_internalSettings->setButtonSpacingRight( m_ui.buttonSpacingRight->value() );
        m_internalSettings->setButtonSpacingLeft( m_ui.buttonSpacingLeft->value() );
        m_internalSettings->setTitlebarTopBottomMargins( m_ui.titlebarTopBottomMargins->value() );
        m_internalSettings->setPercentMaximizedTopBottomMargins( m_ui.percentMaximizedTopBottomMargins->value() );
        m_internalSettings->setTitlebarSideMargins( m_ui.titlebarSideMargins->value() );
        m_internalSettings->setCornerRadius( m_ui.cornerRadius->value() );
        m_internalSettings->setActiveTitlebarOpacity( m_ui.activeTitlebarOpacity->value() );
        m_internalSettings->setInactiveTitlebarOpacity( m_ui.inactiveTitlebarOpacity->value() );
        m_internalSettings->setBoldButtonIcons( m_ui.boldButtonIcons->currentIndex() );
        m_internalSettings->setRedOutline( m_ui.redOutline->isChecked() );
        m_internalSettings->setDrawBorderOnMaximizedWindows( m_ui.drawBorderOnMaximizedWindows->isChecked() );
        m_internalSettings->setDrawSizeGrip( m_ui.drawSizeGrip->isChecked() );
        m_internalSettings->setDrawBackgroundGradient( m_ui.drawBackgroundGradient->isChecked() );
        m_internalSettings->setDrawTitleBarSeparator(m_ui.drawTitleBarSeparator->isChecked());
        m_internalSettings->setAnimationsEnabled( m_ui.animationsEnabled->isChecked() );
        m_internalSettings->setAnimationsSpeedRelativeSystem( m_ui.animationsSpeedRelativeSystem->value() );
        m_internalSettings->setUseTitlebarColorForAllBorders(m_ui.useTitlebarColorForAllBorders->isChecked());
        m_internalSettings->setOpaqueMaximizedTitlebars(m_ui.opaqueMaximizedTitlebars->isChecked());
        m_internalSettings->setBlurTransparentTitlebars(m_ui.blurTransparentTitlebars->isChecked());
        m_internalSettings->setTranslucentButtonBackgrounds(m_ui.translucentButtonBackgrounds->isChecked());
        
        m_internalSettings->setShadowSize( m_ui.shadowSize->currentIndex() );
        m_internalSettings->setShadowStrength( qRound( qreal(m_ui.shadowStrength->value()*255)/100 ) );
        m_internalSettings->setShadowColor( m_ui.shadowColor->color() );
        m_internalSettings->setThinWindowOutlineStyle( m_ui.thinWindowOutlineStyle->currentIndex() );

        // save configuration
        m_internalSettings->save();

        // get list of exceptions and write
        InternalSettingsList exceptions( m_ui.exceptions->exceptions() );
        ExceptionList( exceptions ).writeConfig( m_configuration );

        // sync configuration
        m_configuration->sync();
        setChanged( false );

        // needed to tell kwin to reload when running from external kcmshell
        {
            QDBusMessage message = QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
            QDBusConnection::sessionBus().send(message);
        }

        // needed for breeze style to reload shadows
        {
            QDBusMessage message( QDBusMessage::createSignal("/ClassikstylesDecoration",  "org.kde.Classikstyles.Style", "reparseConfiguration") );
            QDBusConnection::sessionBus().send(message);
        }

    }

    //_________________________________________________________
    void ConfigWidget::defaults()
    {

        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr( new InternalSettings() );
        m_internalSettings->setDefaults();

        // assign to ui
        m_ui.titleAlignment->setCurrentIndex( m_internalSettings->titleAlignment() );
        m_ui.buttonIconStyle->setCurrentIndex( m_internalSettings->buttonIconStyle() );
        m_ui.buttonShape->setCurrentIndex( m_internalSettings->buttonShape() );
        m_ui.backgroundColors->setCurrentIndex( m_internalSettings->backgroundColors() );
        m_ui.alwaysShow->setCurrentIndex( m_internalSettings->alwaysShow() );
        m_ui.alwaysShowIconHighlightUsing->setCurrentIndex( m_internalSettings->alwaysShowIconHighlightUsing() );
        m_ui.iconSize->setCurrentIndex( m_internalSettings->iconSize() );
        m_ui.buttonSpacingRight->setValue( m_internalSettings->buttonSpacingRight() );
        m_ui.buttonSpacingLeft->setValue( m_internalSettings->buttonSpacingLeft() );
        m_ui.titlebarTopBottomMargins->setValue( m_internalSettings->titlebarTopBottomMargins() );
        m_ui.percentMaximizedTopBottomMargins->setValue( m_internalSettings->percentMaximizedTopBottomMargins() );
        m_ui.titlebarSideMargins->setValue( m_internalSettings->titlebarSideMargins() );
        m_ui.cornerRadius->setValue( m_internalSettings->cornerRadius() );
        m_ui.activeTitlebarOpacity->setValue( m_internalSettings->activeTitlebarOpacity() );
        m_ui.inactiveTitlebarOpacity->setValue( m_internalSettings->inactiveTitlebarOpacity() );
        /*setEnabledTransparentTitlebarOptions();*/
        m_ui.boldButtonIcons->setCurrentIndex( m_internalSettings->boldButtonIcons() );
        m_ui.redOutline->setChecked( m_internalSettings->redOutline() );
        m_ui.drawBorderOnMaximizedWindows->setChecked( m_internalSettings->drawBorderOnMaximizedWindows() );
        m_ui.drawSizeGrip->setChecked( m_internalSettings->drawSizeGrip() );
        m_ui.drawBackgroundGradient->setChecked( m_internalSettings->drawBackgroundGradient() );
        m_ui.animationsEnabled->setChecked( m_internalSettings->animationsEnabled() );
        m_ui.animationsSpeedRelativeSystem->setValue( m_internalSettings->animationsSpeedRelativeSystem() );
        setEnabledAnimationsSpeed();
        m_ui.drawTitleBarSeparator->setChecked( m_internalSettings->drawTitleBarSeparator() );
        m_ui.useTitlebarColorForAllBorders->setChecked( m_internalSettings->useTitlebarColorForAllBorders() );
        m_ui.opaqueMaximizedTitlebars->setChecked( m_internalSettings->opaqueMaximizedTitlebars() );
        m_ui.blurTransparentTitlebars->setChecked( m_internalSettings->blurTransparentTitlebars() );
        m_ui.translucentButtonBackgrounds->setChecked( m_internalSettings->translucentButtonBackgrounds() );
        
        m_ui.shadowSize->setCurrentIndex( m_internalSettings->shadowSize() );
        m_ui.shadowStrength->setValue( qRound(qreal(m_internalSettings->shadowStrength()*100)/255 ) );
        m_ui.shadowColor->setColor( m_internalSettings->shadowColor() );
        m_ui.thinWindowOutlineStyle->setCurrentIndex( m_internalSettings->thinWindowOutlineStyle() );

    }

    //_______________________________________________
    void ConfigWidget::updateChanged()
    {

        // check configuration
        if( !m_internalSettings ) return;

        // track modifications
        bool modified( false );

        if ( m_ui.drawTitleBarSeparator->isChecked() != m_internalSettings->drawTitleBarSeparator()) modified = true;
        else if( m_ui.useTitlebarColorForAllBorders->isChecked() != m_internalSettings->useTitlebarColorForAllBorders()) modified = true;
        else if( m_ui.opaqueMaximizedTitlebars->isChecked() != m_internalSettings->opaqueMaximizedTitlebars()) modified = true;
        else if( m_ui.blurTransparentTitlebars->isChecked() != m_internalSettings->blurTransparentTitlebars()) modified = true;
        else if( m_ui.translucentButtonBackgrounds->isChecked() != m_internalSettings->translucentButtonBackgrounds()) modified = true;
        else if( m_ui.titleAlignment->currentIndex() != m_internalSettings->titleAlignment() ) modified = true;
        else if( m_ui.buttonIconStyle->currentIndex() != m_internalSettings->buttonIconStyle() ) modified = true;
        else if( m_ui.buttonShape->currentIndex() != m_internalSettings->buttonShape() ) modified = true;
        else if( m_ui.backgroundColors->currentIndex() != m_internalSettings->backgroundColors() ) modified = true;
        else if( m_ui.alwaysShow->currentIndex() != m_internalSettings->alwaysShow() ) modified = true;
        else if( m_ui.alwaysShowIconHighlightUsing->currentIndex() != m_internalSettings->alwaysShowIconHighlightUsing() ) modified = true;
        else if( m_ui.iconSize->currentIndex() != m_internalSettings->iconSize() ) modified = true;
        else if( m_ui.boldButtonIcons->currentIndex() != m_internalSettings->boldButtonIcons() ) modified = true;
        else if( m_ui.redOutline->isChecked() != m_internalSettings->redOutline() ) modified = true;
        else if( m_ui.drawBorderOnMaximizedWindows->isChecked() !=  m_internalSettings->drawBorderOnMaximizedWindows() ) modified = true;
        else if( m_ui.drawSizeGrip->isChecked() !=  m_internalSettings->drawSizeGrip() ) modified = true;
        else if( m_ui.drawBackgroundGradient->isChecked() !=  m_internalSettings->drawBackgroundGradient() ) modified = true;
        else if( m_ui.buttonSpacingRight->value() != m_internalSettings->buttonSpacingRight() ) modified = true;
        else if( m_ui.buttonSpacingLeft->value() != m_internalSettings->buttonSpacingLeft() ) modified = true;
        else if( m_ui.titlebarTopBottomMargins->value() != m_internalSettings->titlebarTopBottomMargins() ) modified = true;
        else if( m_ui.percentMaximizedTopBottomMargins->value() != m_internalSettings->percentMaximizedTopBottomMargins() ) modified = true;
        else if( m_ui.titlebarSideMargins->value() != m_internalSettings->titlebarSideMargins() ) modified = true;
        else if( m_ui.cornerRadius->value() != m_internalSettings->cornerRadius() ) modified = true;
        else if( m_ui.activeTitlebarOpacity->value() != m_internalSettings->activeTitlebarOpacity() ) modified = true;
        else if( m_ui.inactiveTitlebarOpacity->value() != m_internalSettings->inactiveTitlebarOpacity() ) modified = true;

        // animations
        else if( m_ui.animationsEnabled->isChecked() !=  m_internalSettings->animationsEnabled() ) modified = true;
        else if( m_ui.animationsSpeedRelativeSystem->value() != m_internalSettings->animationsSpeedRelativeSystem() ) modified = true;

        // shadows
        else if( m_ui.shadowSize->currentIndex() !=  m_internalSettings->shadowSize() ) modified = true;
        else if( qRound( qreal(m_ui.shadowStrength->value()*255)/100 ) != m_internalSettings->shadowStrength() ) modified = true;
        else if( m_ui.shadowColor->color() != m_internalSettings->shadowColor() ) modified = true;
        else if( m_ui.thinWindowOutlineStyle->currentIndex() != m_internalSettings->thinWindowOutlineStyle() ) modified = true;
        
        // exceptions
        else if( m_ui.exceptions->isChanged() ) modified = true;

        setChanged( modified );

    }

    //_______________________________________________
    void ConfigWidget::setChanged( bool value )
    {
        emit changed( value );
    }

    //only enable animationsSpeedRelativeSystem and animationsSpeedLabelx when animationsEnabled is checked
    void ConfigWidget::setEnabledAnimationsSpeed()
    {
        m_ui.animationsSpeedRelativeSystem->setEnabled(m_ui.animationsEnabled->isChecked());
        m_ui.animationsSpeedLabel1->setEnabled(m_ui.animationsEnabled->isChecked());
        m_ui.animationsSpeedLabel2->setEnabled(m_ui.animationsEnabled->isChecked());
        m_ui.animationsSpeedLabel3->setEnabled(m_ui.animationsEnabled->isChecked());
        m_ui.animationsSpeedLabel4->setEnabled(m_ui.animationsEnabled->isChecked());
    }
    
    
    /*
     * commented out as now the system colour transparency is also considered
     * TODO:potentially read the system colour settings and enable/disable based on these as well
    //only enable blurTransparentTitlebars and opaqueMaximizedTitlebars options if transparent titlebars are enabled
    void ConfigWidget::setEnabledTransparentTitlebarOptions()
    {
        if ( m_ui.activeTitlebarOpacity->value() != 100 ||  m_ui.inactiveTitlebarOpacity->value() != 100 || ){
            m_ui.opaqueMaximizedTitlebars->setEnabled(true);
            m_ui.blurTransparentTitlebars->setEnabled(true);
        } else {
            m_ui.opaqueMaximizedTitlebars->setEnabled(false);
            m_ui.blurTransparentTitlebars->setEnabled(false);
        }
    }
    */
}
