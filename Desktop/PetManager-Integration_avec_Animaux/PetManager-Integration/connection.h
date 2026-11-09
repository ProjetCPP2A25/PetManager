#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

// Patron Singleton (Meyers) : une seule connexion à la BD pour toute l'application
class Connection
{
public:
    static Connection& createInstance();   // retourne l'instance unique
    bool createConnection();               // établit la connexion à la base
    void closeConnection();
    QString derniereErreur() const;

private:
    QSqlDatabase db;                       // connexion persistante du Singleton

    Connection();                          // constructeur privé
    ~Connection();                         // destructeur privé
    Connection(const Connection&) = delete;            // interdire la copie
    Connection& operator=(const Connection&) = delete; // interdire l'affectation
};

#endif // CONNECTION_H
