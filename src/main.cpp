#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <ktexteditor/editor.h>
#include <ktexteditor/document.h>
#include <kaboutdata.h>
#include <QDebug>
#include "fileprocessor.h"

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);
    qmlRegisterType<FileProcessor>("text.processing", 1, 0, "FileProcessor");

	QQmlApplicationEngine engine;
	engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    qDebug() << KTextEditor::Editor::instance()->aboutData().displayName();
    // create a new document
    KTextEditor::Document *document = KTextEditor::Editor::instance()->createDocument(0);

	return app.exec();
}
