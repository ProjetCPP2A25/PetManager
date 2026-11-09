#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariantList>
#include <QDebug>

class DatabaseManager
{
public:
    static DatabaseManager& instance();
    bool open();
    void close();
    bool isOpen() const;
    QSqlDatabase db() const;
    QSqlDatabase getDatabase() const { return db(); }   // nom utilisé par le module Employés

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool createTables();
    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H