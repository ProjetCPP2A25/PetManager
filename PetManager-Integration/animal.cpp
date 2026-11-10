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
#include <QComboBox>
#include <QFontMetrics>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <algorithm>
#include <QVBoxLayout>
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
    for (const char *c5 : {"00904", "00942", "01400", "02290", "02291", "02292", "12899", "00001", "01017"})
        if (brut.contains(QString("ORA-") + c5)) { code = c5; break; }

    QString lisible;
    if      (code == "00904") lisible = "Colonne introuvable dans PETMANAGER.ANIMAL : ex\u00e9cutez le script sql/ajout_genealogie.sql (colonnes IDPERE / IDMERE).";
    else if (code == "00942") lisible = "Table ANIMAL introuvable (sch\u00e9ma PETMANAGER). Les tables de la base commune ne sont pas cr\u00e9\u00e9es : ex\u00e9cutez le script de cr\u00e9ation puis system_xe.sql.";
    else if (code == "01400") lisible = "Un champ obligatoire est vide.";
    else if (code == "02290") lisible = "Une valeur refus\u00e9e par une contrainte CHECK de la base (esp\u00e8ce, sexe, poids ou statut).";
    else if (code == "02291") lisible = "Le p\u00e8re ou la m\u00e8re choisi(e) n'existe pas dans la base.";
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
// parent inconnu -> NULL en base ; sinon numéro (ANI012 -> 12)
static QVariant parentVersSql(const QString &id)
{
    return id.isEmpty() ? QVariant(QMetaType::fromType<int>()) : QVariant(idVersNombre(id));
}

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
              "DATENAISSANCE, POIDS, DESCRIPTION, STATUT, IDPERE, IDMERE) "
              "VALUES (:id, :photo, :nom, :espece, :race, :sexe, "
              "TO_DATE(:dn, 'YYYY-MM-DD'), :poids, :descr, :statut, :pere, :mere)");
    q.bindValue(":id", num);
    q.bindValue(":pere", parentVersSql(idPere));
    q.bindValue(":mere", parentVersSql(idMere));
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
              "DESCRIPTION = :descr, STATUT = :statut, IDPERE = :pere, IDMERE = :mere WHERE IDANIMAL = :id");
    q.bindValue(":photo", photo);
    q.bindValue(":nom", nom.trimmed());
    q.bindValue(":espece", espece);
    q.bindValue(":race", race.trimmed());
    q.bindValue(":sexe", sexe);
    q.bindValue(":dn", naissance.toString("yyyy-MM-dd"));
    q.bindValue(":poids", poids);
    q.bindValue(":descr", description);
    q.bindValue(":statut", statut);
    q.bindValue(":pere", parentVersSql(idPere));
    q.bindValue(":mere", parentVersSql(idMere));
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
              "TO_CHAR(DATENAISSANCE, 'YYYY-MM-DD'), POIDS, DESCRIPTION, STATUT, IDPERE, IDMERE "
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
        a.idPere      = q.value(10).isNull() ? QString() : nombreVersId(q.value(10).toInt());
        a.idMere      = q.value(11).isNull() ? QString() : nombreVersId(q.value(11).toInt());
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

// ------------------------------------------------------------ métier : poids moyen par race (SQL d'agrégation)
QHash<QString, QPair<double,int>> Animal::poidsMoyenParRace()
{
    QHash<QString, QPair<double,int>> r;
    QSqlQuery q(Connexion::instance().db());
    // on ne compare que des adultes : un chiot pèse forcément moins que la moyenne de sa race
    q.prepare("SELECT UPPER(ESPECE), UPPER(TRIM(RACE)), AVG(POIDS), COUNT(*) FROM PETMANAGER.ANIMAL "
              "WHERE POIDS IS NOT NULL AND DATENAISSANCE <= ADD_MONTHS(TRUNC(SYSDATE), -12) "
              "GROUP BY UPPER(ESPECE), UPPER(TRIM(RACE))");
    if (q.exec())
        while (q.next())
            r.insert(q.value(0).toString() + "|" + q.value(1).toString(),
                     { q.value(2).toDouble(), q.value(3).toInt() });
    return r;
}

