#include "parametrespage.h"
#include "authmanager.h"
#include "customdialog.h"
#include "databasemanager.h"
#include "dialogsauth.h"
#include "langue.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSignalBlocker>
#include <QUrl>
#include <QVBoxLayout>

// ------------------------------------------------------------ lecture des réglages
QString ParametresPage::nomCentre()
{
    QSettings s("PetManager", "PetManager");
    return s.value("general/nomCentre").toString();
}

QString ParametresPage::pageDemarrage()
{
    QSettings s("PetManager", "PetManager");
    return s.value("general/pageDemarrage", "Accueil").toString();
}

bool ParametresPage::alerteSanteAnimaux()
{
    QSettings s("PetManager", "PetManager");
    return s.value("animaux/alerteSante", true).toBool();
}

// ------------------------------------------------------------ construction
ParametresPage::ParametresPage(QWidget *parent) : QWidget(parent)
{
    setObjectName("pageParametres");
    setAttribute(Qt::WA_StyledBackground, true);

    auto lier = [this](auto *widget, const char *cle, const QString &prefixe = QString()) {
        m_traducteurs.append([widget, cle, prefixe]() { widget->setText(prefixe + Langue::t(cle)); });
    };
    auto etiquette = [this, &lier](QVBoxLayout *lay, const char *cle) {
        auto *l = new QLabel;
        l->setObjectName("paLabel");
        lier(l, cle);
        lay->addWidget(l);
    };

    auto *racine = new QVBoxLayout(this);
    racine->setContentsMargins(24, 20, 24, 20);
    racine->setSpacing(16);

    // ---------------------------------------------------------- en-tête
    auto *entete = new QHBoxLayout;
    entete->setSpacing(14);
    auto *icone = new QLabel(QString::fromUtf8("⚙️"));
    icone->setObjectName("paIcone");
    icone->setAlignment(Qt::AlignCenter);
    entete->addWidget(icone);
    auto *textes = new QVBoxLayout;
    textes->setSpacing(2);
    auto *titre = new QLabel;
    titre->setObjectName("paTitre");
    lier(titre, "menu_parametres");
    auto *sousTitre = new QLabel;
    sousTitre->setObjectName("paSousTitre");
    lier(sousTitre, "par_sous");
    textes->addWidget(titre);
    textes->addWidget(sousTitre);
    entete->addLayout(textes);
    entete->addStretch();
    racine->addLayout(entete);

    // ---------------------------------------------------------- cartes (zone défilante)
    auto *zone = new QScrollArea;
    zone->setWidgetResizable(true);
    zone->setFrameShape(QFrame::NoFrame);
    auto *contenuZone = new QWidget;
    auto *grille = new QGridLayout(contenuZone);
    grille->setContentsMargins(0, 0, 8, 0);
    grille->setHorizontalSpacing(14);
    grille->setVerticalSpacing(14);
    grille->setColumnStretch(0, 1);
    grille->setColumnStretch(1, 1);

    // ----- Compte
    QVBoxLayout *lCompte = nullptr;
    QFrame *cCompte = creerCarte("carte_compte", QString::fromUtf8("👤"), lCompte);
    etiquette(lCompte, "compte_connecte");
    auto *rangeeCompte = new QHBoxLayout;
    rangeeCompte->setSpacing(12);
    m_avatar = new QLabel(QString::fromUtf8("👤"));
    m_avatar->setObjectName("paAvatar");
    m_avatar->setAlignment(Qt::AlignCenter);
    rangeeCompte->addWidget(m_avatar);
    auto *infos = new QVBoxLayout;
    infos->setSpacing(0);
    m_nomCompte = new QLabel;
    m_nomCompte->setObjectName("paNom");
    m_roleCompte = new QLabel;
    m_roleCompte->setObjectName("paTexte");
    infos->addWidget(m_nomCompte);
    infos->addWidget(m_roleCompte);
    rangeeCompte->addLayout(infos, 1);
    lCompte->addLayout(rangeeCompte);

    auto *btnChangerUser = new QPushButton;
    btnChangerUser->setObjectName("paSecondaire");
    btnChangerUser->setCursor(Qt::PointingHandCursor);
    lier(btnChangerUser, "changer_user", QString::fromUtf8("🔄  "));
    connect(btnChangerUser, &QPushButton::clicked, this, &ParametresPage::deconnexionDemandee);
    lCompte->addWidget(btnChangerUser);

    auto *btnMotDePasse = new QPushButton;
    btnMotDePasse->setObjectName("paSecondaire");
    btnMotDePasse->setCursor(Qt::PointingHandCursor);
    lier(btnMotDePasse, "mdp_titre", QString::fromUtf8("🔑  "));
    connect(btnMotDePasse, &QPushButton::clicked, this, [this]() {
        if (AuthManager::estConnecte())
            DialogsAuth::changerMotDePasse(window(), AuthManager::utilisateurCourant());
    });
    lCompte->addWidget(btnMotDePasse);
    grille->addWidget(cCompte, 0, 0);

    // ----- Langue
    QVBoxLayout *lLangue = nullptr;
    QFrame *cLangue = creerCarte("carte_langue", QString::fromUtf8("🌐"), lLangue);
    etiquette(lLangue, "langue_label");
    m_langue = new QComboBox;
    m_langue->setFixedHeight(34);
    for (int c = Langue::Francais; c <= Langue::Arabe; ++c)
        m_langue->addItem(Langue::nomLangue(static_cast<Langue::Code>(c)), c);
    lLangue->addWidget(m_langue);
    auto *noteLangue = new QLabel;
    noteLangue->setObjectName("paTexte");
    noteLangue->setWordWrap(true);
    lier(noteLangue, "langue_note");
    lLangue->addWidget(noteLangue);
    lLangue->addStretch();
    connect(m_langue, &QComboBox::currentIndexChanged, this, [this](int) {
        Langue::definir(static_cast<Langue::Code>(m_langue->currentData().toInt()));
    });
    grille->addWidget(cLangue, 0, 1);

    // ----- Général
    QVBoxLayout *lGeneral = nullptr;
    QFrame *cGeneral = creerCarte("carte_general", QString::fromUtf8("🏥"), lGeneral);
    etiquette(lGeneral, "nom_centre");
    m_centre = new QLineEdit;
    m_centre->setFixedHeight(34);
    lGeneral->addWidget(m_centre);
    lGeneral->addStretch();
    grille->addWidget(cGeneral, 1, 0);

    // ----- Démarrage
    QVBoxLayout *lDemarrage = nullptr;
    QFrame *cDemarrage = creerCarte("carte_demarrage", QString::fromUtf8("🚀"), lDemarrage);
    etiquette(lDemarrage, "page_demarrage");
    m_demarrage = new QComboBox;
    m_demarrage->setFixedHeight(34);
    m_demarrage->addItem(QString(), QString("Accueil"));
    m_demarrage->addItem(QString(), QString("Animaux"));
    m_demarrage->addItem(QString(), QString("Stock"));
    m_demarrage->addItem(QString(), QString("Fournisseurs"));
    lDemarrage->addWidget(m_demarrage);
    lDemarrage->addStretch();
    grille->addWidget(cDemarrage, 1, 1);

    // ----- Alertes
    QVBoxLayout *lAlertes = nullptr;
    QFrame *cAlertes = creerCarte("carte_alertes", QString::fromUtf8("🔔"), lAlertes);
    m_alerteSante = new QCheckBox;
    lier(m_alerteSante, "alerte_sante");
    lAlertes->addWidget(m_alerteSante);
    lAlertes->addStretch();
    grille->addWidget(cAlertes, 2, 0);

    // ----- Données
    QVBoxLayout *lDonnees = nullptr;
    QFrame *cDonnees = creerCarte("carte_donnees", QString::fromUtf8("🗄️"), lDonnees);
    etiquette(lDonnees, "base_label");
    auto *cheminBase = new QLabel(DatabaseManager::instance().db().databaseName());
    cheminBase->setObjectName("paTexte");
    cheminBase->setWordWrap(true);
    cheminBase->setTextInteractionFlags(Qt::TextSelectableByMouse);
    lDonnees->addWidget(cheminBase);
    auto *btnDossier = new QPushButton;
    btnDossier->setObjectName("paSecondaire");
    btnDossier->setCursor(Qt::PointingHandCursor);
    lier(btnDossier, "ouvrir_dossier", QString::fromUtf8("📂  "));
    connect(btnDossier, &QPushButton::clicked, this, &ParametresPage::ouvrirDossierBase);
    lDonnees->addWidget(btnDossier, 0, Qt::AlignLeft);
    grille->addWidget(cDonnees, 2, 1);

    // ----- À propos
    QVBoxLayout *lApropos = nullptr;
    QFrame *cApropos = creerCarte("carte_apropos", QString::fromUtf8("ℹ️"), lApropos);
    auto *apropos = new QLabel;
    apropos->setObjectName("paTexte");
    apropos->setTextFormat(Qt::RichText);
    apropos->setWordWrap(true);
    m_traducteurs.append([apropos]() {
        apropos->setText(Langue::t("apropos_texte").arg(QString::fromLatin1(qVersion())));
    });
    lApropos->addWidget(apropos);
    grille->addWidget(cApropos, 3, 0, 1, 2);

    grille->setRowStretch(4, 1);
    zone->setWidget(contenuZone);
    racine->addWidget(zone, 1);

    // ---------------------------------------------------------- boutons du bas
    auto *boutons = new QHBoxLayout;
    boutons->setSpacing(10);
    boutons->addStretch();
    auto *btnReset = new QPushButton;
    btnReset->setObjectName("paSecondaire");
    btnReset->setCursor(Qt::PointingHandCursor);
    lier(btnReset, "reinit");
    connect(btnReset, &QPushButton::clicked, this, &ParametresPage::reinitialiser);
    boutons->addWidget(btnReset);
    auto *btnSave = new QPushButton;
    btnSave->setObjectName("paPrincipal");
    btnSave->setCursor(Qt::PointingHandCursor);
    lier(btnSave, "enreg", QString::fromUtf8("💾  "));
    connect(btnSave, &QPushButton::clicked, this, &ParametresPage::enregistrer);
    boutons->addWidget(btnSave);
    racine->addLayout(boutons);

    // ---------------------------------------------------------- style (limité à cette page)
    setStyleSheet(R"(
#pageParametres { background:#F4F8F8; }
#paIcone { font-size:26px; background:#E6F2F1; border-radius:24px;
           min-width:48px; max-width:48px; min-height:48px; max-height:48px; }
#paTitre { font-size:22px; font-weight:bold; color:#2D3436; }
#paSousTitre { font-size:13px; color:#7B8A8A; }
#paCarte { background:#FFFFFF; border:1px solid #E3E8E7; border-radius:12px; }
#paCarteTitre { font-size:15px; font-weight:bold; color:#2A8C82; }
#paLabel { font-size:12px; font-weight:bold; color:#4E6B68; }
#paTexte { font-size:13px; color:#4E6B68; }
#paNom { font-size:16px; font-weight:bold; color:#2D3436; }
#paAvatar { font-size:24px; background:#E6F2F1; border:1px solid #2A8C82; border-radius:24px;
            min-width:48px; max-width:48px; min-height:48px; max-height:48px; }
