#ifndef GPETMANAGER_H
#define GPETMANAGER_H

#include <QMainWindow>
#include <QPointer>
#include <QFrame>
#include <QIcon>
#include <QMap>
#include <QList>
#include <functional>
#include <QAbstractItemModel>   // module Commandes : modèle du tableau, QModelIndex

#include "produit.h"
#include "commande.h"
#include "rendezvous.h"

class QLabel;
class QPushButton;
class QTimer;
class QAbstractItemView;
class MainWindow;               // interface du module Animaux (mainwindow.h)

QT_BEGIN_NAMESPACE
namespace Ui { class GPetManager; }
QT_END_NAMESPACE

// =====================================================================
//  VUE du module Stock : affiche les données et gère les interactions.
//  Aucune requête SQL ici : tout passe par la classe Produit (modèle).
// =====================================================================
class GPetManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit GPetManager(QWidget *parent = nullptr);
    ~GPetManager();
    void demarrerSurveillance();      // 1er contrôle du stock, appelé par main()

protected:
    bool eventFilter(QObject *objet, QEvent *evenement) override;   // clic sur une carte IA

private slots:
    // =====================================================================
    // === SLOTS GÉNÉRAUX (NAVIGATION ENTRE PAGES) =========================
    // =====================================================================
    // 💡 Navigation commune à tous les modules
    void changerPage(int index);

    // =====================================================================
    // === MODULE STOCK ====================================================
    // =====================================================================
    // 💡 Étudiante responsable du module Stock (Chahed Matoussi) travaille ici uniquement
    // ---- CRUD (slots reliés automatiquement aux boutons du .ui) ----
    void on_btnEnregistrerStock_clicked();      // Ajouter  (Create)
    void on_btnModifierStock_clicked();         // Modifier (Update)
    void on_btnSupprimerStock_clicked();        // Supprimer (Delete)
    void on_btnAnnulerStock_clicked();
    void on_btnExcelStock_clicked();

    // ---- Métier 1 : alerte e-mail de réapprovisionnement ----
    void on_btnVerifierStock_clicked();
    void on_btnEnvoyerAlerteStock_clicked();
    void on_btnExcelSuggestionsStock_clicked();
    void on_btnEnregistrerConfigStock_clicked();
    void on_btnTesterEmailStock_clicked();
    void on_btnViderHistoriqueStock_clicked();

    // ---- Métier 2 : alternatives proposées par l'IA ----
    void on_btnDemanderIAStock_clicked();

    void rafraichir();
    void afficherTableau();                     // Read
    void afficherNotifications();

    // =====================================================================
    // === MODULE ANIMAUX ==================================================
    // =====================================================================
    // 💡 Étudiant(e) responsable du module Animaux écrit ici ses slots

    // =====================================================================
    // === MODULE RENDEZ-VOUS ==============================================
    // =====================================================================
    // 💡 Étudiant(e) responsable du module Rendez-vous écrit ici ses slots
    // ---- Page Rendez-vous (slots reliés automatiquement aux widgets du .ui) ----
    void on_btnAjouterRdv_clicked();             // « + Ajouter » : ouvre le formulaire
    void on_btnEnregistrerRdv_clicked();         // ajout ou modification selon le mode
    void on_btnAnnulerRdv_clicked();             // ferme le formulaire
    void on_btnExporterRdv_clicked();            // export PDF des détails
    void on_tableRdv_clicked(const QModelIndex &index);        // Read : détails du rendez-vous cliqué
    void on_txtRechercheRdv_textChanged(const QString &texte); // métier : recherche par ID
    void on_cmbStatutRdv_currentIndexChanged(int index);       // métier : filtre de statut
    void on_cmbTriRdv_currentIndexChanged(int index);          // métier : tri

    // =====================================================================
    // === MODULE COMMANDES ================================================
    // =====================================================================
    // 💡 Étudiant(e) responsable du module Commandes écrit ici ses slots
    // ---- Page Commandes (slots reliés automatiquement aux widgets du .ui) ----
    void on_btnAjouterCommande_clicked();        // « + Ajouter » : ouvre le formulaire
    void on_btnEnregistrerCommande_clicked();    // ajout ou modification selon le mode
    void on_btnAnnulerCommande_clicked();        // ferme le formulaire
    void on_btnExporterCommande_clicked();       // export PDF des détails
    void on_tableCommande_clicked(const QModelIndex &index);   // Read : détails de la commande cliquée
    void on_txtRechercheCommande_textChanged(const QString &texte);        // métier : recherche par ID
    void on_cmbStatutCommande_currentIndexChanged(int index);              // métier : filtre de statut
    void on_cmbTriCommande_currentIndexChanged(int index);                 // métier : tri

    // =====================================================================
    // === MODULE FOURNISSEURS =============================================
    // =====================================================================
    // 💡 Étudiant(e) responsable du module Fournisseurs écrit ici ses slots

    // =====================================================================
    // === MODULE EMPLOYES =================================================
    // =====================================================================
    // 💡 Étudiant(e) responsable du module Employés écrit ici ses slots

