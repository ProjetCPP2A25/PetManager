#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QResizeEvent>
#include "fournisseur.h"
#include <QList>
#include <QChartView>
#include <QChart>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>

// --- Module Stock ---
#include <QDate>
#include <QDateTime>
#include <QStringList>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QPointer>
#include <QFrame>
#include <QIcon>
#include <functional>

class QLabel;
class QPushButton;
class QTimer;
class QTableWidget;
class GraphiqueCourbes;
class GraphiqueCamembert;
class AnimauxPage;              // --- Module Animaux (intégration)
class AccueilPage;              // --- Pages Connexion / Accueil / Paramètres (intégration)
class ConnexionPage;
class EmployesPage;
class ParametresPage;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// =====================================================================
//                  Structures du module Stock
// =====================================================================

struct ProduitStock {
    QString nom;
    QString categorie;
    QString espece;                  // "Chien", "Chat" ou "Toutes"
    QString usage;                   // "Nutrition", "Soin", "Vaccination", ...
    int quantite = 0;
    int seuil = 0;
    QDate dateExpiration;
    QString fournisseur;
    double prix = 0;
    QMap<QDate, int> consommation;   // quantité utilisée par jour
    QMap<QDate, int> demandeNonSatisfaite;  // demandée mais remplacée / non servie
};

// Alternative proposée pour un produit indisponible
struct AlternativeStock {
    int index = -1;                  // indice du produit alternatif
    int score = 0;                   // compatibilité de 0 à 100 %
    QStringList raisons;             // critères qui expliquent le score
};

// Trace d'une substitution validée par l'utilisateur
struct SubstitutionStock {
    QDateTime date;
    QString produitDemande;
    QString alternative;
    int quantite = 0;
    int score = 0;
    QString motif;
};

struct CommandeStock {
    int numero = 0;
    QString produit;                 // nom du produit commandé
    int quantite = 0;
    QString fournisseur;
    QDate dateCommande;
    QDate dateLivraison;             // prévue (ou réelle une fois reçue)
    double montant = 0;
    QString statut;                  // "En attente", "Reçue", "Annulée"
};

// Résultat de l'analyse prédictive d'un produit
struct AnalyseStock {
    double consoMoyenne = 0;         // unités par jour (30 derniers jours)
    double joursRestants = -1;       // -1 = inconnu (aucune consommation)
    QDate dateRupture;               // date prévue où le stock sera inutilisable
    int priorite = 0;                // 0 aucune, 1 préventive, 2 urgente, 3 critique
    int qteConseillee = 0;
    bool commandeEnCours = false;
};


class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void resizeEvent(QResizeEvent *event) override;
    // Module Stock : clic sur une carte "alternative"
    bool eventFilter(QObject *objet, QEvent *evenement) override;

private slots:
    void on_btn_enregistrer_clicked();
    void on_btn_modifier_clicked();
    void on_btn_annuler_clicked();
    void on_btn_supprimer_clicked(); // Added
    void on_lineEdit_recherche_textChanged(const QString &arg1);
    void on_comboBox_tri_currentIndexChanged(int index);
    void on_btn_export_pdf_clicked();
    void on_btn_export_excel_clicked();
    void on_comboBox_chart_currentIndexChanged(int index);
    void supprimerFournisseur(int id);
    void chargerFormulaire(int id, bool readOnly = false);

    // Navigation entre les modules (menu latéral)
    void on_btn_menu_fournisseurs_clicked();
    void on_btn_menu_stock_clicked();
    void on_btn_menu_animaux_clicked();     // --- Module Animaux (intégration)
    void on_btn_menu_accueil_clicked();
    void on_btn_menu_employes_clicked();
    void on_btn_menu_parametres_clicked();

