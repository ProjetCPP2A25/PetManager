#include "employespage.h"
#include "ui_employespage.h"
#include "employedao.h"
#include <QFileDialog>
#include <QTextDocument>
#include <QPrinter>
#include <QTextStream>
#include <QStringConverter>
#include <QDate>
#include <QFile>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QLayout>
#include <QRandomGenerator>
#include <QMessageBox>

EmployesPage::EmployesPage(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::EmployesPage)
    , m_currentId(0)
    , m_isEditing(false)
{
    ui->setupUi(this);

    // --- Intégration dans l'application principale ---
    // 1) Le style du module (theme.qss d'origine) est appliqué à cette page uniquement
    QFile fichierStyle(":/styles/theme_employes.qss");
    if (fichierStyle.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(QString::fromUtf8(fichierStyle.readAll()));
        fichierStyle.close();
    }
    // 2) La connexion et le menu latéral sont ceux de l'application : on affiche directement la page des employés
    ui->stackedWidget->setCurrentWidget(ui->pageCRUD);
    ui->frameSidebar->hide();
    ui->menubar->hide();
    ui->statusbar->hide();

    // Configuration de la table
    ui->tblEmployes->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tblEmployes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tblEmployes->horizontalHeader()->setStretchLastSection(true);

    // Configuration des tableaux innovants
    ui->tblPlanning->horizontalHeader()->setStretchLastSection(true);
    ui->tblCompetences->horizontalHeader()->setStretchLastSection(true);

    // Date par défaut
    ui->dateEmbauche->setDate(QDate::currentDate());

    // Désactiver le formulaire au démarrage
    viderFormulaire();

    // Charger le logo depuis le dossier du projet
    QPixmap logoPixmap(":/resources/images/logo.png");
    if (logoPixmap.isNull())
        logoPixmap.load("logo.png");
    if (!logoPixmap.isNull() && ui->labelLogo) {
        ui->labelLogo->setPixmap(logoPixmap.scaled(160, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->labelLogo->setAlignment(Qt::AlignCenter);
    }

    // Masquer le badge de notification par défaut
    if (ui->labelNotifBadge) {
        ui->labelNotifBadge->setVisible(false);
    }

    // Charger les données
    chargerTableau();
    mettreAJourStatistiques();
}

EmployesPage::~EmployesPage() {
    delete ui;
}

// --- NAVIGATION GLOBALE (Placeholders pour l'intégration SmartMarket) ---
void EmployesPage::on_btnAccueil_clicked() { QMessageBox::information(this, "Navigation", "Module Accueil (Intégration SmartMarket)"); }
void EmployesPage::on_btnAnimaux_clicked() { QMessageBox::information(this, "Navigation", "Module Animaux (Intégration SmartMarket)"); }
void EmployesPage::on_btnRdv_clicked() { QMessageBox::information(this, "Navigation", "Module Rendez-vous (Intégration SmartMarket)"); }
void EmployesPage::on_btnStock_clicked() { QMessageBox::information(this, "Navigation", "Module Stock (Intégration SmartMarket)"); }
void EmployesPage::on_btnCommandes_clicked() { QMessageBox::information(this, "Navigation", "Module Commandes (Intégration SmartMarket)"); }
void EmployesPage::on_btnFournisseurs_clicked() { QMessageBox::information(this, "Navigation", "Module Fournisseurs (Intégration SmartMarket)"); }
void EmployesPage::on_btnEmployes_clicked() { /* Déjà sur cette page */ }
void EmployesPage::on_btnParametres_clicked() { QMessageBox::information(this, "Navigation", "Module Paramètres (Intégration SmartMarket)"); }

// --- NAVIGATION MODULE EMPLOYÉ ---
void EmployesPage::on_btnLogin_clicked() {
    if (ui->txtLoginUsername->text().isEmpty() || ui->txtLoginPassword->text().isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs");
        return;
    }
    ui->stackedWidget->setCurrentIndex(1);
}

void EmployesPage::on_btnLogout_clicked() {
    ui->stackedWidget->setCurrentIndex(0);
    ui->txtLoginUsername->clear();
    ui->txtLoginPassword->clear();
    ui->tblEmployes->clearSelection();
    viderFormulaire();
}

void EmployesPage::chargerTableau() {
    m_employes = EmployeDAO::obtenirTous();
    ui->tblEmployes->setRowCount(0);

    for (const Employe &emp : m_employes) {
        int row = ui->tblEmployes->rowCount();
        ui->tblEmployes->insertRow(row);
        ui->tblEmployes->setItem(row, 0, new QTableWidgetItem(QString::number(emp.getId())));
        ui->tblEmployes->setItem(row, 1, new QTableWidgetItem(emp.getNom()));
        ui->tblEmployes->setItem(row, 2, new QTableWidgetItem(emp.getPrenom()));
        ui->tblEmployes->setItem(row, 3, new QTableWidgetItem(emp.getEmail()));
        ui->tblEmployes->setItem(row, 4, new QTableWidgetItem(emp.getTelephone()));
        ui->tblEmployes->setItem(row, 5, new QTableWidgetItem(emp.getPoste()));
        ui->tblEmployes->setItem(row, 6, new QTableWidgetItem(QString::number(emp.getSalaire(), 'f', 3)));
        ui->tblEmployes->setItem(row, 7, new QTableWidgetItem(emp.getDateEmbauche().toString("dd/MM/yyyy")));
        ui->tblEmployes->setItem(row, 8, new QTableWidgetItem(emp.getStatut()));
    }
}

void EmployesPage::chargerFormulaire(const Employe &emp) {
    ui->txtNom->setText(emp.getNom());
    ui->txtPrenom->setText(emp.getPrenom());
    ui->txtEmail->setText(emp.getEmail());
    ui->txtTelephone->setText(emp.getTelephone());
    ui->cmbPoste->setCurrentText(emp.getPoste());
    ui->spnSalaire->setValue(emp.getSalaire());
    ui->dateEmbauche->setDate(emp.getDateEmbauche());
    ui->cmbStatut->setCurrentText(emp.getStatut());
}

void EmployesPage::viderFormulaire() {
    m_currentId = 0;
    ui->txtNom->clear();
    ui->txtPrenom->clear();
    ui->txtEmail->clear();
    ui->txtTelephone->clear();
    ui->cmbPoste->setCurrentIndex(0);
    ui->spnSalaire->setValue(0);
    ui->dateEmbauche->setDate(QDate::currentDate());
    ui->cmbStatut->setCurrentIndex(0);
    m_isEditing = false;

    ui->txtNom->setReadOnly(true);
    ui->txtPrenom->setReadOnly(true);
    ui->txtEmail->setReadOnly(true);
    ui->txtTelephone->setReadOnly(true);
    ui->cmbPoste->setEnabled(false);
    ui->spnSalaire->setEnabled(false);
    ui->dateEmbauche->setEnabled(false);
    ui->cmbStatut->setEnabled(false);
}

void EmployesPage::on_tblEmployes_itemSelectionChanged() {
    QList<QTableWidgetItem*> selected = ui->tblEmployes->selectedItems();
    if (selected.isEmpty()) {
        viderFormulaire();
        return;
    }
    int row = selected.first()->row();
    int id = ui->tblEmployes->item(row, 0)->text().toInt();
    m_currentId = id;

    Employe emp = EmployeDAO::obtenirParId(id);
    chargerFormulaire(emp);
    m_isEditing = false;

    ui->txtNom->setReadOnly(true);
    ui->txtPrenom->setReadOnly(true);
    ui->txtEmail->setReadOnly(true);
    ui->txtTelephone->setReadOnly(true);
    ui->cmbPoste->setEnabled(false);
    ui->spnSalaire->setEnabled(false);
    ui->dateEmbauche->setEnabled(false);
    ui->cmbStatut->setEnabled(false);
}

void EmployesPage::on_btnAjouter_clicked() {
    viderFormulaire();
    m_isEditing = false;
    m_currentId = 0;
    ui->tblEmployes->clearSelection();

    ui->txtNom->setReadOnly(false);
    ui->txtPrenom->setReadOnly(false);
    ui->txtEmail->setReadOnly(false);
    ui->txtTelephone->setReadOnly(false);
    ui->cmbPoste->setEnabled(true);
    ui->spnSalaire->setEnabled(true);
    ui->dateEmbauche->setEnabled(true);
    ui->cmbStatut->setEnabled(true);
    ui->txtNom->setFocus();
}

void EmployesPage::on_btnModifier_clicked() {
    if (m_currentId == 0) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner un employé à modifier");
        return;
    }
    m_isEditing = true;

    ui->txtNom->setReadOnly(false);
    ui->txtPrenom->setReadOnly(false);
    ui->txtEmail->setReadOnly(false);
    ui->txtTelephone->setReadOnly(false);
    ui->cmbPoste->setEnabled(true);
    ui->spnSalaire->setEnabled(true);
    ui->dateEmbauche->setEnabled(true);
    ui->cmbStatut->setEnabled(true);
    ui->txtNom->setFocus();
}

void EmployesPage::on_btnSupprimer_clicked() {
    if (m_currentId == 0) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner un employé à supprimer");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmation",
                                                              "Voulez-vous vraiment supprimer cet employé ?", QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (EmployeDAO::supprimer(m_currentId)) {
            QMessageBox::information(this, "Succès", "Employé supprimé avec succès");
            chargerTableau();
            mettreAJourStatistiques();
            viderFormulaire();
            ui->tblEmployes->clearSelection();
        } else {
            QMessageBox::critical(this, "Erreur", "Impossible de supprimer l'employé");
        }
    }
}

