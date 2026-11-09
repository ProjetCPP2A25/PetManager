#include "accueilpage.h"
#include "authmanager.h"
#include "langue.h"
#include "parametrespage.h"

#include <QByteArray>
#include <QDate>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QPushButton>
#include <QTime>
#include <QVBoxLayout>

AccueilPage::AccueilPage(QWidget *parent) : QWidget(parent)
{
    setObjectName("pageAccueil");
    setAttribute(Qt::WA_StyledBackground, true);

    // Associe un texte traduit à un label / bouton (retraduit automatiquement au changement de langue)
    auto lier = [this](auto *widget, const char *cle, const QString &prefixe = QString()) {
        m_traducteurs.append([widget, cle, prefixe]() { widget->setText(prefixe + Langue::t(cle)); });
    };

    auto *racine = new QVBoxLayout(this);
    racine->setContentsMargins(24, 20, 24, 20);
    racine->setSpacing(16);

    // ---------------------------------------------------------- en-tête
    auto *entete = new QHBoxLayout;
    entete->setSpacing(14);
    auto *icone = new QLabel(QString::fromUtf8("🏠"));
    icone->setObjectName("acIcone");
    icone->setAlignment(Qt::AlignCenter);
    entete->addWidget(icone);

    auto *textes = new QVBoxLayout;
    textes->setSpacing(2);
    auto *titre = new QLabel;
    titre->setObjectName("acTitre");
    lier(titre, "menu_accueil");
    m_salutation = new QLabel;
    m_salutation->setObjectName("acSousTitre");
    textes->addWidget(titre);
    textes->addWidget(m_salutation);
    entete->addLayout(textes);
    entete->addStretch();

    // Utilisateur connecté + déconnexion
    m_avatar = new QLabel(QString::fromUtf8("👤"));
    m_avatar->setObjectName("acAvatar");
    m_avatar->setAlignment(Qt::AlignCenter);
    entete->addWidget(m_avatar);
    auto *infosUser = new QVBoxLayout;
    infosUser->setSpacing(0);
    m_nomUser = new QLabel;
    m_nomUser->setObjectName("acNom");
    m_roleUser = new QLabel;
    m_roleUser->setObjectName("acRole");
    infosUser->addWidget(m_nomUser);
    infosUser->addWidget(m_roleUser);
    entete->addLayout(infosUser);
    auto *btnDeco = new QPushButton;
    btnDeco->setObjectName("acDeco");
    btnDeco->setCursor(Qt::PointingHandCursor);
    lier(btnDeco, "deconnexion", QString::fromUtf8("🚪  "));
    connect(btnDeco, &QPushButton::clicked, this, &AccueilPage::deconnexionDemandee);
    entete->addSpacing(6);
    entete->addWidget(btnDeco);
    racine->addLayout(entete);

    // ---------------------------------------------------------- 4 indicateurs
    auto *kpis = new QHBoxLayout;
    kpis->setSpacing(14);
    kpis->addWidget(creerKpi(QString::fromUtf8("🐾"), "kpi_animaux", m_valeur[0], m_detail[0]));
    kpis->addWidget(creerKpi(QString::fromUtf8("📦"), "kpi_produits", m_valeur[1], m_detail[1]));
    kpis->addWidget(creerKpi(QString::fromUtf8("⚠️"), "kpi_alertes", m_valeur[2], m_detail[2]));
    kpis->addWidget(creerKpi(QString::fromUtf8("🚚"), "kpi_fournisseurs", m_valeur[3], m_detail[3]));
    racine->addLayout(kpis);

    // ---------------------------------------------------------- accès rapide
    auto *acces = new QFrame;
    acces->setObjectName("acCarte");
    auto *lAcces = new QVBoxLayout(acces);
    lAcces->setContentsMargins(18, 14, 18, 16);
    lAcces->setSpacing(10);
    auto *tAcces = new QLabel;
    tAcces->setObjectName("acCarteTitre");
    lier(tAcces, "acces_titre");
    lAcces->addWidget(tAcces);

    auto *rangee = new QHBoxLayout;
    rangee->setSpacing(10);
    const char *cles[5] = {"menu_animaux", "menu_stock", "menu_fournisseurs", "menu_employes", "menu_parametres"};
    const QString emojis[5] = {QString::fromUtf8("🐾  "), QString::fromUtf8("📦  "),
                               QString::fromUtf8("🚚  "), QString::fromUtf8("👥  "), QString::fromUtf8("⚙️  ")};
    const int modules[5] = {Animaux, Stock, Fournisseurs, Employes, Parametres};
    for (int i = 0; i < 5; ++i) {
        auto *b = new QPushButton;
        b->setObjectName("acAcces");
        b->setCursor(Qt::PointingHandCursor);
        lier(b, cles[i], emojis[i]);
        const int module = modules[i];
        connect(b, &QPushButton::clicked, this, [this, module]() { emit naviguerVers(module); });
        rangee->addWidget(b);
    }
    lAcces->addLayout(rangee);
    racine->addWidget(acces);

    // ---------------------------------------------------------- bas : alertes | résumé + astuce
    auto *bas = new QHBoxLayout;
    bas->setSpacing(14);

    auto *carteAlertes = new QFrame;
    carteAlertes->setObjectName("acCarte");
    auto *lAlertes = new QVBoxLayout(carteAlertes);
    lAlertes->setContentsMargins(18, 14, 18, 16);
    lAlertes->setSpacing(8);
    auto *tAlertes = new QLabel;
    tAlertes->setObjectName("acCarteTitre");
    lier(tAlertes, "surveiller_titre", QString::fromUtf8("⚠️  "));
    lAlertes->addWidget(tAlertes);
    m_listeAlertes = new QVBoxLayout;
    m_listeAlertes->setSpacing(0);
    lAlertes->addLayout(m_listeAlertes);
    lAlertes->addStretch();
    bas->addWidget(carteAlertes, 3);

    auto *colonneDroite = new QVBoxLayout;
    colonneDroite->setSpacing(14);

    auto *carteResume = new QFrame;
    carteResume->setObjectName("acCarte");
    auto *lResume = new QVBoxLayout(carteResume);
    lResume->setContentsMargins(18, 14, 18, 16);
    lResume->setSpacing(8);
    auto *tResume = new QLabel;
    tResume->setObjectName("acCarteTitre");
    lier(tResume, "resume_titre", QString::fromUtf8("📋  "));
    lResume->addWidget(tResume);
    m_resume = new QLabel;
    m_resume->setObjectName("acResume");
    m_resume->setTextFormat(Qt::RichText);
    m_resume->setWordWrap(true);
    m_resume->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    lResume->addWidget(m_resume);
    lResume->addStretch();
    colonneDroite->addWidget(carteResume, 1);

    auto *carteAstuce = new QFrame;
    carteAstuce->setObjectName("acAstuce");
    auto *lAstuce = new QVBoxLayout(carteAstuce);
    lAstuce->setContentsMargins(18, 14, 18, 16);
    lAstuce->setSpacing(6);
    auto *tAstuce = new QLabel;
    tAstuce->setObjectName("acAstuceTitre");
    lier(tAstuce, "astuce_titre", QString::fromUtf8("💡  "));
    lAstuce->addWidget(tAstuce);
    m_astuce = new QLabel;
    m_astuce->setObjectName("acAstuceTexte");
    m_astuce->setWordWrap(true);
    lAstuce->addWidget(m_astuce);
    colonneDroite->addWidget(carteAstuce);

    bas->addLayout(colonneDroite, 2);
    racine->addLayout(bas, 1);

    // ---------------------------------------------------------- style (limité à cette page)
    setStyleSheet(R"(
#pageAccueil { background:#F4F8F8; }
#acIcone { font-size:26px; background:#E6F2F1; border-radius:24px;
           min-width:48px; max-width:48px; min-height:48px; max-height:48px; }
#acTitre { font-size:22px; font-weight:bold; color:#2D3436; }
#acSousTitre { font-size:13px; color:#7B8A8A; }
#acAvatar { font-size:20px; background:#E6F2F1; border:1px solid #2A8C82; border-radius:20px;
            min-width:40px; max-width:40px; min-height:40px; max-height:40px; }
