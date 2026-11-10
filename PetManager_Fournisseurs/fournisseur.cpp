#include "fournisseur.h"
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>
#include <QSqlError>

Fournisseur::Fournisseur()
    : m_idFournisseur(0), m_prixUnitaire(0), m_quantite(0)
{
}

Fournisseur::Fournisseur(int idFournisseur, const QString &nom, const QString &contact,
                         const QString &telephone, const QString &email, const QString &adresse,
                         const QString &typeProduit, const QString &status, const QDate &dateContrat,
                         double prixUnitaire, int quantite)
    : m_idFournisseur(idFournisseur), m_nom(nom), m_contact(contact),
    m_telephone(telephone), m_email(email), m_adresse(adresse),
    m_typeProduit(typeProduit), m_status(status), m_dateContrat(dateContrat),
    m_prixUnitaire(prixUnitaire), m_quantite(quantite)
{
}

// Getters
int Fournisseur::idFournisseur() const { return m_idFournisseur; }
QString Fournisseur::nom() const { return m_nom; }
QString Fournisseur::contact() const { return m_contact; }
QString Fournisseur::telephone() const { return m_telephone; }
QString Fournisseur::email() const { return m_email; }
QString Fournisseur::adresse() const { return m_adresse; }
QString Fournisseur::typeProduit() const { return m_typeProduit; }
QString Fournisseur::status() const { return m_status; }
QDate Fournisseur::dateContrat() const { return m_dateContrat; }
double Fournisseur::prixUnitaire() const { return m_prixUnitaire; }
int Fournisseur::quantite() const { return m_quantite; }

// Setters
void Fournisseur::setIdFournisseur(int id) { m_idFournisseur = id; }
void Fournisseur::setNom(const QString &v) { m_nom = v; }
void Fournisseur::setContact(const QString &v) { m_contact = v; }
void Fournisseur::setTelephone(const QString &v) { m_telephone = v; }
void Fournisseur::setEmail(const QString &v) { m_email = v; }
void Fournisseur::setAdresse(const QString &v) { m_adresse = v; }
void Fournisseur::setTypeProduit(const QString &v) { m_typeProduit = v; }
void Fournisseur::setStatus(const QString &v) { m_status = v; }
void Fournisseur::setDateContrat(const QDate &v) { m_dateContrat = v; }
void Fournisseur::setPrixUnitaire(double v) { m_prixUnitaire = v; }
void Fournisseur::setQuantite(int v) { m_quantite = v; }


// --- CRUD METHODS ---

bool Fournisseur::ajouter()
{
    QSqlQuery query;
    // On utilise des requêtes préparées pour éviter les injections SQL (exigence du prof)
    query.prepare("INSERT INTO FOURNISSEUR (IDFOURNISSEUR, NOM, CONTACT, TELEPHONE, EMAIL, ADRESSE, TYPEPRODUIT, STATUT, DATECONTRAT) "
                  "VALUES (:id, :nom, :contact, :tel, :email, :adresse, :type, :statut, :date)");

    query.bindValue(":id", m_idFournisseur);
    query.bindValue(":nom", m_nom);
    query.bindValue(":contact", m_contact);
    query.bindValue(":tel", m_telephone);
    query.bindValue(":email", m_email);
    query.bindValue(":adresse", m_adresse);
    query.bindValue(":type", m_typeProduit);
    query.bindValue(":statut", m_status);
    query.bindValue(":date", m_dateContrat);

    if(query.exec()) {
        return true;
    } else {
        qDebug() << "Erreur ajout Fournisseur:" << query.lastError().text();
        return false;
    }
}

bool Fournisseur::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM FOURNISSEUR WHERE IDFOURNISSEUR = :id");
    query.bindValue(":id", id);

    if(query.exec()) {
        return true;
    } else {
        qDebug() << "Erreur suppression Fournisseur:" << query.lastError().text();
        return false;
    }
}

QSqlQueryModel* Fournisseur::afficher()
{
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT IDFOURNISSEUR, NOM, CONTACT, TELEPHONE, EMAIL, TYPEPRODUIT, STATUT FROM FOURNISSEUR ORDER BY IDFOURNISSEUR");

    // Définir les en-têtes de colonnes pour le tableau UI
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Contact"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Email"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Type Produit"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Statut"));

    return model;
}

bool Fournisseur::modifier()
{
    QSqlQuery query;
    query.prepare("UPDATE FOURNISSEUR SET NOM = :nom, CONTACT = :contact, TELEPHONE = :tel, EMAIL = :email, ADRESSE = :adresse, TYPEPRODUIT = :type, STATUT = :statut, DATECONTRAT = :date WHERE IDFOURNISSEUR = :id");

    query.bindValue(":id", m_idFournisseur);
    query.bindValue(":nom", m_nom);
    query.bindValue(":contact", m_contact);
    query.bindValue(":tel", m_telephone);
    query.bindValue(":email", m_email);
    query.bindValue(":adresse", m_adresse);
    query.bindValue(":type", m_typeProduit);
    query.bindValue(":statut", m_status);
    query.bindValue(":date", m_dateContrat.toString("yyyy-MM-dd"));

    if(query.exec()) {
        return true;
    } else {
        qDebug() << "Erreur modification Fournisseur:" << query.lastError().text();
        return false;
    }
}

Fournisseur Fournisseur::obtenirParId(int id)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM FOURNISSEUR WHERE IDFOURNISSEUR = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return Fournisseur(
            query.value("IDFOURNISSEUR").toInt(),
            query.value("NOM").toString(),
            query.value("CONTACT").toString(),
            query.value("TELEPHONE").toString(),
            query.value("EMAIL").toString(),
            query.value("ADRESSE").toString(),
            query.value("TYPEPRODUIT").toString(),
            query.value("STATUT").toString(),
            query.value("DATECONTRAT").toDate(),
            0.0, // prixUnitaire
            0    // quantite
            );
    }

    return Fournisseur();
}