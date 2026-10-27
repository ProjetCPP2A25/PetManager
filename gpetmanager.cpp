#include "gpetmanager.h"
#include "ui_gpetmanager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QScrollBar>
#include <QMessageBox>
#include <QInputDialog>
#include <QTableView>
#include <QTableWidget>
#include <QSqlQueryModel>
#include <QItemSelectionModel>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QLocale>
#include <QFileDialog>
#include <QFile>
#include <QDesktopServices>
#include <QUrl>
#include <QStandardPaths>
#include <QComboBox>
#include <QPair>
#include <QStyledItemDelegate>
#include <QGraphicsDropShadowEffect>
#include <QRegularExpressionValidator>
#include <QApplication>
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QPixmap>
#include <algorithm>
#include <cmath>

// Raccourcit un texte trop long pour un petit emplacement
static QString raccourcir(const QString &texte, int max)
{
    return texte.length() <= max ? texte : texte.left(max - 1) + "…";
}

static const int PAGE_STOCK = 3;
static const int PAGE_ALERTES = 8;

// Colonnes du modèle renvoyé par Produit::afficher()
enum ColonneStock { COL_ID = 0, COL_NOM, COL_BADGE_CAT, COL_QTE, COL_SEUIL, COL_EXPIRATION,
                    COL_EMPLACEMENT, COL_BADGE_STATUT, COL_ACTIONS, COL_CATEGORIE, COL_STATUT,
                    COL_PRIX };

// Largeur des colonnes : 0 = la colonne prend la place libre
static void largeursColonnes(QTableView *t, const QList<int> &largeurs)
{
    for (int c = 0; c < largeurs.size(); ++c) {
        if (largeurs[c] == 0) {
            t->horizontalHeader()->setSectionResizeMode(c, QHeaderView::Stretch);
        } else {
            t->horizontalHeader()->setSectionResizeMode(c, QHeaderView::Fixed);
            t->setColumnWidth(c, largeurs[c]);
        }
    }
}

// =====================================================================
//                         CONSTRUCTEUR
// =====================================================================

