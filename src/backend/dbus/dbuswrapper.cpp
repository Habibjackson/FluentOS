#include "dbuswrapper.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDBusArgument>
#include <QUrl>

#include <QDebug>
#include <QSet>
#include <QTimer>
#include <QtGlobal>

namespace
{
constexpr int DBUS_TIMEOUT_MS = 1000;

constexpr const char *MPRIS_SERVICE_PREFIX = "org.mpris.MediaPlayer2.";
constexpr const char *MPRIS_PATH = "/org/mpris/MediaPlayer2";
constexpr const char *MPRIS_PLAYER_INTERFACE = "org.mpris.MediaPlayer2.Player";
constexpr const char *DBUS_PROPERTIES_INTERFACE = "org.freedesktop.DBus.Properties";

struct MprisSnapshot
{
    QString service;
    bool valid = false;
    bool playing = false;
    bool paused = false;
    bool stopped = true;
    bool hasUsableMedia = false;
    QString title;
    qlonglong length = 0;
    qlonglong position = 0;
    double progress = 0.0;
};

bool isMprisService(const QString &service)
{
    return service.startsWith(QLatin1String(MPRIS_SERVICE_PREFIX));
}

// CRITICAL FIX: Aggressive recursive unwrap for deep D-Bus layers
QVariant unwrapDBusVariant(const QVariant &value)
{
    QVariant current = value;
    bool unwrapped = true;

    while (unwrapped) {
        unwrapped = false;
        
        if (current.userType() == qMetaTypeId<QDBusVariant>()) {
            current = qvariant_cast<QDBusVariant>(current).variant();
            unwrapped = true;
        } 
        else if (current.userType() == qMetaTypeId<QDBusArgument>()) {
            const QDBusArgument arg = current.value<QDBusArgument>();
            if (arg.currentType() == QDBusArgument::VariantType) {
                QVariant inner;
                arg >> inner;
                current = inner;
                unwrapped = true;
            }
        }
    }
    return current;
}

// CRITICAL FIX: Restore the overload to handle direct QDBusVariant arguments
QVariant unwrapDBusVariant(const QDBusVariant &value)
{
    return unwrapDBusVariant(value.variant());
}

// CRITICAL FIX: Manual QDBusArgument Map Iterator (Bypasses rigid Qt Cast failures)
QVariantMap variantToMap(const QVariant &value)
{
    const QVariant unwrapped = unwrapDBusVariant(value);

    if (unwrapped.userType() == qMetaTypeId<QVariantMap>()) {
        return unwrapped.toMap();
    }

    if (unwrapped.userType() == qMetaTypeId<QDBusArgument>()) {
        const QDBusArgument arg = unwrapped.value<QDBusArgument>();
        if (arg.currentType() == QDBusArgument::MapType) {
            QVariantMap map;
            arg.beginMap();
            while (!arg.atEnd()) {
                QString key;
                QVariant val;
                arg.beginMapEntry();
                arg >> key >> val;
                arg.endMapEntry();
                // Recursively unwrap the deeply nested dictionary value
                map.insert(key, unwrapDBusVariant(val));
            }
            arg.endMap();
            return map;
        }
    }

    return QVariantMap(); // Return empty if parsing failed entirely
}

// CRITICAL FIX: Safe demarshalling for String Arrays (Solves missing Artist names)
QStringList extractStringList(const QVariant &value)
{
    QVariant unwrapped = unwrapDBusVariant(value);
    
    if (unwrapped.userType() == qMetaTypeId<QStringList>()) {
        return unwrapped.toStringList();
    }
    
    if (unwrapped.userType() == qMetaTypeId<QDBusArgument>()) {
        const QDBusArgument arg = unwrapped.value<QDBusArgument>();
        if (arg.currentType() == QDBusArgument::ArrayType) {
            QStringList list;
            arg.beginArray();
            while (!arg.atEnd()) {
                QString item;
                arg >> item;
                list << item;
            }
            arg.endArray();
            return list;
        }
    }
    
    return QStringList();
}

bool isNoTrackId(const QVariant &trackIdValue)
{
    const QVariant value = unwrapDBusVariant(trackIdValue);
    if (value.canConvert<QDBusObjectPath>()) {
        const QDBusObjectPath path = qvariant_cast<QDBusObjectPath>(value);
        return path.path().contains(QStringLiteral("NoTrack"), Qt::CaseInsensitive);
    }
    return value.toString().contains(QStringLiteral("NoTrack"), Qt::CaseInsensitive);
}

QString metadataTitle(const QVariantMap &metadata)
{
    QString title = unwrapDBusVariant(metadata.value(QStringLiteral("xesam:title"))).toString().trimmed();
    if (!title.isEmpty()) {
        return title;
    }

    const QStringList artists = extractStringList(metadata.value(QStringLiteral("xesam:artist")));
    if (!artists.isEmpty()) {
        return artists.join(QStringLiteral(", "));
    }

    title = unwrapDBusVariant(metadata.value(QStringLiteral("xesam:url"))).toString().trimmed();
    if (!title.isEmpty()) {
        if (title.startsWith(QStringLiteral("file://"))) {
            return QUrl(title).toLocalFile();
        }
        return title;
    }

    return QString();
}

MprisSnapshot readMprisSnapshot(const QString &serviceName)
{
    MprisSnapshot snapshot;
    snapshot.service = serviceName;

    if (serviceName.isEmpty()) {
        return snapshot;
    }

    QVariantMap props;

    QDBusMessage getAllMsg = QDBusMessage::createMethodCall(
        serviceName,
        QLatin1String(MPRIS_PATH),
        QLatin1String(DBUS_PROPERTIES_INTERFACE),
        QStringLiteral("GetAll")
    );
    getAllMsg << QLatin1String(MPRIS_PLAYER_INTERFACE);

    QDBusMessage allReplyMsg = QDBusConnection::sessionBus().call(getAllMsg, QDBus::Block, DBUS_TIMEOUT_MS);

    if (allReplyMsg.type() == QDBusMessage::ReplyMessage && !allReplyMsg.arguments().isEmpty()) {
        props = variantToMap(allReplyMsg.arguments().first());
    }

    if (props.isEmpty()) {
        auto fetchProperty = [&](const QString &propName) -> QVariant {
            QDBusMessage getMsg = QDBusMessage::createMethodCall(
                serviceName,
                QLatin1String(MPRIS_PATH),
                QLatin1String(DBUS_PROPERTIES_INTERFACE),
                QStringLiteral("Get")
            );
            getMsg << QLatin1String(MPRIS_PLAYER_INTERFACE) << propName;
            
            QDBusMessage replyMsg = QDBusConnection::sessionBus().call(getMsg, QDBus::Block, DBUS_TIMEOUT_MS);
            if (replyMsg.type() == QDBusMessage::ReplyMessage && !replyMsg.arguments().isEmpty()) {
                return unwrapDBusVariant(replyMsg.arguments().first());
            }
            return QVariant();
        };

        QVariant statusReply = fetchProperty(QStringLiteral("PlaybackStatus"));
        if (statusReply.isValid() && !statusReply.isNull()) props[QStringLiteral("PlaybackStatus")] = statusReply;

        QVariant metadataReply = fetchProperty(QStringLiteral("Metadata"));
        if (metadataReply.isValid() && !metadataReply.isNull()) props[QStringLiteral("Metadata")] = metadataReply;

        QVariant positionReply = fetchProperty(QStringLiteral("Position"));
        if (positionReply.isValid() && !positionReply.isNull()) props[QStringLiteral("Position")] = positionReply;
    }

    if (props.isEmpty()) {
        return snapshot;
    }

    const QString playbackStatus = unwrapDBusVariant(props.value(QStringLiteral("PlaybackStatus"))).toString();

    snapshot.playing = playbackStatus == QStringLiteral("Playing");
    snapshot.paused = playbackStatus == QStringLiteral("Paused");
    snapshot.stopped = playbackStatus == QStringLiteral("Stopped");

    const QVariantMap metadata = variantToMap(props.value(QStringLiteral("Metadata")));

    snapshot.title = metadataTitle(metadata);
    snapshot.length = unwrapDBusVariant(metadata.value(QStringLiteral("mpris:length"))).toLongLong();
    snapshot.position = unwrapDBusVariant(props.value(QStringLiteral("Position"))).toLongLong();

    if (snapshot.length > 0) {
        snapshot.progress = qBound(
            0.0,
            static_cast<double>(snapshot.position) / static_cast<double>(snapshot.length),
            1.0
        );
    }

    const bool hasNoTrack = metadata.contains(QStringLiteral("mpris:trackid"))
        && isNoTrackId(metadata.value(QStringLiteral("mpris:trackid")));

    const bool hasTitle = !snapshot.title.isEmpty();
    const bool hasLength = snapshot.length > 0;
    const bool usablePlaybackState = snapshot.playing || snapshot.paused;

    snapshot.hasUsableMedia = usablePlaybackState
        && !hasNoTrack
        && (hasTitle || hasLength || snapshot.playing);

    snapshot.valid = true;

    return snapshot;
}
}

