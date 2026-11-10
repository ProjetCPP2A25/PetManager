#include "connection.h"
#include <QSqlQuery>
#include <QSqlError>

Connection::Connection() {
    db = QSqlDatabase::addDatabase("QODBC");
}

Connection::~Connection() {
    if (db.isOpen())
        db.close();
}

Connection& Connection::createInstance() {
    static Connection instance;
    return instance;
}

bool Connection::createConnection() {
    db.setDatabaseName("Source_Projet2A");
    db.setUserName("elyes");
    db.setPassword("esprit18");

    if (db.open()) {
        qDebug() << "Connexion Oracle établie";
        return true;
    }
    qDebug() << "Echec de la connexion Oracle :" << db.lastError().text();
    return false;
}