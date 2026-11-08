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
    db.setDatabaseName("Source_Projet2A");   // source de données ODBC (commune au groupe)
    db.setUserName("system");            // utilisateur Oracle commun
    db.setPassword("0000");   // mot de passe Oracle commun

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
