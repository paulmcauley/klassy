#include <KColorScheme>
#include <KConfig>
#include <KConfigGroup>

#include <QDebug>

int main()
{
    KConfig globals("kdeglobals");
    KConfigGroup general(&globals, "General");
    if (general.readEntry("ColorScheme") != QLatin1String("Breeze")) {
        return 0;
    }
    QString breezeLightPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("color-schemes/BreezeLight.colors"));
    if (breezeLightPath.isEmpty()) {
        return 0;
    }
    KConfig breezeLight(breezeLightPath, KConfig::SimpleConfig);
    for (const auto &group : breezeLight.groupList()) {
        auto destination = KConfigGroup(&globals, group);
        KConfigGroup(&breezeLight, group).copyTo(&destination, KConfig::Notify);
    }
}
