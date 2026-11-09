#ifndef EMPLOYE_H
#define EMPLOYE_H

#include <QString>
#include <QDate>

class Employe {
public:
    Employe();
    Employe(int id, const QString &nom, const QString &prenom, const QString &email,
            const QString &telephone, const QString &poste, double salaire,
            const QDate &dateEmbauche, const QString &statut);

    // Getters
    int getId() const;
    QString getNom() const;
    QString getPrenom() const;
    QString getNomComplet() const;
    QString getEmail() const;
    QString getTelephone() const;
    QString getPoste() const;
    double getSalaire() const;
    QDate getDateEmbauche() const;
    QString getStatut() const;

    // Setters
    void setId(int id);
    void setNom(const QString &nom);
    void setPrenom(const QString &prenom);
    void setEmail(const QString &email);
    void setTelephone(const QString &telephone);
    void setPoste(const QString &poste);
    void setSalaire(double salaire);
    void setDateEmbauche(const QDate &date);
    void setStatut(const QString &statut);

private:
    int m_id;
    QString m_nom;
    QString m_prenom;
    QString m_email;
    QString m_telephone;
    QString m_poste;
    double m_salaire;
    QDate m_dateEmbauche;
    QString m_statut; // Actif, En congé, Inactif
};

#endif // EMPLOYE_H