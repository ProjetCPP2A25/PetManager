#pragma once
#include <QString>
#include <QDate>
#include <QVector>
#include <QPair>
#include <QColor>
#include <QFrame>
#include <QPixmap>
#include <QWidget>

class QVariantAnimation;

// Entité « Animal » : attributs + CRUD (toutes les requêtes SQL sont ICI, jamais derrière un bouton)
class Animal
{
public:
    // --- attributs (même noms qu'avant : le reste de l'interface ne change pas)
    QString id, nom, espece, race, sexe, statut, description, photo;
    QDate   naissance;
    double  poids = 0;

    Animal() = default;

    // --- CONTRÔLE DE SAISIE (retourne "" si OK, sinon le message d'erreur)
    QString controleSaisie() const;

    // --- CRUD
    bool ajouter();                          // CREATE  : remplit aussi this->id
    bool modifier() const;                   // UPDATE
    static bool supprimer(const QString &id);// DELETE
    static QVector<Animal> afficher();       // READ    : tous les animaux

    // --- métiers (requêtes SQL d'agrégation)
    static QVector<QPair<QString,int>> statParEspece();   // nb d'animaux par espèce
    static int nombreSurveillance();                      // nb d'animaux sous surveillance

    static QString derniereErreur();         // dernier message d'erreur SQL
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