private:
    Ui::MainWindow *ui;
    void chargerTableau();
    void appliquerFiltreEtTri();
    void remplirTableau(const QList<Fournisseur> &liste);
    void viderFormulaire();
    void mettreAJourKPIs(const QList<Fournisseur> &liste);
    void mettreAJourGraphique(const QList<Fournisseur> &liste);
    void mettreAJourRecommandations(const QList<Fournisseur> &liste);

    enum FormMode { AddMode, EditMode, ViewMode };
    void setFormMode(FormMode mode);

    void showMessageBox(const QString &title, const QString &text, QMessageBox::Icon icon);
    bool showConfirmBox(const QString &text);
    bool validateForm(QString &errorMsg, QWidget *&focusWidget);

    QList<Fournisseur> m_fournisseurs;
    int m_currentId;
    QChartView *m_chartView = nullptr;

    AnimauxPage *pageAnimaux = nullptr;     // --- Module Animaux (intégration)
    ConnexionPage *pageConnexion = nullptr;
    EmployesPage *pageEmployes = nullptr;
    AccueilPage *pageAccueil = nullptr;
    ParametresPage *pageParametres = nullptr;
    void configurerAccueilEtParametres();
    void synchroniserMenu(int index);
    void mettreAJourAccueil();
    void verrouillerMenu(bool verrouille);
    void appliquerLangueMenu();
    void afficherPageDemarrage();
    void afficherUtilisateurFournisseurs();
    void deconnecter();

    // =================================================================
    //                       MODULE GESTION DE STOCK
    // =================================================================
private slots:  // module Stock
    void changerPageStock(int index);
    void rafraichir();
    void enregistrerProduit();
    void modifierProduit();
    void supprimerProduit();
    void annulerEdition();
    void exporterExcel();
    void exporterCommandesExcel();
    void afficherNotifications();
    void commanderSelection();
    void toutSelectionner();

private:  // module Stock
    enum Etat { OK = 0, StockBas = 1, Rupture = 2, Expire = 3 };
    enum Priorite { Aucune = 0, Preventive = 1, Urgente = 2, Critique = 3 };

    QVector<ProduitStock> produits;
    QVector<CommandeStock> commandes;
    QVector<SubstitutionStock> substitutions;
    int indexRefAlternatives = -1;
    int prochainNumero = 1001;
    int indexEnEdition = -1;
    QVector<int> indexAlternatives;
    QLabel *badgeNotif = nullptr;
    QPointer<QFrame> toast;
    QTimer *timerToast = nullptr;

    GraphiqueCourbes   *graphConso = nullptr;
    GraphiqueCamembert *graphCategories = nullptr;

    // Configuration
    void initialiserModuleStock();
    void configurerNavigation();
    void configurerTableau();
    void configurerFormulaire();
    void configurerReappro();
    void configurerGraphiques();
    void chargerDonneesExemple();

    // Affichage page Stock
    void afficherTableau();
    void mettreAJourCartes();
    void mettreAJourAlertes();
    void mettreAJourAlternatives();
    void mettreAJourGraphiqueConso();
    void mettreAJourStatistiques();

    // Affichage page Réapprovisionnement
    void afficherSuggestions();
    void afficherCommandes();
    void afficherSubstitutions();

    // Formulaire / CRUD
    void chargerDansFormulaire(int index);
    void viderFormulaireStock();
    void mettreAJourModeFormulaire();
    bool lireFormulaire(ProduitStock &p, bool nouveau);
    void sortieDeStock(int index);

    // Métier : substitution par un produit alternatif
    QVector<AlternativeStock> chercherAlternatives(int indexProduit, int besoin) const;
    void ouvrirSubstitution(int indexProduit, int quantiteDemandee, int alternativePreselectionnee = -1);

    // Métier : réapprovisionnement intelligent
    Etat etatProduit(const ProduitStock &p) const;
    QString libelleEtat(Etat e) const;
    AnalyseStock analyserProduit(int index) const;
    QString libellePriorite(int priorite) const;
    QString couleurPriorite(int priorite) const;
    QString textePrevision(const AnalyseStock &a) const;
    QVector<int> indicesAReapprovisionner(bool inclureCommandes = false) const;
    QSet<QString> nomsEnAlerte() const;
    void signalerNouvellesAlertes(const QSet<QString> &avant);
    bool commandeEnAttente(const QString &nomProduit) const;
    int indiceProduit(const QString &nom) const;
    bool creerCommande(int indexProduit, int quantite, bool silencieux = false);
    void receptionnerCommande(int indexCommande);
    void annulerCommande(int indexCommande);

    // Notification "toast" en bas à droite
    void afficherToast(const QString &nomIcone, const QString &titre, const QString &message,
                       const QString &couleur, const QString &texteBouton = QString(),
                       std::function<void()> action = nullptr);

    // Outils d'affichage
    QWidget* creerBadge(const QString &texte, const QString &fond, const QString &couleur);
    QWidget* creerActions(int index);
    QIcon iconeCategorie(const QString &categorie) const;
    static QIcon icone(const QString &nom);
    void ajusterHauteurTableau(QTableWidget *t, int lignes);
};
#endif // MAINWINDOW_H