DBusWrapper::DBusWrapper()
    : m_systemBusAvailable(false)
    , m_mprisListenerSetup(false)
    , m_mediaProgressTimer(nullptr)
{
    m_systemBusAvailable = QDBusConnection::systemBus().isConnected();

    if (!m_systemBusAvailable) {
        qWarning() << "D-Bus system bus not available, using fallback values";
    }

    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    if (sessionBus.isConnected()) {
        const bool portalConnected = sessionBus.connect(
            QStringLiteral("org.freedesktop.portal.Desktop"),
            QStringLiteral("/org/freedesktop/portal/desktop"),
            QStringLiteral("org.freedesktop.portal.Settings"),
            QStringLiteral("SettingChanged"),
            this,
            SLOT(onPortalSettingChanged(QString,QString,QDBusVariant))
        );

        if (!portalConnected) {
            qWarning() << "Failed to register portal settings listener";
        }

        setupMprisListener();
    }
}

DBusWrapper::~DBusWrapper() = default;

bool DBusWrapper::getSystemColorScheme()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        return true;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.portal.Desktop"),
        QStringLiteral("/org/freedesktop/portal/desktop"),
        QStringLiteral("org.freedesktop.portal.Settings"),
        QStringLiteral("Read")
    );

    message << QStringLiteral("org.freedesktop.appearance")
            << QStringLiteral("color-scheme");

    const QDBusMessage reply = QDBusConnection::sessionBus().call(message, QDBus::Block, DBUS_TIMEOUT_MS);

    if (reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()) {
        const QVariant rawValue = reply.arguments().first();
        const QVariant colorValue = unwrapDBusVariant(rawValue);

        const uint colorScheme = colorValue.toUInt();

        if (colorScheme == 1) {
            return true;
        }
        if (colorScheme == 2) {
            return false;
        }
    }

    return true;
}