void EmployesPage::on_btnEnregistrer_clicked() {
    if (ui->txtNom->text().isEmpty() || ui->txtPrenom->text().isEmpty() ||
        ui->txtEmail->text().isEmpty() || ui->txtTelephone->text().isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs obligatoires (*)");
        return;
    }

    Employe emp;
    emp.setNom(ui->txtNom->text());
    emp.setPrenom(ui->txtPrenom->text());
    emp.setEmail(ui->txtEmail->text());
    emp.setTelephone(ui->txtTelephone->text());
    emp.setPoste(ui->cmbPoste->currentText());
    emp.setSalaire(ui->spnSalaire->value());
    emp.setDateEmbauche(ui->dateEmbauche->date());
    emp.setStatut(ui->cmbStatut->currentText());

    bool success;
    if (m_isEditing) {
        emp.setId(m_currentId);
        success = EmployeDAO::modifier(emp);
    } else {
        success = EmployeDAO::ajouter(emp);
    }

    if (success) {
        QMessageBox::information(this, "Succès", "Employé enregistré avec succès");
        chargerTableau();
        mettreAJourStatistiques();
        viderFormulaire();
        ui->tblEmployes->clearSelection();
    } else {
        QMessageBox::critical(this, "Erreur", "Impossible d'enregistrer l'employé (Email peut-être déjà utilisé)");
    }
}

