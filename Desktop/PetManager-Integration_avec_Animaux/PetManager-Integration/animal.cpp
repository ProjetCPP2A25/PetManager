#include "animal.h"
#include "connexion.h"

#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>
#include <QEasingCurve>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QShowEvent>
#include <QToolTip>
#include <QVariantAnimation>
#include <QtMath>
#include <cmath>

static QString s_erreur;
QString Animal::derniereErreur() { return s_erreur; }

// Construit un message lisible (les messages Oracle en français sortent souvent illisibles via ODBC)
static QString messageErreur(const QSqlQuery &q)
{
    if (!Connexion::instance().db().isOpen())
        return "Base de donn\u00e9es non connect\u00e9e : la connexion Oracle a \u00e9chou\u00e9 au d\u00e9marrage "
               "(voir le premier message d'erreur, ou v\u00e9rifiez USER / PASSWORD dans connexion.cpp).";
    const QSqlError e = q.lastError();
    const QString brut = e.databaseText() + " " + e.driverText() + " " + e.text();
    QString code;
    for (const char *c5 : {"00904", "00942", "01400", "02290", "02292", "12899", "00001", "01017"})
        if (brut.contains(QString("ORA-") + c5)) { code = c5; break; }

    QString lisible;
    if      (code == "00904") lisible = "Colonne introuvable dans PETMANAGER.ANIMAL : relancez le script base_commune_petmanager.sql.";
    else if (code == "00942") lisible = "Table ANIMAL introuvable (sch\u00e9ma PETMANAGER). Les tables de la base commune ne sont pas cr\u00e9\u00e9es : ex\u00e9cutez le script de cr\u00e9ation puis system_xe.sql.";
    else if (code == "01400") lisible = "Un champ obligatoire est vide.";
    else if (code == "02290") lisible = "Une valeur refus\u00e9e par une contrainte CHECK de la base (esp\u00e8ce, sexe, poids ou statut).";
    else if (code == "02292") lisible = "Impossible de supprimer : cet animal a des rendez-vous. Supprimez d'abord ses rendez-vous.";
    else if (code == "12899") lisible = "Valeur trop longue pour la colonne.";
    else if (code == "00001") lisible = "Identifiant d\u00e9j\u00e0 existant.";
    else if (code == "01017") lisible = "Utilisateur ou mot de passe Oracle incorrect.";

    QString msg = lisible.isEmpty() ? QString("Erreur SQL non reconnue") : lisible;
    if (!code.isEmpty()) msg += QString("  [ORA-%1]").arg(code);
    if (lisible.isEmpty()) {
        // on garde seulement les caractères ASCII lisibles du message brut
        QString propre;
        for (const QChar ch : brut)
            if (ch.unicode() >= 32 && ch.unicode() < 127) propre += ch;
        propre = propre.simplified().left(250);
        msg += QString("\nRequete : %1\nCode natif : %2 | Detail : %3")
                   .arg(q.lastQuery().simplified().left(60), e.nativeErrorCode(), propre);
    }
    qWarning() << "Erreur SQL :" << brut << "| requete :" << q.lastQuery();
    return msg;
}

// ------------------------------------------------------------ contrôle de saisie
QString Animal::controleSaisie() const
{
    static const QRegularExpression reNom(QStringLiteral("^[\\p{L}][\\p{L} '\\-]{1,29}$"));
    static const QRegularExpression reRace(QStringLiteral("^[\\p{L}][\\p{L}0-9 '\\-]{1,39}$"));

    if (!reNom.match(nom.trimmed()).hasMatch())
        return "Nom invalide : 2 à 30 lettres (espaces, ' et - autorisés, pas de chiffres).";
    if (espece != "Chien" && espece != "Chat" && espece != "Lapin")
        return "Veuillez choisir une espèce valide (Chien, Chat ou Lapin).";
    if (!reRace.match(race.trimmed()).hasMatch())
        return "Race invalide : 2 à 40 caractères (lettres, chiffres, espaces, ' et -).";
    if (sexe != "Femelle" && sexe != "Mâle")
        return "Veuillez choisir le sexe (Femelle ou Mâle).";
    if (!naissance.isValid() || naissance > QDate::currentDate())
        return "La date de naissance est invalide (elle ne peut pas être dans le futur).";
    if (naissance < QDate::currentDate().addYears(-40))
        return "La date de naissance est trop ancienne (plus de 40 ans).";
    if (poids <= 0 || poids > 200)
        return "Le poids doit être compris entre 0,1 et 200 kg.";
    if (statut != "En bonne santé" && statut != "Surveillance")
        return "Statut de santé invalide.";
    if (description.size() > 500)
        return "La description ne doit pas dépasser 500 caractères.";
    return QString();
}