// ------------------------------------------------------------ métier : catégories d'âge
namespace {
struct SeuilsAge { const char *espece; const char *jeune; int finJeuneMois; int seniorMois; };
const SeuilsAge SEUILS_AGE[] = {
    { "Chien", "Chiot",    12,  84 },     // senior à 7 ans
    { "Chat",  "Chaton",   12, 120 },     // senior à 10 ans
    { "Lapin", "Lapereau",  8,  72 },     // senior à 6 ans
};
const SeuilsAge *seuilsPour(const QString &espece)
{
    for (const SeuilsAge &s : SEUILS_AGE)
        if (espece == QLatin1String(s.espece)) return &s;
    return nullptr;
}
QString ageTexte(int mois)
{
    if (mois < 24) return QString::number(mois) + " mois";
    return QString::number(mois / 12) + " ans";
}
QString pct1(double v) { return QString::number(v, 'f', 1).replace('.', ','); }
}

int Animal::ageMois() const
{
    if (!naissance.isValid()) return 0;
    const QDate t = QDate::currentDate();
    int m = (t.year() - naissance.year()) * 12 + (t.month() - naissance.month());
    if (t.day() < naissance.day()) --m;
    return qMax(0, m);
}

QString Animal::categorieAge() const
{
    const SeuilsAge *s = seuilsPour(espece);
    if (!s) return QStringLiteral("Adulte");
    const int m = ageMois();
    if (m < s->finJeuneMois) return QString::fromUtf8(s->jeune);
    if (m >= s->seniorMois) return QStringLiteral("Senior");
    return QStringLiteral("Adulte");
}

// ------------------------------------------------------------ métier : alertes
QVector<AlerteAnimal> Animal::alertes(const QVector<Animal> &tous)
{
    QVector<AlerteAnimal> res;
    const auto moyennes = poidsMoyenParRace();          // 1 seule requête SQL pour toutes les races
    const QDate today = QDate::currentDate();

    for (const Animal &a : tous) {
        const int m = a.ageMois();

        // --- 1) catégorie d'âge : « bientôt senior » (6 mois avant) / déjà senior
        if (const SeuilsAge *s = seuilsPour(a.espece)) {
            if (m >= s->seniorMois - 6 && m < s->seniorMois) {
                res.append({ a.id, a.nom, "age", "warn",
                    QString("%1 (%2, %3) sera <b>senior dans %4 mois</b> : pensez au bilan de santé et à une alimentation adaptée.")
                        .arg(a.nom, a.espece, ageTexte(m)).arg(s->seniorMois - m), s->seniorMois - m });
            } else if (m >= s->seniorMois) {
                res.append({ a.id, a.nom, "age", "info",
                    QString("%1 est <b>senior</b> (%2) : bilan de santé annuel conseillé.").arg(a.nom, ageTexte(m)), 100 });
            }
        }

        // --- 2) poids : écart >= 30 % par rapport à la moyenne des adultes de la même race (>= 3 animaux)
        if (m >= 12 && a.poids > 0) {
            const QString cle = a.espece.toUpper() + "|" + a.race.trimmed().toUpper();
            const auto it = moyennes.constFind(cle);
            if (it != moyennes.constEnd() && it.value().second >= 3 && it.value().first > 0) {
                const double moy = it.value().first;
                const double ecart = (a.poids - moy) / moy;
                if (qAbs(ecart) >= 0.30) {
                    res.append({ a.id, a.nom, "poids", "warn",
                        QString("%1 pèse <b>%2 kg</b> alors que la moyenne des %3 (%4) est de %5 kg : <b>%6 %7 %</b>.")
                            .arg(a.nom, pct1(a.poids), a.race.trimmed(), a.espece, pct1(moy),
                                 ecart > 0 ? QStringLiteral("+") : QStringLiteral("-"), QString::number(qRound(qAbs(ecart) * 100))),
                        int(qAbs(ecart) * -1000) });
                }
            }
        }

        // --- 3) anniversaires du mois
        if (a.naissance.isValid() && a.naissance.month() == today.month()) {
            const QDate d(today.year(), today.month(), qMin(a.naissance.day(), today.daysInMonth()));
            const int ans = today.year() - a.naissance.year();
            QString t;
            if (ans <= 0)           t = QString("%1 est <b>né(e) ce mois-ci</b> (le %2).").arg(a.nom, d.toString("dd/MM"));
            else if (d == today)    t = QString("\U0001F382 %1 fête son anniversaire <b>aujourd'hui</b> : %2 an(s) !").arg(a.nom).arg(ans);
            else if (d < today)     t = QString("%1 a fêté ses %2 an(s) le %3.").arg(a.nom).arg(ans).arg(d.toString("dd/MM"));
            else                    t = QString("%1 fêtera ses %2 an(s) le <b>%3</b>.").arg(a.nom).arg(ans).arg(d.toString("dd/MM"));
            res.append({ a.id, a.nom, "anniv", "info", t, d.day() });
        }
    }

    auto rang = [](const QString &t) { return t == "age" ? 0 : t == "poids" ? 1 : 2; };
    std::stable_sort(res.begin(), res.end(), [&](const AlerteAnimal &x, const AlerteAnimal &y) {
        if (rang(x.type) != rang(y.type)) return rang(x.type) < rang(y.type);
        return x.tri < y.tri;
    });
    return res;
}

