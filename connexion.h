#ifndef CONNEXION_H
#define CONNEXION_H

#include <QSqlDatabase>
#include <QString>

// Patron Singleton (Meyers) : une seule connexion à la BD pour toute l'application
class Connexion
{
public:
    static Connexion &createInstance();    // retourne l'instance unique
    bool createConnection();               // établit la connexion à la base
    void closeConnection();
    QString derniereErreur() const;

private:
    QSqlDatabase db;                       // connexion persistante du Singleton

    Connexion();                           // constructeur privé
    ~Connexion();                          // destructeur privé
    Connexion(const Connexion &) = delete;             // interdire la copie
    Connexion &operator=(const Connexion &) = delete;  // interdire l'affectation
};

#endif // CONNEXION_H