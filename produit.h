#ifndef PRODUIT_H
#define PRODUIT_H

#include <QString>
#include <QDate>
#include <QList>
#include <QMap>
#include <QPair>
#include <QStringList>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDateTime>

// ---------------------------------------------------------------------
//  Simples regroupements de données (sans méthodes) utilisés par les
//  métiers de la classe Produit.
// ---------------------------------------------------------------------

// Paramètres du serveur d'envoi des alertes (SMTP)
struct ConfigEmail {
    QString serveur = "smtp.gmail.com";
    int port = 465;                    // 465 = SSL direct, 587 = STARTTLS
    QString utilisateur;               // adresse de l'expéditeur
    QString motDePasse;                // mot de passe d'application
    QString destinataire;              // responsable du stock (plusieurs : séparés par ;)
};

// Une ligne de l'historique des alertes envoyées
struct EntreeHistorique {
    QDateTime date;
    QString destinataire;
    QString produits;
    bool automatique = false;
    bool succes = false;
    QString detail;
};

// Une alternative proposée pour un produit recherché ou indisponible
struct SuggestionIA {
    int idProduit = 0;
    QString nom;
    QString categorie;
    int quantite = 0;
    int pertinence = 0;          // 0 à 100 %
    QString raison;              // pourquoi ce produit peut remplacer le besoin
    QString avantages;           // points forts / précautions
};

// =====================================================================
//  Classe purement C++ (MODÈLE) de l'entité Produit -> table STOCK (MR commun)
//  C'est la SEULE classe du module Stock : CRUD, métiers basiques (recherche,
//  tri, fiche de commande Excel, statistiques) et métiers innovants (alerte
//  e-mail, alternatives proposées par l'IA) sont tous écrits ICI.
//  Toutes les requêtes SQL sont préparées : aucune requête derrière un bouton.
// =====================================================================
class Produit
{
    int idProduit;
    QString nom;
    QString categorie;
    int quantite;
    int seuilMin;
    double prixUnitaire;
    QDate dateExpiration;
    QString emplacement;
    QString statut;

public:
    // Constructeurs
    Produit();
    Produit(int idProduit, QString nom, QString categorie, int quantite, int seuilMin,
            double prixUnitaire, QDate dateExpiration, QString emplacement);

    // Getters
    int getIdProduit() const          { return idProduit; }
    QString getNom() const            { return nom; }
    QString getCategorie() const      { return categorie; }
    int getQuantite() const           { return quantite; }
    int getSeuilMin() const           { return seuilMin; }
    double getPrixUnitaire() const    { return prixUnitaire; }
    QDate getDateExpiration() const   { return dateExpiration; }
    QString getEmplacement() const    { return emplacement; }
    QString getStatut() const         { return statut; }

    // Setters
    void setIdProduit(int id)               { idProduit = id; }
    void setNom(const QString &n)           { nom = n; }
    void setCategorie(const QString &c)     { categorie = c; }
    void setQuantite(int q)                 { quantite = q; }
    void setSeuilMin(int s)                 { seuilMin = s; }
    void setPrixUnitaire(double p)          { prixUnitaire = p; }
    void setDateExpiration(const QDate &d)  { dateExpiration = d; }
    void setEmplacement(const QString &e)   { emplacement = e; }

    // ---------------- Fonctionnalités de base (CRUD) ----------------
    bool ajouter();                                              // Create
    QSqlQueryModel *afficher(const QString &rechercheNom = QString(),
                             int triExpiration = 0);             // Read (+ recherche, tri)
    bool modifier();                                             // Update
    bool supprimer(int id);                                      // Delete

    // ---------------- Compléments du CRUD ----------------
    bool chargerParId(int id);                     // remplit l'objet depuis la BD
    bool existe(int id);
    bool nomExiste(const QString &nomProduit, int idExclu);
    int prochainId();
    bool retirerQuantite(int id, int quantiteRetiree);   // sortie de stock + consommation
    bool actualiserStatuts();                             // recalcule STATUT (dates)
    QStringList controleSaisie(bool nouveau) const;       // contrôle de saisie
    QString derniereErreur() const { return erreur; }

