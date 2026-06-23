// src/backend/dbus/dbuswrapper.h

#pragma once
#ifndef DBUSWRAPPER_H
#define DBUSWRAPPER_H

#include <QObject>
#include <QString>
#include <QDBusVariant>
#include <QDBusConnectionInterface>

class DBusWrapper : public QObject
{
    Q_OBJECT

public:
    DBusWrapper();
    ~DBusWrapper();

    std::tuple<double, bool> getBatteryStatus();
    std::tuple<QString, bool> getNetworkStatus();
    
    // Read initial portal color-scheme setting
    bool getSystemColorScheme();

    // MPRIS Media Control Methods
    QString findActiveMediaPlayer();
    bool getMediaPlaybackStatus(const QString &serviceName);
    QString getMediaTitle(const QString &serviceName);
    double getMediaProgress(const QString &serviceName); // Returns 0.0 to 1.0

    // Media Action Signals
    void triggerMediaAction(const QString &serviceName, const QString &action);

signals:
    // Emitted when the Linux system theme changes live via D-Bus
    void systemColorSchemeChanged(bool isDark);

    void mediaPropertiesChanged();

private slots:
    // Internal D-Bus callback listener
    void onPortalSettingChanged(const QString &namespaceName, const QString &key, const QDBusVariant &value);
    void onMprisPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

private:
    void setupMprisListener();

    bool m_systemBusAvailable = false;
    QString m_currentTrackedPlayer;
};

#endif // DBUSWRAPPER_H