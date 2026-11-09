#pragma once
#include <QWidget>
#include <QStringList>
#include <QVector>
#include <QHash>
#include <QString>
#include <QDate>
#include <QPixmap>

class QShowEvent;

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
class QProgressBar;
class QScrollArea;

struct Animal {
    QString id, nom, espece, race, sexe, statut, description;
    QString idProprio, nomProprio, idVet, nomVet, photo;
    QDate naissance;
    double poids = 0;
};

class AnimauxPage : public QWidget
{
    Q_OBJECT
public:
    // avecMenu = false : la page s'intègre dans la fenêtre principale (menu latéral commun)
    // avecMenu = true  : reproduit à l'identique la barre latérale d'origine
    explicit AnimauxPage(QWidget *parent = nullptr, bool avecMenu = false);
    ~AnimauxPage() override;

    // Lecture seule : utilisé par la page Accueil
    int nombreAnimaux() const { return m_animals.size(); }
    int nombreParEspece(const QString &espece) const;
    QStringList animauxSousSurveillance() const;   // "Nom (Espèce) - Statut"

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void applyFilter();
    void saveAnimal();
    void clearForm();
    void choosePhoto();
    void updateDashboard();
    void trierParNom();
    void verifierSanteAnimaux();
    void executerAnalyseIntelligente();
    void traiterQuestionIA();

private:
    bool m_avecMenu = false;
    bool m_premiereAffichage = true;

    QWidget *buildSidebar();
    QWidget *buildCenter();
    QWidget *buildDetail();
    QWidget *buildForm();
    QWidget *buildAIAskWidget();
    QWidget *buildSmartDashboard();
    QWidget *buildStatCard(const QString &title, const QString &icon, QLabel *&valLbl,
                           QLabel *&pctLbl, QProgressBar *&bar, const QString &color);
    void applyStyle();

    void seedData();
    void refreshTable();
    void showDetail(int index);
    void clearDetail();
    void editAnimal(const QString &id);
    void deleteAnimal(const QString &id);
    int  indexOf(const QString &id) const;
    int  currentIndex() const;
    int  requireSelection();
    QString nextId() const;
    QString ownerName(const QString &id) const;
    QString vetName(const QString &id) const;

    static QString ageText(const QDate &d);
    static QPixmap avatar(const Animal &a, int size);
    static QLabel *badge(const QString &text, const QString &bg, const QString &fg);
    static QWidget *wrap(QWidget *w, Qt::Alignment al = Qt::AlignLeft | Qt::AlignVCenter);

    QVector<Animal> m_animals;
    QHash<QString, QString> m_owners;
    QHash<QString, QString> m_vets;
    QString m_editId;
    QString m_photoPath;

    QTableWidget *m_table = nullptr;
    QLineEdit *m_search = nullptr;
    QComboBox *m_filterEspece = nullptr;
    QLabel *m_count = nullptr;

    QLineEdit *m_aiInput = nullptr;
    QLabel *m_aiResponseLbl = nullptr;

    QLabel *m_lblTotal = nullptr;
    QLabel *m_lblAvgWeight = nullptr;
    QLabel *m_lblDogCount = nullptr;     QLabel *m_lblDogPct = nullptr;     QProgressBar *m_barDog = nullptr;
    QLabel *m_lblCatCount = nullptr;     QLabel *m_lblCatPct = nullptr;     QProgressBar *m_barCat = nullptr;
    QLabel *m_lblRabbitCount = nullptr;  QLabel *m_lblRabbitPct = nullptr;  QProgressBar *m_barRabbit = nullptr;
    QPushButton *m_btnAlertStatus = nullptr;
    QPushButton *m_btnSmartAnalysis = nullptr;

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
    QLabel *m_dIdProp = nullptr;
    QLabel *m_dNomProp = nullptr;
    QLabel *m_dIdVet = nullptr;
    QLabel *m_dNomVet = nullptr;

    QFrame *m_form = nullptr;
    QLabel *m_formTitle = nullptr;
    QLabel *m_formPhoto = nullptr;
    QLineEdit *m_fNom = nullptr;
    QLineEdit *m_fRace = nullptr;
    QLineEdit *m_fIdProp = nullptr;
    QLineEdit *m_fIdVet = nullptr;
    QComboBox *m_fEspece = nullptr;
    QComboBox *m_fSexe = nullptr;
    QDateEdit *m_fDate = nullptr;
    QDoubleSpinBox *m_fPoids = nullptr;
    QTextEdit *m_fDesc = nullptr;
    QPushButton *m_btnSave = nullptr;
};