QCheckBox { font-size:13px; color:#2D3436; spacing:8px; }
QPushButton#paPrincipal { background:#2A8C82; color:#FFFFFF; border:none; border-radius:8px;
                          min-height:36px; padding:0 22px; font-size:13px; font-weight:bold; }
QPushButton#paPrincipal:hover { background:#23766D; }
QPushButton#paSecondaire { background:#FFFFFF; color:#2A8C82; border:1.5px solid #2A8C82; border-radius:8px;
                           min-height:34px; padding:0 18px; font-size:13px; font-weight:bold; }
QPushButton#paSecondaire:hover { background:#E6F2F1; }
)");

    connect(&Langue::instance(), &Langue::langueChangee, this, &ParametresPage::retraduire);
    retraduire();
    chargerDansFormulaire();
}

QFrame *ParametresPage::creerCarte(const char *cleTitre, const QString &emoji, QVBoxLayout *&contenu)
{
    auto *carte = new QFrame;
    carte->setObjectName("paCarte");
    contenu = new QVBoxLayout(carte);
    contenu->setContentsMargins(18, 14, 18, 16);
    contenu->setSpacing(8);
    auto *t = new QLabel;
    t->setObjectName("paCarteTitre");
    m_traducteurs.append([t, cleTitre, emoji]() { t->setText(emoji + "  " + Langue::t(cleTitre)); });
    contenu->addWidget(t);
    return carte;
}

