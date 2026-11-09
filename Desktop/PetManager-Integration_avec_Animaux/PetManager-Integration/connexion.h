#pragma once
#include <QSqlDatabase>

// Patron Singleton : UNE SEULE connexion Oracle pour toute l'application
class Connexion
{
public:
    static Connexion &instance();          // point d'accès unique
    bool ouvrir();                         // ouvre la connexion (si pas déjà ouverte)
    void fermer();
    QSqlDatabase db() const;
    QString derniereErreur() const { return m_erreur; }

private:
    Connexion() = default;                 // constructeur privé
    Connexion(const Connexion &) = delete;
    Connexion &operator=(const Connexion &) = delete;
    QString m_erreur;
};