// ------------------------------------------------------------ helpers id (UI "ANI001" <-> base 1)
static int idVersNombre(const QString &id) { return id.mid(3).toInt(); }
static QString nombreVersId(int n)         { return QString("ANI%1").arg(n, 3, 10, QChar('0')); }

// ------------------------------------------------------------ CREATE
bool Animal::ajouter()
{
    QSqlQuery q(Connexion::instance().db());

    // nouvel identifiant = plus grand IDANIMAL + 1 (la table commune n'a pas de séquence)
    if (!q.exec("SELECT NVL(MAX(IDANIMAL), 0) + 1 FROM PETMANAGER.ANIMAL")) {
        s_erreur = messageErreur(q);
        return false;
    }
    if (!q.next()) {
        s_erreur = "Lecture du nouvel identifiant impossible.";
        return false;
    }
    const int num = q.value(0).toInt();
    id = nombreVersId(num);

    q.prepare("INSERT INTO PETMANAGER.ANIMAL (IDANIMAL, PHOTO, NOM, ESPECE, RACE, SEXE, "
              "DATENAISSANCE, POIDS, DESCRIPTION, STATUT) "
              "VALUES (:id, :photo, :nom, :espece, :race, :sexe, "
              "TO_DATE(:dn, 'YYYY-MM-DD'), :poids, :descr, :statut)");
    q.bindValue(":id", num);
    q.bindValue(":photo", photo);
    q.bindValue(":nom", nom.trimmed());
    q.bindValue(":espece", espece);
    q.bindValue(":race", race.trimmed());
    q.bindValue(":sexe", sexe);
    q.bindValue(":dn", naissance.toString("yyyy-MM-dd"));
    q.bindValue(":poids", poids);
    q.bindValue(":descr", description);
    q.bindValue(":statut", statut);
    if (!q.exec()) { s_erreur = messageErreur(q); return false; }
    return true;
}

// ------------------------------------------------------------ UPDATE
bool Animal::modifier() const
{
    QSqlQuery q(Connexion::instance().db());
    q.prepare("UPDATE PETMANAGER.ANIMAL SET PHOTO = :photo, NOM = :nom, ESPECE = :espece, RACE = :race, "
              "SEXE = :sexe, DATENAISSANCE = TO_DATE(:dn, 'YYYY-MM-DD'), POIDS = :poids, "
              "DESCRIPTION = :descr, STATUT = :statut WHERE IDANIMAL = :id");
    q.bindValue(":photo", photo);
    q.bindValue(":nom", nom.trimmed());
    q.bindValue(":espece", espece);
    q.bindValue(":race", race.trimmed());
    q.bindValue(":sexe", sexe);
    q.bindValue(":dn", naissance.toString("yyyy-MM-dd"));
    q.bindValue(":poids", poids);
    q.bindValue(":descr", description);
    q.bindValue(":statut", statut);
    q.bindValue(":id", idVersNombre(id));
    if (!q.exec()) { s_erreur = messageErreur(q); return false; }
    if (q.numRowsAffected() <= 0) { s_erreur = "Aucune ligne modifi\u00e9e (animal introuvable)."; return false; }
    return true;
}

// ------------------------------------------------------------ DELETE
bool Animal::supprimer(const QString &id)
{
    QSqlQuery q(Connexion::instance().db());
    q.prepare("DELETE FROM PETMANAGER.ANIMAL WHERE IDANIMAL = :id");
    q.bindValue(":id", idVersNombre(id));
    if (!q.exec()) {
        s_erreur = messageErreur(q);
        return false;
    }
    if (q.numRowsAffected() <= 0) { s_erreur = "Animal introuvable."; return false; }
    return true;
}