#acNom { font-size:13px; font-weight:bold; color:#2D3436; }
#acRole { font-size:11px; color:#7B8A8A; }
QPushButton#acDeco { background:#FFFFFF; color:#C0392B; border:1px solid #F0C4C0; border-radius:8px;
                     padding:8px 14px; font-size:12px; font-weight:bold; }
QPushButton#acDeco:hover { background:#FDE7E7; border:1px solid #C0392B; }
#acKpi { background:#FFFFFF; border:1px solid #2A8C82; border-radius:12px; }
#acKpiIcone { font-size:26px; }
#acKpiTitre { font-size:12px; color:#4E6B68; }
#acKpiValeur { font-size:28px; font-weight:bold; color:#2A8C82; }
#acKpiDetail { font-size:11px; color:#7B8A8A; }
#acCarte { background:#FFFFFF; border:1px solid #E3E8E7; border-radius:12px; }
#acCarteTitre { font-size:15px; font-weight:bold; color:#2D3436; }
QPushButton#acAcces { background:#FFFFFF; color:#2A8C82; border:1px solid #DADDDC; border-radius:8px;
                      padding:12px 18px; font-size:13px; font-weight:bold; }
QPushButton#acAcces:hover { background:#E6F2F1; border:1px solid #2A8C82; }
#acLigne { font-size:13px; color:#2D3436; padding:7px 0; border-bottom:1px solid #EDF1F0; }
#acVide { font-size:13px; color:#2A8C82; padding:6px 0; }
#acResume { font-size:13px; color:#4E6B68; }
#acAstuce { background:#FFF8E6; border:1px solid #F3DFA6; border-radius:12px; }
#acAstuceTitre { font-size:14px; font-weight:bold; color:#B26A00; }
#acAstuceTexte { font-size:13px; color:#6B5320; }
)");

    connect(&Langue::instance(), &Langue::langueChangee, this, &AccueilPage::retraduire);
    retraduire();
}

