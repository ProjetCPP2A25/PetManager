#include "gpetmanager.h"
#include "ui_gpetmanager.h"


#include <QApplication>
#include <QDateEdit>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIntValidator>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QPageSize>
#include <QScreen>
#include <QStringList>
#include <QPainter>
#include <QPdfWriter>
#include <QPixmap>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <functional>

static const int PAGE_COMMANDES = 4;     // index de la page dans le QStackedWidget

namespace {

// Si le .ui n'a pas de mise en page (widgets posés à des positions fixes, par exemple après un copier-coller dans
// Designer), on en crée une : le menu reste à gauche et le reste occupe toute la largeur de la fenêtre.
// Sans cela, le contenu garde sa largeur d'origine (1366 pixels) et dépasse de l'écran.
void assurerMiseEnPage(QMainWindow *fenetre, QWidget *pages)
{
    QWidget *central = fenetre->centralWidget();
    QWidget *menu = fenetre->findChild<QWidget *>("frameSidebar");
    QWidget *zone = fenetre->findChild<QWidget *>("zoneDroite");
    QWidget *entete = fenetre->findChild<QWidget *>("frameHeader");

    if (zone && !zone->layout()) {                       // en-tête au-dessus, pages en dessous
        auto *v = new QVBoxLayout(zone);
        v->setContentsMargins(0, 0, 0, 0);
        v->setSpacing(0);
        if (entete) {
            entete->setFixedHeight(80);
            v->addWidget(entete);
        }
        v->addWidget(pages, 1);
    }
    if (central && !central->layout()) {                 // menu à gauche, le reste à droite
        auto *h = new QHBoxLayout(central);
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(0);
        if (menu) {
            menu->setFixedWidth(215);
            h->addWidget(menu);
        }
        if (zone) {
            h->addWidget(zone, 1);
        } else {                                         // pas de « zoneDroite » : colonne en-tête + pages
            auto *colonne = new QVBoxLayout;
            colonne->setContentsMargins(0, 0, 0, 0);
            colonne->setSpacing(0);
            if (entete) {
                entete->setFixedHeight(80);
                colonne->addWidget(entete);
            }
            colonne->addWidget(pages, 1);
            h->addLayout(colonne, 1);
        }
    }
}

// Couleurs du badge de statut (fond, texte)
struct CouleursStatut { QString fond, texte; };
CouleursStatut couleursStatut(const QString &statut)
{
    if (statut == "Livrée")  return {"#D1FAE5", "#065F46"};
    if (statut == "Annulée") return {"#FEE2E2", "#991B1B"};
    return {"#DBEAFE", "#1D4ED8"};                            // Commandée
}

/* Colonne « Statut » : badge arrondi coloré */
class StatutDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const override
    {
        QStyleOptionViewItem o(opt);
        initStyleOption(&o, idx);
        o.text.clear();                                         // fond / sélection seulement
        QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &o, p, o.widget);

        const QString statut = idx.data(Qt::DisplayRole).toString();
        const CouleursStatut cs = couleursStatut(statut);
        const QRect r = opt.rect.adjusted(6, 9, -6, -9);

        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        p->setPen(Qt::NoPen);
        p->setBrush(QColor(cs.fond));
        p->drawRoundedRect(r, 9, 9);
        QFont f = opt.font;
        f.setBold(true);
        p->setFont(f);
        p->setPen(QColor(cs.texte));
        p->drawText(r, Qt::AlignCenter, statut);
        p->restore();
    }
};

/* Colonne « Actions » : icône crayon (modifier) et icône poubelle (supprimer), cliquables */
class ActionsDelegate : public QStyledItemDelegate
{
public:
    std::function<void(int ligne, int action)> surAction;   // action : 0 = crayon, 1 = poubelle
    using QStyledItemDelegate::QStyledItemDelegate;

    static constexpr int TAILLE = 22;
    static constexpr int ESPACE = 14;

    // Zone de l'icône i, centrée dans la cellule (dessin et clic utilisent la même zone)
    static QRect zone(const QRect &cellule, int i)
    {
        const int total = 2 * TAILLE + ESPACE;
        const int x0 = cellule.left() + (cellule.width() - total) / 2;
        return QRect(x0 + i * (TAILLE + ESPACE),
                     cellule.top() + (cellule.height() - TAILLE) / 2, TAILLE, TAILLE);
    }

    void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const override
    {
        QStyleOptionViewItem o(opt);
        initStyleOption(&o, idx);
        o.text.clear();
        QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &o, p, o.widget);