GPetManager::GPetManager(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::GPetManager)
{
    ui->setupUi(this);

    timerToast = new QTimer(this);
    timerToast->setSingleShot(true);
    connect(timerToast, &QTimer::timeout, this, [this]() { if (toast) toast->hide(); });

    // Métier 1 : surveillance automatique à intervalle régulier
    timerSurveillance = new QTimer(this);
    connect(timerSurveillance, &QTimer::timeout, this, [this]() {
        Etmp.actualiserStatuts();
        rafraichir();
        verifierStock(true);
    });

    configurerNavigation();
    configurerTableau();
    configurerFormulaire();
    configurerPageAlertes();
    configurerGraphiques();
    chargerParametres();
    chargerFournisseurs();

    // Panneau IA vide au démarrage
    ui->frameAlt1Stock->hide();
    ui->frameAlt2Stock->hide();
    ui->lblAltPourStock->setText(Produit::iaConfiguree() ? "IA Gemini prête"
                                                           : "Moteur local (aucune clé API configurée)");
    ui->lblAucuneSuggestionIAStock->show();

    Etmp.actualiserStatuts();      // statuts à jour (produits expirés depuis hier...)
    viderFormulaire();

    // Appel de la méthode afficher() via l'attribut Etmp (Read)
    ui->tableStock->setModel(Etmp.afficher());
    rafraichir();

    // Recherche (nom ou ID) et tri
    connect(ui->txtRechercheStock, &QLineEdit::textChanged, this, &GPetManager::afficherTableau);
    connect(ui->cmbTriStock, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GPetManager::afficherTableau);
    connect(ui->lblAffichageStock, &QLabel::linkActivated, this, [this](const QString &) {
        demanderAlternatives(ui->txtRechercheStock->text(), QString(), -1);
    });

    // Double-clic sur une ligne = charger le produit dans le formulaire
    connect(ui->tableStock, &QTableView::doubleClicked, this, [this](const QModelIndex &index) {
        chargerDansFormulaire(ui->tableStock->model()->index(index.row(), COL_ID).data().toInt());
    });

    // Statut recalculé pendant la saisie
    connect(ui->spinQuantiteStock, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GPetManager::mettreAJourStatutFormulaire);
    connect(ui->spinSeuilStock, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GPetManager::mettreAJourStatutFormulaire);
    connect(ui->dateExpirationStock, &QDateEdit::dateChanged,
            this, &GPetManager::mettreAJourStatutFormulaire);

    connect(ui->cmbPeriodeStock, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GPetManager::mettreAJourGraphiques);
    connect(ui->cmbProduitGraphStock, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GPetManager::mettreAJourGraphiques);
    connect(ui->btnNotif, &QPushButton::clicked, this, &GPetManager::afficherNotifications);
    connect(ui->btnVoirToutStock,      &QPushButton::clicked, this, [this]() { changerPage(PAGE_ALERTES); });
    connect(ui->btnOuvrirReapproStock, &QPushButton::clicked, this, [this]() { changerPage(PAGE_ALERTES); });
    connect(ui->btnRetourStock,        &QPushButton::clicked, this, [this]() { changerPage(PAGE_STOCK); });
    connect(ui->txtBesoinIAStock, &QLineEdit::returnPressed, this, &GPetManager::on_btnDemanderIAStock_clicked);

    // Les deux réglages de la barre sont enregistrés dès qu'ils changent
    auto reglageSurveillance = [this]() {
        Produit::enregistrerSurveillance(ui->spinIntervalleStock->value(), ui->spinFacteurStock->value());
        timerSurveillance->start(ui->spinIntervalleStock->value() * 60 * 1000);
        afficherSousSeuil();
    };
    connect(ui->spinIntervalleStock, QOverload<int>::of(&QSpinBox::valueChanged), this, reglageSurveillance);
    connect(ui->spinFacteurStock,    QOverload<int>::of(&QSpinBox::valueChanged), this, reglageSurveillance);

    // Clic sur une carte « alternative » = afficher ce produit dans le formulaire
    for (QFrame *carte : {ui->frameAlt1Stock, ui->frameAlt2Stock}) {
        carte->installEventFilter(this);
        for (QLabel *lbl : carte->findChildren<QLabel*>())
            lbl->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
}

// Contrôle automatique du stock au démarrage (métier 1)
void GPetManager::demarrerSurveillance()
{
    QTimer::singleShot(800, this, [this]() { verifierStock(true); });
}

GPetManager::~GPetManager()
{
    delete ui;
}

bool GPetManager::eventFilter(QObject *objet, QEvent *evenement)
{
    // Les graphiques suivent la taille de leur zone
    if (evenement->type() == QEvent::Resize
        && (objet == ui->widgetGraphiqueStock || objet == ui->widgetCamembertStock))
        dessinerGraphiques();

    if (evenement->type() == QEvent::MouseButtonPress) {
        int k = -1;
        if (objet == ui->frameAlt1Stock) k = 0;
        else if (objet == ui->frameAlt2Stock) k = 1;
        if (k >= 0 && k < suggestionsIA.size()) {
            const int id = suggestionsIA[k].idProduit;
            QTimer::singleShot(0, this, [this, id]() { chargerDansFormulaire(id); });
            return true;
        }
    }
    return QMainWindow::eventFilter(objet, evenement);
}

// =====================================================================
//                         CONFIGURATION
// =====================================================================

void GPetManager::configurerNavigation()
{
    // Ordre = ordre des pages du QStackedWidget
    boutonsNav = {ui->btnAccueil, ui->btnAnimaux, ui->btnRendezVous, ui->btnStock,
                  ui->btnCommandes, ui->btnFournisseurs, ui->btnEmployes, ui->btnParametres};

    for (int i = 0; i < boutonsNav.size(); ++i)
        connect(boutonsNav[i], &QPushButton::clicked, this, [this, i]() { changerPage(i); });

    // Pastille rouge sur la cloche
    badgeNotif = new QLabel(ui->btnNotif);
    badgeNotif->setGeometry(22, 2, 18, 18);
    badgeNotif->setAlignment(Qt::AlignCenter);
    badgeNotif->setAttribute(Qt::WA_TransparentForMouseEvents);
    badgeNotif->setStyleSheet("background:#E53935; color:white; border-radius:9px;"
                              "font-size:10px; font-weight:bold;");

    changerPage(PAGE_STOCK);
}

void GPetManager::changerPage(int index)
{
    const QStringList titres = {"Accueil", "Animaux", "Rendez-vous", "Gestion de stock",
                                "Commandes", "Fournisseurs", "Employés", "Paramètres",
                                "Alertes de réapprovisionnement"};
    const QStringList icones = {"home", "paw", "calendar", "box", "cart", "truck", "users", "gear", "mail"};

    ui->SWModules->setCurrentIndex(index);
    ui->lblIconeTitre->setPixmap(icone(icones[index] + "_teal").pixmap(64, 64));
    ui->lblTitre->setText(titres[index]);
    // La page des alertes fait partie du module Stock
    boutonsNav[index == PAGE_ALERTES ? PAGE_STOCK : index]->setChecked(true);
}

void GPetManager::preparerTableau(QAbstractItemView *vue)
{
    QTableView *t = qobject_cast<QTableView*>(vue);
    if (!t) return;
    t->verticalHeader()->setVisible(false);
    t->verticalHeader()->setDefaultSectionSize(42);
    t->horizontalHeader()->setFixedHeight(38);
    t->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    t->horizontalHeader()->setStretchLastSection(false);
    t->horizontalHeader()->setHighlightSections(false);
    t->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    t->setWordWrap(false);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setFocusPolicy(Qt::NoFocus);
}

void GPetManager::configurerTableau()
{
    preparerTableau(ui->tableStock);
    ui->tableStock->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableStock->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableStock->setToolTip("Double-cliquez sur une ligne pour afficher le produit dans le formulaire");

    // Métier basique 2 : tri selon la date d'expiration
    ui->cmbTriStock->addItems({"Aucun tri",
                               "Date d'expiration : la plus proche d'abord",
                               "Date d'expiration : la plus lointaine d'abord"});
    // Métier basique 1 : recherche selon le nom
    ui->txtRechercheStock->setPlaceholderText("Rechercher un produit par nom...");
    ui->txtRechercheStock->setClearButtonEnabled(true);
    ui->txtRechercheStock->addAction(icone("search_grey"), QLineEdit::LeadingPosition);
    ui->lblAffichageStock->setTextFormat(Qt::RichText);
    ui->lblAffichageStock->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
}

void GPetManager::configurerFormulaire()
{
    ui->cmbCategorieStock->addItem("Sélectionner une catégorie");
    ui->cmbCategorieStock->addItems(Produit::categories());
    ui->cmbPeriodeStock->addItems({"7 derniers jours", "30 derniers jours"});
    ui->cmbPeriodeStock->setCurrentIndex(1);

    // Listes déroulantes : délégué standard pour que le texte des options
    // s'affiche correctement avec la feuille de style
    for (QComboBox *combo : findChildren<QComboBox*>())
        combo->setItemDelegate(new QStyledItemDelegate(combo));

    ui->lblAvatar->setPixmap(icone("user_teal").pixmap(26, 26));

    // Contrôle de saisie dès la frappe
    ui->txtNomStock->setValidator(new QRegularExpressionValidator(
        QRegularExpression("^[\\p{L}0-9 '()./%+,-]{0,50}$"), this));
    ui->txtNomStock->setMaxLength(50);
    ui->txtEmplacementStock->setValidator(new QRegularExpressionValidator(
        QRegularExpression("^[\\p{L}0-9 '()./,-]{0,30}$"), this));
    ui->txtEmplacementStock->setMaxLength(30);

    ui->spinIdStock->setRange(1, 99999999);
    ui->spinQuantiteStock->setRange(0, 100000);
    ui->spinSeuilStock->setRange(0, 100000);

    ui->dateExpirationStock->setCalendarPopup(true);
    ui->dateExpirationStock->setDisplayFormat("dd/MM/yyyy");
    ui->dateExpirationStock->setDate(QDate::currentDate().addMonths(6));

    ui->spinPrixStock->setDecimals(3);
    ui->spinPrixStock->setRange(0, 100000);
    ui->spinPrixStock->setSuffix(" TND");
}

void GPetManager::configurerPageAlertes()
{
    ui->spinIntervalleStock->setRange(1, 1440);
    ui->spinIntervalleStock->setSuffix(" min");
    ui->spinFacteurStock->setRange(1, 10);
    ui->spinFacteurStock->setSuffix(" × seuil");
    ui->spinSmtpPortStock->setRange(1, 65535);

    // Produits ayant atteint le seuil
    QTableWidget *s = ui->tableSousSeuilStock;
    s->setColumnCount(10);
    s->setHorizontalHeaderLabels({"ID", "Produit", "Catégorie", "Quantité", "Seuil min.",
                                  "Qté recommandée", "Montant estimé", "Fournisseur",
                                  "Dernière alerte", "Actions"});
    preparerTableau(s);
    s->setSelectionMode(QAbstractItemView::NoSelection);
    largeursColonnes(s, {55, 0, 125, 85, 90, 135, 130, 140, 150, 80});

    // Historique des e-mails
    QTableWidget *h = ui->tableHistoriqueStock;
    h->setColumnCount(5);
    h->setHorizontalHeaderLabels({"Date", "Destinataire", "Produits signalés", "Type", "Résultat"});
    preparerTableau(h);
    h->setSelectionMode(QAbstractItemView::NoSelection);
    largeursColonnes(h, {150, 240, 0, 120, 130});

    // Analyse détaillée de l'IA (page Stock)
    QTableWidget *ia = ui->tableSuggestionsIAStock;
    ia->setColumnCount(7);
    ia->setHorizontalHeaderLabels({"Alternative", "Catégorie", "Stock", "Pertinence",
                                   "Pourquoi cette alternative ?", "Avantages et précautions", "Décision"});
    preparerTableau(ia);
    ia->setSelectionMode(QAbstractItemView::NoSelection);
    ia->setWordWrap(true);
    largeursColonnes(ia, {190, 125, 70, 110, 0, 0, 190});
    ia->hide();
}

void GPetManager::configurerGraphiques()
{
    // Chaque graphique est une image (QPainter) affichée dans un QLabel qui remplit sa zone
    auto placer = [this](QWidget *zone) {
        QLabel *lbl = new QLabel(zone);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        QVBoxLayout *l = new QVBoxLayout(zone);
        l->setContentsMargins(0, 0, 0, 0);
        l->addWidget(lbl);
        zone->installEventFilter(this);       // redessiner quand la zone change de taille
        return lbl;
    };
    graphConso      = placer(ui->widgetGraphiqueStock);
    graphCategories = placer(ui->widgetCamembertStock);

    ui->scrollStock->viewport()->setAutoFillBackground(false);
    ui->scrollReappro->viewport()->setAutoFillBackground(false);
}

void GPetManager::chargerParametres()
{
    const ConfigEmail c = Produit::configurationEmail();
    ui->txtSmtpServeurStock->setText(c.serveur);
    ui->spinSmtpPortStock->setValue(c.port);
    ui->txtSmtpUtilisateurStock->setText(c.utilisateur);
    ui->txtSmtpMotDePasseStock->setText(c.motDePasse);
    ui->txtDestinataireStock->setText(c.destinataire);
    ui->chkAlerteAutoStock->setChecked(Produit::envoiAutomatique());
    ui->txtCleIAStock->setText(Produit::cleApiIA());
    ui->txtModeleIAStock->setText(Produit::modeleIA());

    // Valeurs placées sans déclencher l'enregistrement
    ui->spinIntervalleStock->blockSignals(true);
    ui->spinFacteurStock->blockSignals(true);
    ui->spinIntervalleStock->setValue(Produit::intervalleMinutes());
    ui->spinFacteurStock->setValue(Produit::facteurStockCible());
    ui->spinIntervalleStock->blockSignals(false);
    ui->spinFacteurStock->blockSignals(false);

    timerSurveillance->start(Produit::intervalleMinutes() * 60 * 1000);
}

void GPetManager::chargerFournisseurs()
{
    // Le fournisseur n'est pas un attribut de STOCK (MR commun) : il est retrouvé
    // à partir des commandes (LIGNE_CMD -> COMMANDE -> FOURNISSEUR)
    fournisseurs = Etmp.fournisseurParProduit();
}

// =====================================================================
//                         AFFICHAGE (READ)
// =====================================================================

void GPetManager::rafraichir()
{
    fournisseurs = Etmp.fournisseurParProduit();   // des commandes ont pu être ajoutées
    afficherTableau();
    mettreAJourCartes();
    mettreAJourAlertes();
    chargerListeGraphique();
    mettreAJourGraphiques();
    afficherSousSeuil();
    afficherHistorique();
}

// Le modèle est redemandé à la classe Produit après chaque ajout, modification
// ou suppression : l'affichage est donc toujours actualisé.
void GPetManager::afficherTableau()
{
    QTableView *t = ui->tableStock;
    QAbstractItemModel *ancienModele = t->model();
    QItemSelectionModel *ancienneSelection = t->selectionModel();

    QSqlQueryModel *model = Etmp.afficher(ui->txtRechercheStock->text(), ui->cmbTriStock->currentIndex());
    while (model->canFetchMore()) model->fetchMore();     // toutes les lignes
    t->setModel(model);
    delete ancienneSelection;
    delete ancienModele;

    decorerTableau();
}

// Badges colorés et boutons d'action placés dans les colonnes prévues
void GPetManager::decorerTableau()
{
    QTableView *t = ui->tableStock;
    QAbstractItemModel *m = t->model();
    if (!m) return;

    for (int c = COL_CATEGORIE; c <= COL_PRIX; ++c)
        t->setColumnHidden(c, true);
    largeursColonnes(t, {45, 0, 120, 75, 80, 95, 110, 105, 130});

    int disponibles = 0;
    const int lignes = m->rowCount();
    for (int r = 0; r < lignes; ++r) {
        const int id = m->index(r, COL_ID).data().toInt();
        const QString categorie = m->index(r, COL_CATEGORIE).data().toString();
        const QString statut = m->index(r, COL_STATUT).data().toString();
        if (statut != "Rupture" && statut != "Expire") ++disponibles;

        QString fond, couleur;
        couleursCategorie(categorie, fond, couleur);
        t->setIndexWidget(m->index(r, COL_BADGE_CAT), creerBadge(categorie, fond, couleur));
        t->setIndexWidget(m->index(r, COL_BADGE_STATUT),
                          creerBadge(Produit::libelleStatut(statut), couleurStatut(statut), "white"));
        t->setIndexWidget(m->index(r, COL_ACTIONS), creerActions(id));
    }
    ajusterHauteur(t, lignes);

    const QString filtre = ui->txtRechercheStock->text().trimmed();
    const QString lienIA = " — <a href='ia' style='color:#2A8C86; font-weight:bold'>"
                           "demander une alternative à l'IA</a>";
    if (lignes == 0 && !filtre.isEmpty())
        ui->lblAffichageStock->setText("Aucun produit ne correspond à « " + filtre.toHtmlEscaped() + " »" + lienIA);
    else if (!filtre.isEmpty() && disponibles == 0)
        ui->lblAffichageStock->setText("Les produits trouvés sont indisponibles" + lienIA);
    else
        ui->lblAffichageStock->setText(QString("Affichage de %1 sur %2 produits")
                                           .arg(lignes).arg(Etmp.nombreProduits()));
}

void GPetManager::mettreAJourCartes()
{
    ui->lblValeurTotalStock->setText(QString::number(Etmp.nombreProduits()));
    ui->lblValeurReapproStock->setText(QString::number(Etmp.nombreSousSeuil()));
    ui->lblValeurStock->setText(QLocale(QLocale::French).toString(Etmp.valeurStock(), 'f', 0) + " TND");
}

void GPetManager::mettreAJourAlertes()
{
    const QList<Produit> liste = Etmp.listeSousSeuil();

    ui->lblAlerteBadgeStock->setText(QString::number(liste.size()));
    badgeNotif->setText(QString::number(liste.size()));
    badgeNotif->setVisible(!liste.isEmpty());

    const QList<QLabel*> noms  = {ui->lblAlerte1Stock, ui->lblAlerte2Stock, ui->lblAlerte3Stock};
    const QList<QLabel*> infos = {ui->lblQte1Stock, ui->lblQte2Stock, ui->lblQte3Stock};

    for (int i = 0; i < 3; ++i) {
        if (i < liste.size()) {
            const Produit &p = liste[i];
            const QString couleur = p.getQuantite() == 0 ? "#E53935" : "#F5A623";
            noms[i]->setText(QString("<span style='color:%1'>●</span> %2")
                                 .arg(couleur, raccourcir(p.getNom(), 30).toHtmlEscaped()));
            noms[i]->setToolTip(QString("%1\nQuantité : %2 (seuil %3)\nQuantité recommandée : %4")
                                    .arg(p.getNom()).arg(p.getQuantite()).arg(p.getSeuilMin())
                                    .arg(Produit::quantiteRecommandee(p.getQuantite(), p.getSeuilMin(),
                                                                      ui->spinFacteurStock->value())));
            infos[i]->setText(QString("%1 / %2").arg(p.getQuantite()).arg(p.getSeuilMin()));
            noms[i]->show();
            infos[i]->show();
        } else if (i == 0) {
            noms[0]->setText("<span style='color:#2E8B57; font-weight:bold'>Tout est sous contrôle</span>");
            noms[0]->setToolTip(QString());
            noms[0]->show();
            infos[0]->hide();
        } else {
            noms[i]->hide();
            infos[i]->hide();
        }
    }
}

// Liste des produits proposés au-dessus du graphique (la sélection est conservée)
void GPetManager::chargerListeGraphique()
{
    QComboBox *c = ui->cmbProduitGraphStock;
    const int selection = c->currentData().toInt();
    c->blockSignals(true);
    c->clear();
    c->addItem("3 produits les plus consommés", 0);
    for (const Produit &p : Etmp.listeTous())
        c->addItem(p.getNom(), p.getIdProduit());
    const int pos = c->findData(selection);
    c->setCurrentIndex(pos >= 0 ? pos : 0);
    c->blockSignals(false);
}

// Métier basique 4 : évolution de la consommation + répartition par catégorie
void GPetManager::mettreAJourGraphiques()
{
    // 1) Évolution des quantités consommées (table CONSOMMATION), jour par jour
    const int nbJours = (ui->cmbPeriodeStock->currentIndex() == 0) ? 7 : 30;
    const QDate debut = QDate::currentDate().addDays(-(nbJours - 1));

    QStringList etiquettes;
    for (int d = 0; d < nbJours; ++d)
        etiquettes << debut.addDays(d).toString("dd/MM");

    QList<QPair<int, QString>> series;
    const int idChoisi = ui->cmbProduitGraphStock->currentData().toInt();
    if (idChoisi == 0)
        series = Etmp.produitsLesPlusConsommes(debut, 3);
    else
        series.append(qMakePair(idChoisi, ui->cmbProduitGraphStock->currentText()));

    QStringList noms;
    QList<QList<double>> valeurs;
    QString resume;
    for (const auto &serie : series) {
        const QMap<QDate, int> conso = Etmp.consommationParJour(serie.first, debut);
        QList<double> points;
        int total = 0, pic = 0;
        QDate jourPic;
        for (int d = 0; d < nbJours; ++d) {
            const int v = conso.value(debut.addDays(d), 0);
            points << v;
            total += v;
            if (v > pic) { pic = v; jourPic = debut.addDays(d); }
        }
        if (total == 0 && idChoisi == 0) continue;
        noms << serie.second;
        valeurs << points;
        resume += QString("%1 : %2 unité(s) consommée(s), moyenne %3 / jour%4\n")
                      .arg(serie.second).arg(total)
                      .arg(QString::number(double(total) / nbJours, 'f', 1))
                      .arg(pic > 0 ? QString(", pic le %1 (%2)").arg(jourPic.toString("dd/MM")).arg(pic)
                                   : QString());
    }
    consoEtiquettes = etiquettes;
    consoNoms = noms;
    consoValeurs = valeurs;
    graphConso->setToolTip(resume.trimmed());

    // 2) Répartition des produits par catégorie
    const QMap<QString, int> parCategorie = Etmp.repartitionParCategorie();
    const QStringList categories = Produit::categories();
    categoriesNombres.clear();
    for (const QString &c : categories) categoriesNombres << parCategorie.value(c, 0);

    dessinerGraphiques();
}

// ---------------------------------------------------------------------
//  Dessin des graphiques (QPainter) : chaque graphique est une image
//  affichée dans un QLabel, redessinée quand les données ou la taille changent.
// ---------------------------------------------------------------------

// Pas "lisible" pour un axe : 1, 2, 5, 10, 20, 50...
static double pasLisible(double brut)
{
    double base = 1;
    while (true) {
        for (double m : {1.0, 2.0, 5.0})
            if (m * base >= brut) return m * base;
        base *= 10;
    }
}

// Image vide (transparente) de la taille de la zone, nette sur les écrans haute densité
static QPixmap nouvelleImage(const QSize &taille, qreal ratio)
{
    QPixmap image(taille * ratio);
    image.setDevicePixelRatio(ratio);
    image.fill(Qt::transparent);
    return image;
}

// Évolution de la consommation : une courbe par produit
static QPixmap dessinerCourbes(const QSize &taille, qreal ratio, const QFont &police,
                               const QStringList &etiquettes, const QStringList &noms,
                               const QList<QList<double>> &valeurs, const QList<QColor> &couleurs)
{
    QPixmap image = nouvelleImage(taille, ratio);
    const double largeur = taille.width(), hauteur = taille.height();
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);
    QFont f = police;
    f.setPointSize(7);
    p.setFont(f);
    QFontMetrics fm(f);

    if (noms.isEmpty()) {
        p.setPen(QColor("#6B8080"));
        p.drawText(QRectF(0, 0, largeur, hauteur), Qt::AlignCenter, "Aucune consommation sur la période");
        return image;
    }

    double vmax = 0;
    for (const QList<double> &serie : valeurs)
        for (double v : serie) vmax = std::max(vmax, v);
    // 4 graduations entières : 0, pas, 2 x pas, 3 x pas, 4 x pas
    const double pas = pasLisible(std::max(1.0, vmax / 4.0));
    vmax = pas * 4;

    const double margeG = 26, margeD = 16, margeH = 8, margeB = 36;
    QRectF zone(margeG, margeH, largeur - margeG - margeD, hauteur - margeH - margeB);
    const int n = etiquettes.size();
    auto xPour = [&](int i) { return zone.left() + (n > 1 ? i * zone.width() / (n - 1) : 0.0); };
    auto yPour = [&](double v) { return zone.bottom() - v / vmax * zone.height(); };

    // Grille et axe Y
    for (int k = 0; k <= 4; ++k) {
        const double v = vmax * k / 4.0;
        const double y = yPour(v);
        p.setPen(QPen(QColor("#E8EEEE"), 1));
        p.drawLine(QPointF(zone.left(), y), QPointF(zone.right(), y));
        p.setPen(QColor("#6B8080"));
        p.drawText(QRectF(0, y - 6, margeG - 4, 12), Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(v, 'f', 0));
    }

    // Axe X (une étiquette sur "pas" si la période est longue)
    const int pasEtiquettes = std::max(1, (n + 6) / 7);
    for (int i = 0; i < n; i += pasEtiquettes)
        p.drawText(QRectF(xPour(i) - 18, zone.bottom() + 3, 36, 12), Qt::AlignCenter, etiquettes[i]);

    // Courbes
    for (int s = 0; s < valeurs.size(); ++s) {
        const QColor couleur = couleurs[s % couleurs.size()];
        QPainterPath chemin;
        for (int i = 0; i < valeurs[s].size(); ++i) {
            const QPointF pt(xPour(i), yPour(valeurs[s][i]));
            if (i == 0) chemin.moveTo(pt);
            else        chemin.lineTo(pt);
        }
        p.setPen(QPen(couleur, 2));
        p.setBrush(Qt::NoBrush);
        p.drawPath(chemin);
        if (n <= 10) {
            p.setBrush(couleur);
            for (int i = 0; i < valeurs[s].size(); ++i)
                p.drawEllipse(QPointF(xPour(i), yPour(valeurs[s][i])), 2.5, 2.5);
        }
    }

    // Légende
    double x = 6;
    const double y = hauteur - 10;
    for (int s = 0; s < noms.size(); ++s) {
        const QString nom = fm.elidedText(noms[s], Qt::ElideRight, 85);
        p.setPen(Qt::NoPen);
        p.setBrush(couleurs[s % couleurs.size()]);
        p.drawEllipse(QPointF(x + 4, y), 4, 4);
        p.setPen(QColor("#1E3A3A"));
        p.drawText(QPointF(x + 12, y + 3), nom);
        x += 12 + fm.horizontalAdvance(nom) + 12;
    }
    return image;
}

