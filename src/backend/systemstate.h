// src/backend/systemstate.h
#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QDateTime>
#include <QtQml/qqmlregistration.h>
#include <memory>

class DBusWrapper;

class SystemState : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString currentTime READ currentTime NOTIFY timeChanged)
    Q_PROPERTY(QString currentDate READ currentDate NOTIFY dateChanged)
    Q_PROPERTY(QString activeAppTitle READ activeAppTitle NOTIFY activeAppTitleChanged)
    Q_PROPERTY(double batteryPercent READ batteryPercent NOTIFY batteryChanged)
    Q_PROPERTY(bool isCharging READ isCharging NOTIFY chargingChanged)
    Q_PROPERTY(QString networkStatus READ networkStatus NOTIFY networkChanged)
    Q_PROPERTY(bool isNetworkConnected READ isNetworkConnected NOTIFY networkChanged)

public:
    explicit SystemState(QObject *parent = nullptr);
    ~SystemState();

    QString currentTime() const { return m_currentTime; }
    QString currentDate() const { return m_currentDate; }
    QString activeAppTitle() const { return m_activeAppTitle; }
    double batteryPercent() const { return m_batteryPercent; }
    bool isCharging() const { return m_isCharging; }
    QString networkStatus() const { return m_networkStatus; }
    bool isNetworkConnected() const { return m_isNetworkConnected; }

signals:
    void timeChanged();
    void dateChanged();
    void activeAppTitleChanged();
    void batteryChanged();
    void chargingChanged();
    void networkChanged();

private slots:
    void updateTime();
    void updateActiveAppTitle();
    void updateBattery();
    void updateNetwork();

private:
    std::unique_ptr<DBusWrapper> m_dbus;
    QTimer *m_updateTimer = nullptr;
    QTimer *m_pollTimer = nullptr;

    QString m_currentTime;
    QString m_currentDate;
    QString m_activeAppTitle;
    double m_batteryPercent = 100.0;
    bool m_isCharging = false;
    QString m_networkStatus = "Disconnected";
    bool m_isNetworkConnected = false;
};