        static const QPixmap icones[2] = {QPixmap(":/icones/pen.png"),
                                          QPixmap(":/icones/delete.png")};
        p->save();
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        for (int i = 0; i < 2; ++i)
            p->drawPixmap(zone(opt.rect, i), icones[i]);
        p->restore();
    }

    bool editorEvent(QEvent *e, QAbstractItemModel *, const QStyleOptionViewItem &opt,
                     const QModelIndex &idx) override
    {
        if (e->type() == QEvent::MouseButtonRelease) {
            const auto *me = static_cast<QMouseEvent *>(e);
            for (int i = 0; i < 2; ++i) {
                if (zone(opt.rect, i).adjusted(-4, -4, 4, 4).contains(me->pos())) {
                    if (surAction)
                        surAction(idx.row(), i);
                    return true;
                }
            }
        }
        return false;
    }
};

} // namespace

GPetManager::GPetManager(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::GPetManager)
{
    ui->setupUi(this);

    assurerMiseEnPage(this, ui->SWModules);      // le contenu suit la largeur de la fenêtre

    // La fenêtre doit tenir dans l'écran : taille minimale réduite et fenêtre maximisée dès le constructeur
    // (même si main.cpp utilise show() au lieu de showMaximized(), elle ne dépasse plus de l'écran)
    setMinimumSize(1000, 600);
    if (centralWidget())
        centralWidget()->setMinimumSize(0, 0);
    if (QScreen *ecran = QGuiApplication::primaryScreen()) {
        const QRect zone = ecran->availableGeometry();   // zone utile de l'écran (sans la barre des tâches)
        resize(zone.size());
        move(zone.topLeft());
    }
    setWindowState(windowState() | Qt::WindowMaximized);

    // Logo en haut à gauche du menu latéral
    ui->lblLogo->setAlignment(Qt::AlignCenter);
    ui->lblLogo->setScaledContents(true);
    ui->lblLogo->setPixmap(QPixmap(":/icones/logo.png"));

    // En-tête : nom du médecin, avatar (cercle) et notifications du stock masqués pour le moment
    // (ils seront ajoutés dans une prochaine étape)
    for (const char *nom : {"widgetUtilisateur", "lblNomMedecin", "lblRole", "lblAvatar", "btnNotif"})
        if (QWidget *w = findChild<QWidget *>(nom))
            w->hide();

    configurerNavigation();
    configurerPageCommandes();

    fermerFormulaireCommande();      // le formulaire n'apparaît qu'après « + Ajouter »
    viderDetailsCommande();          // les détails n'apparaissent qu'au clic sur une commande
    rafraichirCommandes();           // Read : affichage de la liste au lancement
}

GPetManager::~GPetManager()
{
    delete ui;
    delete m_modeleCommandes;
}

/* ------------------------------------------------------------------
 *  Navigation (menu latéral commun)
 * ------------------------------------------------------------------ */
QIcon GPetManager::icone(const QString &nom)
{
    return QIcon(":/icones/" + nom + ".png");
}

void GPetManager::configurerNavigation()
{
    // Ordre = ordre des pages du QStackedWidget
    boutonsNav = {ui->btnAccueil, ui->btnAnimaux, ui->btnRendezVous, ui->btnStock,
                  ui->btnCommandes, ui->btnFournisseurs, ui->btnEmployes, ui->btnParametres};

    for (int i = 0; i < boutonsNav.size(); ++i)
        connect(boutonsNav[i], &QPushButton::clicked, this, [this, i]() { changerPage(i); });

    changerPage(PAGE_COMMANDES);
}

void GPetManager::changerPage(int index)
{
    const QStringList titres = {"Accueil", "Animaux", "Rendez-vous", "Gestion de stock",
                                "Gestion des Commandes", "Fournisseurs", "Employés", "Paramètres"};
    const QStringList icones = {"home", "paw", "calendar", "box", "cart", "truck", "users", "gear"};

    ui->SWModules->setCurrentIndex(index);
    ui->lblIconeTitre->setPixmap(icone(icones[index] + "_teal").pixmap(64, 64));
    ui->lblTitre->setText(titres[index]);
    boutonsNav[index]->setChecked(true);
}

/* ------------------------------------------------------------------
 *  Page Commandes : configuration
 * ------------------------------------------------------------------ */