// Répartition par catégorie : anneau + légende
static QPixmap dessinerCamembert(const QSize &taille, qreal ratio, const QFont &police,
                                 const QStringList &noms, const QList<double> &valeurs,
                                 const QList<QColor> &couleurs, const QString &unite)
{
    QPixmap image = nouvelleImage(taille, ratio);
    const double largeur = taille.width(), hauteur = taille.height();
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);
    QFont f = police;
    f.setPointSize(8);
    p.setFont(f);

    double total = 0;
    for (double v : valeurs) total += v;
    if (total <= 0) {
        p.setPen(QColor("#6B8080"));
        p.drawText(QRectF(0, 0, largeur, hauteur), Qt::AlignCenter, "Aucune donnée");
        return image;
    }

    const double d = std::max(10.0, std::min(hauteur - 10.0, largeur * 0.5));
    const QRectF cercle(8, (hauteur - d) / 2, d, d);

    int angle = 90 * 16;
    for (int i = 0; i < valeurs.size(); ++i) {
        const int span = -qRound(valeurs[i] / total * 360.0 * 16);
        p.setPen(QPen(Qt::white, 2));
        p.setBrush(couleurs[i % couleurs.size()]);
        p.drawPie(cercle, angle, span);
        angle += span;
    }

    // Trou central + total
    const double r = d * 0.30;
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);
    p.drawEllipse(cercle.center(), r, r);
    QFont fb = f;
    fb.setBold(true);
    p.setFont(fb);
    p.setPen(QColor("#17363A"));
    p.drawText(QRectF(cercle.center().x() - r, cercle.center().y() - r, 2 * r, 2 * r),
               Qt::AlignCenter, QString::number(total, 'f', 0) + "\n" + unite);

    // Légende
    p.setFont(f);
    const double x = cercle.right() + 14;
    double y = hauteur / 2.0 - noms.size() * 36 / 2.0;
    for (int i = 0; i < noms.size() && i < valeurs.size(); ++i) {
        p.setPen(Qt::NoPen);
        p.setBrush(couleurs[i % couleurs.size()]);
        p.drawRoundedRect(QRectF(x, y + 3, 10, 10), 2, 2);
        p.setPen(QColor("#1E3A3A"));
        p.drawText(QPointF(x + 16, y + 12), noms[i]);
        p.setPen(QColor("#6B8080"));
        const int pct = qRound(valeurs[i] / total * 100);
        p.drawText(QPointF(x + 16, y + 27), QString("%1 (%2 %)").arg(valeurs[i], 0, 'f', 0).arg(pct));
        y += 36;
    }
    return image;
}

