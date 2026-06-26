#include "systemstate.h"
#include "dbus/dbuswrapper.h"
#include <QGuiApplication>
#include <QStandardPaths>
#include <QDebug>
#include <QWindow>
#include <QDir>
#include <QTimer>
#include <cmath>

SystemState::SystemState(QObject *parent)
    : QObject(parent)
    , m_dbus(std::make_unique<DBusWrapper>())
    , m_updateTimer(new QTimer(this))
    , m_pollTimer(new QTimer(this))
{
    // 1. Setup Persistent Storage (~/.config/FluentShell/config.ini)
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/FluentShell/config.ini";
    m_settings = new QSettings(configPath, QSettings::IniFormat, this);

    qDebug() << "DEBUG: Looking for config at:" << configPath;
    // If the file doesn't exist yet, write the defaults immediately
    if (!QFile::exists(configPath)) {
        m_settings->setValue("Appearance/UseSystemTheme", true);
        m_settings->setValue("Appearance/DarkOverride", true);
        m_settings->setValue("Appearance/CustomAccent", QColor("#0A84FF"));
        m_settings->sync(); // Force write to disk
    }

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

    // 4. CRITICAL FIX: Connect MPRIS property updates to trigger instant media state changes
    connect(m_dbus.get(), &DBusWrapper::mediaPropertiesChanged, this, &SystemState::updateMediaState);

    // 5. Existing Timer and Signal mappings
    connect(m_updateTimer, &QTimer::timeout, this, &SystemState::updateTime);
    m_updateTimer->start(1000);

    connect(qApp, &QGuiApplication::focusWindowChanged, this, [this](QWindow *) {
        updateActiveAppTitle();
    });

    // 6. CRITICAL FIX: Update media state periodically alongside hardware sweeps
    connect(m_pollTimer, &QTimer::timeout, this, [this]() {
        updateBattery();
        updateNetwork();
        updateMediaState(); // Sweeps for newly launched media players every 5 seconds
    });
    m_pollTimer->start(5000);

    // Run evaluations
    updateTime();
    updateActiveAppTitle();
    updateBattery();
    updateNetwork();
    updateEffectiveTheme(); // Initial theme setup
    updateMediaState();     // Initial media scan
}

SystemState::~SystemState() = default;

void SystemState::updateEffectiveTheme() {
    bool newIsDark = m_isDark;

    if (m_useSystemTheme) {
        newIsDark = m_systemIsDark;
    } else {
        newIsDark = m_darkOverride;
    }

    if (m_isDark != newIsDark) {
        m_isDark = newIsDark;
        emit isDarkChanged();
    }
}

void SystemState::setUseSystemTheme(bool value)
{
    if (m_useSystemTheme == value) {
        return;
    }

    m_useSystemTheme = value;
    m_settings->setValue(QStringLiteral("Appearance/UseSystemTheme"), value);
    m_settings->sync();

    emit useSystemThemeChanged();
    updateEffectiveTheme();
}

void SystemState::setDarkOverride(bool value)
{
    if (m_darkOverride == value) {
        return;
    }

    m_darkOverride = value;
    m_settings->setValue(QStringLiteral("Appearance/DarkOverride"), value);
    m_settings->sync();

    emit darkOverrideChanged();
    updateEffectiveTheme();
}

void SystemState::setCustomAccent(const QColor &color)
{
    if (m_customAccent == color) {
        return;
    }

    m_customAccent = color;
    m_settings->setValue(QStringLiteral("Appearance/CustomAccent"), color);
    m_settings->sync();

    emit customAccentChanged();
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

    // Avoid tiny floating-point changes causing repeated QML updates.
    if (std::abs(percent - m_batteryPercent) >= 0.1) {
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

// MEDIA PLAYER CONTROLS (MPRIS INTERFACE ACTIONS)
// =========================================================
void SystemState::updateMediaState()
{
    m_dbus->debugDumpMprisPlayers();
    const QString activePlayer = m_dbus->findActiveMediaPlayer();
    const bool active = !activePlayer.isEmpty();

    if (m_activePlayerService != activePlayer) {
        m_activePlayerService = activePlayer;
    }

    if (active != m_isMediaActive) {
        m_isMediaActive = active;
        emit mediaActiveChanged();
    }

    if (!active) {
        if (m_isMediaPlaying) {
            m_isMediaPlaying = false;
            emit mediaPlayingChanged();
        }

        if (!m_mediaTitle.isEmpty()) {
            m_mediaTitle.clear();
            emit mediaTitleChanged();
        }

        if (m_mediaProgress != 0.0) {
            m_mediaProgress = 0.0;
            emit mediaProgressChanged();
        }

        return;
    }

    const bool playing = m_dbus->getMediaPlaybackStatus(activePlayer);
    const QString title = m_dbus->getMediaTitle(activePlayer);
    const double progress = m_dbus->getMediaProgress(activePlayer);

    if (playing != m_isMediaPlaying) {
        m_isMediaPlaying = playing;
        emit mediaPlayingChanged();
    }

    if (title != m_mediaTitle) {
        m_mediaTitle = title;
        emit mediaTitleChanged();
    }

    if (std::abs(progress - m_mediaProgress) > 0.001) {
        m_mediaProgress = progress;
        emit mediaProgressChanged();
    }
}

void SystemState::mediaPrevious()
{
    if (m_activePlayerService.isEmpty()) {
        updateMediaState();
    }

    m_dbus->triggerMediaAction(m_activePlayerService, QStringLiteral("Previous"));
    QTimer::singleShot(150, this, &SystemState::updateMediaState);
}

void SystemState::toggleMediaPlayPause()
{
    if (m_activePlayerService.isEmpty()) {
        updateMediaState();
    }

    m_dbus->triggerMediaAction(m_activePlayerService, QStringLiteral("PlayPause"));
    QTimer::singleShot(150, this, &SystemState::updateMediaState);
}

void SystemState::mediaNext()
{
    if (m_activePlayerService.isEmpty()) {
        updateMediaState();
    }

    m_dbus->triggerMediaAction(m_activePlayerService, QStringLiteral("Next"));
    QTimer::singleShot(150, this, &SystemState::updateMediaState);
}