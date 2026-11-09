/********************************************************************************
** Form generated from reading UI file 'gpetmanager.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GPETMANAGER_H
#define UI_GPETMANAGER_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTimeEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GPetManager
{
public:
    QWidget *centralwidget;
    QHBoxLayout *layoutPrincipal;
    QFrame *frameSidebar;
    QLabel *lblLogo;
    QPushButton *btnAccueil;
    QPushButton *btnAnimaux;
    QPushButton *btnRendezVous;
    QPushButton *btnStock;
    QPushButton *btnCommandes;
    QPushButton *btnFournisseurs;
    QPushButton *btnEmployes;
    QPushButton *btnParametres;
    QWidget *zoneDroite;
    QVBoxLayout *layoutDroite;
    QFrame *frameHeader;
    QHBoxLayout *layoutHeader;
    QLabel *lblIconeTitre;
    QLabel *lblTitre;
    QSpacerItem *espaceHeader;
    QPushButton *btnNotif;
    QSpacerItem *espaceAvatar;
    QLabel *lblAvatar;
    QStackedWidget *SWModules;
    QWidget *pageAccueil;
    QVBoxLayout *layout_pageAccueil;
    QLabel *lbl_pageAccueil;
    QWidget *pageAnimaux;
    QVBoxLayout *layout_pageAnimaux;
    QLabel *lbl_pageAnimaux;
    QWidget *pageRendezVous;
    QVBoxLayout *layoutPageRdv;
    QHBoxLayout *layoutOutilsRdv;
    QLineEdit *txtRechercheRdv;
    QLabel *lblStatutRdv;
    QComboBox *cmbStatutRdv;
    QLabel *lblTriRdv;
    QComboBox *cmbTriRdv;
    QSpacerItem *spacerOutilsRdv;
    QPushButton *btnAjouterRdv;
    QHBoxLayout *layoutCorpsRdv;
    QVBoxLayout *layoutGaucheRdv;
    QTableView *tableRdv;
    QFrame *frameDetailsRdv;
    QVBoxLayout *layoutFrameDetailsRdv;
    QLabel *lblTitreDetailsRdv;
    QGridLayout *layoutDetailsRdv;
    QLabel *lblDetCle0Rdv;
    QLabel *lblDetVal0Rdv;
    QLabel *lblDetCle1Rdv;
    QLabel *lblDetVal1Rdv;
    QLabel *lblDetCle2Rdv;
    QLabel *lblDetVal2Rdv;
    QLabel *lblDetCle3Rdv;
    QLabel *lblDetVal3Rdv;
    QLabel *lblDetCle4Rdv;
    QLabel *lblDetVal4Rdv;
    QLabel *lblDetCle5Rdv;
    QLabel *lblDetVal5Rdv;
    QLabel *lblDetCle6Rdv;
    QLabel *lblDetVal6Rdv;
    QSpacerItem *spacerDetRdv;
    QSpacerItem *spacerFinDetRdv;
    QPushButton *btnExporterRdv;
    QVBoxLayout *layoutDroiteRdv;
    QFrame *frameFormRdv;
    QVBoxLayout *layoutFrameFormRdv;
    QLabel *lblTitreFormRdv;
    QVBoxLayout *layoutChampsRdv;
    QLabel *lblChamp0Rdv;
    QLineEdit *txtIdRdv;
    QLabel *lblChamp1Rdv;
    QDateEdit *dateRdv;
    QLabel *lblChamp2Rdv;
    QTimeEdit *timeRdv;
    QLabel *lblChamp3Rdv;
    QLineEdit *txtAnimalRdv;
    QLabel *lblChamp4Rdv;
    QComboBox *cmbMotifRdv;
    QLabel *lblChamp5Rdv;
    QComboBox *cmbStatutFormRdv;
    QSpacerItem *spacerFormRdv;
    QHBoxLayout *layoutBoutonsFormRdv;
    QPushButton *btnAnnulerRdv;
    QPushButton *btnEnregistrerRdv;
    QSpacerItem *spacerDroiteRdv;
    QWidget *pageStock;
    QVBoxLayout *layout_pageStock;
    QLabel *lbl_pageStock;
    QWidget *pageCommandes;
    QVBoxLayout *layout_pageCommandes;
    QLabel *lbl_pageCommandes;
    QWidget *pageFournisseurs;
    QVBoxLayout *layout_pageFournisseurs;
    QLabel *lbl_pageFournisseurs;
    QWidget *pageEmployes;
    QVBoxLayout *layout_pageEmployes;
    QLabel *lbl_pageEmployes;
    QWidget *pageParametres;
    QVBoxLayout *layout_pageParametres;
    QLabel *lbl_pageParametres;

    void setupUi(QMainWindow *GPetManager)
    {
        if (GPetManager->objectName().isEmpty())
            GPetManager->setObjectName("GPetManager");
        GPetManager->resize(1366, 768);
        GPetManager->setMinimumSize(QSize(1000, 600));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icones/logo.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        GPetManager->setWindowIcon(icon);
        QFont font;
        font.setFamilies({QString::fromUtf8("Segoe UI")});
        font.setPointSize(10);
        GPetManager->setFont(font);
        GPetManager->setStyleSheet(QString::fromUtf8("QMessageBox { background-color: white; }\n"
"QMessageBox QLabel { color: #1E3A3A; }\n"
"QMessageBox QPushButton { background-color: #2A8C86; color: white; border: none; border-radius: 6px;\n"
"                          padding: 6px 18px; min-width: 70px; font-weight: bold; }\n"
"QMessageBox QPushButton:hover { background-color: #1F6F6B; }\n"
""));
        centralwidget = new QWidget(GPetManager);
        centralwidget->setObjectName("centralwidget");
        layoutPrincipal = new QHBoxLayout(centralwidget);
        layoutPrincipal->setSpacing(0);
        layoutPrincipal->setObjectName("layoutPrincipal");
        layoutPrincipal->setContentsMargins(0, 0, 0, 0);
        frameSidebar = new QFrame(centralwidget);
        frameSidebar->setObjectName("frameSidebar");
        frameSidebar->setMinimumSize(QSize(215, 0));
        frameSidebar->setMaximumSize(QSize(215, 16777215));
        lblLogo = new QLabel(frameSidebar);
        lblLogo->setObjectName("lblLogo");
        lblLogo->setGeometry(QRect(42, 10, 130, 130));
        lblLogo->setScaledContents(true);
        lblLogo->setPixmap(QPixmap(QString::fromUtf8(":/icones/logo.png")));
        btnAccueil = new QPushButton(frameSidebar);
        btnAccueil->setObjectName("btnAccueil");
        btnAccueil->setGeometry(QRect(10, 156, 195, 42));
        btnAccueil->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnAccueil->setCheckable(true);
        btnAccueil->setAutoExclusive(true);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icones/home_dark.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon1.addFile(QString::fromUtf8(":/icones/home_white.png"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        btnAccueil->setIcon(icon1);
        btnAccueil->setIconSize(QSize(20, 20));
        btnAnimaux = new QPushButton(frameSidebar);
        btnAnimaux->setObjectName("btnAnimaux");
        btnAnimaux->setGeometry(QRect(10, 202, 195, 42));
        btnAnimaux->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnAnimaux->setCheckable(true);
        btnAnimaux->setAutoExclusive(true);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icones/paw_dark.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon2.addFile(QString::fromUtf8(":/icones/paw_white.png"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        btnAnimaux->setIcon(icon2);
        btnAnimaux->setIconSize(QSize(20, 20));
        btnRendezVous = new QPushButton(frameSidebar);
        btnRendezVous->setObjectName("btnRendezVous");
        btnRendezVous->setGeometry(QRect(10, 248, 195, 42));
        btnRendezVous->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnRendezVous->setCheckable(true);
        btnRendezVous->setAutoExclusive(true);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icones/calendar_dark.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon3.addFile(QString::fromUtf8(":/icones/calendar_white.png"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        btnRendezVous->setIcon(icon3);
        btnRendezVous->setIconSize(QSize(20, 20));
        btnRendezVous->setChecked(true);
        btnStock = new QPushButton(frameSidebar);
        btnStock->setObjectName("btnStock");
        btnStock->setGeometry(QRect(10, 294, 195, 42));
        btnStock->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnStock->setCheckable(true);
        btnStock->setAutoExclusive(true);
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icones/box_dark.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon4.addFile(QString::fromUtf8(":/icones/box_white.png"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        btnStock->setIcon(icon4);
        btnStock->setIconSize(QSize(20, 20));
        btnCommandes = new QPushButton(frameSidebar);
        btnCommandes->setObjectName("btnCommandes");
        btnCommandes->setGeometry(QRect(10, 340, 195, 42));
        btnCommandes->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnCommandes->setCheckable(true);
        btnCommandes->setAutoExclusive(true);
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icones/cart_dark.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon5.addFile(QString::fromUtf8(":/icones/cart_white.png"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        btnCommandes->setIcon(icon5);
        btnCommandes->setIconSize(QSize(20, 20));
        btnFournisseurs = new QPushButton(frameSidebar);
        btnFournisseurs->setObjectName("btnFournisseurs");
        btnFournisseurs->setGeometry(QRect(10, 386, 195, 42));
        btnFournisseurs->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnFournisseurs->setCheckable(true);
        btnFournisseurs->setAutoExclusive(true);
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/icones/truck_dark.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon6.addFile(QString::fromUtf8(":/icones/truck_white.png"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        btnFournisseurs->setIcon(icon6);
        btnFournisseurs->setIconSize(QSize(20, 20));
        btnEmployes = new QPushButton(frameSidebar);
        btnEmployes->setObjectName("btnEmployes");
        btnEmployes->setGeometry(QRect(10, 432, 195, 42));
        btnEmployes->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnEmployes->setCheckable(true);
        btnEmployes->setAutoExclusive(true);
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/icones/users_dark.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon7.addFile(QString::fromUtf8(":/icones/users_white.png"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        btnEmployes->setIcon(icon7);
        btnEmployes->setIconSize(QSize(20, 20));
        btnParametres = new QPushButton(frameSidebar);
        btnParametres->setObjectName("btnParametres");
        btnParametres->setGeometry(QRect(10, 478, 195, 42));
        btnParametres->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnParametres->setCheckable(true);
        btnParametres->setAutoExclusive(true);
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/icones/gear_dark.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon8.addFile(QString::fromUtf8(":/icones/gear_white.png"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        btnParametres->setIcon(icon8);
        btnParametres->setIconSize(QSize(20, 20));

        layoutPrincipal->addWidget(frameSidebar);

        zoneDroite = new QWidget(centralwidget);
        zoneDroite->setObjectName("zoneDroite");
        layoutDroite = new QVBoxLayout(zoneDroite);
        layoutDroite->setSpacing(0);
        layoutDroite->setObjectName("layoutDroite");
        layoutDroite->setContentsMargins(0, 0, 0, 0);
        frameHeader = new QFrame(zoneDroite);
        frameHeader->setObjectName("frameHeader");
        frameHeader->setMinimumSize(QSize(0, 80));
        frameHeader->setMaximumSize(QSize(16777215, 80));
        layoutHeader = new QHBoxLayout(frameHeader);
        layoutHeader->setSpacing(10);
        layoutHeader->setObjectName("layoutHeader");
        layoutHeader->setContentsMargins(20, 0, 25, 0);
        lblIconeTitre = new QLabel(frameHeader);
        lblIconeTitre->setObjectName("lblIconeTitre");
        lblIconeTitre->setMinimumSize(QSize(34, 34));
        lblIconeTitre->setMaximumSize(QSize(34, 34));
        lblIconeTitre->setScaledContents(true);

        layoutHeader->addWidget(lblIconeTitre);

        lblTitre = new QLabel(frameHeader);
        lblTitre->setObjectName("lblTitre");

        layoutHeader->addWidget(lblTitre);

        espaceHeader = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        layoutHeader->addItem(espaceHeader);

        btnNotif = new QPushButton(frameHeader);
        btnNotif->setObjectName("btnNotif");
        btnNotif->setMinimumSize(QSize(40, 40));
        btnNotif->setMaximumSize(QSize(40, 40));
        btnNotif->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/icones/bell_teal.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnNotif->setIcon(icon9);
        btnNotif->setIconSize(QSize(24, 24));

        layoutHeader->addWidget(btnNotif);

        espaceAvatar = new QSpacerItem(10, 20, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);

        layoutHeader->addItem(espaceAvatar);

        lblAvatar = new QLabel(frameHeader);
        lblAvatar->setObjectName("lblAvatar");
        lblAvatar->setMinimumSize(QSize(44, 44));
        lblAvatar->setMaximumSize(QSize(44, 44));
        lblAvatar->setAlignment(Qt::AlignCenter);

        layoutHeader->addWidget(lblAvatar);


        layoutDroite->addWidget(frameHeader);

        SWModules = new QStackedWidget(zoneDroite);
        SWModules->setObjectName("SWModules");
        pageAccueil = new QWidget();
        pageAccueil->setObjectName("pageAccueil");
        layout_pageAccueil = new QVBoxLayout(pageAccueil);
        layout_pageAccueil->setSpacing(0);
        layout_pageAccueil->setObjectName("layout_pageAccueil");
        layout_pageAccueil->setContentsMargins(30, 30, 30, 30);
        lbl_pageAccueil = new QLabel(pageAccueil);
        lbl_pageAccueil->setObjectName("lbl_pageAccueil");
        lbl_pageAccueil->setTextFormat(Qt::RichText);
        lbl_pageAccueil->setAlignment(Qt::AlignCenter);
        lbl_pageAccueil->setWordWrap(true);

        layout_pageAccueil->addWidget(lbl_pageAccueil);

        SWModules->addWidget(pageAccueil);
        pageAnimaux = new QWidget();
        pageAnimaux->setObjectName("pageAnimaux");
        layout_pageAnimaux = new QVBoxLayout(pageAnimaux);
        layout_pageAnimaux->setSpacing(0);
        layout_pageAnimaux->setObjectName("layout_pageAnimaux");
        layout_pageAnimaux->setContentsMargins(30, 30, 30, 30);
        lbl_pageAnimaux = new QLabel(pageAnimaux);
        lbl_pageAnimaux->setObjectName("lbl_pageAnimaux");
        lbl_pageAnimaux->setTextFormat(Qt::RichText);
        lbl_pageAnimaux->setAlignment(Qt::AlignCenter);
        lbl_pageAnimaux->setWordWrap(true);

        layout_pageAnimaux->addWidget(lbl_pageAnimaux);

        SWModules->addWidget(pageAnimaux);
        pageRendezVous = new QWidget();
        pageRendezVous->setObjectName("pageRendezVous");
        pageRendezVous->setStyleSheet(QString::fromUtf8("QWidget#pageRendezVous { background-color: #F4F8F8; }\n"
"QLabel { background: transparent; color: #1E3A3A; }\n"
"\n"
"/* champs de saisie */\n"
"QLineEdit, QComboBox, QDateEdit, QTimeEdit {\n"
"    border: 1px solid #D5E0E0; border-radius: 6px; padding: 0 10px;\n"
"    background: white; color: #1E3A3A; font-size: 13px; min-height: 30px; }\n"
"QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QTimeEdit:focus { border: 1px solid #2A8C86; }\n"
"QComboBox::drop-down, QDateEdit::drop-down {\n"
"    subcontrol-origin: padding; subcontrol-position: center right; width: 30px; border: none; }\n"
"QComboBox::down-arrow, QDateEdit::down-arrow {\n"
"    image: url(:/icones/fleche_bas.png); width: 12px; height: 12px; }\n"
"QTimeEdit::up-button { subcontrol-origin: border; subcontrol-position: top right; width: 24px; border: none; }\n"
"QTimeEdit::down-button { subcontrol-origin: border; subcontrol-position: bottom right; width: 24px; border: none; }\n"
"QTimeEdit::up-arrow { image: url(:/icones/fleche_haut.png); width: "
                        "10px; height: 10px; }\n"