void GPetManager::dessinerGraphiques()
{
    if (!graphConso || !graphCategories) return;
    const qreal ratio = devicePixelRatioF();

    const QSize tailleConso = ui->widgetGraphiqueStock->size();
    if (tailleConso.width() > 0 && tailleConso.height() > 0)
        graphConso->setPixmap(dessinerCourbes(tailleConso, ratio, font(), consoEtiquettes, consoNoms,
                                              consoValeurs,
                                              {QColor("#1F7A6E"), QColor("#F5A623"), QColor("#8E44AD")}));

    const QSize tailleCat = ui->widgetCamembertStock->size();
    if (tailleCat.width() > 0 && tailleCat.height() > 0)
        graphCategories->setPixmap(dessinerCamembert(tailleCat, ratio, font(), Produit::categories(),
                                                     categoriesNombres,
                                                     {QColor("#43A047"), QColor("#E53935"), QColor("#1E88E5")},
                                                     "produits"));
}

// =====================================================================
//                         FORMULAIRE / CRUD
// =====================================================================

// Lecture + contrôle de saisie. Retourne false (et prévient l'utilisateur) si invalide.
bool GPetManager::lireFormulaire(Produit &p, bool nouveau)
{
    const int id = nouveau ? ui->spinIdStock->value() : idEnEdition;
    const QString categorie = ui->cmbCategorieStock->currentIndex() > 0
                                  ? ui->cmbCategorieStock->currentText() : QString();

    p = Produit(id, ui->txtNomStock->text(), categorie, ui->spinQuantiteStock->value(),
                ui->spinSeuilStock->value(), ui->spinPrixStock->value(),
                ui->dateExpirationStock->date(), ui->txtEmplacementStock->text());

    QStringList erreurs = p.controleSaisie(nouveau);
    if (nouveau && Etmp.existe(id))
        erreurs << QString("L'ID %1 est déjà utilisé par un autre produit.").arg(id);
    if (!p.getNom().isEmpty() && Etmp.nomExiste(p.getNom(), nouveau ? -1 : id))
        erreurs << "Un produit portant ce nom existe déjà.";

    if (!erreurs.isEmpty()) {
        QMessageBox::warning(this, "Contrôle de saisie",
                             "Veuillez corriger les champs suivants :\n\n• " + erreurs.join("\n• "));
        return false;
    }
    return true;
}

void GPetManager::on_btnEnregistrerStock_clicked()
{
    // Récupération des informations saisies + instanciation d'un objet Produit
    Produit P;
    if (!lireFormulaire(P, true)) return;

    bool test = P.ajouter();   // insérer le produit dans la table STOCK

    if (test) {
        rafraichir();          // Refresh (Actualiser)
        QMessageBox::information(nullptr, QObject::tr("Succès"),
                                 QObject::tr("Ajout effectué.\nLe produit « %1 » a été ajouté au stock.")
                                     .arg(P.getNom()), QMessageBox::Ok);
        viderFormulaire();
        verifierStock(true);   // métier 1 : le nouveau produit est-il déjà sous le seuil ?
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Échec"),
                              QObject::tr("Ajout non effectué.\n") + P.derniereErreur(), QMessageBox::Ok);
    }
}

void GPetManager::on_btnModifierStock_clicked()
{
    if (idEnEdition < 0) {
        QMessageBox::information(this, "Modifier",
                                 "Choisissez d'abord un produit (crayon ou double-clic sur une ligne).");
        return;
    }
    Produit P;
    if (!lireFormulaire(P, false)) return;

    bool test = P.modifier();

    if (test) {
        rafraichir();
        QMessageBox::information(nullptr, QObject::tr("Succès"),
                                 QObject::tr("Modification effectuée."), QMessageBox::Ok);
        viderFormulaire();
        verifierStock(true);
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Échec"),
                              QObject::tr("Modification non effectuée.\n") + P.derniereErreur(),
                              QMessageBox::Ok);
    }
}

void GPetManager::on_btnSupprimerStock_clicked()
{
    int id = idEnEdition >= 0 ? idEnEdition : idSelectionne();
    if (id < 0) {
        QMessageBox::information(this, "Supprimer",
            "Sélectionnez un produit dans le tableau, ou chargez-le avec le bouton Modifier (crayon).");
        return;
    }
    supprimerProduit(id);
}

