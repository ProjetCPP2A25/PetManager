#ifndef GPETMANAGER_H
#define GPETMANAGER_H

#include <QMainWindow>
#include <QPointer>
#include <QFrame>
#include <QIcon>
#include <QMap>
#include <QList>
#include <functional>

#include "produit.h"

class QLabel;
class QPushButton;
class QTimer;
class QAbstractItemView;

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

    void changerPage(int index);
    void rafraichir();
    void afficherTableau();                     // Read
    void afficherNotifications();

private:
    Ui::GPetManager *ui;
    Produit Etmp;                    // objet temporaire pour appeler les méthodes du modèle (seule classe du module)

    int idEnEdition = -1;            // produit chargé dans le formulaire (-1 = ajout)
    QMap<int, QString> fournisseurs; // IDPRODUIT -> fournisseur (via les commandes)
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
    void chargerFournisseurs();

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
};

#endif // GPETMANAGER_H