void DBusWrapper::onPortalSettingChanged(const QString &namespaceName,
                                         const QString &key,
                                         const QDBusVariant &value)
{
    if (namespaceName != QStringLiteral("org.freedesktop.appearance")
        || key != QStringLiteral("color-scheme")) {
        return;
    }

    const uint colorScheme = unwrapDBusVariant(value).toUInt();

    if (colorScheme == 1) {
        emit systemColorSchemeChanged(true);
    } else if (colorScheme == 2) {
        emit systemColorSchemeChanged(false);
    }
}

std::tuple<double, bool> DBusWrapper::getBatteryStatus()
{
    double percent = 100.0;
    bool charging = false;

    if (!m_systemBusAvailable) {
        return {percent, charging};
    }

    QDBusInterface upower(
        QStringLiteral("org.freedesktop.UPower"),
        QStringLiteral("/org/freedesktop/UPower"),
        QStringLiteral("org.freedesktop.UPower"),
        QDBusConnection::systemBus()
    );

    upower.setTimeout(DBUS_TIMEOUT_MS);

    if (!upower.isValid()) {
        qDebug() << "UPower D-Bus service not available";
        return {percent, charging};
    }

    const QDBusReply<QList<QDBusObjectPath>> devicesReply = upower.call(QStringLiteral("EnumerateDevices"));

    if (!devicesReply.isValid()) {
        return {percent, charging};
    }

    for (const QDBusObjectPath &device : devicesReply.value()) {
        const QString devicePath = device.path();

        if (!devicePath.contains(QStringLiteral("battery"), Qt::CaseInsensitive)) {
            continue;
        }

        QDBusInterface deviceIface(
            QStringLiteral("org.freedesktop.UPower"),
            devicePath,
            QStringLiteral("org.freedesktop.DBus.Properties"),
            QDBusConnection::systemBus()
        );

        deviceIface.setTimeout(DBUS_TIMEOUT_MS);

        const QDBusReply<QDBusVariant> percentReply = deviceIface.call(
            QStringLiteral("Get"),
            QStringLiteral("org.freedesktop.UPower.Device"),
            QStringLiteral("Percentage")
        );

        if (percentReply.isValid()) {
            const QVariant value = unwrapDBusVariant(percentReply.value());
            if (value.canConvert<double>()) {
                percent = value.toDouble();
            }
        }

        const QDBusReply<QDBusVariant> stateReply = deviceIface.call(
            QStringLiteral("Get"),
            QStringLiteral("org.freedesktop.UPower.Device"),
            QStringLiteral("State")
        );

        if (stateReply.isValid()) {
            const QVariant value = unwrapDBusVariant(stateReply.value());
            if (value.canConvert<uint>()) {
                const uint state = value.toUInt();
                charging = (state == 1 || state == 4 || state == 5);
            }
        }
        break;
    }

    return {percent, charging};
}

