// src/backend/systemstate.h
#ifndef SYSTEMSTATE_H
#define SYSTEMSTATE_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QDateTime>
#include <QSettings>
#include <QColor>
#include <memory>

class DBusWrapper;

class SystemState : public QObject
{
    Q_OBJECT
    
    // Core Clock and System Shell Properties
    Q_PROPERTY(QString currentTime READ currentTime NOTIFY timeChanged)
    Q_PROPERTY(QString currentDate READ currentDate NOTIFY dateChanged)
    Q_PROPERTY(QString activeAppTitle READ activeAppTitle NOTIFY activeAppTitleChanged)
    Q_PROPERTY(double batteryPercent READ batteryPercent NOTIFY batteryChanged)
    Q_PROPERTY(bool isCharging READ isCharging NOTIFY chargingChanged)
    Q_PROPERTY(QString networkStatus READ networkStatus NOTIFY networkChanged)
    Q_PROPERTY(bool isNetworkConnected READ isNetworkConnected NOTIFY networkChanged)

    // Configuration & Theme Properties
    Q_PROPERTY(bool isDark READ isDark NOTIFY isDarkChanged)
    Q_PROPERTY(bool useSystemTheme READ useSystemTheme WRITE setUseSystemTheme NOTIFY useSystemThemeChanged)
    Q_PROPERTY(bool darkOverride READ darkOverride WRITE setDarkOverride NOTIFY darkOverrideChanged)
    Q_PROPERTY(QColor customAccent READ customAccent WRITE setCustomAccent NOTIFY customAccentChanged)

    // New: Media Integration Properties (Direct bindings to MediaIndicator.qml)
    Q_PROPERTY(bool isMediaActive READ isMediaActive NOTIFY mediaActiveChanged)
    Q_PROPERTY(bool isMediaPlaying READ isMediaPlaying NOTIFY mediaPlayingChanged)
    Q_PROPERTY(QString mediaTitle READ mediaTitle NOTIFY mediaTitleChanged)
    Q_PROPERTY(double mediaProgress READ mediaProgress NOTIFY mediaProgressChanged)

public:
    explicit SystemState(QObject *parent = nullptr);
    ~SystemState();

    // Getters for Core Shell Info
    QString currentTime() const { return m_currentTime; }
    QString currentDate() const { return m_currentDate; }
    QString activeAppTitle() const { return m_activeAppTitle; }
    double batteryPercent() const { return m_batteryPercent; }
    bool isCharging() const { return m_isCharging; }
    QString networkStatus() const { return m_networkStatus; }
    bool isNetworkConnected() const { return m_isNetworkConnected; }

    // Theme Getters & Setters
    bool isDark() const { return m_isDark; }
    bool useSystemTheme() const { return m_useSystemTheme; }
    bool darkOverride() const { return m_darkOverride; }
    QColor customAccent() const { return m_customAccent; }

    void setUseSystemTheme(bool value);
    void setDarkOverride(bool value);
    void setCustomAccent(const QColor &color);

    // New: Media Control Getters
    bool isMediaActive() const { return m_isMediaActive; }
    bool isMediaPlaying() const { return m_isMediaPlaying; }
    QString mediaTitle() const { return m_mediaTitle; }
    double mediaProgress() const { return m_mediaProgress; }

    // New: Invokable Slots for QML Button Events
    Q_INVOKABLE void mediaPrevious();
    Q_INVOKABLE void toggleMediaPlayPause();
    Q_INVOKABLE void mediaNext();

signals:
    // Core Signals
    void timeChanged();
    void dateChanged();
    void activeAppTitleChanged();
    void batteryChanged();
    void chargingChanged();
    void networkChanged();

    // Theme Signals
    void isDarkChanged();
    void useSystemThemeChanged();
    void darkOverrideChanged();
    void customAccentChanged();

    // New: Media Signals
    void mediaActiveChanged();
    void mediaPlayingChanged();
    void mediaTitleChanged();
    void mediaProgressChanged();

private slots:
    void updateTime();
    void updateActiveAppTitle();
    void updateBattery();
    void updateNetwork();
    void updateEffectiveTheme();
    
    // New: Media Processing Loop Slot
    void updateMediaState();

private:
    std::unique_ptr<DBusWrapper> m_dbus;
    QTimer *m_updateTimer;
    QTimer *m_pollTimer;
    QSettings *m_settings;

    // Core Properties
    QString m_currentTime;
    QString m_currentDate;
    QString m_activeAppTitle;
    double m_batteryPercent = 100.0;
    bool m_isCharging = false;
    QString m_networkStatus = QStringLiteral("Disconnected");
    bool m_isNetworkConnected = false;

    // Theme Properties
    bool m_systemIsDark = true;
    bool m_isDark = true;
    bool m_useSystemTheme = true;
    bool m_darkOverride = true;
    QColor m_customAccent;

    // New: Media Properties
    bool m_isMediaActive = false;
    bool m_isMediaPlaying = false;
    QString m_mediaTitle;
    double m_mediaProgress = 0.0;
    QString m_activePlayerService;
};

#endif // SYSTEMSTATE_H