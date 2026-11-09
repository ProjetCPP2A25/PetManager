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
    static Connexion instance;             // instance unique du Singleton
    return instance;
}

bool Connexion::createConnection()
{
    // ====== À ADAPTER À VOTRE POSTE ======
    db.setDatabaseName("PetManager");      // nom de la source de données ODBC
    db.setUserName("petmanager");              // utilisateur Oracle
    db.setPassword("0000");                    // TODO : votre mot de passe Oracle
    // =====================================

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