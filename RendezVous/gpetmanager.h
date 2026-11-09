#ifndef GPETMANAGER_H
#define GPETMANAGER_H

#include <QIcon>
#include <QList>
#include <QMainWindow>

#include "rendezvous.h"

class QAbstractItemModel;
class QModelIndex;
class QPushButton;

QT_BEGIN_NAMESPACE
namespace Ui { class GPetManager; }
QT_END_NAMESPACE

class GPetManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit GPetManager(QWidget *parent = nullptr);
    ~GPetManager();

private slots:
    //Page Rendez-vous (slots reliés automatiquement aux widgets du .ui)
    void on_btnAjouterRdv_clicked();        //Ajouter : ouvre le formulaire
    void on_btnEnregistrerRdv_clicked();    //ajout ou modification selon le mode
    void on_btnAnnulerRdv_clicked();        //ferme le formulaire
    void on_btnExporterRdv_clicked();       //export PDF des détails
    void on_tableRdv_clicked(const QModelIndex &index);   // détails du rendez-vous cliqué
    void on_txtRechercheRdv_textChanged(const QString &texte);        //recherche par ID
    void on_cmbStatutRdv_currentIndexChanged(int index);              //filtre de statut
    void on_cmbTriRdv_currentIndexChanged(int index);                 //tri

private:
    enum class ModeFormulaire { Ajout, Modification };

    Ui::GPetManager *ui;
    Rendezvous r;                    // instance du modèle : r.ajouter(), r.afficher(), r.modifier(), r.supprimer()
    Rendezvous m_details;            // rendez-vous affiché dans « Détails du rendez-vous »
    ModeFormulaire m_mode = ModeFormulaire::Ajout;
    int m_idEnCours = 0;             // rendez-vous en cours de modification
    QAbstractItemModel *m_modeleRdv = nullptr;
    QList<QPushButton *> boutonsNav;

    // Navigation
    void configurerNavigation();
    void changerPage(int index);
    static QIcon icone(const QString &nom);

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
};

#endif // GPETMANAGER_H