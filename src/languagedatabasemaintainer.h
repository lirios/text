#ifndef LANGUAGEDATABASEMAINTAINER_H
#define LANGUAGEDATABASEMAINTAINER_H

#include <QObject>
#include <QSqlDatabase>
#include <QFileSystemWatcher>

class LanguageDatabaseMaintainer : public QObject
{
    Q_OBJECT
public:
    explicit LanguageDatabaseMaintainer(QObject *parent = 0);
    ~LanguageDatabaseMaintainer();

signals:
    void dbUpdated();
protected:
    void initDB();
protected slots:
    void updateDB();
public slots:
    void init(QString connId);
private:
    QStringList specsDirs;
    QFileSystemWatcher *watcher;
    QString m_connId;
};

#endif // LANGUAGEDATABASEMAINTAINER_H
