// src/backend/dbus/dbuswrapper.cpp
#include "dbuswrapper.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QDBusVariant>
#include <QDebug>

DBusWrapper::DBusWrapper()
{
    m_systemBusAvailable = QDBusConnection::systemBus().isConnected();
    if (!m_systemBusAvailable) {
        qWarning() << "D-Bus system bus not available, using fallback values";
    }

    // New: Hook into the Session Bus to listen for live OS theme updates
    if (QDBusConnection::sessionBus().isConnected()) {
        QDBusConnection::sessionBus().connect(
            "org.freedesktop.portal.Desktop",
            "/org/freedesktop/portal/desktop",
            "org.freedesktop.portal.Settings",
            "SettingChanged",
            this,
            SLOT(onPortalSettingChanged(QString, QString, QDBusVariant))
        );
    }
}

DBusWrapper::~DBusWrapper() = default;

bool DBusWrapper::getSystemColorScheme()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        return true; // Default fallback to dark mode
    }

    QDBusMessage message = QDBusMessage::createMethodCall(
        "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop",
        "org.freedesktop.portal.Settings",
        "Read"
    );
    message << "org.freedesktop.appearance" << "color-scheme";

    QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()) {
        QVariant variant = reply.arguments().first();
        QDBusVariant dbusVariant = qvariant_cast<QDBusVariant>(variant);
        uint colorScheme = dbusVariant.variant().toUInt();
        return (colorScheme == 1); // 1 = Prefer Dark, 2 = Prefer Light
    }

    return true;
}

void DBusWrapper::onPortalSettingChanged(const QString &namespaceName, const QString &key, const QDBusVariant &value)
{
    if (namespaceName == "org.freedesktop.appearance" && key == "color-scheme") {
        uint colorScheme = value.variant().toUInt();
        emit systemColorSchemeChanged(colorScheme == 1);
    }
}

std::tuple<double, bool> DBusWrapper::getBatteryStatus()
{
    double percent = 100.0;
    bool charging = false;

    if (!m_systemBusAvailable) {
        return {percent, charging};
    }

    // Try to query UPower for battery info
    QDBusInterface upower(
        "org.freedesktop.UPower",
        "/org/freedesktop/UPower",
        "org.freedesktop.UPower",
        QDBusConnection::systemBus()
    );

    if (upower.isValid()) {
        const QDBusReply<QList<QDBusObjectPath>> devicesReply = upower.call("EnumerateDevices");
        if (devicesReply.isValid()) {
            for (const QDBusObjectPath &device : devicesReply.value()) {
                const QString devicePath = device.path();
                if (!devicePath.contains("battery", Qt::CaseInsensitive))
                    continue;

                QDBusInterface deviceIface(
                    "org.freedesktop.UPower",
                    devicePath,
                    "org.freedesktop.DBus.Properties",
                    QDBusConnection::systemBus()
                );

                const QDBusReply<QDBusVariant> percentReply = deviceIface.call(
                    "Get",
                    "org.freedesktop.UPower.Device",
                    "Percentage"
                );
                if (percentReply.isValid()) {
                    const QVariant value = percentReply.value().variant();
                    if (value.canConvert<double>())
                        percent = value.toDouble();
                }

                const QDBusReply<QDBusVariant> stateReply = deviceIface.call(
                    "Get",
                    "org.freedesktop.UPower.Device",
                    "State"
                );
                if (stateReply.isValid()) {
                    const QVariant value = stateReply.value().variant();
                    if (value.canConvert<uint>()) {
                        const uint state = value.toUInt();
                        charging = (state == 1 || state == 5);  // 1=charging, 5=fully charged
                    }
                }

                break;
            }
        }
    } else {
        qDebug() << "UPower D-Bus service not available";
    }

    return {percent, charging};
}

std::tuple<QString, bool> DBusWrapper::getNetworkStatus()
{
    QString status = "Disconnected";
    bool connected = false;

    if (!m_systemBusAvailable) {
        return {status, connected};
    }

    // Try to query NetworkManager
    QDBusInterface nm(
        "org.freedesktop.NetworkManager",
        "/org/freedesktop/NetworkManager",
        "org.freedesktop.DBus.Properties",
        QDBusConnection::systemBus()
    );

    if (nm.isValid()) {
        const QDBusReply<QDBusVariant> connectivityReply = nm.call(
            "Get",
            "org.freedesktop.NetworkManager",
            "Connectivity"
        );

        if (connectivityReply.isValid()) {
            const QVariant value = connectivityReply.value().variant();
            if (value.canConvert<uint>()) {
                const uint connectivity = value.toUInt();
                connected = (connectivity == 4);  // 4 = full connectivity

                if (connected) {
                    status = "WiFi";  // Simplified; could query active connection type
                }
            }
        }
    } else {
        qDebug() << "NetworkManager D-Bus service not available";
    }

    return {status, connected};
}