void GPetManager::supprimerProduit(int id)
{
    Produit p;
    if (!p.chargerParId(id)) {
        QMessageBox::warning(this, "Supprimer", QString("Le produit d'ID %1 n'existe pas.").arg(id));
        return;
    }
    if (QMessageBox::question(this, "Supprimer",
            "Voulez-vous vraiment supprimer « " + p.getNom() + " » (ID " + QString::number(id) + ") ?")
        != QMessageBox::Yes)
        return;

    bool test = Etmp.supprimer(id);   // appel de la méthode supprimer() via l'attribut Etmp

    if (test) {
        if (idEnEdition == id) viderFormulaire();
        rafraichir();
        QMessageBox::information(nullptr, QObject::tr("Succès"),
                                 QObject::tr("Suppression effectuée."), QMessageBox::Ok);
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Échec"),
                              QObject::tr("Suppression non effectuée.\n") + Etmp.derniereErreur(),
                              QMessageBox::Ok);
    }
}

void GPetManager::on_btnAnnulerStock_clicked()
{
    viderFormulaire();
    ui->tableStock->clearSelection();
}

int GPetManager::idSelectionne() const
{
    const QModelIndexList lignes = ui->tableStock->selectionModel()
                                       ? ui->tableStock->selectionModel()->selectedRows()
                                       : QModelIndexList();
    if (lignes.isEmpty()) return -1;
    return ui->tableStock->model()->index(lignes.first().row(), COL_ID).data().toInt();
}

void GPetManager::chargerDansFormulaire(int id)
{
    Produit p;
    if (!p.chargerParId(id)) return;

    changerPage(PAGE_STOCK);
    ui->spinIdStock->setValue(p.getIdProduit());
    ui->txtNomStock->setText(p.getNom());
    ui->cmbCategorieStock->setCurrentText(p.getCategorie());
    ui->spinQuantiteStock->setValue(p.getQuantite());
    ui->spinSeuilStock->setValue(p.getSeuilMin());
    ui->spinPrixStock->setValue(p.getPrixUnitaire());
    ui->dateExpirationStock->setDate(p.getDateExpiration());
    ui->txtEmplacementStock->setText(p.getEmplacement());

    idEnEdition = id;
    mettreAJourModeFormulaire();
    mettreAJourStatutFormulaire();
    ui->scrollStock->verticalScrollBar()->setValue(0);
    ui->txtNomStock->setFocus();

    // Produit indisponible : l'IA propose directement des alternatives
    const QString statut = Produit::calculerStatut(p.getQuantite(), p.getSeuilMin(), p.getDateExpiration());
    if (statut == "Rupture" || statut == "Expire")
        demanderAlternatives(p.getNom(), p.getCategorie(), id);
}

// Mode "Ajout" : Enregistrer actif, ID modifiable.
// Mode "Modification" : Modifier et Annuler actifs, ID verrouillé (clé primaire).
void GPetManager::mettreAJourModeFormulaire()
{
    const bool edition = (idEnEdition >= 0);
    ui->btnEnregistrerStock->setEnabled(!edition);
    ui->btnModifierStock->setEnabled(edition);
    ui->btnAnnulerStock->setEnabled(edition);
    ui->spinIdStock->setEnabled(!edition);

    if (edition)
        ui->lblFormTitreStock->setText("Modifier : " + raccourcir(ui->txtNomStock->text(), 22));
    else
        ui->lblFormTitreStock->setText("Ajouter un produit");
}

void GPetManager::mettreAJourStatutFormulaire()
{
    const QString statut = Produit::calculerStatut(ui->spinQuantiteStock->value(), ui->spinSeuilStock->value(),
                                                   ui->dateExpirationStock->date());
    ui->txtStatutStock->setText(Produit::libelleStatut(statut));
    ui->txtStatutStock->setStyleSheet(QString("QLineEdit { background:#F4F7F7; font-weight:bold; color:%1; }")
                                          .arg(couleurStatut(statut)));
}

void GPetManager::viderFormulaire()
{
    ui->spinIdStock->setValue(Etmp.prochainId());
    ui->txtNomStock->clear();
    ui->cmbCategorieStock->setCurrentIndex(0);
    ui->spinQuantiteStock->setValue(0);
    ui->spinSeuilStock->setValue(0);
    ui->spinPrixStock->setValue(0);
    ui->dateExpirationStock->setDate(QDate::currentDate().addMonths(6));
    ui->txtEmplacementStock->clear();
    idEnEdition = -1;
    mettreAJourModeFormulaire();
    mettreAJourStatutFormulaire();
}

// Sortie de stock (utilisation d'un produit) = UPDATE de la quantité.
// Produit indisponible ou quantité insuffisante -> l'IA propose des alternatives.
void GPetManager::sortieDeStock(int id)
{
    Produit p;
    if (!p.chargerParId(id)) return;

    const QString statut = Produit::calculerStatut(p.getQuantite(), p.getSeuilMin(), p.getDateExpiration());
    if (statut == "Rupture" || statut == "Expire") {
        afficherToast("ia_teal", "Produit indisponible",
                      "« " + p.getNom() + " » est " + (statut == "Expire" ? "expiré" : "en rupture")
                          + " : l'IA recherche des alternatives.", "#2A8C86");
        demanderAlternatives(p.getNom(), p.getCategorie(), id);
        return;
    }

    bool ok = false;
    const int q = QInputDialog::getInt(this, "Sortie de stock",
        QString("Quantité utilisée pour « %1 » (en stock : %2) :").arg(p.getNom()).arg(p.getQuantite()),
        1, 1, 100000, 1, &ok);
    if (!ok) return;

    if (q > p.getQuantite()) {
        afficherToast("ia_teal", "Stock insuffisant",
                      QString("Seulement %1 unité(s) de « %2 » : l'IA recherche des alternatives.")
                          .arg(p.getQuantite()).arg(p.getNom()), "#F5A623");
        demanderAlternatives(p.getNom(), p.getCategorie(), id);
        return;
    }

    if (Etmp.retirerQuantite(id, q)) {
        rafraichir();
        afficherToast("check_green", "Sortie de stock enregistrée",
                      QString("%1 unité(s) de « %2 » retirée(s) et ajoutée(s) à l'historique de consommation.")
                          .arg(q).arg(p.getNom()), "#2E8B57");
        verifierStock(true);      // métier 1 : le seuil est-il atteint ?
    } else {
        QMessageBox::critical(this, "Échec", "Sortie de stock non effectuée.\n" + Etmp.derniereErreur());
    }
}

// =====================================================================
//          MÉTIER 1 : ALERTE AUTOMATIQUE DE RÉAPPROVISIONNEMENT
// =====================================================================

// Surveillance : appelée au démarrage, après chaque Create/Update/sortie
// de stock et à intervalle régulier (timer).
void GPetManager::verifierStock(bool automatique)
{
    const QList<Produit> sousSeuil = Etmp.listeSousSeuil();
    Produit::actualiserSuivi(sousSeuil);
    const QList<Produit> nouveaux = Produit::produitsNonSignales(sousSeuil);
    afficherSousSeuil();

    if (nouveaux.isEmpty()) {
        if (!automatique)
            afficherToast("check_green", "Contrôle du stock",
                          sousSeuil.isEmpty() ? QString("Aucun produit n'a atteint son seuil minimal.")
                                              : QString("Tous les produits sous le seuil ont déjà été signalés."),
                          "#2E8B57");
        return;
    }

    auto voir = [this]() { changerPage(PAGE_ALERTES); };
    const QString resume = QString("%1 produit(s) ont atteint le seuil minimal.").arg(nouveaux.size());

    if (automatique && (!Produit::envoiAutomatique() || echecEnvoiAuto)) {
        afficherToast("warning_orange", "Réapprovisionnement nécessaire", resume, "#F5A623", "Voir", voir);
        return;
    }
    if (!Produit::configurationComplete(Produit::configurationEmail())) {
        afficherToast("warning_orange", "Alerte e-mail non configurée",
                      resume + " Configurez l'e-mail du responsable pour l'avertir.", "#F5A623",
                      "Configurer", voir);
        return;
    }
    envoyerAlerte(nouveaux, automatique);
}

bool GPetManager::envoyerAlerte(const QList<Produit> &produits, bool automatique)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString erreur;
    const bool ok = Produit::envoyerAlerte(produits, fournisseurs, automatique, erreur);
    QApplication::restoreOverrideCursor();

    afficherSousSeuil();
    afficherHistorique();

    if (ok) {
        echecEnvoiAuto = false;
        afficherToast("mail_teal", "Alerte e-mail envoyée",
                      QString("%1 produit(s) signalé(s) à %2.")
                          .arg(produits.size()).arg(Produit::configurationEmail().destinataire), "#2A8C86");
    } else if (automatique) {
        echecEnvoiAuto = true;
        afficherToast("warning_red", "Échec de l'alerte e-mail", erreur, "#E53935",
                      "Paramètres", [this]() { changerPage(PAGE_ALERTES); });
    } else {
        QMessageBox::critical(this, "Échec de l'envoi", "L'alerte n'a pas pu être envoyée :\n\n" + erreur);
    }
    return ok;
}

void GPetManager::on_btnVerifierStock_clicked()
{
    Etmp.actualiserStatuts();
    rafraichir();
    verifierStock(false);
}

