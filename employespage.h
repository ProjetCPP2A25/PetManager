#ifndef EMPLOYESPAGE_H
#define EMPLOYESPAGE_H

#include <QMainWindow>
#include <QMessageBox>
#include "employe.h"
#include "employedao.h"
#include <QList>

// Includes pour les graphiques Qt Charts
#include <QChart>
#include <QChartView>
#include <QPieSeries>
#include <QPieSlice>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QPainter>
#include <QRandomGenerator>

QT_BEGIN_NAMESPACE
namespace Ui { class EmployesPage; }
QT_END_NAMESPACE

// Page "Gestion des employés" (intégrée dans la fenêtre principale)
class EmployesPage : public QMainWindow {
    Q_OBJECT

public:
    explicit EmployesPage(QWidget *parent = nullptr);
    ~EmployesPage();

private slots:
    // Navigation Globale (Pour l'intégration future dans SmartMarket)
    void on_btnAccueil_clicked();
    void on_btnAnimaux_clicked();
    void on_btnRdv_clicked();
    void on_btnStock_clicked();
    void on_btnCommandes_clicked();
    void on_btnFournisseurs_clicked();
    void on_btnEmployes_clicked();
    void on_btnParametres_clicked();

    // Navigation Module Employé
    void on_btnLogin_clicked();
    void on_btnLogout_clicked();

    // CRUD
    void on_btnAjouter_clicked();
    void on_btnModifier_clicked();
    void on_btnSupprimer_clicked();
    void on_btnEnregistrer_clicked();
    void on_btnAnnuler_clicked();

    // Sélection dans le tableau
    void on_tblEmployes_itemSelectionChanged();

    // Recherche et tri
    void on_txtRecherche_textChanged(const QString &arg1);
    void on_cmbTri_currentIndexChanged(int index);

    // Export
    void on_btnExporterPDF_clicked();
    void on_btnExporterExcel_clicked();

    // Fonctionnalités innovantes
    void on_btnGenererPlanning_clicked();
    void on_btnVerifierAlertes_clicked();

private:
    Ui::EmployesPage *ui;
    void chargerTableau();
    void chargerFormulaire(const Employe &emp);
    void viderFormulaire();
    void mettreAJourStatistiques();

    QList<Employe> m_employes;
    int m_currentId;
    bool m_isEditing;
};
#endif // EMPLOYESPAGE_H