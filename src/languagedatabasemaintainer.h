#ifndef LANGUAGEDATABASEMAINTAINER_H
#define LANGUAGEDATABASEMAINTAINER_H

#include <QObject>
#include <QSqlDatabase>
#include <QFileSystemWatcher>

class LanguageDatabaseMaintainer : public QObject
{
    Q_OBJECT
public:
    explicit LanguageDatabaseMaintainer(QString connId, QObject *parent = 0);
    ~LanguageDatabaseMaintainer();

signals:
    void dbUpdated();
protected:
    void initDB();
public slots:
    void init();
    void updateDB();
private:
    QStringList specsDirs;
    QFileSystemWatcher *watcher;
    QString m_connId;
};

#endif // LANGUAGEDATABASEMAINTAINER_H