void EmployesPage::on_btnAnnuler_clicked() {
    viderFormulaire();
    ui->tblEmployes->clearSelection();
}

void EmployesPage::on_txtRecherche_textChanged(const QString &arg1) {
    if (arg1.isEmpty()) {
        chargerTableau();
    } else {
        QList<Employe> results = EmployeDAO::rechercherParNom(arg1);
        ui->tblEmployes->setRowCount(0);
        for (const Employe &emp : results) {
            int row = ui->tblEmployes->rowCount();
            ui->tblEmployes->insertRow(row);
            ui->tblEmployes->setItem(row, 0, new QTableWidgetItem(QString::number(emp.getId())));
            ui->tblEmployes->setItem(row, 1, new QTableWidgetItem(emp.getNom()));
            ui->tblEmployes->setItem(row, 2, new QTableWidgetItem(emp.getPrenom()));
            ui->tblEmployes->setItem(row, 3, new QTableWidgetItem(emp.getEmail()));
            ui->tblEmployes->setItem(row, 4, new QTableWidgetItem(emp.getTelephone()));
            ui->tblEmployes->setItem(row, 5, new QTableWidgetItem(emp.getPoste()));
            ui->tblEmployes->setItem(row, 6, new QTableWidgetItem(QString::number(emp.getSalaire(), 'f', 3)));
            ui->tblEmployes->setItem(row, 7, new QTableWidgetItem(emp.getDateEmbauche().toString("dd/MM/yyyy")));
            ui->tblEmployes->setItem(row, 8, new QTableWidgetItem(emp.getStatut()));
        }
    }
}

void EmployesPage::on_cmbTri_currentIndexChanged(int index) {
    if (index == 1) {
        QList<Employe> liste = EmployeDAO::trierParPoste();
        ui->tblEmployes->setRowCount(0);
        for (const Employe &emp : liste) {
            int row = ui->tblEmployes->rowCount();
            ui->tblEmployes->insertRow(row);
            ui->tblEmployes->setItem(row, 0, new QTableWidgetItem(QString::number(emp.getId())));
            ui->tblEmployes->setItem(row, 1, new QTableWidgetItem(emp.getNom()));
            ui->tblEmployes->setItem(row, 2, new QTableWidgetItem(emp.getPrenom()));
            ui->tblEmployes->setItem(row, 3, new QTableWidgetItem(emp.getEmail()));
            ui->tblEmployes->setItem(row, 4, new QTableWidgetItem(emp.getTelephone()));
            ui->tblEmployes->setItem(row, 5, new QTableWidgetItem(emp.getPoste()));
            ui->tblEmployes->setItem(row, 6, new QTableWidgetItem(QString::number(emp.getSalaire(), 'f', 3)));
            ui->tblEmployes->setItem(row, 7, new QTableWidgetItem(emp.getDateEmbauche().toString("dd/MM/yyyy")));
            ui->tblEmployes->setItem(row, 8, new QTableWidgetItem(emp.getStatut()));
        }
    } else {
        chargerTableau();
    }
}