// ------------------------------------------------------------ fenêtre « Alertes »
AlertesDialog::AlertesDialog(const QVector<AlerteAnimal> &alertes, QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Alertes");
    setStyleSheet("QDialog { background:#EEF7F5; }");
    resize(640, 560);
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(16, 14, 16, 14);
    lay->setSpacing(10);

    auto *titre = new QLabel(QString("\U0001F514  Alertes (%1)").arg(alertes.size()));
    titre->setStyleSheet("font-size:16px; font-weight:800; color:#1F5F54;");
    lay->addWidget(titre);

    auto *area = new QScrollArea;
    area->setWidgetResizable(true);
    area->setFrameShape(QFrame::NoFrame);
    auto *content = new QWidget;
    auto *cl = new QVBoxLayout(content);
    cl->setSpacing(6);
    area->setWidget(content);
    lay->addWidget(area, 1);

    if (alertes.isEmpty()) {
        auto *vide = new QLabel("Aucune alerte pour le moment \U0001F389");
        vide->setStyleSheet("color:#5B706B; padding:20px;");
        cl->addWidget(vide);
    }
    const struct { const char *type; const char *titre; } groupes[] = {
        { "age",   "\u23F3  \u00c2ge et cat\u00e9gories" },
        { "poids", "\u2696  Poids \u00e0 surveiller" },
        { "anniv", "\U0001F382  Anniversaires du mois" },
    };
    for (const auto &g : groupes) {
        bool entete = false;
        for (const AlerteAnimal &a : alertes) {
            if (a.type != QLatin1String(g.type)) continue;
            if (!entete) {
                auto *h = new QLabel(QString::fromUtf8(g.titre));
                h->setStyleSheet("font-size:13px; font-weight:800; color:#1E2B30; padding-top:8px;");
                cl->addWidget(h);
                entete = true;
            }
            const bool warn = a.niveau == "warn";
            auto *l = new QLabel(a.texte);
            l->setWordWrap(true);
            l->setTextFormat(Qt::RichText);
            l->setStyleSheet(QString("background:%1; border:1px solid %2; border-radius:8px; padding:8px 10px; color:#243238;")
                                 .arg(warn ? "#FFF4DB" : "white", warn ? "#F0D28A" : "#DCEBE8"));
            cl->addWidget(l);
        }
    }
    cl->addStretch();

    auto *close = new QPushButton("Fermer");
    close->setObjectName("secondary");
    close->setCursor(Qt::PointingHandCursor);
    auto *hb = new QHBoxLayout;
    hb->addStretch();
    hb->addWidget(close);
    lay->addLayout(hb);
    connect(close, &QPushButton::clicked, this, &QDialog::accept);
}

// =====================================================================
// GÉNÉALOGIE (graphe des parents + consanguinité)
// =====================================================================
Pedigree::Pedigree(const QVector<Animal> &tous)
{
    for (const Animal &a : tous)
        m_index.insert(a.id, &a);
}

const Animal *Pedigree::trouver(const QString &id) const { return m_index.value(id, nullptr); }

QString Pedigree::pere(const QString &id) const
{
    const Animal *a = trouver(id);
    return a ? a->idPere : QString();
}

QString Pedigree::mere(const QString &id) const
{
    const Animal *a = trouver(id);
    return a ? a->idMere : QString();
}

