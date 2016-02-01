#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
//#include <ktexteditor/editor.h>
//#include <ktexteditor/document.h>
//#include <kaboutdata.h>
#include <QDebug>
#include "documentmanager.h"
//#include "mdtextview.h"

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);
    qmlRegisterType<DocumentManager>("text.processing", 1, 0, "DocumentManager");

	QQmlApplicationEngine engine;
	engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

//    qDebug() << KTextEditor::Editor::instance()->aboutData().displayName();
    // create a new document
//    KTextEditor::Document *document = KTextEditor::Editor::instance()->createDocument(0);
//    KTextEditor::View *kview = document->createView(NULL);
//    MDTextView *myView = qobject_cast<MDTextView*>(kview);
//    if (myView) {
//        qDebug() << "Wow!";
//    } else {
//        qDebug() << "bad";
//    }

	return app.exec();
}
