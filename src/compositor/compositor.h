// src/compositor/compositor.h
#pragma once
#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandShell>
#include <QtWaylandCompositor/QWaylandXdgShell>
#include <QQmlEngine>
#include <QScreen>

class WindowManager;
class CompositorService;

class Compositor : public QWaylandCompositor {
    Q_OBJECT
    QML_ELEMENT

public:
    explicit Compositor(QScreen *screen = nullptr);
    ~Compositor();

    WindowManager *windowManager() const { return m_windowManager; }
    CompositorService *service() const { return m_service; }

    Q_INVOKABLE void setActiveWindow(QWaylandSurface *surface);
    Q_INVOKABLE QWaylandSurface *activeWindow() const;

signals:
    void windowAdded(QWaylandSurface *surface);
    void windowRemoved(QWaylandSurface *surface);
    void activeWindowChanged();

protected:
    void handleSurfaceCreated(QWaylandSurface *surface);

private:
    WindowManager *m_windowManager = nullptr;
    CompositorService *m_service = nullptr;
    QWaylandXdgShell *m_xdgShell = nullptr;
    QWaylandSurface *m_activeWindow = nullptr;
};