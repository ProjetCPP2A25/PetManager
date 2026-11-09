#include "rendezvousvue.h"
#include "ui_rendezvousvue.h"


#include <QApplication>
#include <QDateEdit>
#include <QTimeEdit>
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

static const int PAGE_RDV = 2;     // index de la page dans le QStackedWidget

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
    if (statut == "En attente") return {"#FED7AA", "#9A3412"};
    if (statut == "Annulé")     return {"#FECDD3", "#9F1239"};
    if (statut == "Terminé")    return {"#E2E8F0", "#334155"};
    return {"#D1FAE5", "#065F46"};                            // Confirmé
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

RendezVousVue::RendezVousVue(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::RendezVousVue)
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
    configurerPageRendezVous();

    fermerFormulaireRdv();           // le formulaire n'apparaît qu'après « + Ajouter »
    viderDetailsRdv();               // les détails n'apparaissent qu'au clic sur un rendez-vous
    rafraichirRdv();                 // Read : affichage de la liste au lancement
}

RendezVousVue::~RendezVousVue()
{
    delete ui;
    delete m_modeleRdv;
}

/* ------------------------------------------------------------------
 *  Navigation (menu latéral commun)
 * ------------------------------------------------------------------ */
QIcon RendezVousVue::icone(const QString &nom)
{
    return QIcon(":/icones/" + nom + ".png");
}

void RendezVousVue::configurerNavigation()
{
    // Ordre = ordre des pages du QStackedWidget
    boutonsNav = {ui->btnAccueil, ui->btnAnimaux, ui->btnRendezVous, ui->btnStock,
                  ui->btnCommandes, ui->btnFournisseurs, ui->btnEmployes, ui->btnParametres};

    for (int i = 0; i < boutonsNav.size(); ++i)
        connect(boutonsNav[i], &QPushButton::clicked, this, [this, i]() { changerPage(i); });

    changerPage(PAGE_RDV);
}

void RendezVousVue::changerPage(int index)
{
    const QStringList titres = {"Accueil", "Animaux", "Gestion des Rendez-vous", "Gestion de stock",
                                "Commandes", "Fournisseurs", "Employés", "Paramètres"};
    const QStringList icones = {"home", "paw", "calendar", "box", "cart", "truck", "users", "gear"};

    ui->SWModules->setCurrentIndex(index);
    ui->lblIconeTitre->setPixmap(icone(icones[index] + "_teal").pixmap(64, 64));
    ui->lblTitre->setText(titres[index]);
    boutonsNav[index]->setChecked(true);
}

/* ------------------------------------------------------------------
 *  Page Rendez-vous : configuration
 * ------------------------------------------------------------------ */
void RendezVousVue::configurerPageRendezVous()
{
    // Listes (remplies sans déclencher les slots automatiques)
    {
        const QSignalBlocker b1(ui->cmbStatutRdv), b2(ui->cmbTriRdv), b3(ui->cmbStatutFormRdv), b4(ui->cmbMotifRdv);
        ui->cmbStatutRdv->addItem("Tous");
        ui->cmbStatutRdv->addItems(Rendezvous::statutsDisponibles());
        ui->cmbTriRdv->addItems(Rendezvous::criteresTri());
        ui->cmbStatutFormRdv->addItems(Rendezvous::statutsDisponibles());
        ui->cmbMotifRdv->addItems(Rendezvous::motifsDisponibles());
    }

    // Calendrier et heure
    ui->dateRdv->setDisplayFormat("dd/MM/yyyy");
    ui->dateRdv->setCalendarPopup(true);
    ui->timeRdv->setDisplayFormat("HH:mm");

    // Contrôle de saisie : seuls des chiffres dans les champs ID
    ui->txtIdRdv->setValidator(new QIntValidator(1, 99999999, this));
    ui->txtAnimalRdv->setValidator(new QIntValidator(1, 99999999, this));

    // Largeurs minimales réduites : la barre d'outils se compresse au lieu de dépasser de l'écran
    ui->txtRechercheRdv->setMinimumWidth(170);
    ui->cmbStatutRdv->setMinimumWidth(120);
    ui->cmbTriRdv->setMinimumWidth(170);
    ui->btnAjouterRdv->setMinimumWidth(110);
    ui->tableRdv->setMinimumWidth(0);

    // Icône de recherche dans le champ
    ui->txtRechercheRdv->addAction(icone("search_grey"), QLineEdit::LeadingPosition);

    // Tableau (QTableView : la vue ; les données viennent du modèle)
    QTableView *t = ui->tableRdv;
    t->verticalHeader()->setDefaultSectionSize(46);
    t->setItemDelegateForColumn(5, new StatutDelegate(t));
    auto *actions = new ActionsDelegate(t);
    actions->surAction = [this](int ligne, int action) {
        // différé : on évite de détruire le modèle pendant le traitement du clic
        QTimer::singleShot(0, this, [this, ligne, action]() { actionSurLigneRdv(ligne, action); });
    };
    t->setItemDelegateForColumn(6, actions);
}