"QTimeEdit::down-arrow { image: url(:/icones/fleche_bas.png); width: 10px; height: 10px; }\n"
"QComboBox QAbstractItemView {\n"
"    background: white; color: #1E3A3A; border: 1px solid #D5E0E0; outline: 0; padding: 4px;\n"
"    selection-background-color: #E6F2F1; selection-color: #1E3A3A; }\n"
"QComboBox QAbstractItemView::item { min-height: 30px; padding-left: 8px; color: #1E3A3A; background: white; }\n"
"QComboBox QAbstractItemView::item:hover, QComboBox QAbstractItemView::item:selected {\n"
"    background: #E6F2F1; color: #1E3A3A; }\n"
"QCalendarWidget QWidget { background-color: white; color: #1E3A3A; }\n"
"QCalendarWidget QToolButton { color: #17363A; background-color: white; font-weight: bold; }\n"
"QCalendarWidget QAbstractItemView:enabled {\n"
"    background-color: white; color: #1E3A3A; selection-background-color: #2A8C86; selection-color: white; }\n"
"QCalendarWidget QAbstractItemView:disabled { color: #9BB0B0; }\n"
"\n"
"/* cadres : formulaire et d\303\251tails */\n"
"QF"
                        "rame#frameFormRdv, QFrame#frameDetailsRdv {\n"
