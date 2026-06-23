#include <QGuiApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include "backend/systemstate.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName("Fluent");
    QGuiApplication::setApplicationDisplayName("Fluent Shell");

    QQmlApplicationEngine engine;

    SystemState* systemState = new SystemState(&app);

    qmlRegisterSingletonInstance("Fluent", 1, 0, "SystemState", systemState);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     []() { QCoreApplication::exit(EXIT_FAILURE); },
                     Qt::QueuedConnection);
    engine.loadFromModule("Fluent", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
