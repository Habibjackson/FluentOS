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
}

DBusWrapper::~DBusWrapper() = default;

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
