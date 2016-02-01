#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>

class SessionManager : public QObject
{
    Q_OBJECT
public:
    SessionManager(const QString &sessionsDir);

signals:

public slots:

private:
    QString m_sessionsDir;
    class KDirWatch *m_dirWatch;
};

#endif // SESSIONMANAGER_H
