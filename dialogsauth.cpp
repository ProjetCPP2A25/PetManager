#include "dialogsauth.h"
#include "authmanager.h"
#include "customdialog.h"
#include "langue.h"
#include "smtpclient.h"

#include <QApplication>
#include <QCheckBox>
#include <QDateTime>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace {

const char *STYLE_DIALOGUE = R"(
QDialog { background:#FFFFFF; }
#dlgTitre { font-size:18px; font-weight:bold; color:#2A8C82; }
#dlgTexte { font-size:13px; color:#4E6B68; }
#dlgLabel { font-size:12px; font-weight:bold; color:#4E6B68; }
#dlgCode { background:#E6F2F1; color:#1E635B; border-radius:8px; padding:8px 12px;
           font-size:14px; font-weight:bold; }
QPushButton#dlgPrincipal { background:#2A8C82; color:#FFFFFF; border:none; border-radius:8px;
                           min-height:36px; padding:0 20px; font-size:13px; font-weight:bold; }
QPushButton#dlgPrincipal:hover { background:#23766D; }
QPushButton#dlgSecondaire { background:#FFFFFF; color:#2A8C82; border:1.5px solid #2A8C82; border-radius:8px;
                            min-height:34px; padding:0 16px; font-size:13px; font-weight:bold; }
QPushButton#dlgSecondaire:hover { background:#E6F2F1; }
QPushButton#dlgLien { background:transparent; border:none; color:#2A8C82; font-size:12px;
                      font-weight:bold; padding:2px 0; }
QPushButton#dlgLien:hover { color:#1E635B; text-decoration:underline; }
QPushButton#dlgLien:disabled { color:#B7CFCB; }
)";

QLabel *creerLabel(const QString &texte, const char *nom)
{
    auto *l = new QLabel(texte);
    l->setObjectName(nom);
    l->setWordWrap(true);
    return l;
}

QLineEdit *creerChampMotDePasse()
{
    auto *e = new QLineEdit;
    e->setEchoMode(QLineEdit::Password);
    e->setFixedHeight(34);
    return e;
}

QPushButton *creerBouton(const QString &texte, const char *nom)
{
    auto *b = new QPushButton(texte);
    b->setObjectName(nom);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

} // namespace

