#ifndef COMMANDEVUE_H
#define COMMANDEVUE_H

#include <QIcon>
#include <QList>
#include <QMainWindow>

#include "commande.h"

class QAbstractItemModel;
class QModelIndex;
class QPushButton;

QT_BEGIN_NAMESPACE
namespace Ui { class CommandeVue; }
QT_END_NAMESPACE

// =====================================================================
//  VUE du module Commandes : affiche les données et gère les interactions.
//  Aucune requête dans la fenêtre : chaque bouton appelle une méthode
//  de l'instance « c » (classe Commande).
// =====================================================================
class CommandeVue : public QMainWindow
{
    Q_OBJECT

public:
    explicit CommandeVue(QWidget *parent = nullptr);
    ~CommandeVue();

private slots:
    // ---- Page Commandes (slots reliés automatiquement aux widgets du .ui) ----
    void on_btnAjouterCommande_clicked();        // « + Ajouter » : ouvre le formulaire
    void on_btnEnregistrerCommande_clicked();    // ajout ou modification selon le mode
    void on_btnAnnulerCommande_clicked();        // ferme le formulaire
    void on_btnExporterCommande_clicked();       // export PDF des détails
    void on_tableCommande_clicked(const QModelIndex &index);   // Read : détails de la commande cliquée
    void on_txtRechercheCommande_textChanged(const QString &texte);        // métier : recherche par ID
    void on_cmbStatutCommande_currentIndexChanged(int index);              // métier : filtre de statut
    void on_cmbTriCommande_currentIndexChanged(int index);                 // métier : tri

private:
    enum class ModeFormulaire { Ajout, Modification };

    Ui::CommandeVue *ui;
    Commande c;                      // instance du modèle : c.ajouter(), c.afficher(), c.modifier(), c.supprimer(id)
    Commande m_details;              // commande affichée dans « Détails d'une commande »
    ModeFormulaire m_mode = ModeFormulaire::Ajout;
    int m_idEnCours = 0;             // commande en cours de modification
    QAbstractItemModel *m_modeleCommandes = nullptr;
    QList<QPushButton *> boutonsNav;

    // Navigation (menu latéral commun à toute l'application)
    void configurerNavigation();
    void changerPage(int index);
    static QIcon icone(const QString &nom);

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
};

#endif // COMMANDEVUE_H