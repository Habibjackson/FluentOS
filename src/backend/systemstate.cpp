// src/backend/systemstate.cpp
#include "systemstate.h"
#include "dbus/dbuswrapper.h"
#include <QGuiApplication>
#include <QStandardPaths>
#include <QDebug>
#include <QWindow>

SystemState::SystemState(QObject *parent)
    : QObject(parent)
    , m_dbus(std::make_unique<DBusWrapper>())
    , m_updateTimer(new QTimer(this))
    , m_pollTimer(new QTimer(this))
{
    // 1. Setup Persistent Storage (~/.config/FluentShell/config.ini)
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/FluentShell/config.ini";
    m_settings = new QSettings(configPath, QSettings::IniFormat, this);

    m_useSystemTheme = m_settings->value("Appearance/UseSystemTheme", true).toBool();
    m_darkOverride = m_settings->value("Appearance/DarkOverride", true).toBool();
    m_customAccent = m_settings->value("Appearance/CustomAccent", QColor("#0A84FF")).value<QColor>();

    // 2. Query initial portal system configuration
    m_systemIsDark = m_dbus->getSystemColorScheme();

    // 3. Connect D-Bus theme changes to runtime processor
    connect(m_dbus.get(), &DBusWrapper::systemColorSchemeChanged, this, [this](bool systemDark) {
        if (m_systemIsDark != systemDark) {
            m_systemIsDark = systemDark;
            updateEffectiveTheme();
        }
    });

    // 4. Existing Timer and Signal mappings
    connect(m_updateTimer, &QTimer::timeout, this, &SystemState::updateTime);
    m_updateTimer->start(1000);

    connect(qApp, &QGuiApplication::focusWindowChanged, this, [this](QWindow *) {
        updateActiveAppTitle();
    });

    connect(m_pollTimer, &QTimer::timeout, this, [this]() {
        updateBattery();
        updateNetwork();
    });
    m_pollTimer->start(5000);

    // Run evaluations
    updateTime();
    updateActiveAppTitle();
    updateBattery();
    updateNetwork();
    updateEffectiveTheme(); // Initial theme setup
}

SystemState::~SystemState() = default;

void SystemState::updateEffectiveTheme()
{
    // Rule implementation: fallback to user configuration file if system mirroring is disabled
    bool targetDark = m_useSystemTheme ? m_systemIsDark : m_darkOverride;
    if (m_isDark != targetDark) {
        m_isDark = targetDark;
        emit isDarkChanged();
    }
}

void SystemState::setUseSystemTheme(bool value)
{
    if (m_useSystemTheme != value) {
        m_useSystemTheme = value;
        m_settings->setValue("Appearance/UseSystemTheme", value);
        emit useSystemThemeChanged();
        updateEffectiveTheme();
    }
}

void SystemState::setDarkOverride(bool value)
{
    if (m_darkOverride != value) {
        m_darkOverride = value;
        m_settings->setValue("Appearance/DarkOverride", value);
        emit darkOverrideChanged();
        updateEffectiveTheme();
    }
}

void SystemState::setCustomAccent(const QColor &color)
{
    if (m_customAccent != color) {
        m_customAccent = color;
        m_settings->setValue("Appearance/CustomAccent", color);
        emit customAccentChanged();
    }
}

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
