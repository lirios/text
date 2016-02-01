#include "sessionmanager.h"

#include <KDirWatch>
#include <QDir>
#include <QStandardPaths>

SessionManager::SessionManager(const QString &sessionsDir = QString()) {
    if(sessionsDir.isEmpty()) {
        m_sessionsDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/liri-text/sessions");
    } else {
        m_sessionsDir = sessionsDir;
    }
    // create dir if needed
    QDir().mkpath(m_sessionsDir);

    m_dirWatch = new KDirWatch(this);
    m_dirWatch->addDir(m_sessionsDir);
    connect(m_dirWatch, SIGNAL(dirty(QString)), this, SLOT(updateSessionList()));

    //updateSessionList();

//    m_activeSession = KateSession::createAnonymous(anonymousSessionFile());
}
