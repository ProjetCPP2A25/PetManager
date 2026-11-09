#include "connexionpage.h"
#include "authmanager.h"
#include "dialogsauth.h"
#include "langue.h"

#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTimer>
#include <QVBoxLayout>

ConnexionPage::ConnexionPage(QWidget *parent) : QWidget(parent)
{
    setObjectName("pageConnexion");
    setAttribute(Qt::WA_StyledBackground, true);

    // Petit utilitaire : associe un texte traduit à un label / bouton
    auto lier = [this](auto *widget, const char *cle, const QString &prefixe = QString()) {
        m_traducteurs.append([widget, cle, prefixe]() { widget->setText(prefixe + Langue::t(cle)); });
    };

    auto *racine = new QHBoxLayout(this);
    racine->setContentsMargins(0, 0, 0, 0);
    racine->setSpacing(0);

    // ================================================================ panneau gauche (marque)
    auto *gauche = new QFrame;
    gauche->setObjectName("cnGauche");
    auto *lg = new QVBoxLayout(gauche);
    lg->setContentsMargins(40, 40, 40, 40);
    lg->setSpacing(14);
    lg->addStretch();

    auto *logo = new QLabel;
    logo->setAlignment(Qt::AlignCenter);
    const QPixmap pix(":/resources/images/logo.png");
    if (!pix.isNull())
        logo->setPixmap(pix.scaled(170, 170, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else
        logo->setText(QString::fromUtf8("🐾"));
    lg->addWidget(logo);

    auto *marque = new QLabel("PET MANAGER");
    marque->setObjectName("cnMarque");
    marque->setAlignment(Qt::AlignCenter);
    lg->addWidget(marque);

    auto *tagline = new QLabel;
    tagline->setObjectName("cnTagline");
    tagline->setAlignment(Qt::AlignCenter);
    tagline->setWordWrap(true);
    lier(tagline, "login_tagline");
    lg->addWidget(tagline);

    auto *points = new QLabel;
    points->setObjectName("cnPoints");
    points->setAlignment(Qt::AlignCenter);
    points->setWordWrap(true);
    lier(points, "login_points");
    lg->addWidget(points);
    lg->addStretch();
    racine->addWidget(gauche, 5);

    // ================================================================ panneau droit (formulaire)
    auto *droite = new QFrame;
    droite->setObjectName("cnDroite");
    auto *ld = new QVBoxLayout(droite);
    ld->setContentsMargins(30, 20, 30, 30);
    ld->setSpacing(0);

    // Sélecteur de langue en haut
    auto *haut = new QHBoxLayout;
    haut->addStretch();
    auto *iconeLangue = new QLabel(QString::fromUtf8("🌐"));
    haut->addWidget(iconeLangue);
    m_langue = new QComboBox;
    m_langue->setFixedHeight(32);
    m_langue->setMinimumWidth(120);
    for (int c = Langue::Francais; c <= Langue::Arabe; ++c)
        m_langue->addItem(Langue::nomLangue(static_cast<Langue::Code>(c)), c);
    haut->addWidget(m_langue);
    ld->addLayout(haut);
    ld->addStretch();

    // Carte de connexion
    auto *carte = new QFrame;
    carte->setObjectName("cnCarte");
    carte->setFixedWidth(430);
    auto *lc = new QVBoxLayout(carte);
    lc->setContentsMargins(30, 28, 30, 26);
    lc->setSpacing(6);

    auto *titre = new QLabel;
    titre->setObjectName("cnTitre");
    lier(titre, "login_titre", QString::fromUtf8("🔐  "));
    lc->addWidget(titre);

    auto *sous = new QLabel;
    sous->setObjectName("cnSous");
    lier(sous, "login_sous");
    lc->addWidget(sous);
    lc->addSpacing(10);

    auto *labelUser = new QLabel;
    labelUser->setObjectName("cnLabel");
    lier(labelUser, "login_user");
    lc->addWidget(labelUser);
    m_user = new QLineEdit;
    m_user->setFixedHeight(36);
    lc->addWidget(m_user);
    lc->addSpacing(4);

    auto *labelPass = new QLabel;
    labelPass->setObjectName("cnLabel");
    lier(labelPass, "login_pass");
    lc->addWidget(labelPass);
    auto *rangeePass = new QHBoxLayout;
    rangeePass->setSpacing(6);
    m_pass = new QLineEdit;
    m_pass->setEchoMode(QLineEdit::Password);
    m_pass->setFixedHeight(36);
    rangeePass->addWidget(m_pass, 1);
    m_btnVoir = new QPushButton(QString::fromUtf8("👁"));
    m_btnVoir->setObjectName("cnVoir");
    m_btnVoir->setCheckable(true);
    m_btnVoir->setCursor(Qt::PointingHandCursor);
    rangeePass->addWidget(m_btnVoir);
    lc->addLayout(rangeePass);

    auto *rangeeOubli = new QHBoxLayout;
    rangeeOubli->addStretch();
    auto *btnOubli = new QPushButton;
    btnOubli->setObjectName("cnLien");
    btnOubli->setCursor(Qt::PointingHandCursor);
    btnOubli->setFlat(true);
    lier(btnOubli, "login_oublie");
    rangeeOubli->addWidget(btnOubli);
    lc->addLayout(rangeeOubli);

    m_erreur = new QLabel;
    m_erreur->setObjectName("cnErreur");
    m_erreur->setWordWrap(true);
    m_erreur->hide();
    lc->addWidget(m_erreur);

    m_btnConnexion = new QPushButton;
    m_btnConnexion->setObjectName("cnPrincipal");
    m_btnConnexion->setCursor(Qt::PointingHandCursor);
    m_btnConnexion->setDefault(true);
    lier(m_btnConnexion, "login_bouton");
    lc->addWidget(m_btnConnexion);

    ld->addWidget(carte, 0, Qt::AlignHCenter);
    ld->addStretch();
    racine->addWidget(droite, 6);

    // ================================================================ comportement
    m_timerBlocage = new QTimer(this);
    m_timerBlocage->setInterval(1000);
    connect(m_timerBlocage, &QTimer::timeout, this, &ConnexionPage::tickBlocage);

    connect(m_btnConnexion, &QPushButton::clicked, this, &ConnexionPage::tenterConnexion);
    connect(m_user, &QLineEdit::returnPressed, this, [this]() { m_pass->setFocus(); });
    connect(m_pass, &QLineEdit::returnPressed, this, &ConnexionPage::tenterConnexion);
    connect(btnOubli, &QPushButton::clicked, this, [this]() { DialogsAuth::motDePasseOublie(window()); });
    connect(m_btnVoir, &QPushButton::toggled, this, [this](bool visible) {
        m_pass->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
    });
    connect(m_langue, &QComboBox::currentIndexChanged, this, [this](int) {
        Langue::definir(static_cast<Langue::Code>(m_langue->currentData().toInt()));
    });
    connect(&Langue::instance(), &Langue::langueChangee, this, &ConnexionPage::retraduire);

    // ================================================================ style (limité à cette page)
    setStyleSheet(R"(
#pageConnexion { background:#F4F8F8; }
#cnGauche { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #2A8C82, stop:1 #1E635B); }
#cnMarque { color:#FFFFFF; font-size:30px; font-weight:bold; }
#cnTagline { color:#E6F2F1; font-size:15px; }
#cnPoints { color:#CDE8E2; font-size:13px; }
#cnDroite { background:#F4F8F8; }
#cnCarte { background:#FFFFFF; border:1px solid #E3E8E7; border-radius:16px; }
#cnTitre { font-size:24px; font-weight:bold; color:#2D3436; }
#cnSous { font-size:13px; color:#7B8A8A; }
#cnLabel { font-size:12px; font-weight:bold; color:#4E6B68; }
#cnSep { font-size:12px; color:#7B8A8A; }
#cnErreur { font-size:12px; font-weight:bold; color:#D9534F; padding:4px 0; }
QPushButton#cnPrincipal { background:#2A8C82; color:#FFFFFF; border:none; border-radius:8px;
                          min-height:40px; font-size:14px; font-weight:bold; }
QPushButton#cnPrincipal:hover { background:#23766D; }
QPushButton#cnPrincipal:disabled { background:#B7CFCB; }
QPushButton#cnLien { background:transparent; border:none; color:#2A8C82; font-size:12px;
                     font-weight:bold; padding:2px 0; }
QPushButton#cnLien:hover { color:#1E635B; text-decoration:underline; }
QPushButton#cnVoir { background:#FFFFFF; border:1px solid #DADDDC; border-radius:6px;
                     min-width:38px; max-width:38px; min-height:34px; max-height:34px; font-size:14px; }
QPushButton#cnVoir:checked { background:#E6F2F1; border:1px solid #2A8C82; }
)");

    retraduire();
    reinitialiser();
}

