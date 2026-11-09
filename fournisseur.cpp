#include "fournisseur.h"

Fournisseur::Fournisseur() : m_idFournisseur(0), m_dateContrat(QDate::currentDate()), m_prixUnitaire(0.0), m_quantite(0) {}

Fournisseur::Fournisseur(int id, const QString &nom, const QString &contact, const QString &tel, const QString &email, const QString &adresse, const QString &type, const QString &status, const QDate &date, double prix, int qte)
    : m_idFournisseur(id), m_nom(nom), m_contact(contact), m_telephone(tel), m_email(email), m_adresse(adresse), m_typeProduit(type), m_status(status), m_dateContrat(date), m_prixUnitaire(prix), m_quantite(qte) {}

int Fournisseur::idFournisseur() const { return m_idFournisseur; }
void Fournisseur::setIdFournisseur(int id) { m_idFournisseur = id; }
QString Fournisseur::nom() const { return m_nom; }
void Fournisseur::setNom(const QString &v) { m_nom = v; }
QString Fournisseur::contact() const { return m_contact; }
void Fournisseur::setContact(const QString &v) { m_contact = v; }
QString Fournisseur::telephone() const { return m_telephone; }
void Fournisseur::setTelephone(const QString &v) { m_telephone = v; }
QString Fournisseur::email() const { return m_email; }
void Fournisseur::setEmail(const QString &v) { m_email = v; }
QString Fournisseur::adresse() const { return m_adresse; }
void Fournisseur::setAdresse(const QString &v) { m_adresse = v; }
QString Fournisseur::typeProduit() const { return m_typeProduit; }
void Fournisseur::setTypeProduit(const QString &v) { m_typeProduit = v; }
QString Fournisseur::status() const { return m_status; }
void Fournisseur::setStatus(const QString &v) { m_status = v; }
QDate Fournisseur::dateContrat() const { return m_dateContrat; }
void Fournisseur::setDateContrat(const QDate &v) { m_dateContrat = v; }
double Fournisseur::prixUnitaire() const { return m_prixUnitaire; }
void Fournisseur::setPrixUnitaire(double v) { m_prixUnitaire = v; }
int Fournisseur::quantite() const { return m_quantite; }
void Fournisseur::setQuantite(int v) { m_quantite = v; }