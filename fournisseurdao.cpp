#include "fournisseurdao.h"
#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

bool FournisseurDAO::inserer(const Fournisseur &f) {
    QSqlQuery q(DatabaseManager::instance().db());
    q.prepare("INSERT INTO fournisseur (nom, contact, telephone, email, adresse, typeProduit, status, dateContrat, prixUnitaire, quantite) "
              "VALUES (:nom, :contact, :telephone, :email, :adresse, :typeProduit, :status, :dateContrat, :prixUnitaire, :quantite)");
    q.bindValue(":nom", f.nom());
    q.bindValue(":contact", f.contact());
    q.bindValue(":telephone", f.telephone());
    q.bindValue(":email", f.email());
    q.bindValue(":adresse", f.adresse());
    q.bindValue(":typeProduit", f.typeProduit());
    q.bindValue(":status", f.status());
    q.bindValue(":dateContrat", f.dateContrat().toString("yyyy-MM-dd"));
    q.bindValue(":prixUnitaire", f.prixUnitaire());
    q.bindValue(":quantite", f.quantite());
    if (!q.exec()) { qCritical() << "Insert error:" << q.lastError().text(); return false; }
    return true;
}

bool FournisseurDAO::modifier(const Fournisseur &f) {
    QSqlQuery q(DatabaseManager::instance().db());
    q.prepare("UPDATE fournisseur SET nom=:nom, contact=:contact, telephone=:telephone, email=:email, adresse=:adresse, "
              "typeProduit=:typeProduit, status=:status, dateContrat=:dateContrat, prixUnitaire=:prixUnitaire, quantite=:quantite "
              "WHERE idFournisseur=:id");
    q.bindValue(":id", f.idFournisseur());
    q.bindValue(":nom", f.nom());
    q.bindValue(":contact", f.contact());
    q.bindValue(":telephone", f.telephone());
    q.bindValue(":email", f.email());
    q.bindValue(":adresse", f.adresse());
    q.bindValue(":typeProduit", f.typeProduit());
    q.bindValue(":status", f.status());
    q.bindValue(":dateContrat", f.dateContrat().toString("yyyy-MM-dd"));
    q.bindValue(":prixUnitaire", f.prixUnitaire());
    q.bindValue(":quantite", f.quantite());
    if (!q.exec()) { qCritical() << "Update error:" << q.lastError().text(); return false; }
    return true;
}

bool FournisseurDAO::supprimer(int idFournisseur) {
    QSqlQuery q(DatabaseManager::instance().db());
    q.prepare("DELETE FROM fournisseur WHERE idFournisseur = :id");
    q.bindValue(":id", idFournisseur);
    if (!q.exec()) { qCritical() << "Delete error:" << q.lastError().text(); return false; }
    return true;
}

Fournisseur FournisseurDAO::obtenirParId(int idFournisseur) {
    Fournisseur f;
    QSqlQuery q(DatabaseManager::instance().db());
    q.prepare("SELECT * FROM fournisseur WHERE idFournisseur = :id");
    q.bindValue(":id", idFournisseur);
    if (q.exec() && q.next()) {
        f.setIdFournisseur(q.value("idFournisseur").toInt());
        f.setNom(q.value("nom").toString());
        f.setContact(q.value("contact").toString());
        f.setTelephone(q.value("telephone").toString());
        f.setEmail(q.value("email").toString());
        f.setAdresse(q.value("adresse").toString());
        f.setTypeProduit(q.value("typeProduit").toString());
        f.setStatus(q.value("status").toString());
        f.setDateContrat(QDate::fromString(q.value("dateContrat").toString(), "yyyy-MM-dd"));
        f.setPrixUnitaire(q.value("prixUnitaire").toDouble());
        f.setQuantite(q.value("quantite").toInt());
    }
    return f;
}

QList<Fournisseur> FournisseurDAO::obtenirTous() {
    QList<Fournisseur> list;
    QSqlQuery q(DatabaseManager::instance().db());
    q.prepare("SELECT * FROM fournisseur ORDER BY nom ASC");
    if (q.exec()) {
        while (q.next()) {
            Fournisseur f;
            f.setIdFournisseur(q.value("idFournisseur").toInt());
            f.setNom(q.value("nom").toString());
            f.setContact(q.value("contact").toString());
            f.setTelephone(q.value("telephone").toString());
            f.setEmail(q.value("email").toString());
            f.setAdresse(q.value("adresse").toString());
            f.setTypeProduit(q.value("typeProduit").toString());
            f.setStatus(q.value("status").toString());
            f.setDateContrat(QDate::fromString(q.value("dateContrat").toString(), "yyyy-MM-dd"));
            f.setPrixUnitaire(q.value("prixUnitaire").toDouble());
            f.setQuantite(q.value("quantite").toInt());
            list.append(f);
        }
    }
    return list;
}

QList<Fournisseur> FournisseurDAO::rechercherParNom(const QString &nom) {
    QList<Fournisseur> list;
    QSqlQuery q(DatabaseManager::instance().db());
    q.prepare("SELECT * FROM fournisseur WHERE nom LIKE :nom ORDER BY nom ASC");
    q.bindValue(":nom", "%" + nom + "%");
    if (q.exec()) {
        while (q.next()) {
            Fournisseur f;
            f.setIdFournisseur(q.value("idFournisseur").toInt());
            f.setNom(q.value("nom").toString());
            f.setContact(q.value("contact").toString());
            f.setTelephone(q.value("telephone").toString());
            f.setEmail(q.value("email").toString());
            f.setAdresse(q.value("adresse").toString());
            f.setTypeProduit(q.value("typeProduit").toString());
            f.setStatus(q.value("status").toString());
            f.setDateContrat(QDate::fromString(q.value("dateContrat").toString(), "yyyy-MM-dd"));
            f.setPrixUnitaire(q.value("prixUnitaire").toDouble());
            f.setQuantite(q.value("quantite").toInt());
            list.append(f);
        }
    }
    return list;
}