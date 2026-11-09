#include "employe.h"

Employe::Employe() : m_id(0), m_salaire(0.0) {}

Employe::Employe(int id, const QString &nom, const QString &prenom, const QString &email,
                 const QString &telephone, const QString &poste, double salaire,
                 const QDate &dateEmbauche, const QString &statut)
    : m_id(id), m_nom(nom), m_prenom(prenom), m_email(email),
    m_telephone(telephone), m_poste(poste), m_salaire(salaire),
    m_dateEmbauche(dateEmbauche), m_statut(statut) {}

// Getters
int Employe::getId() const { return m_id; }
QString Employe::getNom() const { return m_nom; }
QString Employe::getPrenom() const { return m_prenom; }
QString Employe::getNomComplet() const { return m_nom + " " + m_prenom; }
QString Employe::getEmail() const { return m_email; }
QString Employe::getTelephone() const { return m_telephone; }
QString Employe::getPoste() const { return m_poste; }
double Employe::getSalaire() const { return m_salaire; }
QDate Employe::getDateEmbauche() const { return m_dateEmbauche; }
QString Employe::getStatut() const { return m_statut; }

// Setters
void Employe::setId(int id) { m_id = id; }
void Employe::setNom(const QString &nom) { m_nom = nom; }
void Employe::setPrenom(const QString &prenom) { m_prenom = prenom; }
void Employe::setEmail(const QString &email) { m_email = email; }
void Employe::setTelephone(const QString &telephone) { m_telephone = telephone; }
void Employe::setPoste(const QString &poste) { m_poste = poste; }
void Employe::setSalaire(double salaire) { m_salaire = salaire; }
void Employe::setDateEmbauche(const QDate &date) { m_dateEmbauche = date; }
void Employe::setStatut(const QString &statut) { m_statut = statut; }