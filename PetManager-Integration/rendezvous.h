#ifndef RENDEZVOUS_H
#define RENDEZVOUS_H

#include <QDate>
#include <QSqlQueryModel>
#include <QString>
#include <QStringList>

/* Modèle d'affichage du tableau : met en forme l'ID (RDV001) et la date.
 * Colonnes : 0 ID | 1 Date | 2 Heure | 3 ID Animal | 4 Motif | 5 Statut | 6 Actions */

class RendezvousTableModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    using QSqlQueryModel::QSqlQueryModel;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

//Classe (entité) : Rendezvous
class Rendezvous
{
private:

    int id = 0;
    QDate date;
    QString heure;
    QString statut;
    QString motif;
    int idAnimal = 0;
    int idEmploye = 0;
    QString nomAnimal;
    mutable QString m_erreur;

    static QString messageSql(const QString &texteErreur);
    bool creneauOccupe() const;

public:
    // Constructeurs
    Rendezvous() {}
    Rendezvous(int, QDate, QString, QString, QString, int);

    //Destructeur
    ~Rendezvous();

    //Getters
    int getId() const { return id; }
    QDate getDate() const { return date; }
    QString getHeure() const { return heure; }
    QString getStatut() const { return statut; }
    QString getMotif() const { return motif; }
    int getIdAnimal() const { return idAnimal; }
    int getIdEmploye() const { return idEmploye; }
    QString getNomAnimal() const { return nomAnimal; }

    // ---- Setters ----
    void setId(int id) { this->id = id; }
    void setDate(QDate date) { this->date = date; }
    void setHeure(QString heure) { this->heure = heure; }
    void setStatut(QString statut) { this->statut = statut; }
    void setMotif(QString motif) { this->motif = motif; }
    void setIdAnimal(int idAnimal) { this->idAnimal = idAnimal; }
    void setIdEmploye(int idEmploye) { this->idEmploye = idEmploye; }

    //CRUD
    bool ajouter();                      // Create
    bool modifier();                     // Update
    bool supprimer(int);                 // Delete
    QSqlQueryModel *afficher() const;    // Read (liste)
    bool charger(int);                   // Read (un seul rendez-vous)

    //Métiers : recherche par ID et filtre de statut et tri
    QSqlQueryModel *rechercher(const QString &recherche, const QString &statutFiltre, int tri) const;

    //Contrôle de saisie (en C++)
    bool valider(bool creation);
    QString erreur() const { return m_erreur; }

    //Outils
    static int prochainId();
    static bool existe(int id);
    static bool animalExiste(int idAnimal);
    static QStringList motifsDisponibles();
    static QStringList statutsDisponibles();
    static QStringList criteresTri();                // libellés de la liste « Trier par »
    static QString formaterDate(const QDate &d);
    static QString formaterId(int id);               // 1 -> « RDV001 »
};

#endif // RENDEZVOUS_H