// =====================================================================
//                         Mot de passe oublié
//   Étape 1 : saisie de l'e-mail -> envoi d'un code à 6 chiffres
//   Étape 2 : saisie du code reçu (valable 10 min, 3 essais)
//   Étape 3 : nouveau mot de passe
// =====================================================================
void DialogsAuth::motDePasseOublie(QWidget *parent)
{
    QDialog d(parent);
    d.setWindowTitle(Langue::t("oubli_titre"));
    d.setModal(true);
    d.setMinimumWidth(440);
    d.setLayoutDirection(Langue::direction());
    d.setStyleSheet(STYLE_DIALOGUE);

    auto *lay = new QVBoxLayout(&d);
    lay->setContentsMargins(24, 22, 24, 20);
    lay->setSpacing(8);

    lay->addWidget(creerLabel(QString::fromUtf8("🔑  ") + Langue::t("oubli_titre"), "dlgTitre"));

    auto *pages = new QStackedWidget;
    lay->addWidget(pages);

    // ------------------------------------------------ étape 1 : e-mail
    auto *page1 = new QWidget;
    auto *l1 = new QVBoxLayout(page1);
    l1->setContentsMargins(0, 0, 0, 0);
    l1->setSpacing(8);
    l1->addWidget(creerLabel(Langue::t("oubli_sous"), "dlgTexte"));
    l1->addSpacing(4);
    l1->addWidget(creerLabel(Langue::t("oubli_email"), "dlgLabel"));
    auto *champEmail = new QLineEdit;
    champEmail->setFixedHeight(34);
    champEmail->setPlaceholderText(Langue::t("oubli_email_ph"));
    l1->addWidget(champEmail);
    l1->addStretch();
    auto *b1 = new QHBoxLayout;
    b1->addStretch();
    auto *btnAnnuler1 = creerBouton(Langue::t("btn_annuler"), "dlgSecondaire");
    auto *btnEnvoyer = creerBouton(Langue::t("oubli_envoyer"), "dlgPrincipal");
    b1->addWidget(btnAnnuler1);
    b1->addWidget(btnEnvoyer);
    l1->addLayout(b1);
    pages->addWidget(page1);

    // ------------------------------------------------ étape 2 : code
    auto *page2 = new QWidget;
    auto *l2 = new QVBoxLayout(page2);
    l2->setContentsMargins(0, 0, 0, 0);
    l2->setSpacing(8);
    auto *infoCode = creerLabel(QString(), "dlgCode");
    l2->addWidget(infoCode);
    l2->addWidget(creerLabel(Langue::t("oubli_etape_code"), "dlgTexte"));
    l2->addWidget(creerLabel(Langue::t("oubli_code"), "dlgLabel"));
    auto *champCode = new QLineEdit;
    champCode->setMaxLength(6);
    champCode->setFixedHeight(38);
    champCode->setAlignment(Qt::AlignCenter);
    champCode->setPlaceholderText("______");
    champCode->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d{0,6}"), champCode));
    champCode->setStyleSheet("font-size:18px; font-weight:bold; letter-spacing:6px;");
    l2->addWidget(champCode);
    auto *rangeeLiens = new QHBoxLayout;
    auto *btnRetour = creerBouton(Langue::t("oubli_retour"), "dlgLien");
    auto *btnRenvoyer = creerBouton(Langue::t("oubli_renvoyer"), "dlgLien");
    rangeeLiens->addWidget(btnRetour);
    rangeeLiens->addStretch();
    rangeeLiens->addWidget(btnRenvoyer);
    l2->addLayout(rangeeLiens);
    l2->addStretch();
    auto *b2 = new QHBoxLayout;
    b2->addStretch();
    auto *btnAnnuler2 = creerBouton(Langue::t("btn_annuler"), "dlgSecondaire");
    auto *btnVerifier = creerBouton(Langue::t("oubli_verifier"), "dlgPrincipal");
    b2->addWidget(btnAnnuler2);
    b2->addWidget(btnVerifier);
    l2->addLayout(b2);
    pages->addWidget(page2);

    // ------------------------------------------------ étape 3 : nouveau mot de passe
    auto *page3 = new QWidget;
    auto *l3 = new QVBoxLayout(page3);
    l3->setContentsMargins(0, 0, 0, 0);
    l3->setSpacing(8);
    l3->addWidget(creerLabel(Langue::t("oubli_etape_mdp"), "dlgCode"));
    l3->addWidget(creerLabel(Langue::t("oubli_nouveau"), "dlgLabel"));
    auto *champNouveau = creerChampMotDePasse();
    l3->addWidget(champNouveau);
    l3->addWidget(creerLabel(Langue::t("oubli_confirmer"), "dlgLabel"));
    auto *champConfirmer = creerChampMotDePasse();
    l3->addWidget(champConfirmer);
    auto *voir = new QCheckBox(Langue::t("login_voir"));
    l3->addWidget(voir);
    l3->addStretch();
    auto *b3 = new QHBoxLayout;
    b3->addStretch();
    auto *btnAnnuler3 = creerBouton(Langue::t("btn_annuler"), "dlgSecondaire");
    auto *btnChanger = creerBouton(Langue::t("oubli_changer"), "dlgPrincipal");
    b3->addWidget(btnAnnuler3);
    b3->addWidget(btnChanger);
    l3->addLayout(b3);
    pages->addWidget(page3);

    // ------------------------------------------------ état
    QString compte;              // identifiant du compte trouvé grâce à l'e-mail
    QString codeAttendu;
    QDateTime expiration;
    int essais = 0;

    // Génère un code, l'envoie par e-mail (ou l'affiche en mode démo) puis passe à l'étape 2
    auto envoyerCode = [&]() {
        const QString adresse = champEmail->text().trimmed().toLower();
        const QRegularExpression formatEmail("^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$");
        if (!formatEmail.match(adresse).hasMatch()) {
            CustomDialog::error(&d, Langue::t("titre_erreur"), Langue::t("oubli_err_email"));
            champEmail->setFocus();
            return;
        }
        const QString id = AuthManager::utilisateurParEmail(adresse);
        if (id.isEmpty()) {
            CustomDialog::error(&d, Langue::t("titre_erreur"), Langue::t("oubli_email_inconnu"));
            champEmail->setFocus();
            return;
        }

        const QString code = QString::number(QRandomGenerator::global()->bounded(100000, 1000000));

        if (SmtpClient::estConfigure()) {
            const QString texteBouton = btnEnvoyer->text();
            btnEnvoyer->setEnabled(false);
            btnRenvoyer->setEnabled(false);
            btnEnvoyer->setText(Langue::t("oubli_envoi_cours"));
            QApplication::setOverrideCursor(Qt::WaitCursor);
            QApplication::processEvents();

            QString erreur;
            const bool ok = SmtpClient::envoyer(adresse, Langue::t("mail_sujet"),
                                                Langue::t("mail_corps").arg(code), &erreur);

            QApplication::restoreOverrideCursor();
            btnEnvoyer->setText(texteBouton);
            btnEnvoyer->setEnabled(true);
            btnRenvoyer->setEnabled(true);
            if (!ok) {
                CustomDialog::error(&d, Langue::t("titre_erreur"), Langue::t("oubli_err_envoi").arg(erreur));
                return;
            }
            infoCode->setText(Langue::t("oubli_code_envoye_mail").arg(adresse));
        } else {
            infoCode->setText(Langue::t("oubli_code_envoye").arg(code));   // mode démo
        }

        compte = id;
        codeAttendu = code;
        expiration = QDateTime::currentDateTime().addSecs(10 * 60);
        essais = 0;
        champCode->clear();
        pages->setCurrentWidget(page2);
        champCode->setFocus();
    };

    QObject::connect(btnEnvoyer, &QPushButton::clicked, &d, envoyerCode);
    QObject::connect(champEmail, &QLineEdit::returnPressed, &d, envoyerCode);
    QObject::connect(btnRenvoyer, &QPushButton::clicked, &d, envoyerCode);
    QObject::connect(btnRetour, &QPushButton::clicked, &d, [&]() {
        codeAttendu.clear();
        pages->setCurrentWidget(page1);
        champEmail->setFocus();
        champEmail->selectAll();
    });

    auto verifierCode = [&]() {
        const QString titreErreur = Langue::t("titre_erreur");
        if (codeAttendu.isEmpty() || QDateTime::currentDateTime() > expiration) {
            codeAttendu.clear();
            CustomDialog::error(&d, titreErreur, Langue::t("oubli_code_expire"));
            return;
        }
        if (champCode->text().trimmed() != codeAttendu) {
            ++essais;
            champCode->clear();
            champCode->setFocus();
            if (essais >= 3) {
                codeAttendu.clear();
                CustomDialog::error(&d, titreErreur, Langue::t("oubli_trop_essais"));
                pages->setCurrentWidget(page1);
            } else {
                CustomDialog::error(&d, titreErreur,
                                    Langue::t("oubli_err_code") + "\n" + Langue::t("login_restant").arg(3 - essais));
            }
            return;
        }
        codeAttendu.clear();                  // un code ne sert qu'une fois
        pages->setCurrentWidget(page3);
        champNouveau->setFocus();
    };
    QObject::connect(btnVerifier, &QPushButton::clicked, &d, verifierCode);
    QObject::connect(champCode, &QLineEdit::returnPressed, &d, verifierCode);

    QObject::connect(voir, &QCheckBox::toggled, &d, [&](bool visible) {
        const auto mode = visible ? QLineEdit::Normal : QLineEdit::Password;
        champNouveau->setEchoMode(mode);
        champConfirmer->setEchoMode(mode);
    });

    auto changer = [&]() {
        const QString titreErreur = Langue::t("titre_erreur");
        if (champNouveau->text().size() < 4) {
            CustomDialog::error(&d, titreErreur, Langue::t("oubli_err_court"));
            return;
        }
        if (champNouveau->text() != champConfirmer->text()) {
            CustomDialog::error(&d, titreErreur, Langue::t("oubli_err_diff"));
            return;
        }
        AuthManager::definirMotDePasse(compte, champNouveau->text());
        CustomDialog::success(&d, Langue::t("titre_succes"), Langue::t("oubli_ok"));
        d.accept();
    };
    QObject::connect(btnChanger, &QPushButton::clicked, &d, changer);
    QObject::connect(champConfirmer, &QLineEdit::returnPressed, &d, changer);

    for (QPushButton *b : { btnAnnuler1, btnAnnuler2, btnAnnuler3 })
        QObject::connect(b, &QPushButton::clicked, &d, &QDialog::reject);

    pages->setCurrentWidget(page1);
    champEmail->setFocus();
    d.exec();
}