// Recharge le tableau : au lancement, après chaque ajout / modification / suppression,
// et à chaque changement de recherche, de statut ou de tri
void RendezVousVue::rafraichirRdv()
{
    QAbstractItemModel *nouveau = r.rechercher(ui->txtRechercheRdv->text(),
                                               ui->cmbStatutRdv->currentText(),
                                               ui->cmbTriRdv->currentIndex());
    ui->tableRdv->setModel(nouveau);
    delete m_modeleRdv;                     // on détruit l'ancien modèle (sinon fuite de mémoire)
    m_modeleRdv = nouveau;

    QHeaderView *h = ui->tableRdv->horizontalHeader();
    for (int col : {0, 1, 2, 3, 5, 6})
        h->setSectionResizeMode(col, QHeaderView::Fixed);
    h->setSectionResizeMode(4, QHeaderView::Stretch);        // la colonne « Motif » prend la place restante
    ui->tableRdv->setColumnWidth(0, 85);                     // ID RDV
    ui->tableRdv->setColumnWidth(1, 100);                    // Date
    ui->tableRdv->setColumnWidth(2, 65);                     // Heure
    ui->tableRdv->setColumnWidth(3, 85);                     // ID Animal
    ui->tableRdv->setColumnWidth(5, 120);                    // Statut
    ui->tableRdv->setColumnWidth(6, 95);                     // Actions
    ui->tableRdv->clearSelection();
}

void RendezVousVue::on_txtRechercheRdv_textChanged(const QString &) { rafraichirRdv(); }
void RendezVousVue::on_cmbStatutRdv_currentIndexChanged(int) { rafraichirRdv(); }
void RendezVousVue::on_cmbTriRdv_currentIndexChanged(int) { rafraichirRdv(); }

/* ------------------------------------------------------------------
 *  Read : détails d'un rendez-vous (clic sur une ligne du tableau)
 * ------------------------------------------------------------------ */
void RendezVousVue::viderDetailsRdv()
{
    m_details = Rendezvous();
    ui->frameDetailsRdv->setVisible(false);
}

void RendezVousVue::afficherDetailsRdv(int id)
{
    if (!r.charger(id)) {
        QMessageBox::warning(this, "Erreur", r.erreur());
        return;
    }
    m_details = r;

    ui->lblDetVal0Rdv->setText(Rendezvous::formaterId(r.getId()));
    ui->lblDetVal1Rdv->setText(Rendezvous::formaterDate(r.getDate()));
    ui->lblDetVal2Rdv->setText(r.getHeure());
    ui->lblDetVal3Rdv->setText(r.getStatut());
    const CouleursStatut cs = couleursStatut(r.getStatut());
    ui->lblDetVal3Rdv->setAlignment(Qt::AlignCenter);
    ui->lblDetVal3Rdv->setStyleSheet(QString("background-color: %1; color: %2; border-radius: 10px;"
                                             "font-weight: bold; font-size: 13px; padding: 2px 8px;")
                                         .arg(cs.fond, cs.texte));
    ui->lblDetVal4Rdv->setText(QString::number(r.getIdAnimal()));
    ui->lblDetVal5Rdv->setText(r.getNomAnimal());
    ui->lblDetVal6Rdv->setText(r.getMotif());
    ui->frameDetailsRdv->setVisible(true);   // visible seulement après un clic sur un rendez-vous
}