void EmployesPage::mettreAJourStatistiques() {
    m_employes = EmployeDAO::obtenirTous();

    // 1. Graphique en barres : Nombre d'employés par poste
    QMap<QString, int> countPoste;
    for (const Employe &emp : m_employes) countPoste[emp.getPoste()]++;

    QChart *chartPoste = new QChart();
    chartPoste->setTitle("Nombre d'employés par poste");
    QBarSeries *seriesPoste = new QBarSeries();
    QBarSet *setPoste = new QBarSet("Effectif");
    QStringList categoriesPoste;
    for (auto it = countPoste.constBegin(); it != countPoste.constEnd(); ++it) {
        categoriesPoste << it.key();
        *setPoste << it.value();
    }
    seriesPoste->append(setPoste);
    chartPoste->addSeries(seriesPoste);
    QBarCategoryAxis *axisXPoste = new QBarCategoryAxis();
    axisXPoste->append(categoriesPoste);
    chartPoste->addAxis(axisXPoste, Qt::AlignBottom);
    seriesPoste->attachAxis(axisXPoste);

    QValueAxis *axisYPoste = new QValueAxis();
    axisYPoste->setMin(0);
    int maxVal = 0;
    for (int v : countPoste.values()) {
        if (v > maxVal) maxVal = v;
    }
    axisYPoste->setMax(countPoste.isEmpty() ? 5 : maxVal + 1);
    chartPoste->addAxis(axisYPoste, Qt::AlignLeft);
    seriesPoste->attachAxis(axisYPoste);
    chartPoste->legend()->setVisible(false);

    QChartView *viewPoste = new QChartView(chartPoste);
    viewPoste->setRenderHint(QPainter::Antialiasing);

    QLayoutItem *child;
    while ((child = ui->layoutChartPoste->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    ui->layoutChartPoste->addWidget(viewPoste);

    // 2. Graphique circulaire : Répartition selon le statut
    QMap<QString, int> countStatut;
    for (const Employe &emp : m_employes) countStatut[emp.getStatut()]++;

    QChart *chartStatut = new QChart();
    chartStatut->setTitle("Répartition des employés selon leur statut");
    QPieSeries *seriesStatut = new QPieSeries();
    for (auto it = countStatut.constBegin(); it != countStatut.constEnd(); ++it) {
        QPieSlice *slice = seriesStatut->append(it.key(), it.value());
        slice->setLabelVisible(true);
        slice->setLabel(QString("%1 (%2)").arg(it.key()).arg(it.value()));
    }
    chartStatut->addSeries(seriesStatut);
    chartStatut->legend()->setVisible(true);
    chartStatut->legend()->setAlignment(Qt::AlignBottom);

    QChartView *viewStatut = new QChartView(chartStatut);
    viewStatut->setRenderHint(QPainter::Antialiasing);

    while ((child = ui->layoutChartStatut->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    ui->layoutChartStatut->addWidget(viewStatut);
}

void EmployesPage::on_btnGenererPlanning_clicked() {
    m_employes = EmployeDAO::obtenirTous();
    ui->tblPlanning->setRowCount(0);

    QRandomGenerator *rng = QRandomGenerator::global();

    for (const Employe &emp : m_employes) {
        int row = ui->tblPlanning->rowCount();
        ui->tblPlanning->insertRow(row);
        ui->tblPlanning->setItem(row, 0, new QTableWidgetItem(emp.getNomComplet()));
        ui->tblPlanning->setItem(row, 1, new QTableWidgetItem(emp.getPoste()));

        for (int j = 0; j < 5; ++j) {
            QString shift;
            if (emp.getStatut() == "En congé" || emp.getStatut() == "Inactif") {
                shift = "Congé / Absent";
            } else {
                int r = rng->bounded(3);
                if (r == 0) shift = "08h-16h";
                else if (r == 1) shift = "14h-22h";
                else shift = "Repos";
            }
            ui->tblPlanning->setItem(row, 2 + j, new QTableWidgetItem(shift));
        }
    }
    QMessageBox::information(this, "Succès", "Planning intelligent généré en tenant compte des postes et des statuts (congés/absences).");
}

void EmployesPage::on_btnVerifierAlertes_clicked() {
    m_employes = EmployeDAO::obtenirTous();
    ui->tblCompetences->setRowCount(0);

    QMap<QString, QStringList> competencesParPoste = {
        {"Vétérinaire", {"Chirurgie", "Radiologie", "Pharmacologie"}},
        {"Assistant", {"Soins de base", "Contention", "Accueil"}},
        {"Secrétaire", {"Gestion RDV", "Facturation", "Accueil"}},
        {"Toilettteur", {"Toilettage chat", "Toilettage chien", "Soins hygiéniques"}},
        {"Gardien", {"Surveillance", "Entretien locaux", "Nourrissage"}}
    };

    int alertCount = 0;
    QDate today = QDate::currentDate();
    QRandomGenerator *rng = QRandomGenerator::global();

    for (const Employe &emp : m_employes) {
        int row = ui->tblCompetences->rowCount();
        ui->tblCompetences->insertRow(row);
        ui->tblCompetences->setItem(row, 0, new QTableWidgetItem(emp.getNomComplet()));
        ui->tblCompetences->setItem(row, 1, new QTableWidgetItem(emp.getPoste()));

        QStringList comps = competencesParPoste.value(emp.getPoste(), {"Général"});
        ui->tblCompetences->setItem(row, 2, new QTableWidgetItem(comps.join(", ")));

        int monthsAgo = 12 + rng->bounded(25);
        QDate lastFormation = today.addMonths(-monthsAgo);
        ui->tblCompetences->setItem(row, 3, new QTableWidgetItem(lastFormation.toString("dd/MM/yyyy")));

        QTableWidgetItem *statutItem = new QTableWidgetItem();
        if (monthsAgo > 24) {
            statutItem->setText("⚠️ À renouveler");
            statutItem->setBackground(QColor("#FFCCCC"));
            statutItem->setForeground(QColor("#CC0000"));
            statutItem->setTextAlignment(Qt::AlignCenter);
            alertCount++;
        } else {
            statutItem->setText("✅ À jour");
            statutItem->setBackground(QColor("#CCFFCC"));
            statutItem->setForeground(QColor("#006600"));
            statutItem->setTextAlignment(Qt::AlignCenter);
        }
        ui->tblCompetences->setItem(row, 4, statutItem);
    }

    if (alertCount > 0) {
        QMessageBox::warning(this, "Alertes de Formation", QString("Attention : %1 employé(s) ont des formations/certifications à renouveler !").arg(alertCount));
    } else {
        QMessageBox::information(this, "Tout va bien", "Toutes les compétences et formations sont à jour.");
    }
}

void EmployesPage::on_btnExporterPDF_clicked() {
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF", "", "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);

    QString html = "<h1>Liste des Employés</h1><table border='1' cellpadding='5' style='border-collapse: collapse; width: 100%;'>";
    html += "<tr><th>ID</th><th>Nom</th><th>Prénom</th><th>Email</th><th>Téléphone</th><th>Poste</th><th>Salaire</th><th>Date Embauche</th><th>Statut</th></tr>";

    for (const Employe &emp : m_employes) {
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7 TND</td><td>%8</td><td>%9</td></tr>")
        .arg(emp.getId()).arg(emp.getNom()).arg(emp.getPrenom()).arg(emp.getEmail())
            .arg(emp.getTelephone()).arg(emp.getPoste()).arg(emp.getSalaire(), 0, 'f', 3)
            .arg(emp.getDateEmbauche().toString("dd/MM/yyyy")).arg(emp.getStatut());
    }
    html += "</table>";

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);
    QMessageBox::information(this, "Succès", "PDF exporté avec succès :\n" + fileName);
}

void EmployesPage::on_btnExporterExcel_clicked() {
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en CSV", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << QChar(0xFEFF);
        out << "ID;Nom;Prénom;Email;Téléphone;Poste;Salaire;Date Embauche;Statut\n";

        for (const Employe &emp : m_employes) {
            out << QString("%1;%2;%3;%4;%5;%6;%7;%8;%9\n")
            .arg(emp.getId()).arg(emp.getNom()).arg(emp.getPrenom()).arg(emp.getEmail())
                .arg(emp.getTelephone()).arg(emp.getPoste()).arg(emp.getSalaire(), 0, 'f', 3)
                .arg(emp.getDateEmbauche().toString("dd/MM/yyyy")).arg(emp.getStatut());
        }
        file.close();
        QMessageBox::information(this, "Succès", "CSV exporté avec succès :\n" + fileName);
    }
}