// ------------------------------------------------------------ READ
QVector<Animal> Animal::afficher()
{
    QVector<Animal> liste;
    QSqlQuery q(Connexion::instance().db());
    q.prepare("SELECT IDANIMAL, PHOTO, NOM, ESPECE, RACE, SEXE, "
              "TO_CHAR(DATENAISSANCE, 'YYYY-MM-DD'), POIDS, DESCRIPTION, STATUT "
              "FROM PETMANAGER.ANIMAL ORDER BY IDANIMAL");
    if (!q.exec()) { s_erreur = messageErreur(q); return liste; }
    while (q.next()) {
        Animal a;
        a.id          = nombreVersId(q.value(0).toInt());
        a.photo       = q.value(1).toString();
        a.nom         = q.value(2).toString();
        a.espece      = q.value(3).toString();
        a.race        = q.value(4).toString();
        a.sexe        = q.value(5).toString();
        a.naissance   = QDate::fromString(q.value(6).toString(), "yyyy-MM-dd");
        if (!a.naissance.isValid()) a.naissance = QDate::currentDate();
        a.poids       = q.value(7).toDouble();
        a.description = q.value(8).toString();
        a.statut      = q.value(9).isNull() ? QString("En bonne santé") : q.value(9).toString();
        liste.append(a);
    }
    return liste;
}

// ------------------------------------------------------------ métiers
QVector<QPair<QString,int>> Animal::statParEspece()
{
    QVector<QPair<QString,int>> r;
    QSqlQuery q(Connexion::instance().db());
    q.prepare("SELECT ESPECE, COUNT(*) FROM PETMANAGER.ANIMAL GROUP BY ESPECE ORDER BY COUNT(*) DESC");
    if (q.exec())
        while (q.next()) r.append({q.value(0).toString(), q.value(1).toInt()});
    return r;
}

int Animal::nombreSurveillance()
{
    QSqlQuery q(Connexion::instance().db());
    q.prepare("SELECT COUNT(*) FROM PETMANAGER.ANIMAL WHERE STATUT = :s");
    q.bindValue(":s", "Surveillance");
    return (q.exec() && q.next()) ? q.value(0).toInt() : 0;
}

// =====================================================================
// GRAPHIQUES (anciennement charts.cpp)
// =====================================================================
// ============================================================ utilitaires
static QColor mixColor(const QColor &a, const QColor &b, double t)
{
    return QColor(int(a.red()   + (b.red()   - a.red())   * t),
                  int(a.green() + (b.green() - a.green()) * t),
                  int(a.blue()  + (b.blue()  - a.blue())  * t));
}

// dégradé vert -> bleu -> violet (t entre 0 et 1)
static QColor gradColor(double t)
{
    const QColor c0("#2F7F73"), c1("#2563EB"), c2("#9333EA");
    t = qBound(0.0, t, 1.0);
    return t < 0.5 ? mixColor(c0, c1, t * 2.0) : mixColor(c1, c2, (t - 0.5) * 2.0);
}

static QVariantAnimation *makeAnim(QObject *parent)
{
    auto *a = new QVariantAnimation(parent);
    a->setDuration(700);
    a->setStartValue(0.0);
    a->setEndValue(1.0);
    a->setEasingCurve(QEasingCurve::OutCubic);
    return a;
}

// ============================================================ ClickableFrame
ClickableFrame::ClickableFrame(QWidget *parent) : QFrame(parent)
{
    setCursor(Qt::PointingHandCursor);
}

void ClickableFrame::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && rect().contains(e->position().toPoint()))
        emit clicked();
    QFrame::mouseReleaseEvent(e);
}

// ============================================================ DonutChart
DonutChart::DonutChart(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
    m_anim = makeAnim(this);
    connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_progress = v.toDouble();
        update();
    });
}

void DonutChart::setData(const QVector<ChartSlice> &slices, const QString &centerCaption)
{
    m_slices.clear();
    for (const ChartSlice &s : slices)
        if (s.value > 0) m_slices.append(s);      // on ignore les parts vides
    m_caption = centerCaption;
    m_hover = -1;
    m_anim->stop();
    m_anim->start();
}

double DonutChart::total() const
{
    double t = 0;
    for (const ChartSlice &s : m_slices) t += s.value;
    return t;
}