void GPetManager::configurerPageCommandes()
{
    // Listes (remplies sans déclencher les slots automatiques)
    {
        const QSignalBlocker b1(ui->cmbStatutCommande), b2(ui->cmbTriCommande), b3(ui->cmbStatutFormCommande);
        ui->cmbStatutCommande->addItem("Tous");
        ui->cmbStatutCommande->addItems(Commande::statutsDisponibles());
        ui->cmbTriCommande->addItems(Commande::criteresTri());
        ui->cmbStatutFormCommande->addItems(Commande::statutsDisponibles());
    }

    // Calendriers : sélection de la date au clic ; « Non définie » = pas de date de livraison
    const QDate dateMin(2000, 1, 1);
    for (QDateEdit *d : {ui->dateCmdCommande, ui->dateLivCommande}) {
        d->setDisplayFormat("dd/MM/yyyy");
        d->setCalendarPopup(true);
        d->setMinimumDate(dateMin);
    }
    ui->dateLivCommande->setSpecialValueText("Non définie");
    ui->txtQuantiteCommande->setValidator(new QIntValidator(1, 99999999, this));

    // Largeurs minimales réduites : la barre d'outils se compresse au lieu de dépasser de l'écran
    ui->txtRechercheCommande->setMinimumWidth(170);
    ui->cmbStatutCommande->setMinimumWidth(120);
    ui->cmbTriCommande->setMinimumWidth(170);
    ui->btnAjouterCommande->setMinimumWidth(110);
    ui->tableCommande->setMinimumWidth(0);

    // Icône de recherche dans le champ
    ui->txtRechercheCommande->addAction(icone("search_grey"), QLineEdit::LeadingPosition);

    // Tableau (QTableView : la vue ; les données viennent du modèle)
    QTableView *t = ui->tableCommande;
    t->verticalHeader()->setDefaultSectionSize(46);
    t->setItemDelegateForColumn(3, new StatutDelegate(t));
    auto *actions = new ActionsDelegate(t);
    actions->surAction = [this](int ligne, int action) {
        // différé : on évite de détruire le modèle pendant le traitement du clic
        QTimer::singleShot(0, this, [this, ligne, action]() { actionSurLigneCommande(ligne, action); });
    };
    t->setItemDelegateForColumn(4, actions);
}

// Recharge le tableau : au lancement, après chaque ajout / modification / suppression,
// et à chaque changement de recherche, de statut ou de tri
void GPetManager::rafraichirCommandes()
{
    QAbstractItemModel *nouveau = c.rechercher(ui->txtRechercheCommande->text(),
                                               ui->cmbStatutCommande->currentText(),
                                               ui->cmbTriCommande->currentIndex());
    ui->tableCommande->setModel(nouveau);
    delete m_modeleCommandes;               // on détruit l'ancien modèle (sinon fuite de mémoire)
    m_modeleCommandes = nouveau;

    QHeaderView *h = ui->tableCommande->horizontalHeader();
    h->setSectionResizeMode(0, QHeaderView::Fixed);
    h->setSectionResizeMode(1, QHeaderView::Stretch);
    h->setSectionResizeMode(2, QHeaderView::Stretch);
    h->setSectionResizeMode(3, QHeaderView::Fixed);
    h->setSectionResizeMode(4, QHeaderView::Fixed);
    ui->tableCommande->setColumnWidth(0, 90);
    ui->tableCommande->setColumnWidth(3, 150);
    ui->tableCommande->setColumnWidth(4, 130);
    ui->tableCommande->clearSelection();
}

void GPetManager::on_txtRechercheCommande_textChanged(const QString &) { rafraichirCommandes(); }
void GPetManager::on_cmbStatutCommande_currentIndexChanged(int) { rafraichirCommandes(); }
void GPetManager::on_cmbTriCommande_currentIndexChanged(int) { rafraichirCommandes(); }

/* ------------------------------------------------------------------
 *  Read : détails d'une commande (clic sur une ligne du tableau)
 * ------------------------------------------------------------------ */
void GPetManager::viderDetailsCommande()
{
    m_details = Commande();
    ui->frameDetailsCommande->setVisible(false);
}

