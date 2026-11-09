#include "connexion.h"

#include <QDebug>
#include <QSqlError>

Connexion::Connexion()
{
    db = QSqlDatabase::addDatabase("QODBC");
}

Connexion::~Connexion()
{
    if (db.isOpen())
        db.close();
}

Connexion &Connexion::createInstance()
{
    static Connexion instance;
    return instance;
}

bool Connexion::createConnection()
{
    db.setDatabaseName("PetManager");
    db.setUserName("petmanager");
    db.setPassword("0000");


    if (db.open()) {
        qDebug() << "Connexion établie";
        return true;
    }
    qDebug() << "Échec de la connexion :" << db.lastError().text();
    return false;
}

void Connexion::closeConnection()
{
    if (db.isOpen())
        db.close();
}

QString Connexion::derniereErreur() const
{
    return db.lastError().text();
}