QRectF DonutChart::pieRect(const QRectF &r) const
{
    const double side = qMax(60.0, qMin(r.height() - 40.0, r.width() * 0.48));
    return QRectF(r.left() + 18, r.top() + (r.height() - side) / 2.0 - 6, side, side);
}

QRectF DonutChart::legendRow(int i, const QRectF &r) const
{
    const QRectF pie = pieRect(r);
    const double lx = pie.right() + 30;
    const double rowH = 38;
    const double top = r.center().y() - (m_slices.size() * rowH) / 2.0 - 6;
    return QRectF(lx, top + i * rowH, qMax(80.0, r.right() - lx - 14), rowH - 6);
}

void DonutChart::paintChart(QPainter &p, const QRectF &r, int hover, double progress) const
{
    p.setRenderHint(QPainter::Antialiasing);
    const double tot = total();
    if (tot <= 0) {
        QFont f = p.font();
        f.setPixelSize(13);
        p.setFont(f);
        p.setPen(QColor("#6A7B80"));
        p.drawText(r, Qt::AlignCenter, QStringLiteral("Aucune donn\u00e9e \u00e0 afficher"));
        return;
    }

    const QRectF pie = pieRect(r);
    const QPointF c = pie.center();

    // --- les parts (sens horaire, départ à 12h)
    double startDeg = 90.0;
    for (int i = 0; i < m_slices.size(); ++i) {
        const double span = m_slices[i].value / tot * 360.0 * progress;
        const QRectF pr = (i == hover) ? pie.adjusted(-6, -6, 6, 6) : pie;
        p.setPen(QPen(Qt::white, 2));
        p.setBrush(m_slices[i].color);
        p.drawPie(pr, int(startDeg * 16), int(-span * 16));
        startDeg -= span;
    }

    // --- le trou central
    const double hole = pie.width() * 0.60;
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);
    p.drawEllipse(c, hole / 2.0, hole / 2.0);

    // --- texte au centre
    QString top, bottom;
    if (hover >= 0 && hover < m_slices.size()) {
        top = QString::number(qRound(m_slices[hover].value / tot * 100.0)) + " %";
        bottom = m_slices[hover].label;
    } else {
        top = QString::number(qRound(tot));
        bottom = m_caption;
    }
    QFont f = p.font();
    f.setBold(true);
    f.setPixelSize(qMax(14, int(pie.width() * 0.16)));
    p.setFont(f);
    p.setPen(QColor("#1E2B30"));
    p.drawText(QRectF(c.x() - hole / 2, c.y() - hole * 0.36, hole, hole * 0.42), Qt::AlignCenter, top);
    f.setBold(false);
    f.setPixelSize(qMax(10, int(pie.width() * 0.07)));
    p.setFont(f);
    p.setPen(QColor("#6A7B80"));
    p.drawText(QRectF(c.x() - hole / 2, c.y() + hole * 0.04, hole, hole * 0.22), Qt::AlignCenter, bottom);

    // --- la légende
    for (int i = 0; i < m_slices.size(); ++i) {
        const QRectF row = legendRow(i, r);
        if (i == hover) {
            QColor bg = m_slices[i].color;
            bg.setAlpha(30);
            p.setPen(Qt::NoPen);
            p.setBrush(bg);
            p.drawRoundedRect(row, 9, 9);
        }
        p.setPen(Qt::NoPen);
        p.setBrush(m_slices[i].color);
        p.drawEllipse(QPointF(row.left() + 16, row.center().y()), 6, 6);

        QFont lf = p.font();
        lf.setPixelSize(13);
        lf.setBold(true);
        p.setFont(lf);
        p.setPen(QColor("#243238"));
        p.drawText(row.adjusted(34, 0, -8, 0), Qt::AlignVCenter | Qt::AlignLeft, m_slices[i].label);

        lf.setBold(false);
        p.setFont(lf);
        p.setPen(QColor("#5B706B"));
        const int pct = qRound(m_slices[i].value / tot * 100.0);
        p.drawText(row.adjusted(34, 0, -10, 0), Qt::AlignVCenter | Qt::AlignRight,
                   QString::number(int(m_slices[i].value)) + "   \u00b7   " + QString::number(pct) + " %");
    }
}