void GPetManager::on_btnEnvoyerAlerteStock_clicked()
{
    const QList<Produit> liste = Etmp.listeSousSeuil();
    if (liste.isEmpty()) {
        QMessageBox::information(this, "Alerte e-mail", "Aucun produit n'a atteint son seuil minimal.");
        return;
    }
    if (!Produit::configurationComplete(Produit::configurationEmail())) {
        QMessageBox::warning(this, "Alerte e-mail",
                             "Renseignez puis enregistrez les paramètres e-mail (serveur, expéditeur, "
                             "mot de passe d'application et e-mail du responsable).");
        ui->txtSmtpUtilisateurStock->setFocus();
        return;
    }
    envoyerAlerte(liste, false);
}

void GPetManager::afficherSousSeuil()
{
    const QList<Produit> liste = Etmp.listeSousSeuil();
    const int facteur = ui->spinFacteurStock->value();
    const QList<Produit> nonSignales = Produit::produitsNonSignales(liste);

    QTableWidget *t = ui->tableSousSeuilStock;
    t->setRowCount(0);
    t->setRowCount(liste.size());
    double total = 0;
    for (int l = 0; l < liste.size(); ++l) {
        const Produit &p = liste[l];
        const int qte = Produit::quantiteRecommandee(p.getQuantite(), p.getSeuilMin(), facteur);
        total += qte * p.getPrixUnitaire();

        t->setItem(l, 0, new QTableWidgetItem(QString::number(p.getIdProduit())));
        t->setItem(l, 1, new QTableWidgetItem(iconeCategorie(p.getCategorie()), p.getNom()));
        QString fond, couleur;
        couleursCategorie(p.getCategorie(), fond, couleur);
        t->setCellWidget(l, 2, creerBadge(p.getCategorie(), fond, couleur));
        QTableWidgetItem *itQte = new QTableWidgetItem(QString::number(p.getQuantite()));
        itQte->setForeground(QColor(p.getQuantite() == 0 ? "#C62828" : "#E08A00"));
        t->setItem(l, 3, itQte);
        t->setItem(l, 4, new QTableWidgetItem(QString::number(p.getSeuilMin())));
        t->setCellWidget(l, 5, creerBadge(QString::number(qte) + " unités", "#E4F2F1", "#1F7A6E"));
        t->setItem(l, 6, new QTableWidgetItem(QLocale(QLocale::French).toString(qte * p.getPrixUnitaire(), 'f', 3)
                                              + " TND"));
        t->setItem(l, 7, new QTableWidgetItem(fournisseurs.value(p.getIdProduit(), "Non renseigné")));
        const QDateTime derniere = Produit::derniereAlerte(p.getIdProduit());
        t->setItem(l, 8, new QTableWidgetItem(derniere.isValid() ? derniere.toString("dd/MM/yyyy HH:mm")
                                                                 : QString("En attente")));

        QWidget *actions = new QWidget;
        QHBoxLayout *lay = new QHBoxLayout(actions);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(2);
        QPushButton *btnEdit = creerBoutonIcone("pencil_orange", "Modifier ce produit (réapprovisionner)");
        QPushButton *btnIA   = creerBoutonIcone("ia_teal", "Alternatives proposées par l'IA");
        lay->addWidget(btnEdit);
        lay->addWidget(btnIA);
        lay->addStretch();
        const int id = p.getIdProduit();
        const QString nom = p.getNom(), categorie = p.getCategorie();
        connect(btnEdit, &QPushButton::clicked, this, [this, id]() { chargerDansFormulaire(id); });
        connect(btnIA, &QPushButton::clicked, this, [this, id, nom, categorie]() {
            changerPage(PAGE_STOCK);
            demanderAlternatives(nom, categorie, id);
        });
        t->setCellWidget(l, 9, actions);
    }
    ajusterHauteur(t, liste.size());
    t->setVisible(!liste.isEmpty());
    ui->lblAucunSousSeuilStock->setVisible(liste.isEmpty());
    ui->btnEnvoyerAlerteStock->setEnabled(!liste.isEmpty());

    ui->lblCompteursStock->setText(
        QString("<b>%1</b> produit(s) au seuil minimal · <b>%2</b> déjà signalé(s) · "
                "<b style='color:#E08A00'>%3</b> en attente d'alerte · Montant estimé : <b>%4 TND</b> · "
                "Envoi automatique : <b>%5</b>")
            .arg(liste.size()).arg(liste.size() - nonSignales.size()).arg(nonSignales.size())
            .arg(QLocale(QLocale::French).toString(total, 'f', 3))
            .arg(Produit::envoiAutomatique() ? QString("activé") : QString("désactivé")));
}

void GPetManager::afficherHistorique()
{
    const QList<EntreeHistorique> liste = Produit::historiqueAlertes();
    QTableWidget *t = ui->tableHistoriqueStock;
    t->setRowCount(0);
    t->setRowCount(liste.size());
    for (int l = 0; l < liste.size(); ++l) {
        const EntreeHistorique &e = liste[l];
        t->setItem(l, 0, new QTableWidgetItem(e.date.toString("dd/MM/yyyy HH:mm")));
        t->setItem(l, 1, new QTableWidgetItem(e.destinataire));
        QTableWidgetItem *itProduits = new QTableWidgetItem(e.produits);
        itProduits->setToolTip(e.produits);
        t->setItem(l, 2, itProduits);
        t->setItem(l, 3, new QTableWidgetItem(e.automatique ? "Automatique" : "Manuel"));
        QWidget *badge = creerBadge(e.succes ? "Envoyé" : "Échec",
                                    e.succes ? "#2E8B57" : "#E53935", "white");
        badge->setToolTip(e.detail);
        t->setCellWidget(l, 4, badge);
    }
    ajusterHauteur(t, liste.size());
    t->setVisible(!liste.isEmpty());
    ui->lblAucuneAlerteStock->setVisible(liste.isEmpty());
    ui->btnViderHistoriqueStock->setEnabled(!liste.isEmpty());
}

ConfigEmail GPetManager::lireConfigEmail() const
{
    ConfigEmail c;
    c.serveur      = ui->txtSmtpServeurStock->text().trimmed();
    c.port         = ui->spinSmtpPortStock->value();
    c.utilisateur  = ui->txtSmtpUtilisateurStock->text().trimmed();
    c.motDePasse   = ui->txtSmtpMotDePasseStock->text().remove(' ');   // Gmail l'affiche par blocs
    c.destinataire = ui->txtDestinataireStock->text().trimmed();
    return c;
}

void GPetManager::on_btnEnregistrerConfigStock_clicked()
{
    const ConfigEmail c = lireConfigEmail();
    static const QRegularExpression regEmail("^[\\w.+-]+@[\\w-]+(\\.[\\w-]+)+$");

    QStringList erreurs;
    if (!c.utilisateur.isEmpty() && !regEmail.match(c.utilisateur).hasMatch())
        erreurs << "L'e-mail de l'expéditeur est invalide.";
    for (const QString &d : c.destinataire.split(QRegularExpression("[;,]"), Qt::SkipEmptyParts))
        if (!regEmail.match(d.trimmed()).hasMatch())
            erreurs << "E-mail du responsable invalide : " + d.trimmed();
    if (c.serveur.isEmpty())
        erreurs << "Le serveur SMTP est obligatoire.";
    if (!erreurs.isEmpty()) {
        QMessageBox::warning(this, "Contrôle de saisie", "• " + erreurs.join("\n• "));
        return;
    }

    Produit::enregistrerParametres(c, ui->chkAlerteAutoStock->isChecked(), ui->spinIntervalleStock->value(),
                                   ui->spinFacteurStock->value(), ui->txtCleIAStock->text(),
                                   ui->txtModeleIAStock->text());
    timerSurveillance->start(ui->spinIntervalleStock->value() * 60 * 1000);
    echecEnvoiAuto = false;
    afficherSousSeuil();
    afficherToast("check_green", "Paramètres enregistrés",
                  Produit::configurationComplete(c) ? QString("L'alerte e-mail est prête.")
                                  : QString("Paramètres e-mail encore incomplets."),
                  "#2E8B57");
}

void GPetManager::on_btnTesterEmailStock_clicked()
{
    const ConfigEmail c = lireConfigEmail();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString erreur;
    const bool ok = Produit::envoyerTest(c, erreur);
    QApplication::restoreOverrideCursor();

    if (ok)
        QMessageBox::information(this, "Test réussi",
                                 "Un e-mail de test a été envoyé à " + c.destinataire + ".");
    else
        QMessageBox::critical(this, "Test échoué", erreur);
}

void GPetManager::on_btnViderHistoriqueStock_clicked()
{
    if (QMessageBox::question(this, "Historique", "Vider l'historique des alertes e-mail ?")
        != QMessageBox::Yes)
        return;
    Produit::viderHistoriqueAlertes();
    afficherHistorique();
}

// =====================================================================
//       MÉTIER 2 : SUGGESTION INTELLIGENTE DE PRODUITS ALTERNATIFS (IA)
// =====================================================================

void GPetManager::on_btnDemanderIAStock_clicked()
{
    const QString besoin = ui->txtBesoinIAStock->text().trimmed();
    if (besoin.length() < 3) {
        QMessageBox::information(this, "Assistant IA",
                                 "Décrivez le produit recherché (au moins 3 caractères), "
                                 "par exemple : « vermifuge pour chat ».");
        ui->txtBesoinIAStock->setFocus();
        return;
    }
    demanderAlternatives(besoin, QString(), -1);
}