void ConnexionPage::retraduire()
{
    setLayoutDirection(Langue::direction());
    for (const auto &f : m_traducteurs)
        f();
    m_user->setPlaceholderText(Langue::t("login_user_ph"));
    m_pass->setPlaceholderText(Langue::t("login_pass_ph"));
    m_btnVoir->setToolTip(Langue::t("login_voir"));

    const QSignalBlocker blocage(m_langue);       // évite de relancer Langue::definir()
    m_langue->setCurrentIndex(static_cast<int>(Langue::courante()));

    masquerErreur();
}

void ConnexionPage::reinitialiser()
{
    m_pass->clear();
    m_btnVoir->setChecked(false);
    m_essais = 0;
    masquerErreur();

    // Aucun nom d'utilisateur n'est affiché ni pré-rempli : chacun saisit le sien
    m_user->clear();
    m_user->setFocus();
}

void ConnexionPage::afficherErreur(const QString &texte)
{
    m_erreur->setText(QString::fromUtf8("⚠️  ") + texte);
    m_erreur->show();
}

void ConnexionPage::masquerErreur()
{
    if (m_secondesBlocage > 0)
        return;                      // le message de blocage reste visible
    m_erreur->hide();
}

void ConnexionPage::tenterConnexion()
{
    if (m_secondesBlocage > 0)
        return;

    const QString saisie = m_user->text().trimmed();
    const QString motDePasse = m_pass->text();

    if (saisie.isEmpty() || motDePasse.isEmpty()) {
        afficherErreur(Langue::t("login_vide"));
        return;
    }

    if (AuthManager::verifier(saisie, motDePasse)) {
        m_essais = 0;
        m_erreur->hide();
        m_pass->clear();
        emit connexionReussie(AuthManager::normaliser(saisie));
        return;
    }

    ++m_essais;
    m_pass->clear();
    m_pass->setFocus();
    const int restant = 3 - m_essais;
    if (restant <= 0) {
        m_essais = 0;
        bloquer(5);
    } else {
        afficherErreur(Langue::t("login_erreur") + "  " + Langue::t("login_restant").arg(restant));
    }
}

void ConnexionPage::bloquer(int secondes)
{
    m_secondesBlocage = secondes;
    m_btnConnexion->setEnabled(false);
    afficherErreur(Langue::t("login_bloque").arg(m_secondesBlocage));
    m_timerBlocage->start();
}

void ConnexionPage::tickBlocage()
{
    --m_secondesBlocage;
    if (m_secondesBlocage <= 0) {
        m_secondesBlocage = 0;
        m_timerBlocage->stop();
        m_btnConnexion->setEnabled(true);
        m_erreur->hide();
        m_pass->setFocus();
    } else {
        afficherErreur(Langue::t("login_bloque").arg(m_secondesBlocage));
    }
}