void RendezVousVue::on_tableRdv_clicked(const QModelIndex &index)
{
    if (!m_modeleRdv || !index.isValid() || index.column() == 6)    // colonne Actions : gérée par le delegate
        return;
    afficherDetailsRdv(m_modeleRdv->data(m_modeleRdv->index(index.row(), 0), Qt::EditRole).toInt());
}

/* ------------------------------------------------------------------
 *  Formulaire « Ajouter un rendez-vous » / « Modifier le rendez-vous »
 * ------------------------------------------------------------------ */
// Valeurs proposées pour un nouveau rendez-vous : aujourd'hui, prochaine heure pleine
void RendezVousVue::viderFormulaireRdv()
{
    m_mode = ModeFormulaire::Ajout;
    m_idEnCours = 0;
    ui->txtIdRdv->clear();
    ui->txtIdRdv->setReadOnly(false);
    ui->txtIdRdv->setPlaceholderText(QString("Ex : %1").arg(Rendezvous::prochainId()));
    ui->txtAnimalRdv->clear();

    const QTime maintenant = QTime::currentTime();
    ui->dateRdv->setMinimumDate(QDate::currentDate());      // pas de date passée à la création
    if (maintenant.hour() < 23) {
        ui->dateRdv->setDate(QDate::currentDate());
        ui->timeRdv->setTime(QTime(maintenant.hour() + 1, 0));
    } else {
        ui->dateRdv->setDate(QDate::currentDate().addDays(1));
        ui->timeRdv->setTime(QTime(9, 0));
    }
    ui->cmbMotifRdv->setCurrentIndex(0);
    ui->cmbStatutFormRdv->setCurrentIndex(0);
    ui->lblTitreFormRdv->setText("Ajouter un rendez-vous");
}

void RendezVousVue::fermerFormulaireRdv()
{
    viderFormulaireRdv();
    ui->frameFormRdv->setVisible(false);
}

void RendezVousVue::on_btnAjouterRdv_clicked()            // « + Ajouter » : ouvre le formulaire
{
    viderFormulaireRdv();
    ui->tableRdv->clearSelection();
    ui->frameFormRdv->setVisible(true);
    ui->txtIdRdv->setFocus();
}

void RendezVousVue::on_btnAnnulerRdv_clicked()
{
    fermerFormulaireRdv();
    ui->tableRdv->clearSelection();
}

// Icône crayon : charge le rendez-vous dans le formulaire
void RendezVousVue::ouvrirEditionRdv(int id)
{
    if (!r.charger(id)) {
        QMessageBox::warning(this, "Erreur", r.erreur());
        return;
    }
    m_mode = ModeFormulaire::Modification;
    m_idEnCours = id;
    ui->txtIdRdv->setText(QString::number(id));
    ui->txtIdRdv->setReadOnly(true);                         // la clé primaire n'est saisie qu'à l'ajout
    ui->dateRdv->setMinimumDate(QDate(2000, 1, 1));
    ui->dateRdv->setDate(r.getDate());
    ui->timeRdv->setTime(QTime::fromString(r.getHeure(), "HH:mm"));
    ui->txtAnimalRdv->setText(QString::number(r.getIdAnimal()));
    ui->cmbMotifRdv->setCurrentText(r.getMotif());
    ui->cmbStatutFormRdv->setCurrentText(r.getStatut());
    ui->lblTitreFormRdv->setText("Modifier le rendez-vous " + Rendezvous::formaterId(id));
    ui->frameFormRdv->setVisible(true);
    ui->dateRdv->setFocus();
}