"    background: white; border: 1px solid #E0E8E8; border-radius: 10px; }\n"
"QLabel#lblTitreFormRdv, QLabel#lblTitreDetailsRdv {\n"
"    background: #2A8C86; color: white; font-weight: bold; font-size: 14px; padding: 11px 18px;\n"
"    border: none; border-top-left-radius: 10px; border-top-right-radius: 10px; }\n"
"QLabel#lblChamp0Rdv, QLabel#lblChamp1Rdv, QLabel#lblChamp2Rdv,\n"
"QLabel#lblChamp3Rdv, QLabel#lblChamp4Rdv, QLabel#lblChamp5Rdv {\n"
"    color: #6B8080; font-size: 12px; border: none; background: transparent; }\n"
"QLabel#lblDetCle0Rdv, QLabel#lblDetCle1Rdv, QLabel#lblDetCle2Rdv, QLabel#lblDetCle3Rdv,\n"
"QLabel#lblDetCle4Rdv, QLabel#lblDetCle5Rdv, QLabel#lblDetCle6Rdv {\n"
"    color: #6B8080; font-size: 13px; border: none; background: transparent; }\n"
"QLabel#lblDetVal0Rdv, QLabel#lblDetVal1Rdv, QLabel#lblDetVal2Rdv,\n"
"QLabel#lblDetVal4Rdv, QLabel#lblDetVal5Rdv, QLabel#lblDetVal6Rdv {\n"
"    color: #17363A; font-size: 13px; font-weight: bold; b"
                        "order: none; background: transparent; }\n"
"\n"
"/* boutons */\n"
"QPushButton#btnAjouterRdv, QPushButton#btnEnregistrerRdv, QPushButton#btnExporterRdv {\n"
"    background: #2A8C86; color: white; border: none; border-radius: 8px;\n"
"    font-weight: bold; font-size: 13px; padding: 0 18px; }\n"
"QPushButton#btnAjouterRdv:hover, QPushButton#btnEnregistrerRdv:hover,\n"
"QPushButton#btnExporterRdv:hover { background: #1F6F6B; }\n"
"QPushButton#btnAnnulerRdv {\n"
"    background: white; color: #2A8C86; border: 1px solid #2A8C86; border-radius: 8px;\n"
"    font-weight: bold; font-size: 13px; }\n"
"QPushButton#btnAnnulerRdv:hover { background: #E6F2F1; }\n"
"\n"
"/* tableau */\n"
"QTableView#tableRdv {\n"
"    background: white; color: #1E3A3A; font-size: 13px; border: 1px solid #E0E8E8;\n"
"    border-radius: 10px; selection-background-color: #E6F2F1; selection-color: #1E3A3A; }\n"
"QTableView#tableRdv QHeaderView::section {\n"
"    background: white; color: #17363A; font-weight: bold; padding: 10px 8px;\n"
"    "
                        "border: none; border-bottom: 1px solid #E0E8E8; }\n"