QFrame *AccueilPage::creerKpi(const QString &icone, const char *cleTitre, QLabel *&valeur, QLabel *&detail)
{
    auto *carte = new QFrame;
    carte->setObjectName("acKpi");
    auto *h = new QHBoxLayout(carte);
    h->setContentsMargins(16, 14, 16, 14);
    h->setSpacing(12);

    auto *ic = new QLabel(icone);
    ic->setObjectName("acKpiIcone");
    ic->setAlignment(Qt::AlignCenter);
    ic->setFixedWidth(40);
    h->addWidget(ic);

    auto *v = new QVBoxLayout;
    v->setSpacing(0);
    auto *t = new QLabel;
    t->setObjectName("acKpiTitre");
    m_traducteurs.append([t, cleTitre]() { t->setText(Langue::t(cleTitre)); });
    valeur = new QLabel("0");
    valeur->setObjectName("acKpiValeur");
    detail = new QLabel;
    detail->setObjectName("acKpiDetail");
    detail->setWordWrap(true);
    v->addWidget(t);
    v->addWidget(valeur);
    v->addWidget(detail);
    h->addLayout(v, 1);
    return carte;
}

void AccueilPage::retraduire()
{
    setLayoutDirection(Langue::direction());
    for (const auto &f : m_traducteurs)
        f();

    // Astuce du jour : une phrase différente chaque jour
    const QByteArray cle = "tip" + QByteArray::number(QDate::currentDate().dayOfYear() % 6);
    m_astuce->setText(Langue::t(cle.constData()));

    actualiserUtilisateur();
    setDonnees(m_donnees);
}