std::tuple<QString, bool> DBusWrapper::getNetworkStatus()
{
    QString status = QStringLiteral("Disconnected");
    bool connected = false;

    if (!m_systemBusAvailable) {
        return {status, connected};
    }

    QDBusInterface nm(
        QStringLiteral("org.freedesktop.NetworkManager"),
        QStringLiteral("/org/freedesktop/NetworkManager"),
        QStringLiteral("org.freedesktop.DBus.Properties"),
        QDBusConnection::systemBus()
    );

    nm.setTimeout(DBUS_TIMEOUT_MS);

    if (!nm.isValid()) {
        qDebug() << "NetworkManager D-Bus service not available";
        return {status, connected};
    }

    const QDBusReply<QDBusVariant> connectivityReply = nm.call(
        QStringLiteral("Get"),
        QStringLiteral("org.freedesktop.NetworkManager"),
        QStringLiteral("Connectivity")
    );

    if (!connectivityReply.isValid()) {
        return {status, connected};
    }

    const QVariant connectivityValue = unwrapDBusVariant(connectivityReply.value());

    if (!connectivityValue.canConvert<uint>()) {
        return {status, connected};
    }

    const uint connectivity = connectivityValue.toUInt();
    connected = (connectivity == 4);

    if (!connected) {
        return {status, connected};
    }

    status = QStringLiteral("Connected");

    const QDBusReply<QDBusVariant> primaryReply = nm.call(
        QStringLiteral("Get"),
        QStringLiteral("org.freedesktop.NetworkManager"),
        QStringLiteral("PrimaryConnection")
    );

    if (!primaryReply.isValid()) {
        return {status, connected};
    }

    const QVariant primaryValue = unwrapDBusVariant(primaryReply.value());

    if (!primaryValue.canConvert<QDBusObjectPath>()) {
        return {status, connected};
    }

    const QDBusObjectPath primaryPath = qvariant_cast<QDBusObjectPath>(primaryValue);

    if (primaryPath.path().isEmpty() || primaryPath.path() == QStringLiteral("/")) {
        return {status, connected};
    }

    QDBusInterface activeConnection(
        QStringLiteral("org.freedesktop.NetworkManager"),
        primaryPath.path(),
        QStringLiteral("org.freedesktop.DBus.Properties"),
        QDBusConnection::systemBus()
    );

    activeConnection.setTimeout(DBUS_TIMEOUT_MS);

    const QDBusReply<QDBusVariant> typeReply = activeConnection.call(
        QStringLiteral("Get"),
        QStringLiteral("org.freedesktop.NetworkManager.Connection.Active"),
        QStringLiteral("Type")
    );

    if (!typeReply.isValid()) {
        return {status, connected};
    }

    const QString type = unwrapDBusVariant(typeReply.value()).toString();

    if (type == QStringLiteral("802-11-wireless")) {
        status = QStringLiteral("WiFi");
    } else if (type == QStringLiteral("802-3-ethernet")) {
        status = QStringLiteral("Ethernet");
    }

    return {status, connected};
}

// =========================================================
// MPRIS MEDIA CONTROL METHODS
// =========================================================

