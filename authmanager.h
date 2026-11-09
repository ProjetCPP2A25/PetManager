#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <QString>
#include <QStringList>

// Comptes de l'application : malek, chahed, elyes, nessma, wissal, iyed
// Mot de passe par défaut : 0000 (modifiable ; le nouveau mot de passe est enregistré haché dans QSettings)
class AuthManager
{
public:
    static QStringList utilisateurs();                      // identifiants en minuscules
    static QString nomAffiche(const QString &id);           // "Malek"
    static QString emoji(const QString &id);                // avatar de l'utilisateur
    static QString role(const QString &id);                 // rôle (traduit)

    // Adresse e-mail du compte (utilisée pour "mot de passe oublié")
    static QString email(const QString &id);
    static void definirEmail(const QString &id, const QString &adresse);
    static QString utilisateurParEmail(const QString &adresse);  // "" si aucun compte

    static QString normaliser(const QString &saisie);       // "  MaLek " -> "malek" ("" si inconnu)
    static bool verifier(const QString &saisie, const QString &motDePasse);
    static void definirMotDePasse(const QString &id, const QString &motDePasse);

    // Session
    static void connecter(const QString &id);
    static void deconnecter();
    static bool estConnecte();
    static QString utilisateurCourant();                    // "" si personne
    static QString dernierUtilisateur();                    // pour pré-remplir l'écran de connexion
};

#endif // AUTHMANAGER_H