int DonutChart::sliceAt(const QPointF &pos) const
{
    const double tot = total();
    if (tot <= 0) return -1;
    const QRectF all(rect());
    const QRectF pie = pieRect(all);
    const QPointF c = pie.center();
    const double dx = pos.x() - c.x();
    const double dy = pos.y() - c.y();
    const double dist = std::sqrt(dx * dx + dy * dy);
    const double R = pie.width() / 2.0;

    if (dist <= R + 6 && dist >= R * 0.60) {
        const double ang = qRadiansToDegrees(std::atan2(-dy, dx));      // anti-horaire depuis 3h
        const double cw = std::fmod(90.0 - ang + 720.0, 360.0);         // horaire depuis 12h
        double acc = 0;
        for (int i = 0; i < m_slices.size(); ++i) {
            acc += m_slices[i].value / tot * 360.0;
            if (cw < acc) return i;
        }
        return m_slices.size() - 1;
    }
    for (int i = 0; i < m_slices.size(); ++i)            // survol de la légende
        if (legendRow(i, all).contains(pos)) return i;
    return -1;
}

void DonutChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    paintChart(p, QRectF(rect()), m_hover, m_progress);
    if (!m_slices.isEmpty()) {
        QFont f = p.font();
        f.setPixelSize(10);
        p.setFont(f);
        p.setPen(QColor("#98A8A5"));
        p.drawText(QRectF(rect()).adjusted(0, 0, -12, -4), Qt::AlignRight | Qt::AlignBottom,
                   QStringLiteral("Cliquez sur une part pour filtrer la liste"));
    }
}

void DonutChart::mouseMoveEvent(QMouseEvent *e)
{
    const int h = sliceAt(e->position());
    if (h != m_hover) {
        m_hover = h;
        update();
    }
    setCursor(h >= 0 ? Qt::PointingHandCursor : Qt::ArrowCursor);
    if (h >= 0) {
        const double tot = total();
        const int pct = qRound(m_slices[h].value / tot * 100.0);
        QToolTip::showText(e->globalPosition().toPoint(),
                           m_slices[h].label + " : " + QString::number(int(m_slices[h].value))
                               + " (" + QString::number(pct) + " %)", this);
    } else {
        QToolTip::hideText();
    }
}

void DonutChart::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && m_hover >= 0 && m_hover < m_slices.size())
        emit sliceClicked(m_slices[m_hover].key);
}

void DonutChart::leaveEvent(QEvent *)
{
    if (m_hover != -1) {
        m_hover = -1;
        update();
    }
    QToolTip::hideText();
}

void DonutChart::showEvent(QShowEvent *)
{
    m_anim->stop();
    m_anim->start();
}

QPixmap DonutChart::snapshot(const QSize &size) const
{
    const int scale = 2;                                  // x2 pour un PDF net
    QPixmap pm(size * scale);
    pm.fill(Qt::white);
    QPainter p(&pm);
    p.scale(scale, scale);
    paintChart(p, QRectF(QPointF(0, 0), QSizeF(size)), -1, 1.0);
    return pm;
}

// ============================================================ LineChart
LineChart::LineChart(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
}

void LineChart::setData(const QVector<ChartPoint> &pts)
{
    m_pts = pts;
    m_hover = -1;
    update();
}

double LineChart::maxAxis(int &ticks) const
{
    double mx = 0;
    for (const ChartPoint &p : m_pts) mx = qMax(mx, p.value);
    int m = qMax(1, int(std::ceil(mx)));
    ticks = qMin(4, m);
    m = int(std::ceil(double(m) / ticks)) * ticks;
    return m;
}

QRectF LineChart::plotRect(const QRectF &r) const
{
    return r.adjusted(46, 22, -22, -36);
}

QPointF LineChart::pointPos(const QRectF &plot, int i, double maxV) const
{
    const int n = m_pts.size();
    const double inner = 18.0;
    const double x = (n == 1) ? plot.center().x()
                              : plot.left() + inner + i * (plot.width() - 2 * inner) / (n - 1);
    const double y = plot.bottom() - (maxV > 0 ? m_pts[i].value / maxV : 0.0) * plot.height();
    return QPointF(x, y);
}

