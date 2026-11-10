#pragma once
#include <QMainWindow>
#include <QVector>
#include <QHash>
#include <QString>
#include <QDate>
#include <QPixmap>

#include "animal.h"

namespace Ui { class MainWindow; }

class QTableWidget;
class QLineEdit;
class QComboBox;
class QLabel;
class QFrame;
class QDateEdit;
class QDoubleSpinBox;
class QTextEdit;
class QPushButton;
class QWidget;
class QScrollArea;
class QStackedWidget;

// une "analyse intelligente" : niveau = info | warn | ok
struct Insight {
    QString level;
    QString html;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // --- CRUD / liste
    void applyFilter();
    void saveAnimal();
    void clearForm();
    void choosePhoto();
    void trierParNom();
    void resetFilters();
    // --- statistiques
    void updateDashboard();
    void filtrerParEspece(const QString &espece);
    void filtrerParStatut(int index);          // 0 = tous, 1 = bonne santé, 2 = surveillance
    void exporterRapportPdf();
    void traiterQuestionIA();
    // --- généalogie
    void ouvrirArbre();
    // --- alertes
    void ouvrirAlertes();
    void majAlertes();

private:
    Ui::MainWindow *ui;

    // --- construction de l'interface
    QWidget *buildSidebar();
    QWidget *buildCenter();
    QWidget *buildListPage();
    QWidget *buildStatsPage();
    QWidget *buildStatsPanel();
    QWidget *buildDetail();
    QWidget *buildForm();
    QWidget *buildAIAskWidget();
    ClickableFrame *buildKpiCard(const QString &title, const QString &icon, const QString &color,
                                 QLabel *&valLbl, QLabel *&subLbl);
    void applyStyle();
    void showPage(int index);                  // 0 = liste, 1 = statistiques

    // --- données / logique
    void chargerAnimaux();                     // lit la base Oracle -> m_animals
    void refreshTable();
    void showDetail(int index);
    void remplirParents(const QString &exclure = QString());   // combos Père / Mère du formulaire
    void clearDetail();
    void editAnimal(const QString &id);
    void deleteAnimal(const QString &id);
    int  indexOf(const QString &id) const;
    int  currentIndex() const;
    int  requireSelection();
    QVector<Insight> buildInsights() const;

    static bool    estEnBonneSante(const Animal &a);
    static QString ageText(const QDate &d);
    static QPixmap avatar(const Animal &a, int size);
    static QLabel *badge(const QString &text, const QString &bg, const QString &fg);
    static QWidget *wrap(QWidget *w, Qt::Alignment al = Qt::AlignLeft | Qt::AlignVCenter);

    // --- données
    QVector<Animal> m_animals;
    QString m_editId;
    QString m_photoPath;

    // --- navigation
    QStackedWidget *m_pages = nullptr;
    QPushButton *m_tabList = nullptr;
    QPushButton *m_tabStats = nullptr;
    QPushButton *m_btnAlertes = nullptr;
    QScrollArea *m_listScroll = nullptr;

    // --- liste
    QTableWidget *m_table = nullptr;
    QLineEdit *m_search = nullptr;
    QComboBox *m_filterEspece = nullptr;
    QComboBox *m_filterStatut = nullptr;
    QPushButton *m_btnReset = nullptr;
    QLabel *m_count = nullptr;

    // --- statistiques
    QLabel *m_kpiTotal = nullptr;   QLabel *m_kpiTotalSub = nullptr;
    QLabel *m_kpiAge = nullptr;     QLabel *m_kpiAgeSub = nullptr;
    QLabel *m_kpiAlert = nullptr;   QLabel *m_kpiAlertSub = nullptr;
    QLabel *m_kpiSex = nullptr;     QLabel *m_kpiSexSub = nullptr;
    QLabel *m_chartTitle = nullptr;
    QStackedWidget *m_chartStack = nullptr;
    DonutChart *m_donut = nullptr;
    LineChart *m_line = nullptr;
    GaugeWidget *m_gauge = nullptr;
    QLabel *m_gaugeCaption = nullptr;
    QLabel *m_insights = nullptr;
    QLineEdit *m_aiInput = nullptr;
    QLabel *m_aiResponseLbl = nullptr;

    // --- panneau détail
    QLabel *m_dPhoto = nullptr;
    QLabel *m_dNom = nullptr;
    QLabel *m_dStatut = nullptr;
    QLabel *m_dId = nullptr;
    QLabel *m_dEspece = nullptr;
    QLabel *m_dRace = nullptr;
    QLabel *m_dSexe = nullptr;
    QLabel *m_dNaiss = nullptr;
    QLabel *m_dPoids = nullptr;
    QLabel *m_dAge = nullptr;
    QLabel *m_dDesc = nullptr;
    InbreedingBar *m_dConsang = nullptr;     // jauge de consanguinité (fiche animal)
    QLabel *m_dConsangTxt = nullptr;

    // --- formulaire
    QFrame *m_form = nullptr;
    QLabel *m_formTitle = nullptr;
    QLabel *m_formPhoto = nullptr;
    QLineEdit *m_fNom = nullptr;
    QLineEdit *m_fRace = nullptr;
    QComboBox *m_fEspece = nullptr;
    QComboBox *m_fSexe = nullptr;
    QComboBox *m_fStatut = nullptr;
    QDateEdit *m_fDate = nullptr;
    QDoubleSpinBox *m_fPoids = nullptr;
    QTextEdit *m_fDesc = nullptr;
    QComboBox *m_fPere = nullptr;
    QComboBox *m_fMere = nullptr;
    QPushButton *m_btnSave = nullptr;
};
