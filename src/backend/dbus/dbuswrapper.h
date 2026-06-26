#ifndef DBUSWRAPPER_H
#define DBUSWRAPPER_H

#include <QObject>
#include <QString>
#include <tuple> // C++ Standard Library tuple replaces the non-existent QTuple
#include <QDBusVariant>
#include <QVariantMap>
#include <QTimer>

class DBusWrapper : public QObject
{
    Q_OBJECT

public:
    DBusWrapper();
    ~DBusWrapper();

    // Hardware Status Methods
    std::tuple<double, bool> getBatteryStatus();
    std::tuple<QString, bool> getNetworkStatus();
    
    // Portal / Theme Methods
    bool getSystemColorScheme();

    // MPRIS Media Control Methods
    QString findActiveMediaPlayer();
    bool getMediaPlaybackStatus(const QString &serviceName);
    QString getMediaTitle(const QString &serviceName);
    double getMediaProgress(const QString &serviceName); // Returns 0.0 to 1.0

    // Media Action Trigger
    void triggerMediaAction(const QString &serviceName, const QString &action);

    // Diagnostics Debug Dump
    void debugDumpMprisPlayers();

signals:
    void systemColorSchemeChanged(bool isDark);
    
    // Signals for active media state changes
    void mediaPropertiesChanged();

private slots:
    void onPortalSettingChanged(const QString &namespaceName, const QString &key, const QDBusVariant &value);
    
    // Slot registrations matching updated source file
    void onMprisPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);
    void onMprisSeeked(qlonglong position);
    void onDBusNameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);

private:
    void setupMprisListener();
    void refreshMediaProgressTimer();

    bool m_systemBusAvailable = false;
    QString m_currentTrackedPlayer;
    
    // Structural state members
    bool m_mprisListenerSetup = false;
    QTimer *m_mediaProgressTimer = nullptr;
};

#endif // DBUSWRAPPER_H