void GPetManager::demanderAlternatives(const QString &besoin, const QString &categorie, int idExclu)
{
    if (besoin.trimmed().isEmpty() || iaEnCours) return;
    besoinIA = besoin.trimmed();

    ui->txtBesoinIAStock->setText(besoinIA);
    ui->lblAltPourStock->setText("Analyse en cours pour : " + raccourcir(besoinIA, 26));
    ui->btnDemanderIAStock->setEnabled(false);
    ui->frameAlt1Stock->hide();
    ui->frameAlt2Stock->hide();
    ui->lblAucuneAltStock->setText(Produit::iaConfiguree() ? "L'IA analyse le catalogue…"
                                                           : "Recherche locale…");
    ui->lblAucuneAltStock->show();

    // Tout le travail (lecture du catalogue STOCK + appel de l'IA) est fait par la classe Produit
    iaEnCours = true;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString conseil, avertissement;
    bool viaIA = false;
    const QList<SuggestionIA> suggestions =
        Etmp.suggererAlternatives(besoinIA, categorie, idExclu, conseil, viaIA, avertissement);
    QApplication::restoreOverrideCursor();
    iaEnCours = false;

    afficherSuggestionsIA(suggestions, conseil, viaIA, avertissement);
}

void GPetManager::afficherSuggestionsIA(const QList<SuggestionIA> &suggestions, const QString &conseil,
                                        bool viaIA, const QString &avertissement)
{
    suggestionsIA = suggestions;
    ui->btnDemanderIAStock->setEnabled(true);
    ui->lblAltPourStock->setText("Pour : " + raccourcir(besoinIA, 24)
                                 + (viaIA ? "  ·  IA Gemini" : "  ·  moteur local"));

    // ---- Panneau compact (2 cartes) ----
    const QList<QFrame*> cadres = {ui->frameAlt1Stock, ui->frameAlt2Stock};
    const QList<QLabel*> icones = {ui->lblAltIcone1Stock, ui->lblAltIcone2Stock};
    const QList<QLabel*> noms   = {ui->lblAltNom1Stock, ui->lblAltNom2Stock};
    const QList<QLabel*> scores = {ui->lblAltCat1Stock, ui->lblAltCat2Stock};
    const QList<QLabel*> infos  = {ui->lblAltQte1Stock, ui->lblAltQte2Stock};
    for (int k = 0; k < 2; ++k) {
        if (k < suggestions.size()) {
            const SuggestionIA &s = suggestions[k];
            const QString couleur = s.pertinence >= 75 ? "#2E8B57" : (s.pertinence >= 50 ? "#E08A00" : "#6B8080");
            icones[k]->setPixmap(iconeCategorie(s.categorie).pixmap(28, 28));
            noms[k]->setText(raccourcir(s.nom, 26));
            scores[k]->setText(QString("<span style='color:%1; font-weight:bold'>Pertinence : %2 %</span>")
                                   .arg(couleur).arg(s.pertinence));
            infos[k]->setText(QString("Stock : %1 · %2").arg(s.quantite).arg(s.categorie));
            cadres[k]->setToolTip(s.raison + "\n\n" + s.avantages);
            cadres[k]->show();
        } else {
            cadres[k]->hide();
        }
    }
    ui->lblAucuneAltStock->setText("Aucune alternative pertinente n'est disponible en stock.");
    ui->lblAucuneAltStock->setVisible(suggestions.isEmpty());

    // ---- Analyse détaillée (justifications) ----
    QString texte = "<b>Besoin analysé :</b> " + besoinIA.toHtmlEscaped();
    if (!conseil.isEmpty())
        texte += "<br><b>" + QString(viaIA ? "Conseil de l'IA" : "Remarque") + " :</b> " + conseil.toHtmlEscaped();
    if (!avertissement.isEmpty())
        texte += "<br><span style='color:#C62828'><b>IA indisponible :</b> " + avertissement.toHtmlEscaped()
                 + "</span>";
    texte += "<br><span style='color:#6B8080'>Ces propositions sont une aide : la décision finale "
             "reste à l'utilisateur.</span>";
    ui->lblConseilIAStock->setText(texte);

    QTableWidget *t = ui->tableSuggestionsIAStock;
    t->setRowCount(0);
    t->setRowCount(suggestions.size());
    for (int l = 0; l < suggestions.size(); ++l) {
        const SuggestionIA &s = suggestions[l];
        t->setItem(l, 0, new QTableWidgetItem(iconeCategorie(s.categorie), s.nom));
        QString fond, couleur;
        couleursCategorie(s.categorie, fond, couleur);
        t->setCellWidget(l, 1, creerBadge(s.categorie, fond, couleur));
        t->setItem(l, 2, new QTableWidgetItem(QString::number(s.quantite)));
        const QString fondScore = s.pertinence >= 75 ? "#2E8B57" : (s.pertinence >= 50 ? "#F5A623" : "#90A4A4");
        t->setCellWidget(l, 3, creerBadge(QString::number(s.pertinence) + " %", fondScore, "white"));
        t->setItem(l, 4, new QTableWidgetItem(s.raison));
        t->setItem(l, 5, new QTableWidgetItem(s.avantages));

        QWidget *decision = new QWidget;
        QHBoxLayout *lay = new QHBoxLayout(decision);
        lay->setContentsMargins(4, 0, 4, 0);
        lay->setSpacing(6);
        QPushButton *btnUtiliser = new QPushButton("Utiliser");
        QPushButton *btnVoir = new QPushButton("Voir");
        btnUtiliser->setToolTip("Sortie de stock de ce produit à la place du produit demandé");
        btnVoir->setToolTip("Afficher ce produit dans le formulaire");
        btnUtiliser->setStyleSheet("QPushButton{background:#2A8C86;color:white;border:none;border-radius:6px;"
                                   "padding:6px 10px;font-weight:bold;} QPushButton:hover{background:#237872;}");
        btnVoir->setStyleSheet("QPushButton{background:white;color:#2A8C86;border:1px solid #2A8C86;"
                               "border-radius:6px;padding:5px 10px;} QPushButton:hover{background:#E6F2F1;}");
        for (QPushButton *b : {btnUtiliser, btnVoir}) {
            b->setCursor(Qt::PointingHandCursor);
            lay->addWidget(b);
        }
        lay->addStretch();
        const int id = s.idProduit;
        connect(btnUtiliser, &QPushButton::clicked, this, [this, id]() {
            QTimer::singleShot(0, this, [this, id]() { sortieDeStock(id); });
        });
        connect(btnVoir, &QPushButton::clicked, this, [this, id]() { chargerDansFormulaire(id); });
        t->setCellWidget(l, 6, decision);
    }
    t->setVisible(!suggestions.isEmpty());
    // Hauteur des lignes calculée une fois le tableau affiché (largeur réelle connue)
    QTimer::singleShot(0, this, [t]() {
        t->resizeRowsToContents();
        int hauteur = 38 + 4;
        for (int l = 0; l < t->rowCount(); ++l) {
            t->setRowHeight(l, std::max(t->rowHeight(l) + 12, 48));
            hauteur += t->rowHeight(l);
        }
        t->setFixedHeight(hauteur);
    });
    ui->lblAucuneSuggestionIAStock->setVisible(suggestions.isEmpty());
    ui->lblAucuneSuggestionIAStock->setText("Aucune alternative pertinente n'est disponible en stock pour ce besoin.");

    if (!avertissement.isEmpty())
        afficherToast("warning_orange", "IA indisponible : moteur local utilisé", avertissement, "#F5A623");
}

// =====================================================================
//                         NOTIFICATIONS
// =====================================================================

void GPetManager::afficherNotifications()
{
    QMenu menu(this);
    menu.setStyleSheet("QMenu{background:white;border:1px solid #E0E8E8;padding:6px;}"
                       "QMenu::item{padding:7px 16px;color:#1E3A3A;}"
                       "QMenu::item:selected{background:#E6F2F1;}"
                       "QMenu::item:disabled{color:#6B8080;}"
                       "QMenu::separator{height:1px;background:#E0E8E8;margin:4px 8px;}");

    const QList<Produit> liste = Etmp.listeSousSeuil();
    if (liste.isEmpty())
        menu.addAction(icone("check_green"), "Aucune alerte : le stock est sous contrôle")->setEnabled(false);

    for (const Produit &p : liste) {
        const int id = p.getIdProduit();
        const QString nomIcone = p.getQuantite() == 0 ? "warning_red" : "warning_orange";
        QAction *act = menu.addAction(icone(nomIcone),
            QString("%1  —  stock : %2 / seuil : %3").arg(p.getNom()).arg(p.getQuantite()).arg(p.getSeuilMin()));
        connect(act, &QAction::triggered, this, [this, id]() { chargerDansFormulaire(id); });
    }
    menu.addSeparator();
    QAction *ouvrir = menu.addAction(icone("mail_teal"), "Ouvrir les alertes de réapprovisionnement");
    connect(ouvrir, &QAction::triggered, this, [this]() { changerPage(PAGE_ALERTES); });

    menu.exec(ui->btnNotif->mapToGlobal(QPoint(0, ui->btnNotif->height())));
}

