// src/shell/topbar/topbar.cpp
#include "topbar.h"
#include <QDateTime>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDebug>

TopBarBackend::TopBarBackend(QObject *parent)
    : QObject(parent)
    , m_updateTimer(new QTimer(this))
{
    // Update time every second
    connect(m_updateTimer, &QTimer::timeout, this, &TopBarBackend::updateTime);
    m_updateTimer->start(1000);

    // Initial updates
    updateTime();
    updateBattery();
    updateNetwork();

    // Poll battery and network less frequently (every 5 seconds)
    QTimer *pollTimer = new QTimer(this);
    connect(pollTimer, &QTimer::timeout, this, [this]() {
        updateBattery();
        updateNetwork();
    });
    pollTimer->start(5000);
}

void TopBarBackend::updateTime() {
    QDateTime now = QDateTime::currentDateTime();
    QString newTime = now.toString("HH:mm");
    QString newDate = now.toString("ddd, MMM d");

    if (newTime != m_currentTime) {
        m_currentTime = newTime;
        emit timeChanged();
    }
    if (newDate != m_currentDate) {
        m_currentDate = newDate;
        emit dateChanged();
    }
}

void TopBarBackend::updateBattery() {
    // Query UPower D-Bus service
    readBatteryFromUPower();
}

void TopBarBackend::updateNetwork() {
    // Query NetworkManager D-Bus service
    readNetworkFromNetworkManager();
}

void TopBarBackend::readBatteryFromUPower() {
    // D-Bus call to UPower to get battery percentage, charging state
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.freedesktop.UPower",
        "/org/freedesktop/UPower/devices/battery_BAT0",
        "org.freedesktop.DBus.Properties",
        "Get"
    );
    msg << "org.freedesktop.UPower.Device" << "Percentage";

    // Async call
    QDBusPendingCall call = QDBusConnection::systemBus().asyncCall(msg);
    // (In production, connect to finished signal for async result)
}

void TopBarBackend::readNetworkFromNetworkManager() {
    // D-Bus call to NetworkManager for connection state
    m_networkStatus = "WiFi";  // Placeholder
    emit networkChanged();
}