// profondeur = longueur de la plus longue chaîne d'ancêtres connus (0 = aucun parent connu)
int Pedigree::profondeur(const QString &id) const
{
    if (id.isEmpty() || !m_index.contains(id)) return 0;
    auto it = m_profondeur.constFind(id);
    if (it != m_profondeur.constEnd()) return it.value();
    m_profondeur.insert(id, 0);                       // garde anti-boucle (données corrompues)
    const Animal *a = m_index.value(id);
    int d = 0;
    if (!a->idPere.isEmpty() || !a->idMere.isEmpty())
        d = 1 + qMax(profondeur(a->idPere), profondeur(a->idMere));
    m_profondeur[id] = d;
    return d;
}

int Pedigree::generationsConnues(const QString &id) const { return profondeur(id); }

// Coefficient de parenté a(x,y) (méthode tabulaire de Wright / Henderson, version récursive) :
//   a(x,x) = 1 + F(x)
//   a(x,y) = 1/2 * ( a(père(x), y) + a(mère(x), y) )   en développant toujours le plus "jeune" (profondeur max)
// Comme on remonte toujours vers des ancêtres de profondeur plus petite, la récursion se termine
// (le graphe est acyclique) et le cache évite de recalculer deux fois la même paire.
double Pedigree::parente(const QString &x, const QString &y, int garde) const
{
    if (x.isEmpty() || y.isEmpty() || garde > 200) return 0.0;
    if (x == y && !m_index.contains(x)) return 1.0;
    if (!m_index.contains(x) || !m_index.contains(y)) return 0.0;

    const QString cle = (x < y) ? x + QLatin1Char('|') + y : y + QLatin1Char('|') + x;
    auto it = m_parente.constFind(cle);
    if (it != m_parente.constEnd()) return it.value();

    double r;
    if (x == y)
        r = 1.0 + 0.5 * parente(pere(x), mere(x), garde + 1);               // 1 + F(x)
    else if (profondeur(x) >= profondeur(y))
        r = 0.5 * (parente(pere(x), y, garde + 1) + parente(mere(x), y, garde + 1));
    else
        r = 0.5 * (parente(x, pere(y), garde + 1) + parente(x, mere(y), garde + 1));

    m_parente.insert(cle, r);
    return r;
}

// F(x) = 1/2 * a(père, mère) : calculable seulement si les DEUX parents sont connus
double Pedigree::coefficient(const QString &id) const
{
    const Animal *a = trouver(id);
    if (!a || a->idPere.isEmpty() || a->idMere.isEmpty()) return 0.0;
    return 0.5 * parente(a->idPere, a->idMere);
}

double Pedigree::coefficientDescendant(const QString &idPere, const QString &idMere) const
{
    if (idPere.isEmpty() || idMere.isEmpty()) return 0.0;
    return 0.5 * parente(idPere, idMere);
}

bool Pedigree::estDescendantDe(const QString &id, const QString &ancetre) const
{
    if (ancetre.isEmpty()) return false;
    QSet<QString> vus;
    QVector<QString> pile = { pere(id), mere(id) };
    while (!pile.isEmpty()) {
        const QString c = pile.takeLast();
        if (c.isEmpty() || vus.contains(c)) continue;
        if (c == ancetre) return true;
        vus.insert(c);
        pile << pere(c) << mere(c);
    }
    return false;
}

QString Pedigree::niveau(double f)
{
    if (f < 0.0625) return QStringLiteral("Faible");
    if (f < 0.125)  return QStringLiteral("Mod\u00e9r\u00e9");
    if (f < 0.25)   return QStringLiteral("\u00c9lev\u00e9");
    return QStringLiteral("Critique");
}

QColor Pedigree::couleur(double f)
{
    if (f < 0.0625) return QColor("#2E9E6B");
    if (f < 0.125)  return QColor("#E0A800");
    if (f < 0.25)   return QColor("#E67E22");
    return QColor("#C0392B");
}

QString Pedigree::conseil(double f)
{
    if (f < 0.0625)
        return QStringLiteral("Apte \u00e0 la reproduction : peu de risque li\u00e9 \u00e0 la consanguinit\u00e9.");
    if (f < 0.125)
        return QStringLiteral("Reproduction possible, mais choisissez un partenaire non apparent\u00e9 (voir la simulation).");
    if (f < 0.25)
        return QStringLiteral("Reproduction d\u00e9conseill\u00e9e sans avis du v\u00e9t\u00e9rinaire : risque accru de maladies h\u00e9r\u00e9ditaires et de baisse de vitalit\u00e9.");
    return QStringLiteral("Reproduction fortement d\u00e9conseill\u00e9e : niveau \u00e9quivalent \u00e0 un petit issu de fr\u00e8re et s\u0153ur ou de parent \u00d7 enfant.");
}