void RendezVousVue::on_btnEnregistrerRdv_clicked()
{
    // Objet Rendezvous créé avec le constructeur à partir des champs du formulaire
    const bool edition = (m_mode == ModeFormulaire::Modification);
    r = Rendezvous(edition ? m_idEnCours : ui->txtIdRdv->text().toInt(),
                   ui->dateRdv->date(),
                   ui->timeRdv->time().toString("HH:mm"),
                   ui->cmbStatutFormRdv->currentText(),
                   ui->cmbMotifRdv->currentText(),
                   ui->txtAnimalRdv->text().toInt());

    if (!edition) {
        if (r.ajouter()) {
            rafraichirRdv();
            fermerFormulaireRdv();
            QMessageBox::information(this, "Succès",
                                     QString("Rendez-vous %1 ajouté.").arg(Rendezvous::formaterId(r.getId())));
        } else {
            QMessageBox::warning(this, "Ajout impossible", r.erreur());
        }
    } else {
        if (r.modifier()) {
            const int idModifie = r.getId();
            rafraichirRdv();
            if (ui->frameDetailsRdv->isVisible() && m_details.getId() == idModifie)
                afficherDetailsRdv(idModifie);                // détails déjà ouverts : on les met à jour
            fermerFormulaireRdv();
            QMessageBox::information(this, "Succès", "Rendez-vous modifié.");
        } else {
            QMessageBox::warning(this, "Modification impossible", r.erreur());
        }
    }
}

/* ------------------------------------------------------------------
 *  Icônes crayon (modifier) et poubelle (supprimer) de la colonne « Actions »
 * ------------------------------------------------------------------ */
void RendezVousVue::actionSurLigneRdv(int ligne, int action)
{
    if (!m_modeleRdv)
        return;
    const int id = m_modeleRdv->data(m_modeleRdv->index(ligne, 0), Qt::EditRole).toInt();
    ui->tableRdv->selectRow(ligne);

    if (action == 0)
        ouvrirEditionRdv(id);
    else
        supprimerRdv(id);
}

void RendezVousVue::supprimerRdv(int id)
{
    const auto rep = QMessageBox::question(this, "Confirmation",
                                           QString("Supprimer le rendez-vous %1 ?").arg(Rendezvous::formaterId(id)));
    if (rep != QMessageBox::Yes)
        return;

    if (r.supprimer(id)) {
        if (m_idEnCours == id)
            fermerFormulaireRdv();
        const bool etaitAffiche = (m_details.getId() == id);
        rafraichirRdv();
        if (etaitAffiche)
            viderDetailsRdv();
        QMessageBox::information(this, "Succès", "Rendez-vous supprimé.");
    } else {
        QMessageBox::warning(this, "Suppression impossible", r.erreur());
    }
}

/* ------------------------------------------------------------------
 *  Bouton « Exporter » : PDF des détails du rendez-vous affiché
 * ------------------------------------------------------------------ */
void RendezVousVue::on_btnExporterRdv_clicked()
{
    if (m_details.getId() <= 0) {
        QMessageBox::warning(this, "Export", "Cliquez d'abord sur un rendez-vous du tableau.");
        return;
    }

    const QString chemin = QFileDialog::getSaveFileName(
        this, "Exporter le rendez-vous",
        QDir::homePath() + "/rendez-vous_" + Rendezvous::formaterId(m_details.getId()) + ".pdf",
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
    painter.drawText(200, y, "Pet Manager - Rendez-vous " + Rendezvous::formaterId(m_details.getId()));
    y += 300;

    const QStringList lignes = {
                                 "Date : " + Rendezvous::formaterDate(m_details.getDate()),
                                 "Heure : " + m_details.getHeure(),
                                 "Statut : " + m_details.getStatut(),
                                 "ID animal : " + QString::number(m_details.getIdAnimal()),
                                 "Animal : " + m_details.getNomAnimal(),
                                 "Motif : " + m_details.getMotif()};
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