// =====================================================================
//                         NOTIFICATION "TOAST"
// =====================================================================

void GPetManager::afficherToast(const QString &nomIcone, const QString &titre, const QString &message,
                                const QString &couleur, const QString &texteBouton,
                                std::function<void()> action)
{
    if (toast) toast->deleteLater();

    QFrame *cadre = new QFrame(ui->centralwidget);
    cadre->setObjectName("toast");
    cadre->setStyleSheet(QString("#toast { background:white; border:1px solid #E0E8E8;"
                                 "border-left:6px solid %1; border-radius:10px; }").arg(couleur));
    QGraphicsDropShadowEffect *ombre = new QGraphicsDropShadowEffect(cadre);
    ombre->setBlurRadius(24);
    ombre->setOffset(0, 4);
    ombre->setColor(QColor(0, 0, 0, 60));
    cadre->setGraphicsEffect(ombre);

    QHBoxLayout *lay = new QHBoxLayout(cadre);
    lay->setContentsMargins(14, 12, 10, 12);
    lay->setSpacing(12);

    QLabel *lblIcone = new QLabel;
    lblIcone->setPixmap(GPetManager::icone(nomIcone).pixmap(28, 28));
    lblIcone->setFixedSize(28, 28);
    QLabel *lblTexte = new QLabel(QString("<b style='color:#17363A; font-size:13px'>%1</b><br>"
                                          "<span style='color:#5F7373; font-size:12px'>%2</span>")
                                      .arg(titre.toHtmlEscaped(), message.toHtmlEscaped()));
    lblTexte->setWordWrap(true);
    lay->addWidget(lblIcone);
    lay->addWidget(lblTexte, 1);

    if (!texteBouton.isEmpty() && action) {
        QPushButton *btn = new QPushButton(texteBouton);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString("QPushButton{background:%1;color:white;border:none;border-radius:6px;"
                                   "padding:7px 12px;font-weight:bold;}"
                                   "QPushButton:hover{background:#17363A;}").arg(couleur));
        connect(btn, &QPushButton::clicked, this, [this, action]() {
            if (toast) toast->hide();
            QTimer::singleShot(0, this, action);
        });
        lay->addWidget(btn);
    }

    QPushButton *fermer = new QPushButton("×");
    fermer->setFixedSize(26, 26);
    fermer->setCursor(Qt::PointingHandCursor);
    fermer->setStyleSheet("QPushButton{border:none;background:transparent;color:#6B8080;font-size:14px;}"
                          "QPushButton:hover{color:#17363A;}");
    connect(fermer, &QPushButton::clicked, cadre, &QWidget::hide);
    lay->addWidget(fermer, 0, Qt::AlignTop);

    cadre->setFixedWidth(470);
    cadre->adjustSize();
    cadre->move(ui->centralwidget->width() - cadre->width() - 28,
                ui->centralwidget->height() - cadre->height() - 28);
    cadre->show();
    cadre->raise();

    toast = cadre;
    timerToast->start(texteBouton.isEmpty() ? 5000 : 9000);
}

// =====================================================================
//                         OUTILS D'AFFICHAGE
// =====================================================================

void GPetManager::ajusterHauteur(QAbstractItemView *vue, int lignes, int hauteurLigne)
{
    const int visibles = std::max(1, std::min(lignes, 10));
    vue->setFixedHeight(38 + visibles * hauteurLigne + 4);   // 38 = hauteur de l'en-tête
}

// Icône dessinée stockée dans les ressources (dossier icones/)
QIcon GPetManager::icone(const QString &nom)
{
    return QIcon(":/icones/" + nom + ".png");
}

QIcon GPetManager::iconeCategorie(const QString &categorie) const
{
    if (categorie == "Alimentation") return icone("bowl_green");
    if (categorie == "Médicament")   return icone("pill_red");
    return icone("tag_blue");
}

void GPetManager::couleursCategorie(const QString &categorie, QString &fond, QString &couleur)
{
    if (categorie == "Alimentation")    { fond = "#DFF3E6"; couleur = "#2E7D32"; }
    else if (categorie == "Médicament") { fond = "#FDE3E3"; couleur = "#C62828"; }
    else                                { fond = "#E3EEFB"; couleur = "#1565C0"; }
}

QString GPetManager::couleurStatut(const QString &statutBD)
{
    if (statutBD == "Disponible") return "#2E8B57";
    if (statutBD == "Stock bas")  return "#F5A623";
    return "#E53935";                         // Rupture / Expiré
}

QWidget *GPetManager::creerBadge(const QString &texte, const QString &fond, const QString &couleur)
{
    QWidget *w = new QWidget;
    QHBoxLayout *lay = new QHBoxLayout(w);
    lay->setContentsMargins(4, 0, 4, 0);

    QLabel *lbl = new QLabel(texte);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setFixedHeight(24);
    lbl->setStyleSheet(QString("background-color:%1; color:%2; border-radius:12px;"
                               "padding:0 10px; font-size:11px; font-weight:bold;")
                           .arg(fond, couleur));
    lay->addWidget(lbl);
    lay->addStretch();
    return w;
}

QPushButton *GPetManager::creerBoutonIcone(const QString &nomIcone, const QString &infobulle)
{
    QPushButton *b = new QPushButton;
    b->setIcon(icone(nomIcone));
    b->setToolTip(infobulle);
    b->setFixedSize(28, 28);
    b->setIconSize(QSize(18, 18));
    b->setCursor(Qt::PointingHandCursor);
    b->setStyleSheet("QPushButton{border:none;background:transparent;}"
                     "QPushButton:hover{background:#E6F2F1;border-radius:6px;}");
    return b;
}

QWidget *GPetManager::creerActions(int id)
{
    QWidget *w = new QWidget;
    QHBoxLayout *lay = new QHBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(2);

    QPushButton *btnSortie = creerBoutonIcone("sortie_teal", "Sortie de stock (utilisation)");
    QPushButton *btnEdit   = creerBoutonIcone("pencil_orange", "Modifier");
    QPushButton *btnIA     = creerBoutonIcone("ia_teal", "Alternatives proposées par l'IA");
    QPushButton *btnSuppr  = creerBoutonIcone("trash_red", "Supprimer");
    for (QPushButton *b : {btnSortie, btnEdit, btnIA, btnSuppr}) lay->addWidget(b);
    lay->addStretch();

    connect(btnEdit, &QPushButton::clicked, this, [this, id]() { chargerDansFormulaire(id); });
    connect(btnIA, &QPushButton::clicked, this, [this, id]() {
        Produit p;
        if (p.chargerParId(id)) demanderAlternatives(p.getNom(), p.getCategorie(), id);
    });
    // Ces actions reconstruisent le tableau : on les lance juste après le clic
    connect(btnSortie, &QPushButton::clicked, this, [this, id]() {
        QTimer::singleShot(0, this, [this, id]() { sortieDeStock(id); });
    });
    connect(btnSuppr, &QPushButton::clicked, this, [this, id]() {
        QTimer::singleShot(0, this, [this, id]() { supprimerProduit(id); });
    });
    return w;
}

// =====================================================================
//    MÉTIER BASIQUE 3 : FICHE DE COMMANDE DE RÉAPPROVISIONNEMENT (EXCEL)
// =====================================================================

void GPetManager::genererFicheCommande()
{
    const QList<Produit> liste = Etmp.listeSousSeuil();   // produits à réapprovisionner
    if (liste.isEmpty()) {
        QMessageBox::information(this, "Fiche de commande",
                                 "Aucun produit n'a atteint son seuil minimal : rien à commander.");
        return;
    }

    const QString numero = Produit::numeroCommande();
    const QString dossier = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString chemin = QFileDialog::getSaveFileName(this, "Enregistrer la fiche de commande",
                                                  dossier + "/Fiche_commande_" + numero + ".xlsx",
                                                  "Classeur Excel (*.xlsx)");
    if (chemin.isEmpty()) return;
    if (!chemin.endsWith(".xlsx", Qt::CaseInsensitive)) chemin += ".xlsx";

    QString erreur;
    if (!Produit::genererFicheCommande(chemin, liste, fournisseurs, ui->spinFacteurStock->value(), numero, erreur)) {
        QMessageBox::critical(this, "Fiche de commande", "La fiche n'a pas pu être générée :\n" + erreur);
        return;
    }
    if (QMessageBox::question(this, "Fiche de commande",
            QString("La fiche %1 a été générée (%2 produit(s)) :\n%3\n\nVoulez-vous l'ouvrir dans Excel ?")
                .arg(numero).arg(liste.size()).arg(chemin)) == QMessageBox::Yes)
        QDesktopServices::openUrl(QUrl::fromLocalFile(chemin));
}

void GPetManager::on_btnExcelStock_clicked()
{
    genererFicheCommande();
}

void GPetManager::on_btnExcelSuggestionsStock_clicked()
{
    genererFicheCommande();
}