"QTableView#tableRdv QTableCornerButton::section { background: white; border: none; }\n"
"QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }\n"
"QScrollBar::handle:vertical { background: #C5D6D5; border-radius: 5px; min-height: 40px; }\n"
"QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }\n"
"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }\n"
""));
        layoutPageRdv = new QVBoxLayout(pageRendezVous);
        layoutPageRdv->setSpacing(16);
        layoutPageRdv->setObjectName("layoutPageRdv");
        layoutPageRdv->setContentsMargins(30, 24, 30, 24);
        layoutOutilsRdv = new QHBoxLayout();
        layoutOutilsRdv->setSpacing(12);
        layoutOutilsRdv->setObjectName("layoutOutilsRdv");
        txtRechercheRdv = new QLineEdit(pageRendezVous);
        txtRechercheRdv->setObjectName("txtRechercheRdv");
        txtRechercheRdv->setMinimumSize(QSize(170, 36));
        txtRechercheRdv->setMaximumSize(QSize(300, 36));

        layoutOutilsRdv->addWidget(txtRechercheRdv);

        lblStatutRdv = new QLabel(pageRendezVous);
        lblStatutRdv->setObjectName("lblStatutRdv");

        layoutOutilsRdv->addWidget(lblStatutRdv);

        cmbStatutRdv = new QComboBox(pageRendezVous);
        cmbStatutRdv->setObjectName("cmbStatutRdv");
        cmbStatutRdv->setMinimumSize(QSize(120, 36));

        layoutOutilsRdv->addWidget(cmbStatutRdv);

        lblTriRdv = new QLabel(pageRendezVous);
        lblTriRdv->setObjectName("lblTriRdv");

        layoutOutilsRdv->addWidget(lblTriRdv);

        cmbTriRdv = new QComboBox(pageRendezVous);
        cmbTriRdv->setObjectName("cmbTriRdv");
        cmbTriRdv->setMinimumSize(QSize(170, 36));

        layoutOutilsRdv->addWidget(cmbTriRdv);

        spacerOutilsRdv = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        layoutOutilsRdv->addItem(spacerOutilsRdv);

        btnAjouterRdv = new QPushButton(pageRendezVous);
        btnAjouterRdv->setObjectName("btnAjouterRdv");
        btnAjouterRdv->setMinimumSize(QSize(110, 38));
        btnAjouterRdv->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        layoutOutilsRdv->addWidget(btnAjouterRdv);


        layoutPageRdv->addLayout(layoutOutilsRdv);

        layoutCorpsRdv = new QHBoxLayout();
        layoutCorpsRdv->setSpacing(20);
        layoutCorpsRdv->setObjectName("layoutCorpsRdv");
        layoutCorpsRdv->setContentsMargins(0, 0, 0, 0);
        layoutGaucheRdv = new QVBoxLayout();
        layoutGaucheRdv->setSpacing(16);
        layoutGaucheRdv->setObjectName("layoutGaucheRdv");
        layoutGaucheRdv->setContentsMargins(0, 0, 0, 0);
        tableRdv = new QTableView(pageRendezVous);
        tableRdv->setObjectName("tableRdv");
        tableRdv->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableRdv->setSelectionMode(QAbstractItemView::SingleSelection);
        tableRdv->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableRdv->setShowGrid(false);
        tableRdv->horizontalHeader()->setStretchLastSection(true);
        tableRdv->verticalHeader()->setVisible(false);

        layoutGaucheRdv->addWidget(tableRdv);

        frameDetailsRdv = new QFrame(pageRendezVous);
        frameDetailsRdv->setObjectName("frameDetailsRdv");
        frameDetailsRdv->setMinimumSize(QSize(0, 230));
        frameDetailsRdv->setMaximumSize(QSize(16777215, 230));
        layoutFrameDetailsRdv = new QVBoxLayout(frameDetailsRdv);
        layoutFrameDetailsRdv->setSpacing(0);
        layoutFrameDetailsRdv->setObjectName("layoutFrameDetailsRdv");
        layoutFrameDetailsRdv->setContentsMargins(0, 0, 0, 0);
        lblTitreDetailsRdv = new QLabel(frameDetailsRdv);
        lblTitreDetailsRdv->setObjectName("lblTitreDetailsRdv");
        lblTitreDetailsRdv->setMinimumSize(QSize(0, 46));
        lblTitreDetailsRdv->setMaximumSize(QSize(16777215, 46));

        layoutFrameDetailsRdv->addWidget(lblTitreDetailsRdv);

        layoutDetailsRdv = new QGridLayout();
        layoutDetailsRdv->setObjectName("layoutDetailsRdv");
        layoutDetailsRdv->setHorizontalSpacing(14);
        layoutDetailsRdv->setVerticalSpacing(10);
        layoutDetailsRdv->setContentsMargins(20, 14, 20, 14);
        lblDetCle0Rdv = new QLabel(frameDetailsRdv);
        lblDetCle0Rdv->setObjectName("lblDetCle0Rdv");

        layoutDetailsRdv->addWidget(lblDetCle0Rdv, 0, 0, 1, 1);

        lblDetVal0Rdv = new QLabel(frameDetailsRdv);
        lblDetVal0Rdv->setObjectName("lblDetVal0Rdv");

        layoutDetailsRdv->addWidget(lblDetVal0Rdv, 0, 1, 1, 1);

        lblDetCle1Rdv = new QLabel(frameDetailsRdv);
        lblDetCle1Rdv->setObjectName("lblDetCle1Rdv");

        layoutDetailsRdv->addWidget(lblDetCle1Rdv, 1, 0, 1, 1);

        lblDetVal1Rdv = new QLabel(frameDetailsRdv);
        lblDetVal1Rdv->setObjectName("lblDetVal1Rdv");

        layoutDetailsRdv->addWidget(lblDetVal1Rdv, 1, 1, 1, 1);

        lblDetCle2Rdv = new QLabel(frameDetailsRdv);
        lblDetCle2Rdv->setObjectName("lblDetCle2Rdv");

        layoutDetailsRdv->addWidget(lblDetCle2Rdv, 2, 0, 1, 1);

        lblDetVal2Rdv = new QLabel(frameDetailsRdv);
        lblDetVal2Rdv->setObjectName("lblDetVal2Rdv");

        layoutDetailsRdv->addWidget(lblDetVal2Rdv, 2, 1, 1, 1);

        lblDetCle3Rdv = new QLabel(frameDetailsRdv);
        lblDetCle3Rdv->setObjectName("lblDetCle3Rdv");

        layoutDetailsRdv->addWidget(lblDetCle3Rdv, 3, 0, 1, 1);

        lblDetVal3Rdv = new QLabel(frameDetailsRdv);
        lblDetVal3Rdv->setObjectName("lblDetVal3Rdv");
        lblDetVal3Rdv->setMaximumSize(QSize(130, 26));

        layoutDetailsRdv->addWidget(lblDetVal3Rdv, 3, 1, 1, 1);

        lblDetCle4Rdv = new QLabel(frameDetailsRdv);
        lblDetCle4Rdv->setObjectName("lblDetCle4Rdv");

        layoutDetailsRdv->addWidget(lblDetCle4Rdv, 0, 3, 1, 1);

        lblDetVal4Rdv = new QLabel(frameDetailsRdv);
        lblDetVal4Rdv->setObjectName("lblDetVal4Rdv");

        layoutDetailsRdv->addWidget(lblDetVal4Rdv, 0, 4, 1, 1);

        lblDetCle5Rdv = new QLabel(frameDetailsRdv);
        lblDetCle5Rdv->setObjectName("lblDetCle5Rdv");

        layoutDetailsRdv->addWidget(lblDetCle5Rdv, 1, 3, 1, 1);

        lblDetVal5Rdv = new QLabel(frameDetailsRdv);
        lblDetVal5Rdv->setObjectName("lblDetVal5Rdv");

        layoutDetailsRdv->addWidget(lblDetVal5Rdv, 1, 4, 1, 1);

        lblDetCle6Rdv = new QLabel(frameDetailsRdv);
        lblDetCle6Rdv->setObjectName("lblDetCle6Rdv");

        layoutDetailsRdv->addWidget(lblDetCle6Rdv, 2, 3, 1, 1);

        lblDetVal6Rdv = new QLabel(frameDetailsRdv);
        lblDetVal6Rdv->setObjectName("lblDetVal6Rdv");

        layoutDetailsRdv->addWidget(lblDetVal6Rdv, 2, 4, 1, 1);

        spacerDetRdv = new QSpacerItem(120, 20, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);

        layoutDetailsRdv->addItem(spacerDetRdv, 0, 2, 1, 1);

        spacerFinDetRdv = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        layoutDetailsRdv->addItem(spacerFinDetRdv, 0, 5, 1, 1);

        btnExporterRdv = new QPushButton(frameDetailsRdv);
        btnExporterRdv->setObjectName("btnExporterRdv");
        btnExporterRdv->setMaximumSize(QSize(150, 36));
        btnExporterRdv->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        layoutDetailsRdv->addWidget(btnExporterRdv, 3, 3, 1, 2);


        layoutFrameDetailsRdv->addLayout(layoutDetailsRdv);


        layoutGaucheRdv->addWidget(frameDetailsRdv);

        layoutGaucheRdv->setStretch(0, 1);

        layoutCorpsRdv->addLayout(layoutGaucheRdv);

        layoutDroiteRdv = new QVBoxLayout();
        layoutDroiteRdv->setSpacing(0);
        layoutDroiteRdv->setObjectName("layoutDroiteRdv");
        layoutDroiteRdv->setContentsMargins(0, 0, 0, 0);
        frameFormRdv = new QFrame(pageRendezVous);
        frameFormRdv->setObjectName("frameFormRdv");
        frameFormRdv->setMinimumSize(QSize(320, 0));
        frameFormRdv->setMaximumSize(QSize(320, 16777215));
        layoutFrameFormRdv = new QVBoxLayout(frameFormRdv);
        layoutFrameFormRdv->setSpacing(0);
        layoutFrameFormRdv->setObjectName("layoutFrameFormRdv");
        layoutFrameFormRdv->setContentsMargins(0, 0, 0, 0);
        lblTitreFormRdv = new QLabel(frameFormRdv);
        lblTitreFormRdv->setObjectName("lblTitreFormRdv");
        lblTitreFormRdv->setMinimumSize(QSize(0, 46));
        lblTitreFormRdv->setMaximumSize(QSize(16777215, 46));

        layoutFrameFormRdv->addWidget(lblTitreFormRdv);

        layoutChampsRdv = new QVBoxLayout();
        layoutChampsRdv->setSpacing(6);
        layoutChampsRdv->setObjectName("layoutChampsRdv");
        layoutChampsRdv->setContentsMargins(20, 14, 20, 18);
        lblChamp0Rdv = new QLabel(frameFormRdv);
        lblChamp0Rdv->setObjectName("lblChamp0Rdv");

        layoutChampsRdv->addWidget(lblChamp0Rdv);

        txtIdRdv = new QLineEdit(frameFormRdv);
        txtIdRdv->setObjectName("txtIdRdv");

        layoutChampsRdv->addWidget(txtIdRdv);

        lblChamp1Rdv = new QLabel(frameFormRdv);
        lblChamp1Rdv->setObjectName("lblChamp1Rdv");

        layoutChampsRdv->addWidget(lblChamp1Rdv);

        dateRdv = new QDateEdit(frameFormRdv);
        dateRdv->setObjectName("dateRdv");
        dateRdv->setCalendarPopup(true);

        layoutChampsRdv->addWidget(dateRdv);

        lblChamp2Rdv = new QLabel(frameFormRdv);
        lblChamp2Rdv->setObjectName("lblChamp2Rdv");

        layoutChampsRdv->addWidget(lblChamp2Rdv);

        timeRdv = new QTimeEdit(frameFormRdv);
        timeRdv->setObjectName("timeRdv");

        layoutChampsRdv->addWidget(timeRdv);

        lblChamp3Rdv = new QLabel(frameFormRdv);
        lblChamp3Rdv->setObjectName("lblChamp3Rdv");

        layoutChampsRdv->addWidget(lblChamp3Rdv);

        txtAnimalRdv = new QLineEdit(frameFormRdv);
        txtAnimalRdv->setObjectName("txtAnimalRdv");

        layoutChampsRdv->addWidget(txtAnimalRdv);

        lblChamp4Rdv = new QLabel(frameFormRdv);
        lblChamp4Rdv->setObjectName("lblChamp4Rdv");

        layoutChampsRdv->addWidget(lblChamp4Rdv);

        cmbMotifRdv = new QComboBox(frameFormRdv);
        cmbMotifRdv->setObjectName("cmbMotifRdv");

        layoutChampsRdv->addWidget(cmbMotifRdv);

        lblChamp5Rdv = new QLabel(frameFormRdv);
        lblChamp5Rdv->setObjectName("lblChamp5Rdv");

        layoutChampsRdv->addWidget(lblChamp5Rdv);

        cmbStatutFormRdv = new QComboBox(frameFormRdv);
        cmbStatutFormRdv->setObjectName("cmbStatutFormRdv");

        layoutChampsRdv->addWidget(cmbStatutFormRdv);

        spacerFormRdv = new QSpacerItem(20, 10, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        layoutChampsRdv->addItem(spacerFormRdv);

        layoutBoutonsFormRdv = new QHBoxLayout();
        layoutBoutonsFormRdv->setSpacing(10);
        layoutBoutonsFormRdv->setObjectName("layoutBoutonsFormRdv");
        btnAnnulerRdv = new QPushButton(frameFormRdv);
        btnAnnulerRdv->setObjectName("btnAnnulerRdv");
        btnAnnulerRdv->setMinimumSize(QSize(0, 38));
        btnAnnulerRdv->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        layoutBoutonsFormRdv->addWidget(btnAnnulerRdv);

        btnEnregistrerRdv = new QPushButton(frameFormRdv);
        btnEnregistrerRdv->setObjectName("btnEnregistrerRdv");
        btnEnregistrerRdv->setMinimumSize(QSize(0, 38));
        btnEnregistrerRdv->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        layoutBoutonsFormRdv->addWidget(btnEnregistrerRdv);


        layoutChampsRdv->addLayout(layoutBoutonsFormRdv);


        layoutFrameFormRdv->addLayout(layoutChampsRdv);


        layoutDroiteRdv->addWidget(frameFormRdv);

        spacerDroiteRdv = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        layoutDroiteRdv->addItem(spacerDroiteRdv);


        layoutCorpsRdv->addLayout(layoutDroiteRdv);

        layoutCorpsRdv->setStretch(0, 1);

        layoutPageRdv->addLayout(layoutCorpsRdv);

        SWModules->addWidget(pageRendezVous);
        pageStock = new QWidget();
        pageStock->setObjectName("pageStock");
        layout_pageStock = new QVBoxLayout(pageStock);
        layout_pageStock->setSpacing(0);
        layout_pageStock->setObjectName("layout_pageStock");
        layout_pageStock->setContentsMargins(30, 30, 30, 30);
        lbl_pageStock = new QLabel(pageStock);
        lbl_pageStock->setObjectName("lbl_pageStock");
        lbl_pageStock->setTextFormat(Qt::RichText);
        lbl_pageStock->setAlignment(Qt::AlignCenter);
        lbl_pageStock->setWordWrap(true);

        layout_pageStock->addWidget(lbl_pageStock);

        SWModules->addWidget(pageStock);
        pageCommandes = new QWidget();
        pageCommandes->setObjectName("pageCommandes");
        layout_pageCommandes = new QVBoxLayout(pageCommandes);
        layout_pageCommandes->setSpacing(0);
        layout_pageCommandes->setObjectName("layout_pageCommandes");
        layout_pageCommandes->setContentsMargins(30, 30, 30, 30);
        lbl_pageCommandes = new QLabel(pageCommandes);
        lbl_pageCommandes->setObjectName("lbl_pageCommandes");
        lbl_pageCommandes->setTextFormat(Qt::RichText);
        lbl_pageCommandes->setAlignment(Qt::AlignCenter);
        lbl_pageCommandes->setWordWrap(true);

        layout_pageCommandes->addWidget(lbl_pageCommandes);

        SWModules->addWidget(pageCommandes);
        pageFournisseurs = new QWidget();
        pageFournisseurs->setObjectName("pageFournisseurs");
        layout_pageFournisseurs = new QVBoxLayout(pageFournisseurs);
        layout_pageFournisseurs->setSpacing(0);
        layout_pageFournisseurs->setObjectName("layout_pageFournisseurs");
        layout_pageFournisseurs->setContentsMargins(30, 30, 30, 30);
        lbl_pageFournisseurs = new QLabel(pageFournisseurs);
        lbl_pageFournisseurs->setObjectName("lbl_pageFournisseurs");
        lbl_pageFournisseurs->setTextFormat(Qt::RichText);
        lbl_pageFournisseurs->setAlignment(Qt::AlignCenter);
        lbl_pageFournisseurs->setWordWrap(true);

        layout_pageFournisseurs->addWidget(lbl_pageFournisseurs);

        SWModules->addWidget(pageFournisseurs);
        pageEmployes = new QWidget();
        pageEmployes->setObjectName("pageEmployes");
        layout_pageEmployes = new QVBoxLayout(pageEmployes);
        layout_pageEmployes->setSpacing(0);
        layout_pageEmployes->setObjectName("layout_pageEmployes");
        layout_pageEmployes->setContentsMargins(30, 30, 30, 30);
        lbl_pageEmployes = new QLabel(pageEmployes);
        lbl_pageEmployes->setObjectName("lbl_pageEmployes");
        lbl_pageEmployes->setTextFormat(Qt::RichText);
        lbl_pageEmployes->setAlignment(Qt::AlignCenter);
        lbl_pageEmployes->setWordWrap(true);

        layout_pageEmployes->addWidget(lbl_pageEmployes);

        SWModules->addWidget(pageEmployes);
        pageParametres = new QWidget();
        pageParametres->setObjectName("pageParametres");
        layout_pageParametres = new QVBoxLayout(pageParametres);
        layout_pageParametres->setSpacing(0);
        layout_pageParametres->setObjectName("layout_pageParametres");
        layout_pageParametres->setContentsMargins(30, 30, 30, 30);
        lbl_pageParametres = new QLabel(pageParametres);
        lbl_pageParametres->setObjectName("lbl_pageParametres");
        lbl_pageParametres->setTextFormat(Qt::RichText);
        lbl_pageParametres->setAlignment(Qt::AlignCenter);
        lbl_pageParametres->setWordWrap(true);

        layout_pageParametres->addWidget(lbl_pageParametres);

        SWModules->addWidget(pageParametres);

        layoutDroite->addWidget(SWModules);


        layoutPrincipal->addWidget(zoneDroite);

        GPetManager->setCentralWidget(centralwidget);

        retranslateUi(GPetManager);

        SWModules->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(GPetManager);
    } // setupUi

    void retranslateUi(QMainWindow *GPetManager)
    {
        GPetManager->setWindowTitle(QCoreApplication::translate("GPetManager", "Pet Manager - Smart Pet Care Center", nullptr));
        centralwidget->setStyleSheet(QCoreApplication::translate("GPetManager", "#centralwidget { background-color:#F4F8F8; }\n"
"\n"
"/* ----- Champs de saisie (style commun \303\240 toute l'application) ----- */\n"
"QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit {\n"
"  border:1px solid #D5E0E0; border-radius:6px; padding:0 10px;\n"
"  background:white; color:#1E3A3A; font-size:13px; min-height:30px; }\n"
"QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus {\n"
"  border:1px solid #2A8C86; }\n"
"QComboBox::drop-down, QDateEdit::drop-down {\n"
"  subcontrol-origin:padding; subcontrol-position:center right; width:30px; border:none; }\n"
"QComboBox::down-arrow, QDateEdit::down-arrow {\n"
"  image:url(:/icones/fleche_bas.png); width:12px; height:12px; }\n"
"QComboBox QAbstractItemView {\n"
"  background:white; color:#1E3A3A; border:1px solid #D5E0E0; outline:0; padding:4px;\n"
"  selection-background-color:#E6F2F1; selection-color:#1E3A3A; }\n"
"QComboBox QAbstractItemView::item {\n"
"  min-height:30px; padding-left:8px; color:#1E3A3A; backgrou"
                        "nd:white; }\n"
"QComboBox QAbstractItemView::item:hover, QComboBox QAbstractItemView::item:selected {\n"
"  background:#E6F2F1; color:#1E3A3A; }\n"
"QSpinBox::up-button, QDoubleSpinBox::up-button {\n"
"  subcontrol-origin:border; subcontrol-position:top right; width:28px;\n"
"  border:none; border-left:1px solid #E0E8E8; border-top-right-radius:6px; }\n"
"QSpinBox::down-button, QDoubleSpinBox::down-button {\n"
"  subcontrol-origin:border; subcontrol-position:bottom right; width:28px;\n"
"  border:none; border-left:1px solid #E0E8E8; border-bottom-right-radius:6px; }\n"
"QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover,\n"
"QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover { background:#E6F2F1; }\n"
"QSpinBox::up-arrow, QDoubleSpinBox::up-arrow { image:url(:/icones/fleche_haut.png); width:10px; height:10px; }\n"
"QSpinBox::down-arrow, QDoubleSpinBox::down-arrow { image:url(:/icones/fleche_bas.png); width:10px; height:10px; }\n"
"\n"
"/* ----- Barre de d\303\251filement ----- */\n"
"QScrol"
                        "lArea { background:transparent; border:none; }\n"
"QScrollBar:vertical { background:transparent; width:10px; margin:0; }\n"
"QScrollBar::handle:vertical { background:#C5D6D5; border-radius:5px; min-height:40px; }\n"
"QScrollBar::handle:vertical:hover { background:#2A8C86; }\n"
"QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }\n"
"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background:none; }\n"
"QScrollBar:horizontal { background:transparent; height:10px; margin:0; }\n"
"QScrollBar::handle:horizontal { background:#C5D6D5; border-radius:5px; min-width:40px; }\n"
"QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width:0; }\n"
"\n"
"/* ----- Page Commandes ----- */\n"
"QFrame#frameFormCommande, QFrame#frameDetailsCommande { background:white; border:1px solid #E0E8E8; border-radius:10px; }\n"
"QLabel#lblTitreFormCommande, QLabel#lblTitreDetailsCommande {\n"
"  background:#2A8C86; color:white; font-weight:bold; font-size:14px; padding:11px 18px;\n"
"  b"
                        "order:none; border-top-left-radius:10px; border-top-right-radius:10px; }\n"
"QLabel#lblChamp0Commande, QLabel#lblChamp1Commande, QLabel#lblChamp2Commande,\n"
"QLabel#lblChamp3Commande, QLabel#lblChamp4Commande { color:#6B8080; font-size:12px; border:none; background:transparent; }\n"
"QLabel#lblDetCle0Commande, QLabel#lblDetCle1Commande, QLabel#lblDetCle2Commande, QLabel#lblDetCle3Commande,\n"
"QLabel#lblDetCle4Commande, QLabel#lblDetCle5Commande, QLabel#lblDetCle6Commande { color:#6B8080; font-size:13px; border:none; background:transparent; }\n"
"QLabel#lblDetVal0Commande, QLabel#lblDetVal1Commande, QLabel#lblDetVal2Commande,\n"
"QLabel#lblDetVal4Commande, QLabel#lblDetVal5Commande, QLabel#lblDetVal6Commande { color:#17363A; font-size:13px; font-weight:bold; border:none; background:transparent; }\n"
"QLabel#lblStatutCommande, QLabel#lblTriCommande { color:#1E3A3A; font-size:13px; }\n"
"QPushButton#btnAjouterCommande, QPushButton#btnEnregistrerCommande, QPushButton#btnExporterCommande {\n"
"  background:#2A8C8"
                        "6; color:white; border:none; border-radius:8px; font-weight:bold; font-size:13px; padding:0 18px; }\n"
"QPushButton#btnAjouterCommande:hover, QPushButton#btnEnregistrerCommande:hover, QPushButton#btnExporterCommande:hover { background:#1F6F6B; }\n"
"QPushButton#btnAnnulerCommande { background:white; color:#2A8C86; border:1px solid #2A8C86; border-radius:8px; font-weight:bold; font-size:13px; }\n"
"QPushButton#btnAnnulerCommande:hover { background:#E6F2F1; }\n"
"QTableView#tableCommande { background:white; color:#1E3A3A; font-size:13px; border:1px solid #E0E8E8; border-radius:10px;\n"
"  selection-background-color:#E6F2F1; selection-color:#1E3A3A; }\n"
"QTableView#tableCommande QHeaderView::section { background:white; color:#17363A; font-weight:bold; padding:10px 8px; border:none; border-bottom:1px solid #E0E8E8; }\n"
"QCalendarWidget QWidget { background-color:white; color:#1E3A3A; }\n"
"QCalendarWidget QToolButton { color:#17363A; background-color:white; font-weight:bold; }\n"
"QCalendarWidget QAbstractItemVi"
                        "ew:enabled { background-color:white; color:#1E3A3A; selection-background-color:#2A8C86; selection-color:white; }\n"
"QCalendarWidget QAbstractItemView:disabled { color:#9BB0B0; }\n"
"QMessageBox { background-color:white; }\n"
"QMessageBox QLabel { color:#1E3A3A; }\n"
"QMessageBox QPushButton { background-color:#2A8C86; color:white; border:none; border-radius:6px; padding:6px 18px; min-width:70px; font-weight:bold; }\n"
"QMessageBox QPushButton:hover { background-color:#1F6F6B; }\n"
"\n"
"QToolTip { background:white; color:#1E3A3A; border:1px solid #D5E0E0; padding:4px; }", nullptr));
        frameSidebar->setStyleSheet(QCoreApplication::translate("GPetManager", ".QFrame { background-color:white; border:none; border-right:1px solid #E0E8E8; }\n"
"QPushButton { text-align:left; padding-left:15px; border:none; border-radius:8px;\n"
"  color:#1E3A3A; font-size:14px; background:transparent; }\n"
"QPushButton:hover { background-color:#E6F2F1; }\n"
"QPushButton:checked { background-color:#2A8C86; color:white; font-weight:bold; }", nullptr));
#if QT_CONFIG(tooltip)
        lblLogo->setToolTip(QCoreApplication::translate("GPetManager", "Pet Manager - Smart Pet Care Center", nullptr));
#endif // QT_CONFIG(tooltip)
        btnAccueil->setText(QCoreApplication::translate("GPetManager", "   Accueil", nullptr));
        btnAnimaux->setText(QCoreApplication::translate("GPetManager", "   Animaux", nullptr));
        btnRendezVous->setText(QCoreApplication::translate("GPetManager", "   Rendez-vous", nullptr));
        btnStock->setText(QCoreApplication::translate("GPetManager", "   Stock", nullptr));
        btnCommandes->setText(QCoreApplication::translate("GPetManager", "   Commandes", nullptr));
        btnFournisseurs->setText(QCoreApplication::translate("GPetManager", "   Fournisseurs", nullptr));
        btnEmployes->setText(QCoreApplication::translate("GPetManager", "   Employ\303\251s", nullptr));
        btnParametres->setText(QCoreApplication::translate("GPetManager", "   Param\303\250tres", nullptr));
        frameHeader->setStyleSheet(QCoreApplication::translate("GPetManager", ".QFrame { background-color:#F4F8F8; border:none; border-bottom:1px solid #E0E8E8; }", nullptr));
        lblIconeTitre->setText(QString());
        lblTitre->setStyleSheet(QCoreApplication::translate("GPetManager", "color:#17363A; font-size:26px; font-weight:bold;", nullptr));
        lblTitre->setText(QCoreApplication::translate("GPetManager", "Gestion des Rendez-vous", nullptr));
        btnNotif->setStyleSheet(QCoreApplication::translate("GPetManager", "QPushButton{border:none;background:transparent;font-size:22px;} QPushButton:hover{background:#E6F2F1;border-radius:8px;}", nullptr));
        btnNotif->setText(QString());
#if QT_CONFIG(tooltip)
        btnNotif->setToolTip(QCoreApplication::translate("GPetManager", "Notifications du stock", nullptr));
#endif // QT_CONFIG(tooltip)
        lblAvatar->setStyleSheet(QCoreApplication::translate("GPetManager", "border:2px solid #2A8C86; border-radius:22px; background:white;", nullptr));
        lblAvatar->setText(QString());
        lbl_pageAccueil->setText(QCoreApplication::translate("GPetManager", "<p><img src=\":/icones/home_teal.png\" width=\"80\" height=\"80\"></p><p style=\"font-size:24px; font-weight:bold; color:#17363A\">Accueil</p><p style=\"font-size:13px; color:#6B8080\">Cette page sera d\303\251velopp\303\251e dans le module \302\253 Accueil \302\273.</p>", nullptr));
        lbl_pageAnimaux->setText(QCoreApplication::translate("GPetManager", "<p><img src=\":/icones/paw_teal.png\" width=\"80\" height=\"80\"></p><p style=\"font-size:24px; font-weight:bold; color:#17363A\">Animaux</p><p style=\"font-size:13px; color:#6B8080\">Cette page sera d\303\251velopp\303\251e dans le module \302\253 Animaux \302\273.</p>", nullptr));
        txtRechercheRdv->setPlaceholderText(QCoreApplication::translate("GPetManager", "Rechercher par ID ...", nullptr));
        lblStatutRdv->setText(QCoreApplication::translate("GPetManager", "Statut :", nullptr));
        lblTriRdv->setText(QCoreApplication::translate("GPetManager", "Trier par :", nullptr));
        btnAjouterRdv->setText(QCoreApplication::translate("GPetManager", "+ Ajouter", nullptr));
        lblTitreDetailsRdv->setText(QCoreApplication::translate("GPetManager", "D\303\251tails du rendez-vous", nullptr));
        lblDetCle0Rdv->setText(QCoreApplication::translate("GPetManager", "ID", nullptr));
        lblDetVal0Rdv->setText(QString());
        lblDetCle1Rdv->setText(QCoreApplication::translate("GPetManager", "Date", nullptr));
        lblDetVal1Rdv->setText(QString());
        lblDetCle2Rdv->setText(QCoreApplication::translate("GPetManager", "Heure", nullptr));
        lblDetVal2Rdv->setText(QString());
        lblDetCle3Rdv->setText(QCoreApplication::translate("GPetManager", "Statut", nullptr));
        lblDetVal3Rdv->setText(QString());
        lblDetCle4Rdv->setText(QCoreApplication::translate("GPetManager", "ID Animal", nullptr));
        lblDetVal4Rdv->setText(QString());
        lblDetCle5Rdv->setText(QCoreApplication::translate("GPetManager", "Animal", nullptr));
        lblDetVal5Rdv->setText(QString());
        lblDetCle6Rdv->setText(QCoreApplication::translate("GPetManager", "Motif", nullptr));
        lblDetVal6Rdv->setText(QString());
        btnExporterRdv->setText(QCoreApplication::translate("GPetManager", "Exporter", nullptr));
        lblTitreFormRdv->setText(QCoreApplication::translate("GPetManager", "Ajouter un rendez-vous", nullptr));
        lblChamp0Rdv->setText(QCoreApplication::translate("GPetManager", "ID du rendez-vous", nullptr));
        txtIdRdv->setPlaceholderText(QCoreApplication::translate("GPetManager", "Entrer l'ID du rendez-vous", nullptr));
        lblChamp1Rdv->setText(QCoreApplication::translate("GPetManager", "Date", nullptr));
        lblChamp2Rdv->setText(QCoreApplication::translate("GPetManager", "Heure", nullptr));
        lblChamp3Rdv->setText(QCoreApplication::translate("GPetManager", "ID Animal", nullptr));
        txtAnimalRdv->setPlaceholderText(QCoreApplication::translate("GPetManager", "Entrer l'ID de l'animal", nullptr));
        lblChamp4Rdv->setText(QCoreApplication::translate("GPetManager", "Motif", nullptr));
        lblChamp5Rdv->setText(QCoreApplication::translate("GPetManager", "Statut", nullptr));
        btnAnnulerRdv->setText(QCoreApplication::translate("GPetManager", "Annuler", nullptr));
        btnEnregistrerRdv->setText(QCoreApplication::translate("GPetManager", "Enregistrer", nullptr));
        lbl_pageStock->setText(QCoreApplication::translate("GPetManager", "<p><img src=\":/icones/box_teal.png\" width=\"80\" height=\"80\"></p><p style=\"font-size:24px; font-weight:bold; color:#17363A\">Stock</p><p style=\"font-size:13px; color:#6B8080\">Cette page sera d\303\251velopp\303\251e dans le module \302\253 Stock \302\273.</p>", nullptr));
        lbl_pageCommandes->setText(QCoreApplication::translate("GPetManager", "<p><img src=\":/icones/cart_teal.png\" width=\"80\" height=\"80\"></p><p style=\"font-size:24px; font-weight:bold; color:#17363A\">Commandes</p><p style=\"font-size:13px; color:#6B8080\">Cette page sera d\303\251velopp\303\251e dans le module \302\253 Commandes \302\273.</p>", nullptr));
        lbl_pageFournisseurs->setText(QCoreApplication::translate("GPetManager", "<p><img src=\":/icones/truck_teal.png\" width=\"80\" height=\"80\"></p><p style=\"font-size:24px; font-weight:bold; color:#17363A\">Fournisseurs</p><p style=\"font-size:13px; color:#6B8080\">Cette page sera d\303\251velopp\303\251e dans le module \302\253 Fournisseurs \302\273.</p>", nullptr));
        lbl_pageEmployes->setText(QCoreApplication::translate("GPetManager", "<p><img src=\":/icones/users_teal.png\" width=\"80\" height=\"80\"></p><p style=\"font-size:24px; font-weight:bold; color:#17363A\">Employ\303\251s</p><p style=\"font-size:13px; color:#6B8080\">Cette page sera d\303\251velopp\303\251e dans le module \302\253 Employ\303\251s \302\273.</p>", nullptr));
        lbl_pageParametres->setText(QCoreApplication::translate("GPetManager", "<p><img src=\":/icones/gear_teal.png\" width=\"80\" height=\"80\"></p><p style=\"font-size:24px; font-weight:bold; color:#17363A\">Param\303\250tres</p><p style=\"font-size:13px; color:#6B8080\">Cette page sera d\303\251velopp\303\251e dans le module \302\253 Param\303\250tres \302\273.</p>", nullptr));
    } // retranslateUi

};

namespace Ui {
    class GPetManager: public Ui_GPetManager {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GPETMANAGER_H