void DBusWrapper::setupMprisListener()
{
    if (m_mprisListenerSetup) {
        return;
    }

    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    if (!sessionBus.isConnected()) {
        qWarning() << "D-Bus session bus not available, MPRIS disabled";
        return;
    }

    m_mprisListenerSetup = true;

    const bool propertiesConnected = sessionBus.connect(
        QString(),
        QLatin1String(MPRIS_PATH),
        QLatin1String(DBUS_PROPERTIES_INTERFACE),
        QStringLiteral("PropertiesChanged"),
        QStringList() << QLatin1String(MPRIS_PLAYER_INTERFACE),
        QStringLiteral("sa{sv}as"),
        this,
        SLOT(onMprisPropertiesChanged(QString,QVariantMap,QStringList))
    );

    if (!propertiesConnected) {
        qWarning() << "Failed to register MPRIS PropertiesChanged listener";
    }

    const bool seekedConnected = sessionBus.connect(
        QString(),
        QLatin1String(MPRIS_PATH),
        QLatin1String(MPRIS_PLAYER_INTERFACE),
        QStringLiteral("Seeked"),
        this,
        SLOT(onMprisSeeked(qlonglong))
    );

    if (!seekedConnected) {
        qWarning() << "Failed to register MPRIS Seeked listener";
    }

    const bool nameOwnerConnected = sessionBus.connect(
        QStringLiteral("org.freedesktop.DBus"),
        QStringLiteral("/org/freedesktop/DBus"),
        QStringLiteral("org.freedesktop.DBus"),
        QStringLiteral("NameOwnerChanged"),
        this,
        SLOT(onDBusNameOwnerChanged(QString,QString,QString))
    );

    if (!nameOwnerConnected) {
        qWarning() << "Failed to register DBus NameOwnerChanged listener";
    }

    if (!m_mediaProgressTimer) {
        m_mediaProgressTimer = new QTimer(this);
        m_mediaProgressTimer->setInterval(1000);

        connect(m_mediaProgressTimer, &QTimer::timeout, this, [this]() {
            const QString activePlayer = findActiveMediaPlayer();
            if (!activePlayer.isEmpty() && getMediaPlaybackStatus(activePlayer)) {
                emit mediaPropertiesChanged();
            } else {
                refreshMediaProgressTimer();
            }
        });
    }

    findActiveMediaPlayer();
    refreshMediaProgressTimer();

    emit mediaPropertiesChanged();
}

void DBusWrapper::onMprisPropertiesChanged(const QString &interfaceName,
                                           const QVariantMap &changedProperties,
                                           const QStringList &invalidatedProperties)
{
    Q_UNUSED(changedProperties);
    Q_UNUSED(invalidatedProperties);

    if (interfaceName != QLatin1String(MPRIS_PLAYER_INTERFACE)) {
        return;
    }

    findActiveMediaPlayer();
    refreshMediaProgressTimer();

    emit mediaPropertiesChanged();
}

void DBusWrapper::onMprisSeeked(qlonglong position)
{
    Q_UNUSED(position);

    findActiveMediaPlayer();
    refreshMediaProgressTimer();

    emit mediaPropertiesChanged();
}

void DBusWrapper::onDBusNameOwnerChanged(const QString &name,
                                         const QString &oldOwner,
                                         const QString &newOwner)
{
    if (!isMprisService(name)) {
        return;
    }

    const bool serviceRemoved = !oldOwner.isEmpty() && newOwner.isEmpty();

    if (serviceRemoved && m_currentTrackedPlayer == name) {
        m_currentTrackedPlayer.clear();
    }

    findActiveMediaPlayer();
    refreshMediaProgressTimer();

    emit mediaPropertiesChanged();
}

void DBusWrapper::refreshMediaProgressTimer()
{
    if (!m_mediaProgressTimer) {
        return;
    }

    const QString activePlayer = findActiveMediaPlayer();
    const bool shouldRun = !activePlayer.isEmpty() && getMediaPlaybackStatus(activePlayer);

    if (shouldRun && !m_mediaProgressTimer->isActive()) {
        m_mediaProgressTimer->start();
    } else if (!shouldRun && m_mediaProgressTimer->isActive()) {
        m_mediaProgressTimer->stop();
    }
}

