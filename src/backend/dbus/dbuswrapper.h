// src/backend/dbus/dbuswrapper.h

#pragma once
#ifndef DBUSWRAPPER_H
#define DBUSWRAPPER_H

#include <QObject>
#include <QString>
#include <QDBusVariant>

class DBusWrapper : public QObject
{
    Q_OBJECT

public:
    DBusWrapper();
    ~DBusWrapper();

    std::tuple<double, bool> getBatteryStatus();
    std::tuple<QString, bool> getNetworkStatus();
    
    // New: Read initial portal color-scheme setting
    bool getSystemColorScheme();

signals:
    // New: Emitted when the Linux system theme changes live via D-Bus
    void systemColorSchemeChanged(bool isDark);

private slots:
    // New: Internal D-Bus callback listener
    void onPortalSettingChanged(const QString &namespaceName, const QString &key, const QDBusVariant &value);

private:
    bool m_systemBusAvailable = false;
};

#endif // DBUSWRAPPER_H