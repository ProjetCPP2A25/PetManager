#ifndef FOURNISSEUR_H
#define FOURNISSEUR_H

#include <QString>
#include <QDate>

class Fournisseur
{
public:
    Fournisseur();
    Fournisseur(int idFournisseur, const QString &nom, const QString &contact,
                const QString &telephone, const QString &email, const QString &adresse,
                const QString &typeProduit, const QString &status, const QDate &dateContrat,
                double prixUnitaire, int quantite);

    int idFournisseur() const;      void setIdFournisseur(int id);
    QString nom() const;            void setNom(const QString &v);
    QString contact() const;        void setContact(const QString &v);
    QString telephone() const;      void setTelephone(const QString &v);
    QString email() const;          void setEmail(const QString &v);
    QString adresse() const;        void setAdresse(const QString &v);
    QString typeProduit() const;    void setTypeProduit(const QString &v);
    QString status() const;         void setStatus(const QString &v);
    QDate dateContrat() const;      void setDateContrat(const QDate &v);
    double prixUnitaire() const;    void setPrixUnitaire(double v);
    int quantite() const;           void setQuantite(int v);

private:
    int m_idFournisseur;
    QString m_nom, m_contact, m_telephone, m_email, m_adresse;
    QString m_typeProduit, m_status;
    QDate m_dateContrat;
    double m_prixUnitaire;
    int m_quantite;
};

#endif // FOURNISSEUR_H