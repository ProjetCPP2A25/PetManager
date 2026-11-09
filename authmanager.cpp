#include "authmanager.h"
#include "langue.h"

#include <QCryptographicHash>
#include <QSettings>

namespace {

struct Compte {
    const char *id;
    const char *nom;
    const char *emoji;
    const char *roleCle;     // rôle d'affichage (cosmétique) : à modifier ici si besoin
    const char *email;       // adresse qui reçoit le code "mot de passe oublié" : METTEZ VOS VRAIES ADRESSES
};

const Compte COMPTES[] = {
    {"malek",  "Malek",  "🐶", "role_admin", "malek@petmanager.tn"},
    {"chahed", "Chahed", "🐱", "role_stock", "chahed@petmanager.tn"},
    {"elyes",  "Elyes",  "🐰", "role_fourn", "elyes@petmanager.tn"},
    {"nessma", "Nessma", "🦊", "role_vet", "nessma@petmanager.tn"},
    {"wissal", "Wissal", "🐹", "role_accueil", "wissal@petmanager.tn"},
    {"iyed",   "Iyed",   "🐢", "role_vet", "iyed@petmanager.tn"},
};

const char *MOT_DE_PASSE_PAR_DEFAUT = "0000";

QString utilisateurSession;          // utilisateur connecté (en mémoire seulement)

const Compte *trouver(const QString &id)
{
    for (const Compte &c : COMPTES)
        if (id == QLatin1String(c.id))
            return &c;
    return nullptr;
}

QString hacher(const QString &id, const QString &motDePasse)
{
    const QByteArray brut = (id + ":" + motDePasse).toUtf8();
    return QString::fromLatin1(QCryptographicHash::hash(brut, QCryptographicHash::Sha256).toHex());
}

} // namespace

QStringList AuthManager::utilisateurs()
{
    QStringList liste;
    for (const Compte &c : COMPTES)
        liste << QString::fromLatin1(c.id);
    return liste;
}

QString AuthManager::nomAffiche(const QString &id)
{
    const Compte *c = trouver(id);
    return c ? QString::fromUtf8(c->nom) : id;
}

QString AuthManager::emoji(const QString &id)
{
    const Compte *c = trouver(id);
    return c ? QString::fromUtf8(c->emoji) : QString::fromUtf8("👤");
}

QString AuthManager::role(const QString &id)
{
    const Compte *c = trouver(id);
    return c ? Langue::t(c->roleCle) : QString();
}

QString AuthManager::email(const QString &id)
{
    const Compte *c = trouver(id);
    if (!c)
        return QString();
    QSettings s("PetManager", "PetManager");
    return s.value("auth/email_" + id, QString::fromUtf8(c->email)).toString();
}

void AuthManager::definirEmail(const QString &id, const QString &adresse)
{
    if (!trouver(id))
        return;
    QSettings s("PetManager", "PetManager");
    s.setValue("auth/email_" + id, adresse.trimmed().toLower());
    s.sync();
}

QString AuthManager::utilisateurParEmail(const QString &adresse)
{
    const QString cherche = adresse.trimmed().toLower();
    if (cherche.isEmpty())
        return QString();
    for (const Compte &c : COMPTES) {
        const QString id = QString::fromLatin1(c.id);
        if (email(id).trimmed().toLower() == cherche)
            return id;
    }
    return QString();
}

QString AuthManager::normaliser(const QString &saisie)
{
    const QString id = saisie.trimmed().toLower();
    return trouver(id) ? id : QString();
}

bool AuthManager::verifier(const QString &saisie, const QString &motDePasse)
{
    const QString id = normaliser(saisie);
    if (id.isEmpty())
        return false;

    QSettings s("PetManager", "PetManager");
    const QString hacheEnregistre = s.value("auth/" + id).toString();
    if (!hacheEnregistre.isEmpty())
        return hacheEnregistre == hacher(id, motDePasse);
    return motDePasse == QLatin1String(MOT_DE_PASSE_PAR_DEFAUT);
}

void AuthManager::definirMotDePasse(const QString &id, const QString &motDePasse)
{
    if (!trouver(id))
        return;
    QSettings s("PetManager", "PetManager");
    s.setValue("auth/" + id, hacher(id, motDePasse));
    s.sync();
}

void AuthManager::connecter(const QString &id)
{
    utilisateurSession = id;
    QSettings s("PetManager", "PetManager");
    s.setValue("auth/dernierUtilisateur", id);
    s.sync();
}

void AuthManager::deconnecter()
{
    utilisateurSession.clear();
}

bool AuthManager::estConnecte()
{
    return !utilisateurSession.isEmpty();
}

QString AuthManager::utilisateurCourant()
{
    return utilisateurSession;
}

QString AuthManager::dernierUtilisateur()
{
    QSettings s("PetManager", "PetManager");
    return s.value("auth/dernierUtilisateur").toString();
}