void LineChart::paintChart(QPainter &p, const QRectF &r, int hover) const
{
    p.setRenderHint(QPainter::Antialiasing);
    QFont f = p.font();
    f.setPixelSize(11);
    p.setFont(f);

    if (m_pts.isEmpty()) {
        p.setPen(QColor("#6A7B80"));
        p.drawText(r, Qt::AlignCenter, QStringLiteral("Aucune donn\u00e9e \u00e0 afficher"));
        return;
    }

    int ticks = 1;
    const double maxV = maxAxis(ticks);
    const QRectF plot = plotRect(r);
    const int n = m_pts.size();

    // --- grille + axe Y
    for (int t = 0; t <= ticks; ++t) {
        const double y = plot.bottom() - plot.height() * t / ticks;
        p.setPen(QPen(QColor("#E3EDEB"), 1, t == 0 ? Qt::SolidLine : Qt::DashLine));
        p.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
        p.setPen(QColor("#6A7B80"));
        p.drawText(QRectF(r.left(), y - 9, plot.left() - r.left() - 8, 18),
                   Qt::AlignRight | Qt::AlignVCenter, QString::number(int(maxV * t / ticks)));
    }

    // --- positions
    QVector<QPointF> pos;
    for (int i = 0; i < n; ++i) pos.append(pointPos(plot, i, maxV));

    // --- axe X
    const int step = qMax(1, int(std::ceil(n / 8.0)));
    p.setPen(QColor("#6A7B80"));
    for (int i = 0; i < n; i += step)
        p.drawText(QRectF(pos[i].x() - 30, plot.bottom() + 8, 60, 18), Qt::AlignCenter, m_pts[i].label);

    // --- ligne de survol
    if (hover >= 0 && hover < n) {
        p.setPen(QPen(QColor("#B7CBC7"), 1, Qt::DashLine));
        p.drawLine(QPointF(pos[hover].x(), plot.top()), QPointF(pos[hover].x(), plot.bottom()));
    }

    // --- courbe lissée (Bézier)
    QPainterPath line;
    line.moveTo(pos[0]);
    for (int i = 0; i + 1 < n; ++i) {
        const QPointF p0 = i > 0 ? pos[i - 1] : pos[i];
        const QPointF p1 = pos[i];
        const QPointF p2 = pos[i + 1];
        const QPointF p3 = (i + 2 < n) ? pos[i + 2] : p2;
        QPointF c1 = p1 + (p2 - p0) / 6.0;
        QPointF c2 = p2 - (p3 - p1) / 6.0;
        c1.setY(qBound(plot.top(), c1.y(), plot.bottom()));
        c2.setY(qBound(plot.top(), c2.y(), plot.bottom()));
        line.cubicTo(c1, c2, p2);
    }

    // --- aire sous la courbe
    if (n > 1) {
        QPainterPath area = line;
        area.lineTo(pos.last().x(), plot.bottom());
        area.lineTo(pos.first().x(), plot.bottom());
        area.closeSubpath();
        QLinearGradient ag(0, plot.top(), 0, plot.bottom());
        ag.setColorAt(0.0, QColor(37, 99, 235, 90));
        ag.setColorAt(1.0, QColor(47, 127, 115, 6));
        p.setPen(Qt::NoPen);
        p.setBrush(ag);
        p.drawPath(area);
    }

    // --- trait en dégradé
    QLinearGradient lg(plot.left(), 0, plot.right(), 0);
    lg.setColorAt(0.0, gradColor(0.0));
    lg.setColorAt(0.5, gradColor(0.5));
    lg.setColorAt(1.0, gradColor(1.0));
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QBrush(lg), 3.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawPath(line);

    // --- points colorés + valeurs
    for (int i = 0; i < n; ++i) {
        const QColor col = gradColor(n > 1 ? double(i) / (n - 1) : 0.0);
        const double rad = (i == hover) ? 7.5 : 5.5;
        p.setBrush(Qt::white);
        p.setPen(QPen(col, 3));
        p.drawEllipse(pos[i], rad, rad);
        if (n <= 12 || i == hover) {
            QFont vf = p.font();
            vf.setBold(true);
            vf.setPixelSize(12);
            p.setFont(vf);
            p.setPen(col.darker(115));
            p.drawText(QRectF(pos[i].x() - 20, pos[i].y() - 28, 40, 18), Qt::AlignCenter,
                       QString::number(int(m_pts[i].value)));
            vf.setBold(false);
            vf.setPixelSize(11);
            p.setFont(vf);
        }
    }
}