void GPetManager::afficherDetailsCommande(int id)
{
    if (!c.charger(id)) {
        QMessageBox::warning(this, "Erreur", c.erreur());
        return;
    }
    m_details = c;

    ui->lblDetVal0Commande->setText(Commande::formaterId(c.getId()));
    ui->lblDetVal1Commande->setText(Commande::formaterDate(c.getDateCommande()));
    ui->lblDetVal2Commande->setText(c.getDateLivraison().isValid()
                                        ? Commande::formaterDate(c.getDateLivraison()) : QString("—"));
    ui->lblDetVal3Commande->setText(c.getStatut());
    const CouleursStatut cs = couleursStatut(c.getStatut());
    ui->lblDetVal3Commande->setAlignment(Qt::AlignCenter);
    ui->lblDetVal3Commande->setStyleSheet(QString("background-color: %1; color: %2; border-radius: 10px;"
                                                  "font-weight: bold; font-size: 13px; padding: 2px 8px;")
                                              .arg(cs.fond, cs.texte));
    ui->lblDetVal4Commande->setText(c.getNomFournisseur().isEmpty() ? QString("—") : c.getNomFournisseur());
    ui->lblDetVal5Commande->setText(c.getQuantite() > 0 ? QString::number(c.getQuantite()) : QString("—"));
    ui->lblDetVal6Commande->setText(c.getMontantTotal() > 0
                                        ? QString::number(c.getMontantTotal(), 'f', 3) + " TND"
                                        : QString("Non défini"));
    ui->frameDetailsCommande->setVisible(true);   // visible seulement après un clic sur une commande
}

void GPetManager::on_tableCommande_clicked(const QModelIndex &index)
{
    if (!m_modeleCommandes || !index.isValid() || index.column() == 4)    // colonne Actions : gérée par le delegate
        return;
    afficherDetailsCommande(m_modeleCommandes->data(m_modeleCommandes->index(index.row(), 0), Qt::EditRole).toInt());
}

/* ------------------------------------------------------------------
 *  Formulaire « Ajouter une commande » / « Modifier la commande »
 * ------------------------------------------------------------------ */
void GPetManager::viderFormulaireCommande()
{
    m_mode = ModeFormulaire::Ajout;
    m_idEnCours = 0;
    ui->dateCmdCommande->setDate(QDate::currentDate());
    ui->dateLivCommande->setDate(ui->dateLivCommande->minimumDate());   // « Non définie »
    ui->txtFournisseurCommande->clear();
    ui->txtQuantiteCommande->clear();
    ui->cmbStatutFormCommande->setCurrentIndex(0);
    ui->lblTitreFormCommande->setText("Ajouter une commande");
}

void GPetManager::fermerFormulaireCommande()
{
    viderFormulaireCommande();
    ui->frameFormCommande->setVisible(false);
}

void GPetManager::on_btnAjouterCommande_clicked()       // « + Ajouter » : ouvre le formulaire
{
    viderFormulaireCommande();
    ui->tableCommande->clearSelection();
    ui->frameFormCommande->setVisible(true);
    ui->dateCmdCommande->setFocus();
}

void GPetManager::on_btnAnnulerCommande_clicked()
{
    fermerFormulaireCommande();
    ui->tableCommande->clearSelection();
}

// Icône crayon : charge la commande dans le formulaire
void GPetManager::ouvrirEditionCommande(int id)
{
    if (!c.charger(id)) {
        QMessageBox::warning(this, "Erreur", c.erreur());
        return;
    }
    m_mode = ModeFormulaire::Modification;
    m_idEnCours = id;
    ui->dateCmdCommande->setDate(c.getDateCommande());
    ui->dateLivCommande->setDate(c.getDateLivraison().isValid() ? c.getDateLivraison()
                                                                : ui->dateLivCommande->minimumDate());
    ui->txtFournisseurCommande->setText(c.getNomFournisseur());
    ui->txtQuantiteCommande->setText(c.getQuantite() > 0 ? QString::number(c.getQuantite()) : QString());
    ui->cmbStatutFormCommande->setCurrentText(c.getStatut());
    ui->lblTitreFormCommande->setText("Modifier la commande " + Commande::formaterId(id));
    ui->frameFormCommande->setVisible(true);
    ui->dateCmdCommande->setFocus();
}

