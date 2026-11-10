#include "connexion.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QRegularExpression>

// >>> À ADAPTER si besoin <<<
static const char *DSN      = "Source_Projet";   // source ODBC 64 bits
static const char *USER     = "system";
static const char *PASSWORD = "0000";            // <-- ton mot de passe Oracle
static const char *SCHEMA   = "PETMANAGER";      // schema qui contient les tables de l'équipe ("" = celui de USER)

Connexion &Connexion::instance()
{
    static Connexion c;                          // créée une seule fois
    return c;
}

bool Connexion::ouvrir()
{
    QSqlDatabase db = QSqlDatabase::contains("qt_sql_default_connection")
                          ? QSqlDatabase::database()
                          : QSqlDatabase::addDatabase("QODBC");
    if (db.isOpen()) return true;
    db.setDatabaseName(DSN);
    db.setUserName(USER);
    db.setPassword(PASSWORD);
    if (!db.open()) {
        const QString brut = db.lastError().databaseText() + " " + db.lastError().driverText()
                             + " " + db.lastError().text();
        QString code;
        for (const char *c5 : {"28000", "01017", "12541", "12514", "12505"})
            if (brut.contains(QString("ORA-") + c5)) { code = c5; break; }
        if      (code == "28000") m_erreur = "Le compte Oracle est verrouill\u00e9 (ORA-28000). Faites : ALTER USER system ACCOUNT UNLOCK;";
        else if (code == "01017") m_erreur = "Utilisateur ou mot de passe Oracle incorrect (ORA-01017). V\u00e9rifiez USER et PASSWORD dans connexion.cpp.";
        else if (code == "12541" || code == "12514" || code == "12505")
                                  m_erreur = "Service Oracle injoignable (ORA-" + code + "). V\u00e9rifiez que Oracle XE est d\u00e9marr\u00e9 (services Windows : OracleServiceXE et OracleXETNSListener).";
        else {
            QString propre;
            for (const QChar ch : brut)
                if (ch.unicode() >= 32 && ch.unicode() < 127) propre += ch;
            m_erreur = "Connexion impossible. " + propre.simplified().left(250);
            if (propre.contains("Data source name not found", Qt::CaseInsensitive))
                m_erreur = "Source ODBC introuvable : le DSN \"" + QString(DSN) + "\" n'existe pas (ODBC 64 bits).";
        }
        return false;
    }
    // les tables de l'équipe sont dans le schéma PETMANAGER : on s'y place pour cette connexion
    if (QString(SCHEMA).size() > 0 && QString(USER).toUpper() != QString(SCHEMA).toUpper()) {
        QSqlQuery q(db);
        if (!q.exec(QString("ALTER SESSION SET CURRENT_SCHEMA = %1").arg(SCHEMA))) {
            m_erreur = QString("Connexion r\u00e9ussie mais le sch\u00e9ma %1 est inaccessible.").arg(SCHEMA);
            return false;
        }
    }
    m_erreur.clear();
    return true;
}

void Connexion::fermer()
{
    if (QSqlDatabase::contains("qt_sql_default_connection"))
        QSqlDatabase::database().close();
}

QSqlDatabase Connexion::db() const
{
    return QSqlDatabase::database();
}
