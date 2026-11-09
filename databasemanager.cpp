#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::DatabaseManager() {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbPath);
    m_db.setDatabaseName(dbPath + "/petmanager.db");
    qDebug() << "Database path:" << dbPath + "/petmanager.db";
}

DatabaseManager::~DatabaseManager() {
    close();
}

bool DatabaseManager::open() {
    if (m_db.isOpen()) {
        qDebug() << "Database already open";
        return true;
    }

    bool ok = m_db.open();
    if (ok) {
        qDebug() << "Database opened successfully";
        ok = createTables();
    } else {
        qCritical() << "DB open error:" << m_db.lastError().text();
    }
    return ok;
}

void DatabaseManager::close() {
    if (m_db.isOpen()) {
        m_db.close();
        qDebug() << "Database closed";
    }
}

bool DatabaseManager::isOpen() const {
    return m_db.isOpen();
}

QSqlDatabase DatabaseManager::db() const {
    return m_db;
}

bool DatabaseManager::createTables() {
    if (!m_db.isOpen()) {
        qCritical() << "Cannot create tables: database not open";
        return false;
    }

    QSqlQuery q;
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS fournisseur (
            idFournisseur INTEGER PRIMARY KEY AUTOINCREMENT,
            nom TEXT NOT NULL,
            contact TEXT NOT NULL,
            telephone TEXT NOT NULL,
            email TEXT NOT NULL,
            adresse TEXT,
            typeProduit TEXT NOT NULL,
            status TEXT NOT NULL,
            dateContrat TEXT NOT NULL,
            prixUnitaire REAL NOT NULL DEFAULT 0,
            quantite INTEGER NOT NULL DEFAULT 0
        )
    )";

    if (!q.exec(sql)) {
        qCritical() << "Create table error:" << q.lastError().text();
        return false;
    }
    qDebug() << "Table 'fournisseur' created or already exists";

    // Module Employés : la table est CONSERVÉE d'un lancement à l'autre (plus de DROP TABLE)
    const QString sqlEmploye = R"(
        CREATE TABLE IF NOT EXISTS employe (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            nom TEXT NOT NULL,
            prenom TEXT NOT NULL,
            email TEXT,
            telephone TEXT,
            poste TEXT NOT NULL,
            salaire REAL DEFAULT 0,
            dateEmbauche TEXT NOT NULL,
            statut TEXT DEFAULT 'Actif'
        )
    )";
    if (!q.exec(sqlEmploye)) {
        qCritical() << "Create table error (employe):" << q.lastError().text();
        return false;
    }
    qDebug() << "Table 'employe' created or already exists";
    return true;
}