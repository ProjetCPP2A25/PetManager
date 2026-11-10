#pragma once
#include <QString>
#include <QDate>
#include <QVector>
#include <QPair>
#include <QColor>
#include <QFrame>
#include <QPixmap>
#include <QWidget>
#include <QDialog>
#include <QHash>
#include <QSet>

class QVariantAnimation;
class QGraphicsScene;
class QGraphicsView;
class QComboBox;
class QLabel;

// une alerte affichée dans la fenêtre « 🔔 Alertes »
struct AlerteAnimal {
    QString idAnimal, nom;
    QString type;       // "age" | "poids" | "anniv"
    QString niveau;     // "warn" (à surveiller) | "info"
    QString texte;
    int     tri = 0;    // ordre d'affichage dans le groupe (ex: jour de l'anniversaire)
};

// Entité « Animal » : attributs + CRUD (toutes les requêtes SQL sont ICI, jamais derrière un bouton)
class Animal
{
public:
    // --- attributs (même noms qu'avant : le reste de l'interface ne change pas)
    QString id, nom, espece, race, sexe, statut, description, photo;
    QString idPere, idMere;                  // généalogie : "ANI001" ou vide si inconnu
    QDate   naissance;
    double  poids = 0;

    Animal() = default;

    // --- CONTRÔLE DE SAISIE (retourne "" si OK, sinon le message d'erreur)
    QString controleSaisie() const;
    // contrôle de la généalogie (père mâle, mère femelle, même espèce, nés avant, pas de boucle)
    QString controleGenealogie(const QVector<Animal> &tous) const;

    // --- CRUD
    bool ajouter();                          // CREATE  : remplit aussi this->id
    bool modifier() const;                   // UPDATE
    static bool supprimer(const QString &id);// DELETE
    static QVector<Animal> afficher();       // READ    : tous les animaux

    // --- métiers (requêtes SQL d'agrégation)
    static QVector<QPair<QString,int>> statParEspece();   // nb d'animaux par espèce
    static int nombreSurveillance();                      // nb d'animaux sous surveillance
    // poids moyen des adultes (>= 12 mois) par espèce + race : { "CHIEN|LABRADOR" -> (moyenne, effectif) }
    static QHash<QString, QPair<double,int>> poidsMoyenParRace();

    // --- catégorie d'âge (selon l'espèce) et alertes
    int     ageMois() const;
    QString categorieAge() const;                         // Chiot/Chaton/Lapereau, Adulte, Senior
    static QVector<AlerteAnimal> alertes(const QVector<Animal> &tous);  // âge, poids, anniversaires

    static QString derniereErreur();         // dernier message d'erreur SQL
};

// ------------------------------------------------------------------ fenêtre « 🔔 Alertes »
class AlertesDialog : public QDialog
{
    Q_OBJECT
public:
    AlertesDialog(const QVector<AlerteAnimal> &alertes, QWidget *parent = nullptr);
};

// =====================================================================
// GÉNÉALOGIE : graphe des parents + coefficient de consanguinité (Wright)
// =====================================================================
// Aucun SQL ici : on travaille sur la liste d'animaux déjà chargée (Animal::afficher()).
// Le pedigree est un graphe orienté acyclique : chaque animal pointe vers son père et sa mère.
class Pedigree
{
public:
    explicit Pedigree(const QVector<Animal> &tous);

    const Animal *trouver(const QString &id) const;
    QString pere(const QString &id) const;
    QString mere(const QString &id) const;

    // F(x) = 1/2 * a(père, mère)  où a(x,y) = coefficient de parenté (méthode tabulaire récursive)
    double coefficient(const QString &id) const;
    // F d'un futur petit issu de ces deux parents
    double coefficientDescendant(const QString &idPere, const QString &idMere) const;
    // nombre de générations d'ancêtres connues (0 = aucun parent connu)
    int generationsConnues(const QString &id) const;
    // vrai si "ancetre" se trouve dans les ancêtres de "id"
    bool estDescendantDe(const QString &id, const QString &ancetre) const;

    // interprétation du taux (F en fraction : 0.0625 = 6,25 %)
    static QString niveau(double f);          // Faible / Modéré / Élevé / Critique
    static QColor  couleur(double f);
    static QString conseil(double f);         // phrase sur l'aptitude à la reproduction

private:
    double parente(const QString &x, const QString &y, int garde = 0) const;
    int    profondeur(const QString &id) const;

    QHash<QString, const Animal *> m_index;
    mutable QHash<QString, int>    m_profondeur;
    mutable QHash<QString, double> m_parente;
};

