// src/backend/systemstate.cpp
#include "systemstate.h"
#include "dbus/dbuswrapper.h"
#include <QGuiApplication>
#include <QDebug>
#include <QWindow>

SystemState::SystemState(QObject *parent)
    : QObject(parent)
    , m_dbus(std::make_unique<DBusWrapper>())
    , m_updateTimer(new QTimer(this))
    , m_pollTimer(new QTimer(this))
{
    // Update time every second
    connect(m_updateTimer, &QTimer::timeout, this, &SystemState::updateTime);
    m_updateTimer->start(1000);

    connect(qApp, &QGuiApplication::focusWindowChanged, this, [this](QWindow *) {
        updateActiveAppTitle();
    });

    // Poll battery/network every 5 seconds
    connect(m_pollTimer, &QTimer::timeout, this, [this]() {
        updateBattery();
        updateNetwork();
    });
    m_pollTimer->start(5000);

    // Initial updates
    updateTime();
    updateActiveAppTitle();
    updateBattery();
    updateNetwork();
}

SystemState::~SystemState() = default;

void SystemState::updateTime()
{
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

    updateActiveAppTitle();
}

void SystemState::updateActiveAppTitle()
{
    QString newTitle = QGuiApplication::applicationDisplayName();

    if (QWindow *focusWindow = QGuiApplication::focusWindow()) {
        const QString focusTitle = focusWindow->title().trimmed();
        if (!focusTitle.isEmpty()) {
            newTitle = focusTitle;
        }
    }

    if (newTitle.isEmpty()) {
        newTitle = QStringLiteral("Fluent Shell");
    }

    if (newTitle != m_activeAppTitle) {
        m_activeAppTitle = newTitle;
        emit activeAppTitleChanged();
    }
}

void SystemState::updateBattery()
{
    auto [percent, charging] = m_dbus->getBatteryStatus();

    if (percent != m_batteryPercent) {
        m_batteryPercent = percent;
        emit batteryChanged();
    }
    if (charging != m_isCharging) {
        m_isCharging = charging;
        emit chargingChanged();
    }
}

void SystemState::updateNetwork()
{
    auto [status, connected] = m_dbus->getNetworkStatus();

    if (status != m_networkStatus || connected != m_isNetworkConnected) {
        m_networkStatus = status;
        m_isNetworkConnected = connected;
        emit networkChanged();
    }
}