void AccueilPage::actualiserUtilisateur()
{
    const QString id = AuthManager::utilisateurCourant();
    m_avatar->setText(id.isEmpty() ? QString::fromUtf8("👤") : AuthManager::emoji(id));
    m_nomUser->setText(id.isEmpty() ? QString() : AuthManager::nomAffiche(id));
    m_roleUser->setText(id.isEmpty() ? QString() : AuthManager::role(id));

    const QString formule = Langue::t(QTime::currentTime().hour() >= 18 ? "acc_bonsoir" : "acc_bonjour");
    const QString date = Langue::locale().toString(QDate::currentDate(), "dddd d MMMM yyyy");
    const QString centre = ParametresPage::nomCentre().trimmed();

    QString texte = formule;
    if (!id.isEmpty())
        texte += " " + AuthManager::nomAffiche(id);
    texte += QString::fromUtf8(" 👋   —   ") + date;
    if (!centre.isEmpty())
        texte += QString::fromUtf8("   ·   ") + centre;
    m_salutation->setText(texte);
}

void AccueilPage::setDonnees(const AccueilDonnees &d)
{
    m_donnees = d;

    // Indicateurs
    m_valeur[0]->setText(QString::number(d.animaux));
    m_detail[0]->setText(Langue::t("det_surveillance").arg(d.animauxSurveillance));

    m_valeur[1]->setText(QString::number(d.produits));
    m_detail[1]->setText(Langue::t("det_commandes").arg(d.commandesEnAttente));

    m_valeur[2]->setText(QString::number(d.ruptures + d.stockBas + d.expires));
    m_detail[2]->setText(Langue::t("det_alertes").arg(d.ruptures).arg(d.stockBas).arg(d.expires));

    m_valeur[3]->setText(QString::number(d.fournisseursActifs));
    m_detail[3]->setText(Langue::t("det_sur").arg(d.fournisseurs));

    // Liste "À surveiller" : on vide, puis on reconstruit
    while (QLayoutItem *item = m_listeAlertes->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    if (d.alertes.isEmpty()) {
        auto *vide = new QLabel(QString::fromUtf8("✅  ") + Langue::t("rien"));
        vide->setObjectName("acVide");
        vide->setWordWrap(true);
        m_listeAlertes->addWidget(vide);
    } else {
        const int maxLignes = 8;
        for (int i = 0; i < d.alertes.size() && i < maxLignes; ++i) {
            auto *ligne = new QLabel(d.alertes[i]);
            ligne->setObjectName("acLigne");
            ligne->setWordWrap(true);
            m_listeAlertes->addWidget(ligne);
        }
        if (d.alertes.size() > maxLignes) {
            auto *reste = new QLabel(Langue::t("autres").arg(d.alertes.size() - maxLignes));
            reste->setObjectName("acVide");
            m_listeAlertes->addWidget(reste);
        }
    }

    // Résumé
    const QString resume =
        "<p><b>" + QString::fromUtf8("🐾 ") + Langue::t("menu_animaux") + "</b><br>" +
            Langue::t("res_animaux").arg(d.chiens).arg(d.chats).arg(d.lapins) + "<br>" +
            Langue::t("det_surveillance").arg(d.animauxSurveillance) + "</p>" +
        "<p><b>" + QString::fromUtf8("📦 ") + Langue::t("menu_stock") + "</b><br>" +
            Langue::t("res_stock").arg(d.produits).arg(d.commandesEnAttente) + "</p>" +
        "<p><b>" + QString::fromUtf8("🚚 ") + Langue::t("menu_fournisseurs") + "</b><br>" +
            Langue::t("res_fourn").arg(d.fournisseurs).arg(d.fournisseursActifs) + "</p>" +
        "<p><b>" + QString::fromUtf8("👥 ") + Langue::t("menu_employes") + "</b><br>" +
            Langue::t("res_employes").arg(d.employes).arg(d.employesActifs) + "</p>";
    m_resume->setText(resume);
}