// ------------------------------------------------------------------ barre de consanguinité (4 zones colorées)
class InbreedingBar : public QWidget
{
    Q_OBJECT
public:
    explicit InbreedingBar(QWidget *parent = nullptr);
    void setValue(double fraction, bool valid = true);   // fraction : 0.125 = 12,5 %
    QSize sizeHint() const override { return QSize(260, 44); }
    QSize minimumSizeHint() const override { return QSize(160, 40); }
protected:
    void paintEvent(QPaintEvent *) override;
private:
    double m_f = 0;
    bool   m_valid = false;
};

// ------------------------------------------------------------------ fenêtre « Arbre généalogique »
class PedigreeDialog : public QDialog
{
    Q_OBJECT
public:
    PedigreeDialog(const Animal &animal, const QVector<Animal> &tous, QWidget *parent = nullptr);
private:
    void dessinerArbre();
    void majSimulation();

    Animal m_animal;
    QVector<Animal> m_tous;
    Pedigree m_ped;
    QGraphicsScene *m_scene = nullptr;
    QGraphicsView  *m_view = nullptr;
    InbreedingBar  *m_bar = nullptr;
    QLabel *m_taux = nullptr;
    QLabel *m_verdict = nullptr;
    QLabel *m_infos = nullptr;
    QComboBox *m_partenaire = nullptr;
    QLabel *m_simu = nullptr;
};

// =====================================================================
// GRAPHIQUES (anciennement charts.h) : widgets de statistiques
// =====================================================================
// ------------------------------------------------------------------ données
struct ChartSlice {
    QString key;        // valeur technique renvoyée au clic (ex: "Chien")
    QString label;      // texte affiché (ex: "Chiens")
    double  value = 0;
    QColor  color;
};

struct ChartPoint {
    QString label;      // abscisse (ex: "2024")
    double  value = 0;
};

// ------------------------------------------------------------------ carte cliquable (KPI)
class ClickableFrame : public QFrame
{
    Q_OBJECT
public:
    explicit ClickableFrame(QWidget *parent = nullptr);
signals:
    void clicked();
protected:
    void mouseReleaseEvent(QMouseEvent *e) override;
};

// ------------------------------------------------------------------ graphique en cercle (donut)
class DonutChart : public QWidget
{
    Q_OBJECT
public:
    explicit DonutChart(QWidget *parent = nullptr);
    void setData(const QVector<ChartSlice> &slices, const QString &centerCaption);
    QPixmap snapshot(const QSize &size) const;     // image haute définition (pour le PDF)
    QSize sizeHint() const override { return QSize(460, 250); }
    QSize minimumSizeHint() const override { return QSize(340, 220); }
signals:
    void sliceClicked(const QString &key);
protected:
    void paintEvent(QPaintEvent *) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *) override;
    void showEvent(QShowEvent *) override;
private:
    void   paintChart(QPainter &p, const QRectF &r, int hover, double progress) const;
    QRectF pieRect(const QRectF &r) const;
    QRectF legendRow(int i, const QRectF &r) const;
    int    sliceAt(const QPointF &pos) const;
    double total() const;

    QVector<ChartSlice> m_slices;
    QString m_caption;
    int     m_hover = -1;
    double  m_progress = 1.0;
    QVariantAnimation *m_anim = nullptr;
};

// ------------------------------------------------------------------ courbe
class LineChart : public QWidget
{
    Q_OBJECT
public:
    explicit LineChart(QWidget *parent = nullptr);
    void setData(const QVector<ChartPoint> &pts);
    QPixmap snapshot(const QSize &size) const;
    QSize sizeHint() const override { return QSize(460, 250); }
    QSize minimumSizeHint() const override { return QSize(340, 220); }
protected:
    void paintEvent(QPaintEvent *) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *) override;
private:
    void    paintChart(QPainter &p, const QRectF &r, int hover) const;
    QRectF  plotRect(const QRectF &r) const;
    QPointF pointPos(const QRectF &plot, int i, double maxV) const;
    double  maxAxis(int &ticks) const;
    int     pointAt(const QPointF &pos) const;

    QVector<ChartPoint> m_pts;
    int m_hover = -1;
};

// ------------------------------------------------------------------ jauge (score)
class GaugeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GaugeWidget(QWidget *parent = nullptr);
    void setValue(int pct, bool valid = true);
    QSize sizeHint() const override { return QSize(260, 170); }
    QSize minimumSizeHint() const override { return QSize(200, 140); }
protected:
    void paintEvent(QPaintEvent *) override;
    void showEvent(QShowEvent *) override;
private:
    int    m_value = 0;
    bool   m_valid = false;
    double m_progress = 1.0;
    QVariantAnimation *m_anim = nullptr;
};
