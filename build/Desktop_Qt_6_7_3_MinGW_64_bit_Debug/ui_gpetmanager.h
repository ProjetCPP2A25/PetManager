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
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GPetManager
{
public:
    QWidget *centralwidget;
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
    QVBoxLayout *layout_pageRendezVous;
    QLabel *lbl_pageRendezVous;
    QWidget *pageStock;
    QVBoxLayout *layout_pageStock;
    QLabel *lbl_pageStock;
    QWidget *pageCommandes;
    QVBoxLayout *layoutPageCommandes;
    QHBoxLayout *layoutOutilsCommande;
    QLineEdit *txtRechercheCommande;
    QLabel *lblStatutCommande;
    QComboBox *cmbStatutCommande;
    QLabel *lblTriCommande;
    QComboBox *cmbTriCommande;
    QSpacerItem *spacerOutilsCommande;
    QPushButton *btnAjouterCommande;
    QHBoxLayout *layoutCorpsCommande;
    QVBoxLayout *layoutGaucheCommande;
    QTableView *tableCommande;
    QFrame *frameDetailsCommande;
    QVBoxLayout *layoutFrameDetailsCommande;
    QLabel *lblTitreDetailsCommande;
    QGridLayout *layoutDetailsCommande;
    QLabel *lblDetCle0Commande;
    QLabel *lblDetVal0Commande;
    QLabel *lblDetCle1Commande;
    QLabel *lblDetVal1Commande;
    QLabel *lblDetCle2Commande;
    QLabel *lblDetVal2Commande;
    QLabel *lblDetCle3Commande;
    QLabel *lblDetVal3Commande;
    QLabel *lblDetCle4Commande;
    QLabel *lblDetVal4Commande;
    QLabel *lblDetCle5Commande;
    QLabel *lblDetVal5Commande;
    QLabel *lblDetCle6Commande;
    QLabel *lblDetVal6Commande;
    QSpacerItem *spacerDetCommande;
    QPushButton *btnExporterCommande;
    QSpacerItem *spacerFinDetCommande;
    QVBoxLayout *layoutDroiteCommande;
    QFrame *frameFormCommande;
    QVBoxLayout *layoutFrameFormCommande;
    QLabel *lblTitreFormCommande;
    QVBoxLayout *layoutChampsCommande;
    QLabel *lblChamp0Commande;
    QDateEdit *dateCmdCommande;
    QLabel *lblChamp1Commande;
    QDateEdit *dateLivCommande;
    QLabel *lblChamp2Commande;
    QLineEdit *txtFournisseurCommande;
    QLabel *lblChamp3Commande;
    QLineEdit *txtQuantiteCommande;
    QLabel *lblChamp4Commande;
    QComboBox *cmbStatutFormCommande;
    QSpacerItem *spacerFormCommande;
    QHBoxLayout *layoutBoutonsFormCommande;
    QPushButton *btnAnnulerCommande;
    QPushButton *btnEnregistrerCommande;
    QSpacerItem *spacerDroiteCommande;
    QWidget *pageFournisseurs;
    QVBoxLayout *layout_pageFournisseurs;
    QLabel *lbl_pageFournisseurs;
    QWidget *pageEmployes;
    QVBoxLayout *layout_pageEmployes;
    QLabel *lbl_pageEmployes;
    QWidget *pageParametres;
    QVBoxLayout *layout_pageParametres;
    QLabel *lbl_pageParametres;
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
    QStatusBar *statusbar;

    void setupUi(QMainWindow *GPetManager)
    {
        if (GPetManager->objectName().isEmpty())
            GPetManager->setObjectName("GPetManager");
        GPetManager->resize(800, 600);
        GPetManager->setStyleSheet(QString::fromUtf8("QMessageBox { background-color: white; }\n"
"QMessageBox QLabel { color: #1E3A3A; }\n"
"QMessageBox QPushButton { background-color: #2A8C86; color: white; border: none; border-radius: 6px;\n"
"                          padding: 6px 18px; min-width: 70px; font-weight: bold; }\n"
"QMessageBox QPushButton:hover { background-color: #1F6F6B; }"));
        centralwidget = new QWidget(GPetManager);
        centralwidget->setObjectName("centralwidget");
        zoneDroite = new QWidget(centralwidget);
        zoneDroite->setObjectName("zoneDroite");
        zoneDroite->setGeometry(QRect(215, 0, 1151, 768));
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
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icones/bell_teal.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnNotif->setIcon(icon);
        btnNotif->setIconSize(QSize(24, 24));

        layoutHeader->addWidget(btnNotif);

        espaceAvatar = new QSpacerItem(10, 20, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);

        layoutHeader->addItem(espaceAvatar);

        lblAvatar = new QLabel(frameHeader);
        lblAvatar->setObjectName("lblAvatar");
        lblAvatar->setMinimumSize(QSize(44, 44));
        lblAvatar->setMaximumSize(QSize(44, 44));
        lblAvatar->setAlignment(Qt::AlignmentFlag::AlignCenter);

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
        lbl_pageAccueil->setTextFormat(Qt::TextFormat::RichText);
        lbl_pageAccueil->setAlignment(Qt::AlignmentFlag::AlignCenter);
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
        lbl_pageAnimaux->setTextFormat(Qt::TextFormat::RichText);
        lbl_pageAnimaux->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lbl_pageAnimaux->setWordWrap(true);

        layout_pageAnimaux->addWidget(lbl_pageAnimaux);

        SWModules->addWidget(pageAnimaux);
        pageRendezVous = new QWidget();
        pageRendezVous->setObjectName("pageRendezVous");
        layout_pageRendezVous = new QVBoxLayout(pageRendezVous);
        layout_pageRendezVous->setSpacing(0);
        layout_pageRendezVous->setObjectName("layout_pageRendezVous");
        layout_pageRendezVous->setContentsMargins(30, 30, 30, 30);
        lbl_pageRendezVous = new QLabel(pageRendezVous);
        lbl_pageRendezVous->setObjectName("lbl_pageRendezVous");
        lbl_pageRendezVous->setTextFormat(Qt::TextFormat::RichText);
        lbl_pageRendezVous->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lbl_pageRendezVous->setWordWrap(true);

        layout_pageRendezVous->addWidget(lbl_pageRendezVous);

        SWModules->addWidget(pageRendezVous);
        pageStock = new QWidget();
        pageStock->setObjectName("pageStock");
        layout_pageStock = new QVBoxLayout(pageStock);
        layout_pageStock->setSpacing(0);
        layout_pageStock->setObjectName("layout_pageStock");
        layout_pageStock->setContentsMargins(30, 30, 30, 30);
        lbl_pageStock = new QLabel(pageStock);
        lbl_pageStock->setObjectName("lbl_pageStock");
        lbl_pageStock->setTextFormat(Qt::TextFormat::RichText);
        lbl_pageStock->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lbl_pageStock->setWordWrap(true);

        layout_pageStock->addWidget(lbl_pageStock);

        SWModules->addWidget(pageStock);
        pageCommandes = new QWidget();
        pageCommandes->setObjectName("pageCommandes");
        pageCommandes->setStyleSheet(QString::fromUtf8("QWidget#pageCommandes { background-color: #F4F8F8; }\n"
"QLabel { background: transparent; color: #1E3A3A; }\n"
"\n"
"/* champs de saisie */\n"
"QLineEdit, QComboBox, QDateEdit {\n"
"    border: 1px solid #D5E0E0; border-radius: 6px; padding: 0 10px;\n"
"    background: white; color: #1E3A3A; font-size: 13px; min-height: 30px; }\n"
"QLineEdit:focus, QComboBox:focus, QDateEdit:focus { border: 1px solid #2A8C86; }\n"
"QComboBox::drop-down, QDateEdit::drop-down {\n"
"    subcontrol-origin: padding; subcontrol-position: center right; width: 30px; border: none; }\n"
"QComboBox::down-arrow, QDateEdit::down-arrow {\n"
"    image: url(:/icones/fleche_bas.png); width: 12px; height: 12px; }\n"
"QComboBox QAbstractItemView {\n"
"    background: white; color: #1E3A3A; border: 1px solid #D5E0E0; outline: 0; padding: 4px;\n"
"    selection-background-color: #E6F2F1; selection-color: #1E3A3A; }\n"
"QComboBox QAbstractItemView::item { min-height: 30px; padding-left: 8px; color: #1E3A3A; background: white; }\n"
"QComboBox QAbs"
                        "tractItemView::item:hover, QComboBox QAbstractItemView::item:selected {\n"
"    background: #E6F2F1; color: #1E3A3A; }\n"
"QCalendarWidget QWidget { background-color: white; color: #1E3A3A; }\n"
"QCalendarWidget QToolButton { color: #17363A; background-color: white; font-weight: bold; }\n"
"QCalendarWidget QAbstractItemView:enabled {\n"
"    background-color: white; color: #1E3A3A; selection-background-color: #2A8C86; selection-color: white; }\n"
"QCalendarWidget QAbstractItemView:disabled { color: #9BB0B0; }\n"
"\n"
"/* cadres : formulaire et d\303\251tails */\n"
"QFrame#frameFormCommande, QFrame#frameDetailsCommande {\n"
"    background: white; border: 1px solid #E0E8E8; border-radius: 10px; }\n"
"QLabel#lblTitreFormCommande, QLabel#lblTitreDetailsCommande {\n"
"    background: #2A8C86; color: white; font-weight: bold; font-size: 14px; padding: 11px 18px;\n"
"    border: none; border-top-left-radius: 10px; border-top-right-radius: 10px; }\n"
"QLabel#lblChamp0Commande, QLabel#lblChamp1Commande, QLabel#lblCham"
                        "p2Commande,\n"
"QLabel#lblChamp3Commande, QLabel#lblChamp4Commande {\n"
"    color: #6B8080; font-size: 12px; border: none; background: transparent; }\n"
"QLabel#lblDetCle0Commande, QLabel#lblDetCle1Commande, QLabel#lblDetCle2Commande, QLabel#lblDetCle3Commande,\n"
"QLabel#lblDetCle4Commande, QLabel#lblDetCle5Commande, QLabel#lblDetCle6Commande {\n"
"    color: #6B8080; font-size: 13px; border: none; background: transparent; }\n"
"QLabel#lblDetVal0Commande, QLabel#lblDetVal1Commande, QLabel#lblDetVal2Commande,\n"
"QLabel#lblDetVal4Commande, QLabel#lblDetVal5Commande, QLabel#lblDetVal6Commande {\n"
"    color: #17363A; font-size: 13px; font-weight: bold; border: none; background: transparent; }\n"
"\n"
"/* boutons */\n"
"QPushButton#btnAjouterCommande, QPushButton#btnEnregistrerCommande, QPushButton#btnExporterCommande {\n"
"    background: #2A8C86; color: white; border: none; border-radius: 8px;\n"
"    font-weight: bold; font-size: 13px; padding: 0 18px; }\n"
"QPushButton#btnAjouterCommande:hover, QPushButton"
                        "#btnEnregistrerCommande:hover,\n"
"QPushButton#btnExporterCommande:hover { background: #1F6F6B; }\n"
"QPushButton#btnAnnulerCommande {\n"
"    background: white; color: #2A8C86; border: 1px solid #2A8C86; border-radius: 8px;\n"
"    font-weight: bold; font-size: 13px; }\n"
"QPushButton#btnAnnulerCommande:hover { background: #E6F2F1; }\n"
"\n"
"/* tableau */\n"
"QTableView#tableCommande {\n"
"    background: white; color: #1E3A3A; font-size: 13px; border: 1px solid #E0E8E8;\n"
"    border-radius: 10px; selection-background-color: #E6F2F1; selection-color: #1E3A3A; }\n"
"QTableView#tableCommande QHeaderView::section {\n"
"    background: white; color: #17363A; font-weight: bold; padding: 10px 8px;\n"
"    border: none; border-bottom: 1px solid #E0E8E8; }\n"
"QTableView#tableCommande QTableCornerButton::section { background: white; border: none; }\n"
"QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }\n"
"QScrollBar::handle:vertical { background: #C5D6D5; border-radius: 5px; min-height: 40px"
                        "; }\n"
"QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }\n"
"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"));
        layoutPageCommandes = new QVBoxLayout(pageCommandes);
        layoutPageCommandes->setSpacing(16);
        layoutPageCommandes->setObjectName("layoutPageCommandes");
        layoutPageCommandes->setContentsMargins(30, 24, 30, 24);
        layoutOutilsCommande = new QHBoxLayout();
        layoutOutilsCommande->setSpacing(12);
        layoutOutilsCommande->setObjectName("layoutOutilsCommande");
        txtRechercheCommande = new QLineEdit(pageCommandes);
        txtRechercheCommande->setObjectName("txtRechercheCommande");
        txtRechercheCommande->setMinimumSize(QSize(250, 32));
        txtRechercheCommande->setMaximumSize(QSize(300, 36));

        layoutOutilsCommande->addWidget(txtRechercheCommande);

        lblStatutCommande = new QLabel(pageCommandes);
        lblStatutCommande->setObjectName("lblStatutCommande");

        layoutOutilsCommande->addWidget(lblStatutCommande);

        cmbStatutCommande = new QComboBox(pageCommandes);
        cmbStatutCommande->setObjectName("cmbStatutCommande");
        cmbStatutCommande->setMinimumSize(QSize(150, 32));

        layoutOutilsCommande->addWidget(cmbStatutCommande);

        lblTriCommande = new QLabel(pageCommandes);
        lblTriCommande->setObjectName("lblTriCommande");

        layoutOutilsCommande->addWidget(lblTriCommande);

        cmbTriCommande = new QComboBox(pageCommandes);
        cmbTriCommande->setObjectName("cmbTriCommande");
        cmbTriCommande->setMinimumSize(QSize(230, 32));

        layoutOutilsCommande->addWidget(cmbTriCommande);

        spacerOutilsCommande = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        layoutOutilsCommande->addItem(spacerOutilsCommande);

        btnAjouterCommande = new QPushButton(pageCommandes);
        btnAjouterCommande->setObjectName("btnAjouterCommande");
        btnAjouterCommande->setMinimumSize(QSize(140, 38));
        btnAjouterCommande->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        layoutOutilsCommande->addWidget(btnAjouterCommande);


        layoutPageCommandes->addLayout(layoutOutilsCommande);

        layoutCorpsCommande = new QHBoxLayout();
        layoutCorpsCommande->setSpacing(20);
        layoutCorpsCommande->setObjectName("layoutCorpsCommande");
        layoutCorpsCommande->setContentsMargins(0, 0, 0, 0);
        layoutGaucheCommande = new QVBoxLayout();
        layoutGaucheCommande->setSpacing(16);
        layoutGaucheCommande->setObjectName("layoutGaucheCommande");
        layoutGaucheCommande->setContentsMargins(0, 0, 0, 0);
        tableCommande = new QTableView(pageCommandes);
        tableCommande->setObjectName("tableCommande");
        tableCommande->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
        tableCommande->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
        tableCommande->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        tableCommande->setShowGrid(false);
        tableCommande->horizontalHeader()->setStretchLastSection(true);
        tableCommande->verticalHeader()->setVisible(false);

        layoutGaucheCommande->addWidget(tableCommande);

        frameDetailsCommande = new QFrame(pageCommandes);
        frameDetailsCommande->setObjectName("frameDetailsCommande");
        frameDetailsCommande->setMinimumSize(QSize(0, 230));
        frameDetailsCommande->setMaximumSize(QSize(16777215, 230));
        layoutFrameDetailsCommande = new QVBoxLayout(frameDetailsCommande);
        layoutFrameDetailsCommande->setSpacing(0);
        layoutFrameDetailsCommande->setObjectName("layoutFrameDetailsCommande");
        layoutFrameDetailsCommande->setContentsMargins(0, 0, 0, 0);
        lblTitreDetailsCommande = new QLabel(frameDetailsCommande);
        lblTitreDetailsCommande->setObjectName("lblTitreDetailsCommande");
        lblTitreDetailsCommande->setMinimumSize(QSize(0, 46));
        lblTitreDetailsCommande->setMaximumSize(QSize(16777215, 46));

        layoutFrameDetailsCommande->addWidget(lblTitreDetailsCommande);

        layoutDetailsCommande = new QGridLayout();
        layoutDetailsCommande->setObjectName("layoutDetailsCommande");
        layoutDetailsCommande->setHorizontalSpacing(14);
        layoutDetailsCommande->setVerticalSpacing(10);
        layoutDetailsCommande->setContentsMargins(20, 14, 20, 14);
        lblDetCle0Commande = new QLabel(frameDetailsCommande);
        lblDetCle0Commande->setObjectName("lblDetCle0Commande");

        layoutDetailsCommande->addWidget(lblDetCle0Commande, 0, 0, 1, 1);

        lblDetVal0Commande = new QLabel(frameDetailsCommande);
        lblDetVal0Commande->setObjectName("lblDetVal0Commande");

        layoutDetailsCommande->addWidget(lblDetVal0Commande, 0, 1, 1, 1);

        lblDetCle1Commande = new QLabel(frameDetailsCommande);
        lblDetCle1Commande->setObjectName("lblDetCle1Commande");

        layoutDetailsCommande->addWidget(lblDetCle1Commande, 1, 0, 1, 1);

        lblDetVal1Commande = new QLabel(frameDetailsCommande);
        lblDetVal1Commande->setObjectName("lblDetVal1Commande");

        layoutDetailsCommande->addWidget(lblDetVal1Commande, 1, 1, 1, 1);

        lblDetCle2Commande = new QLabel(frameDetailsCommande);
        lblDetCle2Commande->setObjectName("lblDetCle2Commande");

        layoutDetailsCommande->addWidget(lblDetCle2Commande, 2, 0, 1, 1);

        lblDetVal2Commande = new QLabel(frameDetailsCommande);
        lblDetVal2Commande->setObjectName("lblDetVal2Commande");

        layoutDetailsCommande->addWidget(lblDetVal2Commande, 2, 1, 1, 1);

        lblDetCle3Commande = new QLabel(frameDetailsCommande);
        lblDetCle3Commande->setObjectName("lblDetCle3Commande");

        layoutDetailsCommande->addWidget(lblDetCle3Commande, 3, 0, 1, 1);

        lblDetVal3Commande = new QLabel(frameDetailsCommande);
        lblDetVal3Commande->setObjectName("lblDetVal3Commande");
        lblDetVal3Commande->setMaximumSize(QSize(130, 26));

        layoutDetailsCommande->addWidget(lblDetVal3Commande, 3, 1, 1, 1);

        lblDetCle4Commande = new QLabel(frameDetailsCommande);
        lblDetCle4Commande->setObjectName("lblDetCle4Commande");

        layoutDetailsCommande->addWidget(lblDetCle4Commande, 0, 3, 1, 1);

        lblDetVal4Commande = new QLabel(frameDetailsCommande);
        lblDetVal4Commande->setObjectName("lblDetVal4Commande");

        layoutDetailsCommande->addWidget(lblDetVal4Commande, 0, 4, 1, 1);

        lblDetCle5Commande = new QLabel(frameDetailsCommande);
        lblDetCle5Commande->setObjectName("lblDetCle5Commande");

        layoutDetailsCommande->addWidget(lblDetCle5Commande, 1, 3, 1, 1);

        lblDetVal5Commande = new QLabel(frameDetailsCommande);
        lblDetVal5Commande->setObjectName("lblDetVal5Commande");

        layoutDetailsCommande->addWidget(lblDetVal5Commande, 1, 4, 1, 1);

        lblDetCle6Commande = new QLabel(frameDetailsCommande);
        lblDetCle6Commande->setObjectName("lblDetCle6Commande");

        layoutDetailsCommande->addWidget(lblDetCle6Commande, 2, 3, 1, 1);

        lblDetVal6Commande = new QLabel(frameDetailsCommande);
        lblDetVal6Commande->setObjectName("lblDetVal6Commande");

        layoutDetailsCommande->addWidget(lblDetVal6Commande, 2, 4, 1, 1);

        spacerDetCommande = new QSpacerItem(120, 20, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);

        layoutDetailsCommande->addItem(spacerDetCommande, 0, 2, 1, 1);

        btnExporterCommande = new QPushButton(frameDetailsCommande);
        btnExporterCommande->setObjectName("btnExporterCommande");
        btnExporterCommande->setMaximumSize(QSize(150, 36));
        btnExporterCommande->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        layoutDetailsCommande->addWidget(btnExporterCommande, 3, 3, 1, 2);

        spacerFinDetCommande = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        layoutDetailsCommande->addItem(spacerFinDetCommande, 0, 5, 1, 1);


        layoutFrameDetailsCommande->addLayout(layoutDetailsCommande);


        layoutGaucheCommande->addWidget(frameDetailsCommande);

        layoutGaucheCommande->setStretch(0, 1);

        layoutCorpsCommande->addLayout(layoutGaucheCommande);

        layoutDroiteCommande = new QVBoxLayout();
        layoutDroiteCommande->setSpacing(0);
        layoutDroiteCommande->setObjectName("layoutDroiteCommande");
        layoutDroiteCommande->setContentsMargins(0, 0, 0, 0);
        frameFormCommande = new QFrame(pageCommandes);
        frameFormCommande->setObjectName("frameFormCommande");
        frameFormCommande->setMinimumSize(QSize(340, 0));
        frameFormCommande->setMaximumSize(QSize(340, 16777215));
        layoutFrameFormCommande = new QVBoxLayout(frameFormCommande);
        layoutFrameFormCommande->setSpacing(0);
        layoutFrameFormCommande->setObjectName("layoutFrameFormCommande");
        layoutFrameFormCommande->setContentsMargins(0, 0, 0, 0);
        lblTitreFormCommande = new QLabel(frameFormCommande);
        lblTitreFormCommande->setObjectName("lblTitreFormCommande");
        lblTitreFormCommande->setMinimumSize(QSize(0, 46));
        lblTitreFormCommande->setMaximumSize(QSize(16777215, 46));

        layoutFrameFormCommande->addWidget(lblTitreFormCommande);

        layoutChampsCommande = new QVBoxLayout();
        layoutChampsCommande->setSpacing(6);
        layoutChampsCommande->setObjectName("layoutChampsCommande");
        layoutChampsCommande->setContentsMargins(20, 14, 20, 18);
        lblChamp0Commande = new QLabel(frameFormCommande);
        lblChamp0Commande->setObjectName("lblChamp0Commande");

        layoutChampsCommande->addWidget(lblChamp0Commande);

        dateCmdCommande = new QDateEdit(frameFormCommande);
        dateCmdCommande->setObjectName("dateCmdCommande");
        dateCmdCommande->setCalendarPopup(true);

        layoutChampsCommande->addWidget(dateCmdCommande);

        lblChamp1Commande = new QLabel(frameFormCommande);
        lblChamp1Commande->setObjectName("lblChamp1Commande");

        layoutChampsCommande->addWidget(lblChamp1Commande);

        dateLivCommande = new QDateEdit(frameFormCommande);
        dateLivCommande->setObjectName("dateLivCommande");
        dateLivCommande->setCalendarPopup(true);

        layoutChampsCommande->addWidget(dateLivCommande);

        lblChamp2Commande = new QLabel(frameFormCommande);
        lblChamp2Commande->setObjectName("lblChamp2Commande");

        layoutChampsCommande->addWidget(lblChamp2Commande);

        txtFournisseurCommande = new QLineEdit(frameFormCommande);
        txtFournisseurCommande->setObjectName("txtFournisseurCommande");

        layoutChampsCommande->addWidget(txtFournisseurCommande);

        lblChamp3Commande = new QLabel(frameFormCommande);
        lblChamp3Commande->setObjectName("lblChamp3Commande");

        layoutChampsCommande->addWidget(lblChamp3Commande);

        txtQuantiteCommande = new QLineEdit(frameFormCommande);
        txtQuantiteCommande->setObjectName("txtQuantiteCommande");

        layoutChampsCommande->addWidget(txtQuantiteCommande);

        lblChamp4Commande = new QLabel(frameFormCommande);
        lblChamp4Commande->setObjectName("lblChamp4Commande");

        layoutChampsCommande->addWidget(lblChamp4Commande);

        cmbStatutFormCommande = new QComboBox(frameFormCommande);
        cmbStatutFormCommande->setObjectName("cmbStatutFormCommande");

        layoutChampsCommande->addWidget(cmbStatutFormCommande);

        spacerFormCommande = new QSpacerItem(20, 10, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        layoutChampsCommande->addItem(spacerFormCommande);

        layoutBoutonsFormCommande = new QHBoxLayout();
        layoutBoutonsFormCommande->setSpacing(10);
        layoutBoutonsFormCommande->setObjectName("layoutBoutonsFormCommande");
        btnAnnulerCommande = new QPushButton(frameFormCommande);
        btnAnnulerCommande->setObjectName("btnAnnulerCommande");
        btnAnnulerCommande->setMinimumSize(QSize(0, 38));
        btnAnnulerCommande->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        layoutBoutonsFormCommande->addWidget(btnAnnulerCommande);

        btnEnregistrerCommande = new QPushButton(frameFormCommande);
        btnEnregistrerCommande->setObjectName("btnEnregistrerCommande");
        btnEnregistrerCommande->setMinimumSize(QSize(0, 38));
        btnEnregistrerCommande->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        layoutBoutonsFormCommande->addWidget(btnEnregistrerCommande);


        layoutChampsCommande->addLayout(layoutBoutonsFormCommande);


        layoutFrameFormCommande->addLayout(layoutChampsCommande);


        layoutDroiteCommande->addWidget(frameFormCommande);

        spacerDroiteCommande = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        layoutDroiteCommande->addItem(spacerDroiteCommande);


        layoutCorpsCommande->addLayout(layoutDroiteCommande);

        layoutCorpsCommande->setStretch(0, 1);

        layoutPageCommandes->addLayout(layoutCorpsCommande);

        SWModules->addWidget(pageCommandes);
        pageFournisseurs = new QWidget();
        pageFournisseurs->setObjectName("pageFournisseurs");
        layout_pageFournisseurs = new QVBoxLayout(pageFournisseurs);
        layout_pageFournisseurs->setSpacing(0);
        layout_pageFournisseurs->setObjectName("layout_pageFournisseurs");
        layout_pageFournisseurs->setContentsMargins(30, 30, 30, 30);
        lbl_pageFournisseurs = new QLabel(pageFournisseurs);
        lbl_pageFournisseurs->setObjectName("lbl_pageFournisseurs");
        lbl_pageFournisseurs->setTextFormat(Qt::TextFormat::RichText);
        lbl_pageFournisseurs->setAlignment(Qt::AlignmentFlag::AlignCenter);
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
        lbl_pageEmployes->setTextFormat(Qt::TextFormat::RichText);
        lbl_pageEmployes->setAlignment(Qt::AlignmentFlag::AlignCenter);
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
        lbl_pageParametres->setTextFormat(Qt::TextFormat::RichText);
        lbl_pageParametres->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lbl_pageParametres->setWordWrap(true);

        layout_pageParametres->addWidget(lbl_pageParametres);

        SWModules->addWidget(pageParametres);

        layoutDroite->addWidget(SWModules);

        frameSidebar = new QFrame(centralwidget);
        frameSidebar->setObjectName("frameSidebar");
        frameSidebar->setGeometry(QRect(0, 0, 215, 768));
        frameSidebar->setMinimumSize(QSize(215, 0));
        frameSidebar->setMaximumSize(QSize(215, 16777215));
        lblLogo = new QLabel(frameSidebar);
        lblLogo->setObjectName("lblLogo");
        lblLogo->setGeometry(QRect(42, 10, 130, 130));
        lblLogo->setPixmap(QPixmap(QString::fromUtf8(":/icones/logo.png")));
        lblLogo->setScaledContents(true);
        btnAccueil = new QPushButton(frameSidebar);
        btnAccueil->setObjectName("btnAccueil");
        btnAccueil->setGeometry(QRect(10, 156, 195, 42));
        btnAccueil->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icones/home_dark.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon1.addFile(QString::fromUtf8(":/icones/home_white.png"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        btnAccueil->setIcon(icon1);
        btnAccueil->setIconSize(QSize(20, 20));
        btnAccueil->setCheckable(true);
        btnAccueil->setAutoExclusive(true);
        btnAnimaux = new QPushButton(frameSidebar);
        btnAnimaux->setObjectName("btnAnimaux");
        btnAnimaux->setGeometry(QRect(10, 202, 195, 42));
        btnAnimaux->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icones/paw_dark.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon2.addFile(QString::fromUtf8(":/icones/paw_white.png"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        btnAnimaux->setIcon(icon2);
        btnAnimaux->setIconSize(QSize(20, 20));
        btnAnimaux->setCheckable(true);
        btnAnimaux->setAutoExclusive(true);
        btnRendezVous = new QPushButton(frameSidebar);
        btnRendezVous->setObjectName("btnRendezVous");
        btnRendezVous->setGeometry(QRect(10, 248, 195, 42));
        btnRendezVous->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icones/calendar_dark.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon3.addFile(QString::fromUtf8(":/icones/calendar_white.png"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        btnRendezVous->setIcon(icon3);
        btnRendezVous->setIconSize(QSize(20, 20));
        btnRendezVous->setCheckable(true);
        btnRendezVous->setAutoExclusive(true);
        btnStock = new QPushButton(frameSidebar);
        btnStock->setObjectName("btnStock");
        btnStock->setGeometry(QRect(10, 294, 195, 42));
        btnStock->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icones/box_dark.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon4.addFile(QString::fromUtf8(":/icones/box_white.png"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        btnStock->setIcon(icon4);
        btnStock->setIconSize(QSize(20, 20));
        btnStock->setCheckable(true);
        btnStock->setChecked(true);
        btnStock->setAutoExclusive(true);
        btnCommandes = new QPushButton(frameSidebar);
        btnCommandes->setObjectName("btnCommandes");
        btnCommandes->setGeometry(QRect(10, 340, 195, 42));
        btnCommandes->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icones/cart_dark.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon5.addFile(QString::fromUtf8(":/icones/cart_white.png"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        btnCommandes->setIcon(icon5);
        btnCommandes->setIconSize(QSize(20, 20));
        btnCommandes->setCheckable(true);
        btnCommandes->setAutoExclusive(true);
        btnFournisseurs = new QPushButton(frameSidebar);
        btnFournisseurs->setObjectName("btnFournisseurs");
        btnFournisseurs->setGeometry(QRect(10, 386, 195, 42));
        btnFournisseurs->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/icones/truck_dark.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon6.addFile(QString::fromUtf8(":/icones/truck_white.png"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        btnFournisseurs->setIcon(icon6);
        btnFournisseurs->setIconSize(QSize(20, 20));
        btnFournisseurs->setCheckable(true);
        btnFournisseurs->setAutoExclusive(true);
        btnEmployes = new QPushButton(frameSidebar);
        btnEmployes->setObjectName("btnEmployes");
        btnEmployes->setGeometry(QRect(10, 432, 195, 42));
        btnEmployes->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/icones/users_dark.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon7.addFile(QString::fromUtf8(":/icones/users_white.png"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        btnEmployes->setIcon(icon7);
        btnEmployes->setIconSize(QSize(20, 20));
        btnEmployes->setCheckable(true);
        btnEmployes->setAutoExclusive(true);
        btnParametres = new QPushButton(frameSidebar);
        btnParametres->setObjectName("btnParametres");
        btnParametres->setGeometry(QRect(10, 478, 195, 42));
        btnParametres->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/icones/gear_dark.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        icon8.addFile(QString::fromUtf8(":/icones/gear_white.png"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        btnParametres->setIcon(icon8);
        btnParametres->setIconSize(QSize(20, 20));
        btnParametres->setCheckable(true);
        btnParametres->setAutoExclusive(true);
        GPetManager->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(GPetManager);
        statusbar->setObjectName("statusbar");
        GPetManager->setStatusBar(statusbar);

        retranslateUi(GPetManager);

        SWModules->setCurrentIndex(4);


        QMetaObject::connectSlotsByName(GPetManager);
    } // setupUi

    void retranslateUi(QMainWindow *GPetManager)
    {
        GPetManager->setWindowTitle(QCoreApplication::translate("GPetManager", "GPetManager", nullptr));
        frameHeader->setStyleSheet(QCoreApplication::translate("GPetManager", ".QFrame { background-color:#F4F8F8; border:none; border-bottom:1px solid #E0E8E8; }", nullptr));
        lblIconeTitre->setText(QString());
        lblTitre->setStyleSheet(QCoreApplication::translate("GPetManager", "color:#17363A; font-size:26px; font-weight:bold;", nullptr));
        lblTitre->setText(QCoreApplication::translate("GPetManager", "Commandes", nullptr));
#if QT_CONFIG(tooltip)
        btnNotif->setToolTip(QCoreApplication::translate("GPetManager", "Notifications du stock", nullptr));
#endif // QT_CONFIG(tooltip)
        btnNotif->setStyleSheet(QCoreApplication::translate("GPetManager", "QPushButton{border:none;background:transparent;font-size:22px;} QPushButton:hover{background:#E6F2F1;border-radius:8px;}", nullptr));
        btnNotif->setText(QString());
        lblAvatar->setStyleSheet(QCoreApplication::translate("GPetManager", "border:2px solid #2A8C86; border-radius:22px; background:white;", nullptr));
        lblAvatar->setText(QString());
        lbl_pageAccueil->setText(QCoreApplication::translate("GPetManager", "<p><img src=\":/icones/home_teal.png\" width=\"80\" height=\"80\"></p><p style=\"font-size:24px; font-weight:bold; color:#17363A\">Accueil</p><p style=\"font-size:13px; color:#6B8080\">Cette page sera d\303\251velopp\303\251e dans le module \302\253 Accueil \302\273.</p>", nullptr));
        lbl_pageAnimaux->setText(QCoreApplication::translate("GPetManager", "<p><img src=\":/icones/paw_teal.png\" width=\"80\" height=\"80\"></p><p style=\"font-size:24px; font-weight:bold; color:#17363A\">Animaux</p><p style=\"font-size:13px; color:#6B8080\">Cette page sera d\303\251velopp\303\251e dans le module \302\253 Animaux \302\273.</p>", nullptr));
        lbl_pageRendezVous->setText(QCoreApplication::translate("GPetManager", "<p><img src=\":/icones/calendar_teal.png\" width=\"80\" height=\"80\"></p><p style=\"font-size:24px; font-weight:bold; color:#17363A\">Rendez-vous</p><p style=\"font-size:13px; color:#6B8080\">Cette page sera d\303\251velopp\303\251e dans le module \302\253 Rendez-vous \302\273.</p>", nullptr));
        lbl_pageStock->setText(QCoreApplication::translate("GPetManager", "<p><img src=\":/icones/box_teal.png\" width=\"80\" height=\"80\"></p><p style=\"font-size:24px; font-weight:bold; color:#17363A\">Stock</p><p style=\"font-size:13px; color:#6B8080\">Cette page sera d\303\251velopp\303\251e dans le module \302\253 Stock \302\273.</p>", nullptr));
        txtRechercheCommande->setPlaceholderText(QCoreApplication::translate("GPetManager", "Rechercher par ID ...", nullptr));
        lblStatutCommande->setText(QCoreApplication::translate("GPetManager", "Statut :", nullptr));
        lblTriCommande->setText(QCoreApplication::translate("GPetManager", "Trier par :", nullptr));
        btnAjouterCommande->setText(QCoreApplication::translate("GPetManager", "+ Ajouter", nullptr));
        lblTitreDetailsCommande->setText(QCoreApplication::translate("GPetManager", "D\303\251tails d'une commande", nullptr));
        lblDetCle0Commande->setText(QCoreApplication::translate("GPetManager", "ID", nullptr));
        lblDetVal0Commande->setText(QString());
        lblDetCle1Commande->setText(QCoreApplication::translate("GPetManager", "Date de commande", nullptr));
        lblDetVal1Commande->setText(QString());
        lblDetCle2Commande->setText(QCoreApplication::translate("GPetManager", "Date de livraison", nullptr));
        lblDetVal2Commande->setText(QString());
        lblDetCle3Commande->setText(QCoreApplication::translate("GPetManager", "Statut", nullptr));
        lblDetVal3Commande->setText(QString());
        lblDetCle4Commande->setText(QCoreApplication::translate("GPetManager", "Fournisseur", nullptr));
        lblDetVal4Commande->setText(QString());
        lblDetCle5Commande->setText(QCoreApplication::translate("GPetManager", "Quantit\303\251", nullptr));
        lblDetVal5Commande->setText(QString());
        lblDetCle6Commande->setText(QCoreApplication::translate("GPetManager", "Montant total", nullptr));
        lblDetVal6Commande->setText(QString());
        btnExporterCommande->setText(QCoreApplication::translate("GPetManager", "Exporter", nullptr));
        lblTitreFormCommande->setText(QCoreApplication::translate("GPetManager", "Ajouter une commande", nullptr));
        lblChamp0Commande->setText(QCoreApplication::translate("GPetManager", "Date de commande", nullptr));
        lblChamp1Commande->setText(QCoreApplication::translate("GPetManager", "Date de livraison", nullptr));
        lblChamp2Commande->setText(QCoreApplication::translate("GPetManager", "Nom du fournisseur", nullptr));
        txtFournisseurCommande->setPlaceholderText(QCoreApplication::translate("GPetManager", "Entrer le nom du fournisseur", nullptr));
        lblChamp3Commande->setText(QCoreApplication::translate("GPetManager", "Quantit\303\251", nullptr));
        txtQuantiteCommande->setPlaceholderText(QCoreApplication::translate("GPetManager", "Entrer la quantit\303\251", nullptr));
        lblChamp4Commande->setText(QCoreApplication::translate("GPetManager", "Statut", nullptr));
        btnAnnulerCommande->setText(QCoreApplication::translate("GPetManager", "Annuler", nullptr));
        btnEnregistrerCommande->setText(QCoreApplication::translate("GPetManager", "Enregistrer", nullptr));
        lbl_pageFournisseurs->setText(QCoreApplication::translate("GPetManager", "<p><img src=\":/icones/truck_teal.png\" width=\"80\" height=\"80\"></p><p style=\"font-size:24px; font-weight:bold; color:#17363A\">Fournisseurs</p><p style=\"font-size:13px; color:#6B8080\">Cette page sera d\303\251velopp\303\251e dans le module \302\253 Fournisseurs \302\273.</p>", nullptr));
        lbl_pageEmployes->setText(QCoreApplication::translate("GPetManager", "<p><img src=\":/icones/users_teal.png\" width=\"80\" height=\"80\"></p><p style=\"font-size:24px; font-weight:bold; color:#17363A\">Employ\303\251s</p><p style=\"font-size:13px; color:#6B8080\">Cette page sera d\303\251velopp\303\251e dans le module \302\253 Employ\303\251s \302\273.</p>", nullptr));
        lbl_pageParametres->setText(QCoreApplication::translate("GPetManager", "<p><img src=\":/icones/gear_teal.png\" width=\"80\" height=\"80\"></p><p style=\"font-size:24px; font-weight:bold; color:#17363A\">Param\303\250tres</p><p style=\"font-size:13px; color:#6B8080\">Cette page sera d\303\251velopp\303\251e dans le module \302\253 Param\303\250tres \302\273.</p>", nullptr));
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
    } // retranslateUi

};

namespace Ui {
    class GPetManager: public Ui_GPetManager {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GPETMANAGER_H