private:
    Ui::GPetManager *ui;

    // =====================================================================
    // === MODULE STOCK : attributs et méthodes privées ====================
    // =====================================================================
    // 💡 Module Stock (Chahed Matoussi)
    Produit Ptmp;                    // objet temporaire pour appeler les méthodes du modèle (seule classe du module)

    int idEnEdition = -1;            // produit chargé dans le formulaire (-1 = ajout)
    QList<SuggestionIA> suggestionsIA;
    QString besoinIA;
    bool echecEnvoiAuto = false;     // évite de renvoyer en boucle après un échec
    bool iaEnCours = false;          // une seule demande à l'IA à la fois

    QList<QPushButton*> boutonsNav;
    QLabel *badgeNotif = nullptr;
    QPointer<QFrame> toast;
    QTimer *timerToast = nullptr;
    QTimer *timerSurveillance = nullptr;
    // Graphiques dessinés (QPainter) dans des QLabel
    QLabel *graphConso = nullptr;
    QLabel *graphCategories = nullptr;
    QStringList consoEtiquettes, consoNoms;
    QList<QList<double>> consoValeurs;
    QList<double> categoriesNombres;

    // Configuration
    void configurerNavigation();
    void configurerTableau();
    void configurerFormulaire();
    void configurerPageAlertes();
    void configurerGraphiques();
    void chargerParametres();

    // Affichage page Stock
    void decorerTableau();
    void mettreAJourCartes();
    void mettreAJourAlertes();
    void chargerListeGraphique();
    void mettreAJourGraphiques();
    void dessinerGraphiques();
    void genererFicheCommande();

    // Formulaire / CRUD
    bool lireFormulaire(Produit &p, bool nouveau);
    void chargerDansFormulaire(int id);
    void viderFormulaire();
    void mettreAJourModeFormulaire();
    void mettreAJourStatutFormulaire();
    void supprimerProduit(int id);
    void sortieDeStock(int id);
    int idSelectionne() const;

    // Métier 1
    void verifierStock(bool automatique);
    bool envoyerAlerte(const QList<Produit> &produits, bool automatique);
    void afficherSousSeuil();
    void afficherHistorique();
    ConfigEmail lireConfigEmail() const;

    // Métier 2
    void demanderAlternatives(const QString &besoin, const QString &categorie, int idExclu);
    void afficherSuggestionsIA(const QList<SuggestionIA> &suggestions, const QString &conseil,
                               bool viaIA, const QString &avertissement);

    // Outils d'affichage
    void afficherToast(const QString &nomIcone, const QString &titre, const QString &message,
                       const QString &couleur, const QString &texteBouton = QString(),
                       std::function<void()> action = nullptr);
    QWidget *creerBadge(const QString &texte, const QString &fond, const QString &couleur);
    QWidget *creerActions(int id);
    QPushButton *creerBoutonIcone(const QString &icone, const QString &infobulle);
    static void couleursCategorie(const QString &categorie, QString &fond, QString &couleur);
    static QString couleurStatut(const QString &statutBD);
    QIcon iconeCategorie(const QString &categorie) const;
    static QIcon icone(const QString &nom);
    void preparerTableau(QAbstractItemView *vue);
    void ajusterHauteur(QAbstractItemView *vue, int lignes, int hauteurLigne = 42);

    // =====================================================================
    // === MODULE ANIMAUX : attributs et méthodes privées ==================
    // =====================================================================
    // 💡 Module Animaux (ex. objet temporaire de sa classe métier)
    MainWindow *animauxVue = nullptr;   // interface Animaux (CRUD Oracle + statistiques), placée dans la page « Animaux »

    // =====================================================================
    // === MODULE RENDEZ-VOUS : attributs et méthodes privées ==============
    // =====================================================================
    // 💡 Module Rendez-vous
    enum class ModeFormulaireRdv { Ajout, Modification };

    Rendezvous r;                    // instance du modèle : r.ajouter(), r.afficher(), r.modifier(), r.supprimer(id)
    Rendezvous m_detailsRdv;         // rendez-vous affiché dans « Détails du rendez-vous »
    ModeFormulaireRdv m_modeRdv = ModeFormulaireRdv::Ajout;
    int m_idEnCoursRdv = 0;          // rendez-vous en cours de modification
    QAbstractItemModel *m_modeleRdv = nullptr;

    // Page Rendez-vous
    void configurerPageRendezVous();
    void rafraichirRdv();                        // recharge le tableau
    void viderFormulaireRdv();
    void fermerFormulaireRdv();
    void ouvrirEditionRdv(int id);
    void afficherDetailsRdv(int id);
    void viderDetailsRdv();
    void actionSurLigneRdv(int ligne, int action);   // 0 : crayon (modifier), 1 : poubelle (supprimer)
    void supprimerRdv(int id);

    // =====================================================================
    // === MODULE COMMANDES : attributs et méthodes privées ================
    // =====================================================================
    // 💡 Module Commandes
    enum class ModeFormulaire { Ajout, Modification };

    Commande c;                      // instance du modèle : c.ajouter(), c.afficher(), c.modifier(), c.supprimer(id)
    Commande m_details;              // commande affichée dans « Détails d'une commande »
    ModeFormulaire m_mode = ModeFormulaire::Ajout;
    int m_idEnCours = 0;             // commande en cours de modification
    QAbstractItemModel *m_modeleCommandes = nullptr;

    // Page Commandes
    void configurerPageCommandes();
    void rafraichirCommandes();                  // recharge le tableau
    void viderFormulaireCommande();
    void fermerFormulaireCommande();
    void ouvrirEditionCommande(int id);
    void afficherDetailsCommande(int id);
    void viderDetailsCommande();
    void actionSurLigneCommande(int ligne, int action);   // 0 : crayon (modifier), 1 : poubelle (supprimer)
    void supprimerCommande(int id);

    // =====================================================================
    // === MODULE FOURNISSEURS : attributs et méthodes privées =============
    // =====================================================================
    // 💡 Module Fournisseurs (ex. objet temporaire de sa classe métier)

    // =====================================================================
    // === MODULE EMPLOYES : attributs et méthodes privées =================
    // =====================================================================
    // 💡 Module Employés (ex. objet temporaire de sa classe métier)
};

#endif // GPETMANAGER_H
