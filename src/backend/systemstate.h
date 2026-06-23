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
    
    // Existing Shell Properties
    Q_PROPERTY(QString currentTime READ currentTime NOTIFY timeChanged)
    Q_PROPERTY(QString currentDate READ currentDate NOTIFY dateChanged)
    Q_PROPERTY(QString activeAppTitle READ activeAppTitle NOTIFY activeAppTitleChanged)
    Q_PROPERTY(double batteryPercent READ batteryPercent NOTIFY batteryChanged)
    Q_PROPERTY(bool isCharging READ isCharging NOTIFY chargingChanged)
    Q_PROPERTY(QString networkStatus READ networkStatus NOTIFY networkChanged)
    Q_PROPERTY(bool isNetworkConnected READ isNetworkConnected NOTIFY networkChanged)

    // New Configuration & Theme Properties
    Q_PROPERTY(bool isDark READ isDark NOTIFY isDarkChanged)
    Q_PROPERTY(bool useSystemTheme READ useSystemTheme WRITE setUseSystemTheme NOTIFY useSystemThemeChanged)
    Q_PROPERTY(bool darkOverride READ darkOverride WRITE setDarkOverride NOTIFY darkOverrideChanged)
    Q_PROPERTY(QColor customAccent READ customAccent WRITE setCustomAccent NOTIFY customAccentChanged)

public:
    explicit SystemState(QObject *parent = nullptr);
    ~SystemState();

    // Existing Getters
    QString currentTime() const { return m_currentTime; }
    QString currentDate() const { return m_currentDate; }
    QString activeAppTitle() const { return m_activeAppTitle; }
    double batteryPercent() const { return m_batteryPercent; }
    bool isCharging() const { return m_isCharging; }
    QString networkStatus() const { return m_networkStatus; }
    bool isNetworkConnected() const { return m_isNetworkConnected; }

    // New Getters & Setters
    bool isDark() const { return m_isDark; }
    bool useSystemTheme() const { return m_useSystemTheme; }
    bool darkOverride() const { return m_darkOverride; }
    QColor customAccent() const { return m_customAccent; }

    void setUseSystemTheme(bool value);
    void setDarkOverride(bool value);
    void setCustomAccent(const QColor &color);

signals:
    // Existing Signals
    void timeChanged();
    void dateChanged();
    void activeAppTitleChanged();
    void batteryChanged();
    void chargingChanged();
    void networkChanged();

    // New Theme Signals
    void isDarkChanged();
    void useSystemThemeChanged();
    void darkOverrideChanged();
    void customAccentChanged();

private slots:
    void updateTime();
    void updateActiveAppTitle();
    void updateBattery();
    void updateNetwork();
    
    // New: Dynamic calculator determining final runtime appearance
    void updateEffectiveTheme();

private:
    std::unique_ptr<DBusWrapper> m_dbus;
    QTimer *m_updateTimer;
    QTimer *m_pollTimer;
    QSettings *m_settings; // Configuration compiler file

    // Existing storage states
    QString m_currentTime;
    QString m_currentDate;
    QString m_activeAppTitle;
    double m_batteryPercent = 100.0;
    bool m_isCharging = false;
    QString m_networkStatus = QStringLiteral("Disconnected");
    bool m_isNetworkConnected = false;

    // New persistent storage properties
    bool m_systemIsDark = true;
    bool m_isDark = true;
    bool m_useSystemTheme = true;
    bool m_darkOverride = true;
    QColor m_customAccent;
};

#endif // SYSTEMSTATE_H