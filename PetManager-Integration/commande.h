#ifndef COMMANDE_H
#define COMMANDE_H

#include <QDate>
#include <QSqlQueryModel>
#include <QString>
#include <QStringList>

// =====================================================================
//  MODÈLE : classe Commande (entité) + modèle d'affichage du tableau
// =====================================================================

/* Modèle d'affichage (lecture seule) : met en forme l'ID (C001) et les dates.
 * Colonnes : 0 ID | 1 Date commande | 2 Date livraison | 3 Statut | 4 Actions */
class CommandeTableModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    using QSqlQueryModel::QSqlQueryModel;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

/* Classe purement C++ : entité COMMANDE (couche MODÈLE).
 * Elle contient les attributs et toutes les méthodes de la commande.
 * La fenêtre (GPetManager) n'a qu'à appeler ces méthodes, par l'instance « c ». */
class Commande
{
private:
    // ---- Attributs (privés : on y accède par les getters / setters) ----
    int id = 0;
    QDate dateCommande;
    QDate dateLivraison;          // facultative (date invalide = NULL en base)
    QString statut;
    double montantTotal = 0;      // 0 = non défini
    int idFournisseur = 0;
    int idEmploye = 0;            // facultatif pour l'instant (0 -> NULL en base)
    int quantite = 0;
    QString nomFournisseur;       // saisi par l'utilisateur ; converti en idFournisseur
    mutable QString m_erreur;

    // ---- Méthode interne (réservée à la classe) ----
    static QString messageSql(const QString &texteErreur);

public:
    // ---- Constructeurs ----
    Commande() {}
    Commande(int, QDate, QDate, QString, QString, int);
    // ---- Destructeur ----
    ~Commande();

    // ---- Getters ----
    int getId() const { return id; }
    QDate getDateCommande() const { return dateCommande; }
    QDate getDateLivraison() const { return dateLivraison; }
    QString getStatut() const { return statut; }
    double getMontantTotal() const { return montantTotal; }
    int getIdFournisseur() const { return idFournisseur; }
    int getIdEmploye() const { return idEmploye; }
    int getQuantite() const { return quantite; }
    QString getNomFournisseur() const { return nomFournisseur; }

    // ---- Setters ----
    void setId(int id) { this->id = id; }
    void setDateCommande(QDate d) { this->dateCommande = d; }
    void setDateLivraison(QDate d) { this->dateLivraison = d; }
    void setStatut(QString s) { this->statut = s; }
    void setMontantTotal(double m) { this->montantTotal = m; }
    void setIdFournisseur(int f) { this->idFournisseur = f; }
    void setIdEmploye(int e) { this->idEmploye = e; }
    void setQuantite(int q) { this->quantite = q; }
    void setNomFournisseur(QString n) { this->nomFournisseur = n; }

    // ---- Fonctionnalités de base relatives à l'entité Commande (CRUD) ----
    bool ajouter();                 // Create
    bool modifier();                // Update
    bool supprimer(int);            // Delete
    QSqlQueryModel *afficher() const;   // Read (liste) - l'appelant détruit le modèle
    bool charger(int);              // Read (une seule commande) : remplit l'objet courant

    // ---- Métiers : recherche par ID, filtre de statut et tri ----
    QSqlQueryModel *rechercher(const QString &recherche, const QString &statutFiltre, int tri) const;

    // ---- Contrôle de saisie (en C++) ----
    bool valider(bool creation);
    QString erreur() const { return m_erreur; }   // dernier message d'erreur

    // ---- Outils ----
    static int prochainId();
    static bool existe(int id);
    static QStringList statutsDisponibles();
    static QStringList criteresTri();              // libellés de la liste « Trier par »
    static QString formaterDate(const QDate &d);
    static QString formaterId(int id);                           // 1 -> « C001 »

};

#endif // COMMANDE_H