    static QString calculerStatut(int quantite, int seuilMin, const QDate &dateExpiration);
    static QString libelleStatut(const QString &statutBD);   // "Expire" -> "Expiré"
    static QStringList categories();
    static int quantiteRecommandee(int quantite, int seuilMin, int facteur);

    // ---------------- Statistiques ----------------
    int nombreProduits();
    double valeurStock();
    int nombreSousSeuil();
    QMap<QString, int> repartitionParCategorie();
    QMap<int, QString> fournisseurParProduit();  // IDPRODUIT -> fournisseur (dernière commande)
    QList<Produit> listeTous();
    QMap<QDate, int> consommationParJour(int id, const QDate &debut);
    QList<QPair<int, QString>> produitsLesPlusConsommes(const QDate &debut, int nombre);

    // ---------------- Métier basique : fiche de commande (Excel .xlsx) ----------------
    static QString numeroCommande();
    static bool genererFicheCommande(const QString &chemin, const QList<Produit> &produits,
                                     const QMap<int, QString> &fournisseurs, int facteur,
                                     const QString &numero, QString &erreurFiche);

    // ---------------- Métier 1 : alerte de réapprovisionnement par e-mail ----------------
    QList<Produit> listeSousSeuil();

    // Paramètres (fichier petmanager.ini à côté de l'exécutable)
    static ConfigEmail configurationEmail();
    static bool configurationComplete(const ConfigEmail &config);
    static bool envoiAutomatique();
    static int intervalleMinutes();
    static int facteurStockCible();
    static QString cleApiIA();
    static QString modeleIA();
    static void enregistrerParametres(const ConfigEmail &config, bool automatique, int intervalle,
                                      int facteur, const QString &cleIA, const QString &modele);
    static void enregistrerSurveillance(int intervalle, int facteur);

    // Surveillance : un produit n'est signalé qu'une fois tant qu'il reste sous le seuil
    static void actualiserSuivi(const QList<Produit> &sousSeuil);
    static QList<Produit> produitsNonSignales(const QList<Produit> &sousSeuil);
    static QDateTime derniereAlerte(int idProduit);

    // Envoi + historique
    static bool envoyerAlerte(const QList<Produit> &produits, const QMap<int, QString> &fournisseurs,
                              bool automatique, QString &erreurEnvoi);
    static bool envoyerTest(const ConfigEmail &config, QString &erreurEnvoi);
    static QList<EntreeHistorique> historiqueAlertes();
    static void viderHistoriqueAlertes();

    // ---------------- Métier 2 : alternatives proposées par l'IA ----------------
    QList<Produit> catalogueDisponible();         // quantité > 0 et non expiré
    static bool iaConfiguree();
    // Analyse le besoin avec l'IA (Google Gemini) ; sans clé ou sans Internet,
    // le moteur local (mots-clés + catégorie) prend le relais.
    QList<SuggestionIA> suggererAlternatives(const QString &besoin, const QString &categorieBesoin,
                                             int idExclu, QString &conseil, bool &viaIA,
                                             QString &avertissement);
    static QList<SuggestionIA> suggestionsLocales(const QString &besoin, const QString &categorieBesoin,
                                                  int idExclu, const QList<Produit> &catalogue);

private:
    QString erreur;
    static Produit depuisRequete(const QSqlQuery &q);

    // Outils internes des métiers
    static QString fichierParametres();
    static bool envoyerEmail(const ConfigEmail &config, const QString &sujet,
                             const QString &html, QString &erreurEnvoi);
    static QString construireHtmlAlerte(const QList<Produit> &produits,
                                        const QMap<int, QString> &fournisseurs);
    static void ajouterHistorique(const EntreeHistorique &entree);
    static QString construirePromptIA(const QString &besoin, const QString &categorieBesoin,
                                      int idExclu, const QList<Produit> &catalogue);
    static bool analyserReponseIA(const QByteArray &json, const QList<Produit> &catalogue, int idExclu,
                                  QList<SuggestionIA> &suggestions, QString &conseil, QString &erreurIA);
};

#endif // PRODUIT_H
