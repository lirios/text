#include "languagedatabasemaintainer.h"

#include <QSqlQuery>
#include <QDir>
#include <QStandardPaths>
#include "languageloader.h"

LanguageDatabaseMaintainer::LanguageDatabaseMaintainer(QString connId, QObject *parent) :
    m_connId(connId),
    QObject(parent) {
    // List of language specification directories, ascending by priority
    specsDirs = QStringList({
                #ifdef GTKSOURCEVIEW_LANGUAGE_SPECS
                                             QString(GTKSOURCEVIEW_LANGUAGE_SPECS),
                #endif
                                             QString(LIRI_LANGUAGE_SPECS)
                                                         });
    initDB();
}

LanguageDatabaseMaintainer::~LanguageDatabaseMaintainer() {
    delete watcher;
    QSqlDatabase::removeDatabase(m_connId);
}

void LanguageDatabaseMaintainer::init() {
    updateDB();
    watcher = new QFileSystemWatcher(specsDirs);
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, &LanguageDatabaseMaintainer::updateDB);
}

void LanguageDatabaseMaintainer::initDB() {
    QDir dataDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if(!dataDir.exists())
        dataDir.mkpath(".");

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connId);
    db.setDatabaseName(dataDir.filePath("languages.db"));
    db.open();
    QSqlQuery("CREATE TABLE IF NOT EXISTS languages "
              "(id TEXT PRIMARY KEY, spec_path TEXT, mime_types TEXT, globs TEXT, display TEXT)",
              db);
}

void LanguageDatabaseMaintainer::updateDB() {
    LanguageLoader ll;
    for (QDir dir : specsDirs) {
        for (QFileInfo file : dir.entryInfoList()) {
            if(file.isFile()) {
                LanguageMetadata langData = ll.loadMetadata(file.absoluteFilePath());
                QSqlQuery(QStringLiteral("UPDATE languages SET "
                                         "id='%1',spec_path='%2',mime_types='%3',globs='%4',display='%5' "
                                         "WHERE id='%1'").arg(
                              langData.id, file.absoluteFilePath(), langData.mimeTypes, langData.globs, langData.name),
                          QSqlDatabase::database(m_connId));
                // If update failed, insert
                QSqlQuery(QStringLiteral("INSERT INTO languages (id, spec_path, mime_types, globs, display) "
                                         "SELECT '%1','%2','%3','%4','%5' "
                                         "WHERE (SELECT Changes() = 0)").arg(
                              langData.id, file.absoluteFilePath(), langData.mimeTypes, langData.globs, langData.name),
                          QSqlDatabase::database(m_connId));
            }
        }
    }

    // Search for obsolete entries
    QSqlQuery query(QStringLiteral("SELECT id, spec_path FROM languages"),
                    QSqlDatabase::database(m_connId));
    while (query.next()) {
        QFileInfo file(query.value(1).toString());
        if(!file.exists())
            QSqlQuery(QStringLiteral("DELETE FROM languages "
                                     "WHERE id='%1'").arg(query.value(0).toString()),
                      QSqlDatabase::database(m_connId));
    }

    emit dbUpdated();
}