int LineChart::pointAt(const QPointF &pos) const
{
    if (m_pts.isEmpty()) return -1;
    int ticks = 1;
    const double maxV = maxAxis(ticks);
    const QRectF plot = plotRect(QRectF(rect()));
    int best = -1;
    double bestDx = 1e9;
    for (int i = 0; i < m_pts.size(); ++i) {
        const double dx = std::abs(pointPos(plot, i, maxV).x() - pos.x());
        if (dx < bestDx) {
            bestDx = dx;
            best = i;
        }
    }
    const double reach = m_pts.size() > 1 ? qMax(16.0, plot.width() / (2.0 * m_pts.size())) : 30.0;
    return bestDx <= reach ? best : -1;
}

void LineChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    paintChart(p, QRectF(rect()), m_hover);
}

void LineChart::mouseMoveEvent(QMouseEvent *e)
{
    const int h = pointAt(e->position());
    if (h != m_hover) {
        m_hover = h;
        update();
    }
    if (h >= 0) {
        const int v = int(m_pts[h].value);
        QToolTip::showText(e->globalPosition().toPoint(),
                           m_pts[h].label + " : " + QString::number(v)
                               + (v > 1 ? QStringLiteral(" animaux n\u00e9s") : QStringLiteral(" animal n\u00e9")), this);
    } else {
        QToolTip::hideText();
    }
}

void LineChart::leaveEvent(QEvent *)
{
    if (m_hover != -1) {
        m_hover = -1;
        update();
    }
    QToolTip::hideText();
}

QPixmap LineChart::snapshot(const QSize &size) const
{
    const int scale = 2;
    QPixmap pm(size * scale);
    pm.fill(Qt::white);
    QPainter p(&pm);
    p.scale(scale, scale);
    paintChart(p, QRectF(QPointF(0, 0), QSizeF(size)), -1);
    return pm;
}

// ============================================================ GaugeWidget
GaugeWidget::GaugeWidget(QWidget *parent) : QWidget(parent)
{
    m_anim = makeAnim(this);
    connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_progress = v.toDouble();
        update();
    });
}

void GaugeWidget::setValue(int pct, bool valid)
{
    m_value = qBound(0, pct, 100);
    m_valid = valid;
    m_anim->stop();
    m_anim->start();
}

void GaugeWidget::showEvent(QShowEvent *)
{
    m_anim->stop();
    m_anim->start();
}

void GaugeWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const double w = qMax(100.0, qMin(width() - 40.0, (height() - 36.0) * 2.0));
    const QRectF r((width() - w) / 2.0, 14.0, w, w);
    const double pen = qMax(10.0, w * 0.075);
    const QRectF arc = r.adjusted(pen / 2, pen / 2, -pen / 2, -pen / 2);

    QColor col("#98A8A5");
    QString verdict = QStringLiteral("\u2014");
    if (m_valid) {
        if (m_value >= 80)      { col = QColor("#2E9E6B"); verdict = QStringLiteral("Excellent"); }
        else if (m_value >= 50) { col = QColor("#D97706"); verdict = QStringLiteral("Correct"); }
        else                    { col = QColor("#C0392B"); verdict = QStringLiteral("Critique"); }
    }

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor("#E2ECE9"), pen, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(arc, 180 * 16, -180 * 16);

    const double shown = m_valid ? m_value * m_progress : 0.0;
    if (shown > 0.5) {
        p.setPen(QPen(col, pen, Qt::SolidLine, Qt::RoundCap));
        p.drawArc(arc, 180 * 16, -int(shown * 1.8 * 16));
    }

    const QPointF c = arc.center();
    QFont f = font();
    f.setBold(true);
    f.setPixelSize(qMax(16, int(w * 0.17)));
    p.setFont(f);
    p.setPen(col);
    p.drawText(QRectF(c.x() - w / 2, c.y() - w * 0.27, w, w * 0.22), Qt::AlignCenter,
               m_valid ? QString::number(qRound(shown)) + " %" : QStringLiteral("\u2014"));

    f.setPixelSize(qMax(11, int(w * 0.065)));
    p.setFont(f);
    p.drawText(QRectF(c.x() - w / 2, c.y() - w * 0.03, w, w * 0.12), Qt::AlignCenter, verdict);
}
