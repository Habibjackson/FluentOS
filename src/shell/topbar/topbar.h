// src/shell/topbar/topbar.h
#pragma once
#include <QObject>
#include <QDateTime>
#include <QTimer>
#include <QDBusConnection>
#include <QtQml/qqmlregistration.h>

class TopBarBackend : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString currentTime READ currentTime NOTIFY timeChanged)
    Q_PROPERTY(QString currentDate READ currentDate NOTIFY dateChanged)
    Q_PROPERTY(double batteryPercent READ batteryPercent NOTIFY batteryChanged)
    Q_PROPERTY(bool isCharging READ isCharging NOTIFY chargingChanged)
    Q_PROPERTY(QString networkStatus READ networkStatus NOTIFY networkChanged)

public:
    explicit TopBarBackend(QObject *parent = nullptr);

    QString currentTime() const { return m_currentTime; }
    QString currentDate() const { return m_currentDate; }
    double batteryPercent() const { return m_batteryPercent; }
    bool isCharging() const { return m_isCharging; }
    QString networkStatus() const { return m_networkStatus; }

signals:
    void timeChanged();
    void dateChanged();
    void batteryChanged();
    void chargingChanged();
    void networkChanged();

private slots:
    void updateTime();
    void updateBattery();
    void updateNetwork();

private:
    void readBatteryFromUPower();
    void readNetworkFromNetworkManager();

    QTimer *m_updateTimer = nullptr;
    QString m_currentTime;
    QString m_currentDate;
    double m_batteryPercent = 100.0;
    bool m_isCharging = false;
    QString m_networkStatus;
};