// ancêtres d'un animal (lui-même inclus)
static QSet<QString> ancetresDe(const Pedigree &ped, const QString &id)
{
    QSet<QString> res;
    QVector<QString> pile = { id };
    while (!pile.isEmpty()) {
        const QString c = pile.takeLast();
        if (c.isEmpty() || res.contains(c)) continue;
        res.insert(c);
        pile << ped.pere(c) << ped.mere(c);
    }
    return res;
}

// ------------------------------------------------------------ contrôle de saisie : généalogie
QString Animal::controleGenealogie(const QVector<Animal> &tous) const
{
    if (idPere.isEmpty() && idMere.isEmpty()) return QString();
    if (!id.isEmpty() && (idPere == id || idMere == id))
        return "Un animal ne peut pas être son propre parent.";
    if (!idPere.isEmpty() && idPere == idMere)
        return "Le père et la mère doivent être deux animaux différents.";

    Pedigree ped(tous);
    struct Cas { const QString *pid; QString role; QString article; QString sexeAttendu; };
    const Cas cas[] = { { &idPere, "père", "Le", "Mâle" }, { &idMere, "mère", "La", "Femelle" } };
    for (const Cas &c : cas) {
        if (c.pid->isEmpty()) continue;
        const Animal *p = ped.trouver(*c.pid);
        if (!p) return QString("%1 %2 choisi(e) n'existe pas.").arg(c.article, c.role);
        if (p->sexe != c.sexeAttendu)
            return QString("%1 %2 (%3) doit être de sexe %4.").arg(c.article, c.role, p->nom, c.sexeAttendu);
        if (!espece.isEmpty() && p->espece != espece)
            return QString("%1 %2 (%3) doit être de la même espèce (%4).").arg(c.article, c.role, p->nom, espece);
        if (naissance.isValid() && p->naissance >= naissance)
            return QString("%1 %2 (%3) doit être né(e) avant l'animal.").arg(c.article, c.role, p->nom);
        if (!id.isEmpty() && ped.estDescendantDe(p->id, id))
            return QString("%1 descend déjà de cet animal : boucle généalogique impossible.").arg(p->nom);
    }
    return QString();
}

// ------------------------------------------------------------ barre de consanguinité
InbreedingBar::InbreedingBar(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(40);
}

void InbreedingBar::setValue(double fraction, bool valid)
{
    m_f = fraction;
    m_valid = valid;
    update();
}

void InbreedingBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    static const double seuils[5] = { 0.0, 0.0625, 0.125, 0.25, 0.5 };
    static const QColor zones[4] = { QColor("#2E9E6B"), QColor("#E0A800"), QColor("#E67E22"), QColor("#C0392B") };

    const QRectF bar(4, 14, width() - 8, 10);
    const double segW = bar.width() / 4.0;

    QPainterPath clip;
    clip.addRoundedRect(bar, 5, 5);
    p.save();
    p.setClipPath(clip);
    for (int i = 0; i < 4; ++i) {
        QColor c = m_valid ? zones[i] : QColor("#C9D6D3");
        c.setAlpha(m_valid ? 200 : 255);
        p.fillRect(QRectF(bar.left() + i * segW, bar.top(), segW + 1, bar.height()), c);
    }
    p.restore();

    // graduations
    QFont f = font();
    f.setPixelSize(9);
    p.setFont(f);
    p.setPen(QColor("#5B706B"));
    static const char *ticks[3] = { "6,25 %", "12,5 %", "25 %" };
    for (int i = 1; i <= 3; ++i) {
        const double x = bar.left() + i * segW;
        p.drawLine(QPointF(x, bar.bottom()), QPointF(x, bar.bottom() + 3));
        p.drawText(QRectF(x - 24, bar.bottom() + 3, 48, 12), Qt::AlignCenter, QString::fromUtf8(ticks[i - 1]));
    }

    if (!m_valid) {
        p.setPen(QColor("#4A5A5A"));
        p.drawText(QRectF(0, 0, width(), 13), Qt::AlignCenter, QStringLiteral("Taux non calculable (parents inconnus)"));
        return;
    }

    // repère (triangle) : position proportionnelle à l'intérieur de chaque zone
    const double v = qBound(0.0, m_f, 0.5);
    int seg = 3;
    for (int i = 0; i < 4; ++i)
        if (v < seuils[i + 1]) { seg = i; break; }
    const double frac = (v - seuils[seg]) / (seuils[seg + 1] - seuils[seg]);
    const double x = bar.left() + (seg + qBound(0.0, frac, 1.0)) * segW;
    QPainterPath tri;
    tri.moveTo(x - 5, 1);
    tri.lineTo(x + 5, 1);
    tri.lineTo(x, 12);
    tri.closeSubpath();
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#1E2B30"));
    p.drawPath(tri);
}

