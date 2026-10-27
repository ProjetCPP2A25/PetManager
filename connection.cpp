#include "connection.h"

Connection::Connection()
{
    db = QSqlDatabase::addDatabase("QODBC");
}

Connection::~Connection()
{
    if (db.isOpen())
        db.close();
}

Connection& Connection::createInstance()
{
    static Connection instance;   // instance unique du Singleton
    return instance;
}

bool Connection::createConnection()
{
    db.setDatabaseName("Source_Projet2A");   // source de données ODBC
    db.setUserName("chahed");                // utilisateur Oracle
    db.setPassword("esprit18");              // mot de passe Oracle

    if (db.open()) {
        qDebug() << "Connexion établie";
        return true;
    }
    qDebug() << "Échec de la connexion :" << db.lastError().text();
    return false;
}

void Connection::closeConnection()
{
    if (db.isOpen())
        db.close();
}

QString Connection::derniereErreur() const
{
    return db.lastError().text();
}
