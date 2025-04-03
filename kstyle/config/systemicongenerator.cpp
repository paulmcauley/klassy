/*
 * SPDX-FileCopyrightText: 2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "systemicongenerator.h"
#include "renderdecorationbuttonicon.h"
#include <KLocalizedString>
#include <KSharedConfig>
#include <QApplication>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QStandardPaths>
#include <QStringBuilder>
#include <QSvgGenerator>
#include <QVariantMap>

namespace Breeze
{

SystemIconGenerator::SystemIconGenerator(InternalSettingsPtr internalSettings)
    : m_internalSettings(internalSettings)
{
}

void SystemIconGenerator::generate()
{
    if (m_internalSettings->buttonIconStyle() == InternalSettings::EnumButtonIconStyle::StyleSystemIconTheme) {
        return;
    }

    addSystemScales();

    QString iconsPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) % QStringLiteral("/icons");

    QString lightIconsPath = iconsPath + QStringLiteral("/klassy");
    DecorationColors decorationColorsLight(false);
    decorationColorsLight.generateDecorationAndButtonColors(QApplication::palette(),
                                                            m_internalSettings,
                                                            QColor(QStringLiteral("#232629")),
                                                            QColor(QStringLiteral("#dee0e2")),
                                                            QColor(QStringLiteral("#232629")),
                                                            QColor(QStringLiteral("#dee0e2")),
                                                            "",
                                                            true,
                                                            true); // titlebar colours based on Breeze Light
    generateIconThemeDir(lightIconsPath, QStringLiteral("Klassy"), m_internalSettings->klassyIconThemeInherits(), decorationColorsLight);

    QString darkIconsPath = iconsPath + QStringLiteral("/klassy-dark");
    DecorationColors decorationColorsDark(false);
    decorationColorsDark.generateDecorationAndButtonColors(QApplication::palette(),
                                                           m_internalSettings,
                                                           QColor(QStringLiteral("#fcfcfc")),
                                                           QColor(QStringLiteral("#31363b")),
                                                           QColor(QStringLiteral("#fcfcfc")),
                                                           QColor(QStringLiteral("#31363b")),
                                                           "",
                                                           true,
                                                           true); // titlebar colours based on Breeze Dark
    generateIconThemeDir(darkIconsPath, QStringLiteral("Klassy Dark"), m_internalSettings->klassyDarkIconThemeInherits(), decorationColorsDark);
}

void SystemIconGenerator::generateIconThemeDir(const QString themeDirPath,
                                               const QString themeName,
                                               const QString inherits,
                                               const DecorationColors &decorationColors)
{
    QDir iconDir(themeDirPath);
    if (iconDir.exists()) {
        iconDir.removeRecursively();
    }

    KConfig themeIndex(themeDirPath % QStringLiteral("/index.theme"));
    KConfigGroup iconThemeGroup = themeIndex.group("Icon Theme");
    iconThemeGroup.writeEntry("Name", themeName);
    iconThemeGroup.writeEntry("Comment", themeName + i18n(" by Paul A McAuley, auto-generated by Klassy window decoration"));
    iconThemeGroup.writeEntry("DisplayDepth", "32");
    iconThemeGroup.writeEntry("Inherits", inherits);
    iconThemeGroup.writeEntry("Example", "folder");
    iconThemeGroup.writeEntry("FollowsColorScheme", "true");

    iconThemeGroup.writeEntry("DesktopDefault", "48");
    iconThemeGroup.writeEntry("DesktopSizes", "16,22,32,48,64,128,256");
    iconThemeGroup.writeEntry("ToolbarDefault", "22");
    iconThemeGroup.writeEntry("ToolbarSizes", "16,22,32,48");
    iconThemeGroup.writeEntry("MainToolbarDefault", "22");
    iconThemeGroup.writeEntry("MainToolbarSizes", "16,22,32,48");
    iconThemeGroup.writeEntry("SmallDefault", "16");
    iconThemeGroup.writeEntry("SmallSizes", "16,22,32,48");
    iconThemeGroup.writeEntry("PanelDefault", "48");
    iconThemeGroup.writeEntry("PanelSizes", "16,22,32,48,64,128,256");
    iconThemeGroup.writeEntry("DialogDefault", "32");
    iconThemeGroup.writeEntry("DialogSizes", "16,22,32,48,64,128,256");

    iconThemeGroup.writeEntry("KDE-Extensions", ".svg");

    // used the Application Menu button type as a reference to define a "bland" standard icon colour which the KDE svg renderer should replace from the system
    // colour scheme
    QColor blandIconColor = decorationColors.buttonPalette(DecorationButtonType::ApplicationMenu)->active()->foregroundNormal;
    if (!blandIconColor.isValid()) {
        blandIconColor = decorationColors.buttonPalette(DecorationButtonType::ApplicationMenu)->active()->foregroundHover;
    }
    QString blandIconColorString = blandIconColor.name();

    for (int i = 0; i < m_scales.count(); i++) {
        for (auto size = m_iconSizes.begin(); size != m_iconSizes.end(); size++) {
            QString svgDirName = QString::number(size.value()) % QStringLiteral("-") % QString::number(i);
            QString svgDirPath = themeDirPath % QStringLiteral("/") % svgDirName;
            QDir dir(svgDirPath);
            dir.mkpath(svgDirPath);

            if (i == 0) {
                if (!iconThemeGroup.readEntry("Directories", "").isEmpty()) {
                    iconThemeGroup.writeEntry("Directories", iconThemeGroup.readEntry("Directories") + "," + svgDirName);
                } else {
                    iconThemeGroup.writeEntry("Directories", svgDirName);
                }
            } else {
                if (!iconThemeGroup.readEntry("ScaledDirectories", "").isEmpty()) {
                    iconThemeGroup.writeEntry("ScaledDirectories", iconThemeGroup.readEntry("ScaledDirectories") + "," + svgDirName);
                } else {
                    iconThemeGroup.writeEntry("ScaledDirectories", svgDirName);
                }
            }

            KConfigGroup svgDirGroup = themeIndex.group(svgDirName);
            svgDirGroup.writeEntry("Size", QString::number(size.value()));
            if (i != 0) {
                svgDirGroup.writeEntry("Scale", QString::number(m_scales[i]));
            }
            svgDirGroup.writeEntry("Context", "Actions");
            if (size.value() == 32) {
                svgDirGroup.writeEntry("Type", "Scalable");
                svgDirGroup.writeEntry("MinSize", "32");
                svgDirGroup.writeEntry("MaxSize", "256");
            } else {
                svgDirGroup.writeEntry("Type", "Fixed");
            }

            for (auto &iconType : m_iconTypes) {
                QSvgGenerator svgGenerator;
                QFile file(svgDirPath % QStringLiteral("/") % iconType.name % QStringLiteral(".svg"));
                if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    continue;
                }

                svgGenerator.setOutputDevice(&file);
                int scaledWidth = qRound(size.value() * m_scales.at(i));
                QSize iconSizeScaled(scaledWidth, scaledWidth);
                svgGenerator.setSize(iconSizeScaled);
                svgGenerator.setViewBox(QRect(QPoint(0, 0), iconSizeScaled));
                svgGenerator.setResolution(qRound(96 * m_scales.at(i)));
                svgGenerator.setDescription(i18n("Auto-generated by Klassy window decoration"));
                std::unique_ptr<QPainter> painter = std::make_unique<QPainter>();
                painter->begin(&svgGenerator);

                painter->setViewport(QRect(QPoint(0, 0), iconSizeScaled));
                painter->setRenderHints(QPainter::RenderHint::Antialiasing);

                QColor textColor = decorationColors.buttonPalette(iconType.type)->active()->foregroundNormal;
                if (!textColor.isValid()) {
                    textColor = decorationColors.buttonPalette(iconType.type)->active()->foregroundHover;
                }
                QString textColorString = textColor.name();
                QPen pen((QColor(textColorString)));

                bool boldButtons =
                    (m_internalSettings->boldButtonIcons() == InternalSettings::EnumBoldButtonIcons::BoldIconsBold
                     || (m_internalSettings->boldButtonIcons() == InternalSettings::EnumBoldButtonIcons::BoldIconsHiDpiOnly && m_scales.at(i) >= 1.2));

                // paint the close background to SVG
                if (iconType.type == DecorationButtonType::Close && iconType.name != QStringLiteral("window-close-symbolic")) {
                    painter->setWindow(0, 0, 16, 16);
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(decorationColors.buttonPalette(iconType.type)->active()->backgroundHover);

                    if (m_internalSettings->buttonShape() == InternalSettings::EnumButtonShape::ShapeSmallCircle) {
                        boldButtons ? painter->drawEllipse(QRectF(0, 0, 16, 16)) : painter->drawEllipse(QRectF(1, 1, 14, 14));
                    } else {
                        qreal cornerRadius = 0;
                        if (m_internalSettings->buttonShape() == InternalSettings::EnumButtonShape::ShapeSmallRoundedSquare
                            || m_internalSettings->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullHeightRoundedRectangle
                            || m_internalSettings->buttonShape() == InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangle
                            || m_internalSettings->buttonShape() == InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangleGrouped) {
                            if (m_internalSettings->buttonCornerRadius() == InternalSettings::EnumButtonCornerRadius::Custom) {
                                cornerRadius = m_internalSettings->buttonCustomCornerRadius();
                            } else {
                                cornerRadius = m_internalSettings->windowCornerRadius();
                            }
                        }

                        if ((cornerRadius < 0.4 && m_internalSettings->windowCornerRadius() < 4))
                            painter->drawRect(QRectF(2, 2, 12, 12));
                        else
                            painter->drawRoundedRect(QRectF(2, 2, 12, 12), 20, 20, Qt::RelativeSize);
                    }
                    pen.setColor(textColor = decorationColors.buttonPalette(iconType.type)->active()->foregroundHover);
                }

                // paint the icon to SVG
                auto [iconRenderer,
                      localRenderingWidth](RenderDecorationButtonIcon::factory(m_internalSettings, painter.get(), false, boldButtons, m_scales.at(i)));
                painter->setWindow(0, 0, localRenderingWidth, localRenderingWidth);

                pen.setWidthF(PenWidth::Symbol * qMax((qreal)1.0, qreal(localRenderingWidth) / iconSizeScaled.width()));
                painter->setPen(pen);
                iconRenderer->setForceEvenSquares(true);
                iconRenderer->setStrokeToFilledPath(true);

                iconRenderer->renderIcon(iconType.type, iconType.checked);

                painter->end();
                file.close();

                // modify SVG XML attributes so KIconLoader can replace the colours with those from the current colour scheme
                if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
                    continue;
                }

                QDomDocument svgXml;
                if (!svgXml.setContent(&file)) {
                    file.close();
                    continue;
                } else {
                    file.close();
                    file.remove();
                }

                QDomNodeList svgElements = svgXml.elementsByTagName(QStringLiteral("svg"));
                if (!svgElements.count()) {
                    file.close();
                    continue;
                }

                QDomNode svgElement = svgElements.at(0);
                // add system colours CSS
                QDomElement styleElement = svgXml.createElement(QStringLiteral("style"));
                styleElement.setAttribute(QStringLiteral("id"), QStringLiteral("current-color-scheme"));
                styleElement.setAttribute(QStringLiteral("type"), QStringLiteral("text/css"));
                QDomText styleText = svgXml.createTextNode(QStringLiteral(".ColorScheme-Text {color:") % textColorString % QStringLiteral(";}"));
                QDomElement svgFirstChild = svgElement.firstChildElement();
                svgElement.insertBefore(styleElement, svgFirstChild);
                styleElement.appendChild(styleText);

                QDomNodeList svgChildNodes = svgElement.childNodes();
                for (int j = 0; j < svgChildNodes.count(); j++) {
                    QDomElement svgChildElement = svgChildNodes.at(j).toElement();
                    if (!svgChildElement.isNull() && svgChildElement.tagName() == QStringLiteral("g")) {
                        QDomNodeList svgGroups = svgChildElement.childNodes();
                        for (int k = svgGroups.count() - 1; k >= 0; k--) { // looping backwards as we remove nodes
                            QDomElement svgGroupElement = svgGroups.at(k).toElement();
                            if (!svgGroupElement.isNull() && svgGroupElement.tagName() == QStringLiteral("g")) {
                                if (!svgGroupElement.hasChildNodes()) { // remove empty groups
                                    svgChildElement.removeChild(svgGroupElement);
                                } else if (svgGroupElement.attribute(QStringLiteral("fill")) == QStringLiteral("none")
                                           && svgGroupElement.attribute(QStringLiteral("stroke"))
                                               == QStringLiteral("none")) { // remove invisible groups - fixes rendering in GTK apps
                                    svgChildElement.removeChild(svgGroupElement);
                                } else { // change attributes so KIconLoader can use system colours
                                    // overwrite bland colours with system colour
                                    if (textColorString == blandIconColorString) {
                                        svgGroupElement.setAttribute(QStringLiteral("class"), QStringLiteral("ColorScheme-Text"));
                                        if (svgGroupElement.attribute(QStringLiteral("stroke")) == textColorString) {
                                            svgGroupElement.setAttribute(QStringLiteral("stroke"), QStringLiteral("currentColor"));
                                        }
                                        if (svgGroupElement.attribute(QStringLiteral("fill")) == textColorString) {
                                            svgGroupElement.setAttribute(QStringLiteral("fill"), QStringLiteral("currentColor"));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    continue;
                }
                QTextStream output(&file);
                svgXml.save(output, 4);

                file.close();
            }
        }
    }

    themeIndex.sync();
}

void SystemIconGenerator::addSystemScales()
{
    QString outputsPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kscreen/outputs"), QStandardPaths::LocateDirectory);
    QDir outputsDir(outputsPath);
    if (!outputsDir.exists())
        return;

    QStringList outputsFiles = outputsDir.entryList(QDir::Files);

    for (QString &outputFile : outputsFiles) {
        QFile file(outputsPath % QStringLiteral("/") % outputFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        file.close();
        QJsonObject jsonObj = document.object();
        QVariantMap jsonMap = jsonObj.toVariantMap();
        qreal scale = jsonMap.value(QStringLiteral("scale"), 1.0).toDouble();
        if (scale < (0.5 - 0.0001) || scale > (3.0 + 0.0001))
            continue;

        bool scaleInList = false;
        for (auto i = m_scales.begin(); i != m_scales.end(); i++) {
            if (qAbs(*i - scale) < 0.0001) {
                scaleInList = true;
                break;
            }
        }

        if (!scaleInList) {
            m_scales.append(scale);
        }
    }
}

}