void GPetManager::on_btnEnregistrerCommande_clicked()
{
    // Dates choisies dans les calendriers ; « Non définie » (date minimale) = pas de livraison
    const QDate dateCmd = ui->dateCmdCommande->date();
    const QDate dateLiv = (ui->dateLivCommande->date() == ui->dateLivCommande->minimumDate())
                              ? QDate() : ui->dateLivCommande->date();

    // Objet Commande créé avec le constructeur à partir des champs du formulaire
    const bool edition = (m_mode == ModeFormulaire::Modification);
    c = Commande(edition ? m_idEnCours : 0, dateCmd, dateLiv,
                 ui->cmbStatutFormCommande->currentText(),
                 ui->txtFournisseurCommande->text().trimmed(),
                 ui->txtQuantiteCommande->text().toInt());

    if (!edition) {
        if (c.ajouter()) {
            rafraichirCommandes();
            fermerFormulaireCommande();
            QMessageBox::information(this, "Succès",
                                     QString("Commande %1 ajoutée.").arg(Commande::formaterId(c.getId())));
        } else {
            QMessageBox::warning(this, "Ajout impossible", c.erreur());
        }
    } else {
        if (c.modifier()) {
            const int idModifie = c.getId();
            rafraichirCommandes();
            if (ui->frameDetailsCommande->isVisible() && m_details.getId() == idModifie)
                afficherDetailsCommande(idModifie);       // détails déjà ouverts : on les met à jour
            fermerFormulaireCommande();
            QMessageBox::information(this, "Succès", "Commande modifiée.");
        } else {
            QMessageBox::warning(this, "Modification impossible", c.erreur());
        }
    }
}

/* ------------------------------------------------------------------
 *  Icônes crayon (modifier) et poubelle (supprimer) de la colonne « Actions »
 * ------------------------------------------------------------------ */
void GPetManager::actionSurLigneCommande(int ligne, int action)
{
    if (!m_modeleCommandes)
        return;
    const int id = m_modeleCommandes->data(m_modeleCommandes->index(ligne, 0), Qt::EditRole).toInt();
    ui->tableCommande->selectRow(ligne);

    if (action == 0)
        ouvrirEditionCommande(id);
    else
        supprimerCommande(id);
}

void GPetManager::supprimerCommande(int id)
{
    const auto rep = QMessageBox::question(this, "Confirmation",
                                           QString("Supprimer la commande %1 ?").arg(Commande::formaterId(id)));
    if (rep != QMessageBox::Yes)
        return;

    if (c.supprimer(id)) {
        if (m_idEnCours == id)
            fermerFormulaireCommande();
        const bool etaitAffichee = (m_details.getId() == id);
        rafraichirCommandes();
        if (etaitAffichee)
            viderDetailsCommande();
        QMessageBox::information(this, "Succès", "Commande supprimée.");
    } else {
        QMessageBox::warning(this, "Suppression impossible", c.erreur());
    }
}

/* ------------------------------------------------------------------
 *  Bouton « Exporter » : PDF des détails de la commande affichée
 * ------------------------------------------------------------------ */
void GPetManager::on_btnExporterCommande_clicked()
{
    if (m_details.getId() <= 0) {
        QMessageBox::warning(this, "Export", "Cliquez d'abord sur une commande du tableau.");
        return;
    }

    const QString chemin = QFileDialog::getSaveFileName(
        this, "Exporter la commande",
        QDir::homePath() + "/commande_" + Commande::formaterId(m_details.getId()) + ".pdf",
        "Fichiers PDF (*.pdf)");
    if (chemin.isEmpty())
        return;

    QFile fichier(chemin);
    if (!fichier.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Erreur", "Impossible de créer le PDF.");
        return;
    }

    QPdfWriter pdf(&fichier);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setResolution(300);
    QPainter painter(&pdf);

    int y = 300;
    painter.setFont(QFont("Segoe UI", 18, QFont::Bold));
    painter.setPen(QColor("#2A8C86"));
    painter.drawText(200, y, "Pet Manager - Commande " + Commande::formaterId(m_details.getId()));
    y += 300;

    const QStringList lignes = {
                                 "Date de commande : " + Commande::formaterDate(m_details.getDateCommande()),
                                 "Date de livraison : " + (m_details.getDateLivraison().isValid()
                                                               ? Commande::formaterDate(m_details.getDateLivraison()) : QString("—")),
                                 "Statut : " + m_details.getStatut(),
                                 "Fournisseur : " + m_details.getNomFournisseur(),
                                 "Quantité : " + QString::number(m_details.getQuantite()),
                                 "Montant total : " + (m_details.getMontantTotal() > 0
                                                           ? QString::number(m_details.getMontantTotal(), 'f', 3) + " TND"
                                                           : QString("Non défini"))};
    painter.setFont(QFont("Segoe UI", 12));
    painter.setPen(Qt::black);
    for (const QString &l : lignes) {
        painter.drawText(200, y, l);
        y += 160;
    }
    painter.end();
    fichier.close();
    QMessageBox::information(this, "Succès", "PDF exporté avec succès.");
    QDesktopServices::openUrl(QUrl::fromLocalFile(chemin));
}