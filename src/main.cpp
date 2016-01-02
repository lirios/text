#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include "fileprocessor.h"

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);
    qmlRegisterType<FileProcessor>("text.processing", 1, 0, "FileProcessor");

	QQmlApplicationEngine engine;
	engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

	return app.exec();
}