// ------------------------------------------------------------ fenêtre « Arbre généalogique »
PedigreeDialog::PedigreeDialog(const Animal &animal, const QVector<Animal> &tous, QWidget *parent)
    : QDialog(parent), m_animal(animal), m_tous(tous), m_ped(m_tous)
{
    setWindowTitle("Arbre généalogique — " + m_animal.nom);
    setStyleSheet("QDialog { background:#EEF7F5; }");
    resize(920, 780);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(16, 14, 16, 14);
    lay->setSpacing(10);

    auto *titre = new QLabel(QString("\U0001F333  Arbre généalogique de <b>%1</b> (%2)").arg(m_animal.nom.toHtmlEscaped(), m_animal.id));
    titre->setStyleSheet("font-size:16px; color:#1F5F54;");
    lay->addWidget(titre);

    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setStyleSheet("background:white; border:1px solid #DCEBE8; border-radius:10px;");
    lay->addWidget(m_view, 1);

    auto *legende = new QLabel("<span style='color:#4A90D9'>■</span> Mâle &nbsp;&nbsp; "
                               "<span style='color:#D9609A'>■</span> Femelle &nbsp;&nbsp; "
                               "<span style='color:#E67E22'>■</span> Ancêtre commun au père et à la mère (cause de la consanguinité)");
    legende->setStyleSheet("font-size:11px; color:#4A5A5A;");
    lay->addWidget(legende);

    // --- panneau consanguinité
    auto *card = new QFrame;
    card->setStyleSheet("QFrame#carte { background:white; border:1px solid #DCEBE8; border-radius:12px; }");
    card->setObjectName("carte");
    auto *cl = new QVBoxLayout(card);
    cl->setContentsMargins(16, 12, 16, 12);
    cl->setSpacing(6);

    auto *rowTop = new QHBoxLayout;
    auto *ct = new QLabel("TAUX DE CONSANGUINITÉ");
    ct->setStyleSheet("font-size:10px; font-weight:800; letter-spacing:1px; color:#2F7F73;");
    m_taux = new QLabel;
    rowTop->addWidget(ct);
    rowTop->addStretch();
    rowTop->addWidget(m_taux);
    cl->addLayout(rowTop);

    m_bar = new InbreedingBar;
    cl->addWidget(m_bar);

    m_verdict = new QLabel;
    m_verdict->setWordWrap(true);
    cl->addWidget(m_verdict);

    m_infos = new QLabel;
    m_infos->setWordWrap(true);
    m_infos->setStyleSheet("font-size:11px; color:#5B706B;");
    cl->addWidget(m_infos);

    // --- simulation d'accouplement
    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color:#DCEBE8;");
    cl->addWidget(sep);
    auto *rowSim = new QHBoxLayout;
    auto *st = new QLabel("<b>Simulation d'accouplement</b> — partenaire :");
    m_partenaire = new QComboBox;
    m_partenaire->addItem("— Choisir un partenaire —", QString());
    for (const Animal &b : m_tous) {
        if (b.id == m_animal.id || b.espece != m_animal.espece) continue;
        if ((b.sexe != "Mâle" && b.sexe != "Femelle") || b.sexe == m_animal.sexe) continue;
        m_partenaire->addItem(QString("%1 (%2)").arg(b.nom, b.id), b.id);
    }
    rowSim->addWidget(st);
    rowSim->addWidget(m_partenaire, 1);
    cl->addLayout(rowSim);
    m_simu = new QLabel;
    m_simu->setWordWrap(true);
    cl->addWidget(m_simu);
    lay->addWidget(card);

    auto *close = new QPushButton("Fermer");
    close->setObjectName("secondary");
    close->setCursor(Qt::PointingHandCursor);
    auto *hb = new QHBoxLayout;
    hb->addStretch();
    hb->addWidget(close);
    lay->addLayout(hb);
    connect(close, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_partenaire, &QComboBox::currentIndexChanged, this, [this] { majSimulation(); });

    // --- valeurs
    const bool valide = !m_animal.idPere.isEmpty() && !m_animal.idMere.isEmpty();
    const double f = m_ped.coefficient(m_animal.id);
    m_bar->setValue(f, valide);
    if (valide) {
        const QColor c = Pedigree::couleur(f);
        m_taux->setText(QString("<span style='font-size:24px; font-weight:800; color:%1'>%2 %</span>")
                            .arg(c.name(), QString::number(f * 100.0, 'f', 2).replace('.', ',')));
        m_verdict->setText(QString("<b style='color:%1'>Niveau : %2</b> — %3")
                               .arg(c.name(), Pedigree::niveau(f), Pedigree::conseil(f)));
    } else {
        m_taux->setText("<span style='font-size:24px; font-weight:800; color:#98A8A5'>—</span>");
        m_verdict->setText("Le père et la mère ne sont pas tous les deux renseignés : le taux ne peut pas être calculé. "
                           "Utilisez « Modifier » pour les indiquer.");
    }

    QString infos = QString("Générations d'ancêtres connues : <b>%1</b>").arg(m_ped.generationsConnues(m_animal.id));
    if (valide) {
        QSet<QString> communs = ancetresDe(m_ped, m_animal.idPere);
        communs.intersect(ancetresDe(m_ped, m_animal.idMere));
        QStringList noms;
        for (const QString &id : std::as_const(communs))
            if (const Animal *a = m_ped.trouver(id)) noms << a->nom;
        noms.sort();
        infos += noms.isEmpty() ? "  ·  Aucun ancêtre commun connu"
                                : "  ·  Ancêtre(s) commun(s) : <b>" + noms.join(", ").toHtmlEscaped() + "</b>";
    }
    if (m_ped.generationsConnues(m_animal.id) < 3)
        infos += "<br>⚠ Pedigree incomplet : seuls les ancêtres enregistrés sont pris en compte. "
                 "Un taux de 0 % ne garantit donc pas l'absence de consanguinité.";
    infos += "<br><i>Indicatif — la décision d'élevage revient au vétérinaire.</i>";
    m_infos->setText(infos);

    majSimulation();
    dessinerArbre();
}