void ParametresPage::retraduire()
{
    setLayoutDirection(Langue::direction());
    for (const auto &f : m_traducteurs)
        f();

    m_centre->setPlaceholderText(Langue::t("nom_centre_ph"));

    // Libellés traduits des pages de démarrage (l'identifiant interne reste en français)
    const char *cles[4] = {"menu_accueil", "menu_animaux", "menu_stock", "menu_fournisseurs"};
    for (int i = 0; i < 4; ++i)
        m_demarrage->setItemText(i, Langue::t(cles[i]));

    {
        const QSignalBlocker blocage(m_langue);       // évite de relancer Langue::definir()
        m_langue->setCurrentIndex(static_cast<int>(Langue::courante()));
    }
    actualiserCompte();
}

void ParametresPage::actualiserCompte()
{
    const QString id = AuthManager::utilisateurCourant();
    m_avatar->setText(id.isEmpty() ? QString::fromUtf8("👤") : AuthManager::emoji(id));
    m_nomCompte->setText(id.isEmpty() ? QString() : AuthManager::nomAffiche(id));
    m_roleCompte->setText(id.isEmpty() ? QString() : AuthManager::role(id));
}

void ParametresPage::chargerDansFormulaire()
{
    m_centre->setText(nomCentre());
    const int i = m_demarrage->findData(pageDemarrage());
    m_demarrage->setCurrentIndex(i >= 0 ? i : 0);
    m_alerteSante->setChecked(alerteSanteAnimaux());
}

// ------------------------------------------------------------ actions
void ParametresPage::enregistrer()
{
    QSettings s("PetManager", "PetManager");
    s.setValue("general/nomCentre", m_centre->text().trimmed());
    s.setValue("general/pageDemarrage", m_demarrage->currentData().toString());
    s.setValue("animaux/alerteSante", m_alerteSante->isChecked());
    s.sync();

    emit reglagesEnregistres();
    CustomDialog::success(window(), Langue::t("titre_succes"), Langue::t("ok_enreg"));
}

void ParametresPage::reinitialiser()
{
    if (!CustomDialog::confirm(window(), Langue::t("titre_confirm"), Langue::t("confirm_reinit"),
                               Langue::t("reinit"), true))
        return;

    QSettings s("PetManager", "PetManager");
    s.remove("general/nomCentre");
    s.remove("general/pageDemarrage");
    s.remove("animaux");
    s.sync();

    chargerDansFormulaire();
    emit reglagesEnregistres();
}

void ParametresPage::ouvrirDossierBase()
{
    const QString chemin = DatabaseManager::instance().db().databaseName();
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(chemin).absolutePath()));
}
