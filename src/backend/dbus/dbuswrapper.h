// src/backend/dbus/dbuswrapper.h
#pragma once

#include <QString>
#include <tuple>

class DBusWrapper {
public:
    DBusWrapper();
    ~DBusWrapper();

    std::tuple<double, bool> getBatteryStatus();
    std::tuple<QString, bool> getNetworkStatus();

private:
    bool m_systemBusAvailable = false;
};