void PedigreeDialog::majSimulation()
{
    const QString idP = m_partenaire->currentData().toString();
    if (idP.isEmpty()) {
        m_simu->setText("<span style='color:#5B706B'>Choisissez un partenaire pour estimer la consanguinité de leurs futurs petits.</span>");
        return;
    }
    const Animal *b = m_ped.trouver(idP);
    if (!b) return;
    const bool moiMale = (m_animal.sexe == "Mâle");
    const QString pere = moiMale ? m_animal.id : b->id;
    const QString mere = moiMale ? b->id : m_animal.id;
    const double f = m_ped.coefficientDescendant(pere, mere);
    const QColor c = Pedigree::couleur(f);
    m_simu->setText(QString("Petits de <b>%1</b> × <b>%2</b> : consanguinité estimée "
                            "<b style='color:%3'>%4 %</b> (%5)")
                        .arg(m_animal.nom.toHtmlEscaped(), b->nom.toHtmlEscaped(), c.name(),
                             QString::number(f * 100.0, 'f', 2).replace('.', ','), Pedigree::niveau(f)));
}

void PedigreeDialog::dessinerArbre()
{
    m_scene->clear();
    const int    G  = 4;                       // animal + 3 générations d'ancêtres
    const double W  = 150, H = 48, GX = 56, P = 62;
    const int    feuilles = 1 << (G - 1);

    // ids[g][k] : g = génération (0 = l'animal), k = position ; père = 2k, mère = 2k+1
    QVector<QVector<QString>> ids(G);
    ids[0] = { m_animal.id };
    for (int g = 1; g < G; ++g) {
        ids[g].resize(1 << g);
        for (int k = 0; k < (1 << g); ++k) {
            const QString enfant = ids[g - 1][k / 2];
            ids[g][k] = enfant.isEmpty() ? QString() : (k % 2 == 0 ? m_ped.pere(enfant) : m_ped.mere(enfant));
        }
    }

    // ancêtres communs au père et à la mère -> surlignés en orange
    QSet<QString> communs = ancetresDe(m_ped, m_animal.idPere);
    communs.intersect(ancetresDe(m_ped, m_animal.idMere));

    auto centre = [&](int g, int k) {
        const double span = double(feuilles >> g);
        return QPointF(g * (W + GX), (k + 0.5) * span * P);
    };

    // titres de colonnes
    const char *titres[4] = { "Animal", "Parents", "Grands-parents", "Arrière-grands-parents" };
    for (int g = 0; g < G; ++g) {
        auto *t = m_scene->addSimpleText(QString::fromUtf8(titres[g]));
        QFont f = t->font(); f.setPixelSize(11); f.setBold(true);
        t->setFont(f);
        t->setBrush(QColor("#2F7F73"));
        t->setPos(g * (W + GX), -30);
    }

    // liens (coudes) puis cases
    QPen lien(QColor("#9DB8B2"), 1.6);
    for (int g = 0; g < G - 1; ++g) {
        for (int k = 0; k < (1 << g); ++k) {
            if (ids[g][k].isEmpty()) continue;
            const QPointF c = centre(g, k);
            const QPointF cp = centre(g + 1, 2 * k), cm = centre(g + 1, 2 * k + 1);
            const double xm = c.x() + W + GX / 2.0;
            QPainterPath path;
            path.moveTo(c.x() + W, c.y());
            path.lineTo(xm, c.y());
            path.moveTo(xm, cp.y());
            path.lineTo(xm, cm.y());
            path.moveTo(xm, cp.y());
            path.lineTo(cp.x(), cp.y());
            path.moveTo(xm, cm.y());
            path.lineTo(cm.x(), cm.y());
            m_scene->addPath(path, lien);
        }
    }

    for (int g = 0; g < G; ++g) {
        for (int k = 0; k < (1 << g); ++k) {
            if (g > 0 && ids[g - 1][k / 2].isEmpty()) continue;     // pas de case sous un parent inconnu
            const QString id = ids[g][k];
            const Animal *a = id.isEmpty() ? nullptr : m_ped.trouver(id);
            const QPointF c = centre(g, k);
            const QRectF r(c.x(), c.y() - H / 2, W, H);

            QColor bord("#9AA9A6"), fond("#F3F6F5");
            Qt::PenStyle style = Qt::DashLine;
            double ep = 1.5;
            if (a) {
                style = Qt::SolidLine;
                const bool male = a->sexe == "Mâle";
                bord = male ? QColor("#4A90D9") : QColor("#D9609A");
                fond = male ? QColor("#E6F1FB") : QColor("#FCE9F2");
                ep = 2;
                if (g == 0) { fond = QColor("#CDE8E2"); bord = QColor("#2F7F73"); ep = 3; }
                if (g > 0 && communs.contains(id)) { bord = QColor("#E67E22"); fond = QColor("#FFF1DE"); ep = 3.2; }
            }
            QPainterPath box;
            box.addRoundedRect(r, 8, 8);
            m_scene->addPath(box, QPen(bord, ep, style), QBrush(fond));

            const QString nom = a ? a->nom : QStringLiteral("Inconnu");
            QFont fn; fn.setPixelSize(12); fn.setBold(true);
            auto *t1 = m_scene->addSimpleText(QFontMetrics(fn).elidedText(nom, Qt::ElideRight, int(W - 16)), fn);
            t1->setBrush(a ? QColor("#1E2B30") : QColor("#8A9996"));
            t1->setPos(r.left() + 8, r.top() + 7);
            if (a) {
                QFont fs; fs.setPixelSize(10);
                auto *t2 = m_scene->addSimpleText(QString("%1 · %2").arg(a->id, a->sexe == "Mâle" ? QStringLiteral("♂") : QStringLiteral("♀")), fs);
                t2->setBrush(QColor("#5B706B"));
                t2->setPos(r.left() + 8, r.top() + 26);
            }
        }
    }
    m_view->setSceneRect(m_scene->itemsBoundingRect().adjusted(-16, -12, 16, 12));
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