// =========================================================
// MPRIS MEDIA CONTROL METHODS (NEW)
// =========================================================

void DBusWrapper::setupMprisListener()
{
    // Dynamically connect to the PropertiesChanged signal on any service path
    // implementing the org.mpris.MediaPlayer2.Player interface
    bool mprisPropertyConnected = QDBusConnection::sessionBus().connect(
        "", // Connect to any registered service name matching the pattern below
        "/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        this,
        SLOT(onMprisPropertiesChanged(QString, QVariantMap, QStringList))
    );

    if (!mprisPropertyConnected) {
        qWarning() << "Failed to register MPRIS global PropertiesChanged listener";
    }
}

void DBusWrapper::onMprisPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(changedProperties);
    Q_UNUSED(invalidatedProperties);
    
    // Notify the backend engine to recalculate state whenever metadata/playback state shifts
    if (interfaceName == "org.mpris.MediaPlayer2.Player") {
        emit mediaPropertiesChanged();
    }
}

QString DBusWrapper::findActiveMediaPlayer()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        return QString();
    }

    QDBusReply<QStringList> services = QDBusConnection::sessionBus().interface()->registeredServiceNames();
    if (!services.isValid()) {
        return QString();
    }

    // Step 1: Scan for players that are actively playing music (highest priority)
    for (const QString &service : services.value()) {
        if (service.startsWith("org.mpris.MediaPlayer2.")) {
            if (getMediaPlaybackStatus(service)) {
                m_currentTrackedPlayer = service;
                return service;
            }
        }
    }

    // Step 2: Fall back to any registered MPRIS player currently open (e.g., paused)
    for (const QString &service : services.value()) {
        if (service.startsWith("org.mpris.MediaPlayer2.")) {
            m_currentTrackedPlayer = service;
            return service;
        }
    }

    m_currentTrackedPlayer.clear();
    return QString();
}

bool DBusWrapper::getMediaPlaybackStatus(const QString &serviceName)
{
    if (serviceName.isEmpty()) {
        return false;
    }

    QDBusInterface player(
        serviceName,
        "/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties",
        QDBusConnection::sessionBus()
    );

    if (player.isValid()) {
        QDBusReply<QDBusVariant> reply = player.call("Get", "org.mpris.MediaPlayer2.Player", "PlaybackStatus");
        if (reply.isValid()) {
            QString status = reply.value().variant().toString();
            return (status == "Playing");
        }
    }
    return false;
}

QString DBusWrapper::getMediaTitle(const QString &serviceName)
{
    if (serviceName.isEmpty()) {
        return QString();
    }

    QDBusInterface player(
        serviceName,
        "/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties",
        QDBusConnection::sessionBus()
    );

    if (player.isValid()) {
        QDBusReply<QDBusVariant> reply = player.call("Get", "org.mpris.MediaPlayer2.Player", "Metadata");
        if (reply.isValid()) {
            QVariantMap metadata = qvariant_cast<QVariantMap>(reply.value().variant());
            // Standard MPRIS metadata key for the song title is "xesam:title"
            return metadata.value("xesam:title").toString();
        }
    }
    return QString();
}

double DBusWrapper::getMediaProgress(const QString &serviceName)
{
    if (serviceName.isEmpty()) {
        return 0.0;
    }

    QDBusInterface player(
        serviceName,
        "/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties",
        QDBusConnection::sessionBus()
    );

    if (player.isValid()) {
        // Step 1: Read total length (lives inside the Metadata map, stored in microseconds)
        QDBusReply<QDBusVariant> metadataReply = player.call("Get", "org.mpris.MediaPlayer2.Player", "Metadata");
        double totalLength = 0.0;
        if (metadataReply.isValid()) {
            QVariantMap metadata = qvariant_cast<QVariantMap>(metadataReply.value().variant());
            totalLength = metadata.value("mpris:length").toDouble();
        }

        if (totalLength <= 0.0) {
            return 0.0;
        }

        // Step 2: Read current playback position (stored in microseconds)
        QDBusReply<QDBusVariant> positionReply = player.call("Get", "org.mpris.MediaPlayer2.Player", "Position");
        double position = 0.0;
        if (positionReply.isValid()) {
            position = positionReply.value().variant().toDouble();
        }

        // Return progress normalized between 0.0 and 1.0
        return qBound(0.0, position / totalLength, 1.0);
    }
    return 0.0;
}

void DBusWrapper::triggerMediaAction(const QString &serviceName, const QString &action)
{
    if (serviceName.isEmpty()) {
        return;
    }

    // Call MPRIS methods directly (e.g. PlayPause, Next, Previous) on the Player interface
    QDBusInterface player(
        serviceName,
        "/org/mpris/MediaPlayer2",
        "org.mpris.MediaPlayer2.Player",
        QDBusConnection::sessionBus()
    );

    if (player.isValid()) {
        player.call(action);
    }
}