QString DBusWrapper::findActiveMediaPlayer()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        m_currentTrackedPlayer.clear();
        return QString();
    }

    QDBusConnectionInterface *busInterface = QDBusConnection::sessionBus().interface();

    if (!busInterface) {
        m_currentTrackedPlayer.clear();
        return QString();
    }

    const QDBusReply<QStringList> servicesReply = busInterface->registeredServiceNames();

    if (!servicesReply.isValid()) {
        m_currentTrackedPlayer.clear();
        return QString();
    }

    QStringList mprisServices;

    for (const QString &service : servicesReply.value()) {
        if (isMprisService(service)) {
            mprisServices.append(service);
        }
    }

    mprisServices.sort();

    if (mprisServices.isEmpty()) {
        m_currentTrackedPlayer.clear();
        return QString();
    }

    QList<MprisSnapshot> snapshots;

    for (const QString &service : mprisServices) {
        const MprisSnapshot snapshot = readMprisSnapshot(service);
        if (snapshot.valid && snapshot.hasUsableMedia) {
            snapshots.append(snapshot);
        }
    }

    if (snapshots.isEmpty()) {
        m_currentTrackedPlayer.clear();
        return QString();
    }

    for (const MprisSnapshot &snapshot : snapshots) {
        if (snapshot.service == m_currentTrackedPlayer && snapshot.playing) {
            return snapshot.service;
        }
    }

    for (const MprisSnapshot &snapshot : snapshots) {
        if (snapshot.playing) {
            m_currentTrackedPlayer = snapshot.service;
            return snapshot.service;
        }
    }

    for (const MprisSnapshot &snapshot : snapshots) {
        if (snapshot.service == m_currentTrackedPlayer) {
            return snapshot.service;
        }
    }

    m_currentTrackedPlayer = snapshots.first().service;
    return m_currentTrackedPlayer;
}

bool DBusWrapper::getMediaPlaybackStatus(const QString &serviceName)
{
    const MprisSnapshot snapshot = readMprisSnapshot(serviceName);
    return snapshot.valid && snapshot.hasUsableMedia && snapshot.playing;
}

QString DBusWrapper::getMediaTitle(const QString &serviceName)
{
    const MprisSnapshot snapshot = readMprisSnapshot(serviceName);

    if (!snapshot.valid || !snapshot.hasUsableMedia) {
        return QString();
    }

    if (!snapshot.title.isEmpty()) {
        return snapshot.title;
    }

    if (snapshot.playing) {
        return QStringLiteral("Media playing");
    }

    return QString();
}

double DBusWrapper::getMediaProgress(const QString &serviceName)
{
    const MprisSnapshot snapshot = readMprisSnapshot(serviceName);

    if (!snapshot.valid || !snapshot.hasUsableMedia) {
        return 0.0;
    }

    return snapshot.progress;
}

void DBusWrapper::triggerMediaAction(const QString &serviceName, const QString &action)
{
    if (serviceName.isEmpty()) {
        return;
    }

    static const QSet<QString> allowedActions = {
        QStringLiteral("PlayPause"),
        QStringLiteral("Play"),
        QStringLiteral("Pause"),
        QStringLiteral("Stop"),
        QStringLiteral("Next"),
        QStringLiteral("Previous")
    };

    if (!allowedActions.contains(action)) {
        qWarning() << "Rejected invalid MPRIS action:" << action;
        return;
    }

    QDBusMessage actionMsg = QDBusMessage::createMethodCall(
        serviceName,
        QLatin1String(MPRIS_PATH),
        QLatin1String(MPRIS_PLAYER_INTERFACE),
        action
    );

    const QDBusMessage reply = QDBusConnection::sessionBus().call(actionMsg, QDBus::Block, DBUS_TIMEOUT_MS);

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "MPRIS action failed:"
                   << action
                   << serviceName
                   << reply.errorName()
                   << reply.errorMessage();
    }
}

void DBusWrapper::debugDumpMprisPlayers()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        qDebug() << "MPRIS debug: session bus not connected";
        return;
    }

    QDBusConnectionInterface *busInterface = QDBusConnection::sessionBus().interface();

    if (!busInterface) {
        qDebug() << "MPRIS debug: no DBus connection interface";
        return;
    }

    const QDBusReply<QStringList> servicesReply = busInterface->registeredServiceNames();

    if (!servicesReply.isValid()) {
        qDebug() << "MPRIS debug: cannot list registered services";
        return;
    }

    qDebug() << "========== MPRIS PLAYERS ==========";

    for (const QString &service : servicesReply.value()) {
        if (!isMprisService(service)) {
            continue;
        }

        const MprisSnapshot snapshot = readMprisSnapshot(service);

        qDebug() << "Service:" << service
                 << "valid=" << snapshot.valid
                 << "usable=" << snapshot.hasUsableMedia
                 << "playing=" << snapshot.playing
                 << "paused=" << snapshot.paused
                 << "stopped=" << snapshot.stopped
                 << "title=" << snapshot.title
                 << "length=" << snapshot.length
                 << "position=" << snapshot.position
                 << "progress=" << snapshot.progress;
    }

    qDebug() << "Current tracked player:" << m_currentTrackedPlayer;
    qDebug() << "===================================";
}