// =====================================================================
//                       Changer mon mot de passe
// =====================================================================
void DialogsAuth::changerMotDePasse(QWidget *parent, const QString &utilisateur)
{
    QDialog d(parent);
    d.setWindowTitle(Langue::t("mdp_titre"));
    d.setModal(true);
    d.setMinimumWidth(400);
    d.setLayoutDirection(Langue::direction());
    d.setStyleSheet(STYLE_DIALOGUE);

    auto *lay = new QVBoxLayout(&d);
    lay->setContentsMargins(24, 22, 24, 20);
    lay->setSpacing(8);

    lay->addWidget(creerLabel(AuthManager::emoji(utilisateur) + "  " + Langue::t("mdp_titre"), "dlgTitre"));
    lay->addSpacing(4);

    lay->addWidget(creerLabel(Langue::t("mdp_actuel"), "dlgLabel"));
    auto *champActuel = creerChampMotDePasse();
    lay->addWidget(champActuel);

    lay->addWidget(creerLabel(Langue::t("oubli_nouveau"), "dlgLabel"));
    auto *champNouveau = creerChampMotDePasse();
    lay->addWidget(champNouveau);

    lay->addWidget(creerLabel(Langue::t("oubli_confirmer"), "dlgLabel"));
    auto *champConfirmer = creerChampMotDePasse();
    lay->addWidget(champConfirmer);

    lay->addSpacing(6);
    auto *boutons = new QHBoxLayout;
    boutons->addStretch();
    auto *btnAnnuler = creerBouton(Langue::t("btn_annuler"), "dlgSecondaire");
    auto *btnValider = creerBouton(Langue::t("enreg"), "dlgPrincipal");
    boutons->addWidget(btnAnnuler);
    boutons->addWidget(btnValider);
    lay->addLayout(boutons);

    QObject::connect(btnAnnuler, &QPushButton::clicked, &d, &QDialog::reject);
    QObject::connect(btnValider, &QPushButton::clicked, &d, [&]() {
        const QString titreErreur = Langue::t("titre_erreur");
        if (!AuthManager::verifier(utilisateur, champActuel->text())) {
            CustomDialog::error(&d, titreErreur, Langue::t("mdp_err_actuel"));
            return;
        }
        if (champNouveau->text().size() < 4) {
            CustomDialog::error(&d, titreErreur, Langue::t("oubli_err_court"));
            return;
        }
        if (champNouveau->text() != champConfirmer->text()) {
            CustomDialog::error(&d, titreErreur, Langue::t("oubli_err_diff"));
            return;
        }
        AuthManager::definirMotDePasse(utilisateur, champNouveau->text());
        CustomDialog::success(&d, Langue::t("titre_succes"), Langue::t("mdp_ok"));
        d.accept();
    });

    d.exec();
}
