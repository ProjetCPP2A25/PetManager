#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connexion.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QMarginsF>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPainterPath>
#include <QPdfWriter>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTableWidget>
#include <QTextDocument>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <algorithm>

// ============================================================ helpers
static const char *STATUT_OK = "En bonne sant\u00e9";

// âge en mois (jamais négatif)
static int ageMonths(const QDate &d)
{
    const QDate t = QDate::currentDate();
    int m = (t.year() - d.year()) * 12 + t.month() - d.month();
    if (t.day() < d.day()) --m;
    return qMax(0, m);
}

bool MainWindow::estEnBonneSante(const Animal &a)
{
    return a.statut == QString::fromUtf8(STATUT_OK);
}

QString MainWindow::ageText(const QDate &d)
{
    QDate t = QDate::currentDate();
    int months = (t.year() - d.year()) * 12 + t.month() - d.month();
    if (t.day() < d.day()) months--;
    if (months < 12) return QString("%1 mois").arg(qMax(0, months));
    int y = months / 12;
    const QString plural = y > 1 ? "s" : "";
    return QString("%1 an%2").arg(y).arg(plural);
}

QPixmap MainWindow::avatar(const Animal &a, int size)
{
    QPixmap out(size, size);
    out.fill(Qt::transparent);
    QPainter p(&out);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath clip;
    clip.addRoundedRect(0, 0, size, size, size * 0.22, size * 0.22);
    p.setClipPath(clip);
    QPixmap src;
    if (!a.photo.isEmpty() && src.load(a.photo)) {
        QPixmap s = src.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        p.drawPixmap((size - s.width()) / 2, (size - s.height()) / 2, s);
    } else {
        QColor c = a.espece == "Chien" ? QColor("#F3D9B1")
                   : a.espece == "Chat"  ? QColor("#D5D9DE") : QColor("#E7DDF2");
        p.fillRect(0, 0, size, size, c);
        QFont f = p.font();
        f.setPixelSize(int(size * 0.55));
        p.setFont(f);
        QString e = a.espece == "Chien" ? QStringLiteral("\U0001F436")
                    : a.espece == "Chat"  ? QStringLiteral("\U0001F431") : QStringLiteral("\U0001F430");
        p.drawText(QRect(0, 0, size, size), Qt::AlignCenter, e);
    }
    return out;
}

QLabel *MainWindow::badge(const QString &text, const QString &bg, const QString &fg)
{
    auto *l = new QLabel(text);
    l->setStyleSheet(QString("background:%1; color:%2; border-radius:9px;"
                             "padding:3px 10px; font-size:11px; font-weight:600;")
                     .arg(bg, fg));
    return l;
}

QWidget *MainWindow::wrap(QWidget *w, Qt::Alignment al)
{
    auto *c = new QWidget;
    auto *l = new QHBoxLayout(c);
    l->setContentsMargins(4, 0, 4, 0);
    l->addWidget(w, 0, al);
    return c;
}

// ============================================================ destructeur
MainWindow::~MainWindow()
{
    delete ui;
}

// ============================================================ constructeur
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Pet Manager - Gestion des animaux");
    resize(1500, 950);
    setMinimumSize(1200, 800);
    if (!Connexion::instance().ouvrir())
        QMessageBox::critical(this, "Base de donn\u00e9es",
            "Connexion Oracle impossible :\n" + Connexion::instance().derniereErreur());
    chargerAnimaux();

    auto *central = new QWidget;
    central->setObjectName("central");
    auto *root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(buildSidebar());
    root->addWidget(buildCenter(), 1);
    root->addWidget(buildDetail());
    setCentralWidget(central);
    statusBar()->setSizeGripEnabled(false);

    applyStyle();
    refreshTable();
    updateDashboard();
    if (!m_animals.isEmpty()) {
        m_table->selectRow(0);
        showDetail(0);
    } else {
        clearDetail();
    }
}

// ============================================================ READ depuis Oracle
void MainWindow::chargerAnimaux()
{
    m_animals = Animal::afficher();        // la requête SELECT est dans la classe Animal
    if (m_animals.isEmpty() && !Animal::derniereErreur().isEmpty()
        && Connexion::instance().db().isOpen())
        QMessageBox::warning(this, "Lecture des animaux", Animal::derniereErreur());
}

// ============================================================ barre latérale
QWidget *MainWindow::buildSidebar()
{
    auto *side = new QFrame;
    side->setObjectName("sidebar");
    side->setFixedWidth(215);
    auto *lay = new QVBoxLayout(side);
    lay->setContentsMargins(14, 16, 14, 16);
    lay->setSpacing(6);
    auto *logo = new QLabel("\U0001F43E  <span style='color:#2F7F73'>PET</span> "
                            "<span style='color:#2F7F73'>MANAGER</span>");
    logo->setObjectName("logo");
    lay->addWidget(logo);
    lay->addSpacing(26);
    auto *grp = new QButtonGroup(side);
    grp->setExclusive(true);
    const QStringList items = {"Accueil", "Animaux", "Rendez-vous", "Stock",
                               "Commandes", "Fournisseurs", "Employés", "Paramètres"};
    for (int i = 0; i < items.size(); ++i) {
        auto *b = new QPushButton(items[i]);
        b->setObjectName("nav");
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        b->setChecked(i == 1);
        grp->addButton(b);
        lay->addWidget(b);
    }
    lay->addStretch();
    auto *foot = new QLabel("Des animaux en bonne santé,\ndes maîtres plus heureux ! \U0001F43E");
    foot->setObjectName("footer");
    foot->setAlignment(Qt::AlignCenter);
    foot->setWordWrap(true);
    lay->addWidget(foot);
    return side;
}

QWidget *MainWindow::buildAIAskWidget()
{
    auto *box = new QFrame;
    box->setObjectName("aiCard");
    auto *lay = new QVBoxLayout(box);
    lay->setContentsMargins(14, 10, 14, 10);
    lay->setSpacing(8);
    auto *top = new QHBoxLayout;
    auto *lblIcon = new QLabel("\u2728  <b>PetAI Ask</b> - Assistant Interactif");
    lblIcon->setStyleSheet("font-size: 13px; color: #1F5F54;");
    top->addWidget(lblIcon);
    top->addStretch();
    m_aiInput = new QLineEdit;
    m_aiInput->setPlaceholderText("\U0001F4A1 Posez une question (ex: 'Qui est sous surveillance ?', 'Poids moyen des chiens ?')...");
    m_aiInput->setMinimumHeight(36);
    auto *btnAsk = new QPushButton("Demander");
    btnAsk->setObjectName("primary");
    btnAsk->setFixedHeight(36);
    btnAsk->setCursor(Qt::PointingHandCursor);
    auto *inputLay = new QHBoxLayout;
    inputLay->addWidget(m_aiInput, 1);
    inputLay->addWidget(btnAsk);
    m_aiResponseLbl = new QLabel("\U0001F4AC Posez une question \u00e0 l'IA pour obtenir une analyse en temps r\u00e9el sur la base de donn\u00e9es.");
    m_aiResponseLbl->setStyleSheet("color: #4A5A5A; font-size: 12px; font-style: italic;");
    m_aiResponseLbl->setWordWrap(true);
    lay->addLayout(top);
    lay->addLayout(inputLay);
    lay->addWidget(m_aiResponseLbl);
    connect(btnAsk, &QPushButton::clicked, this, &MainWindow::traiterQuestionIA);
    connect(m_aiInput, &QLineEdit::returnPressed, this, &MainWindow::traiterQuestionIA);
    return box;
}

void MainWindow::traiterQuestionIA()
{
    QString query = m_aiInput->text().trimmed().toLower();
    if (query.isEmpty()) return;
    if (query.contains("surveillance") || query.contains("malade") || query.contains("alerte")) {
        QStringList res;
        for (const auto &a : m_animals) {
            if (a.statut != "En bonne sant\u00e9") res.append(QString("<b>%1</b> (%2)").arg(a.nom, a.espece));
        }
        if (res.isEmpty()) m_aiResponseLbl->setText("\U0001F916 <b>PetAI</b> : Tous les animaux sont actuellement en bonne sant\u00e9 !");
        else m_aiResponseLbl->setText("\U0001F916 <b>PetAI</b> : Animal(aux) n\u00e9cessitant un suivi : " + res.join(", "));
    }
    else if (query.contains("poids") || query.contains("moyenne")) {
        double sum = 0;
        for (const auto &a : m_animals) sum += a.poids;
        double avg = m_animals.isEmpty() ? 0 : sum / m_animals.size();
        m_aiResponseLbl->setText(QString("\U0001F916 <b>PetAI</b> : Le poids moyen de l'ensemble du cheptel est de <b>%1 kg</b>.").arg(avg, 0, 'f', 1));
    }
    else if (query.contains("\u00e2g\u00e9") || query.contains("doyen") || query.contains("plus grand")) {
        if (m_animals.isEmpty()) return;
        Animal doyen = m_animals[0];
        for (const auto &a : m_animals) {
            if (a.naissance < doyen.naissance) doyen = a;
        }
        m_aiResponseLbl->setText(QString("\U0001F916 <b>PetAI</b> : L'animal le plus \u00e2g\u00e9 est <b>%1</b> (%2, n\u00e9 le %3).")
                                 .arg(doyen.nom, doyen.espece, doyen.naissance.toString("dd/MM/yyyy")));
    }
    else {
        m_aiResponseLbl->setText(QString("\U0001F916 <b>PetAI</b> : Analyse de la requ\u00eate '%1' termin\u00e9e. %2 animal(aux) actuellement enregistr\u00e9s.")
                                 .arg(m_aiInput->text(), QString::number(m_animals.size())));
    }
}

// ============================================================ STATISTIQUES : cartes KPI
ClickableFrame *MainWindow::buildKpiCard(const QString &title, const QString &icon, const QString &color,
                                         QLabel *&valLbl, QLabel *&subLbl)
{
    auto *card = new ClickableFrame;
    card->setObjectName("kpi");
    auto *l = new QVBoxLayout(card);
    l->setContentsMargins(16, 12, 16, 12);
    l->setSpacing(2);
    auto *top = new QHBoxLayout;
    auto *t = new QLabel(title);
    t->setStyleSheet("font-size:10px; font-weight:800; letter-spacing:1px; color:#6A7B80;");
    auto *ic = new QLabel(icon);
    ic->setStyleSheet("font-size:16px;");
    top->addWidget(t);
    top->addStretch();
    top->addWidget(ic);
    valLbl = new QLabel("0");
    valLbl->setStyleSheet(QString("font-size:28px; font-weight:900; color:%1;").arg(color));
    subLbl = new QLabel;
    subLbl->setStyleSheet("font-size:11px; color:#5B706B;");
    subLbl->setWordWrap(true);
    l->addLayout(top);
    l->addWidget(valLbl);
    l->addWidget(subLbl);
    return card;
}

// ============================================================ STATISTIQUES : panneau principal
QWidget *MainWindow::buildStatsPanel()
{
    auto *panel = new QWidget;
    auto *lay = new QVBoxLayout(panel);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(14);

    // ---- titre + export PDF
    auto *head = new QHBoxLayout;
    auto *title = new QLabel("\U0001F4CA  Tableau de bord statistique");
    title->setObjectName("sectionTitle");
    auto *btnPdf = new QPushButton("\u2B07  Exporter le rapport PDF");
    btnPdf->setObjectName("secondary");
    btnPdf->setCursor(Qt::PointingHandCursor);
    head->addWidget(title);
    head->addStretch();
    head->addWidget(btnPdf);
    lay->addLayout(head);

    // ---- 4 cartes KPI (certaines sont cliquables : elles ouvrent la liste filtrée)
    auto *k1 = buildKpiCard("TOTAL ANIMAUX", "\U0001F43E", "#2F7F73", m_kpiTotal, m_kpiTotalSub);
    auto *k2 = buildKpiCard("\u00c2GE MOYEN", "\U0001F382", "#2563EB", m_kpiAge, m_kpiAgeSub);
    auto *k3 = buildKpiCard("SOUS SURVEILLANCE", "\u26A0", "#C0392B", m_kpiAlert, m_kpiAlertSub);
    auto *k4 = buildKpiCard("FEMELLES / M\u00c2LES", "\u26A5", "#9333EA", m_kpiSex, m_kpiSexSub);
    k4->setObjectName("kpiStatic");          // carte non cliquable
    k4->setCursor(Qt::ArrowCursor);
    auto *kpis = new QHBoxLayout;
    kpis->setSpacing(12);
    for (ClickableFrame *k : {k1, k2, k3, k4}) kpis->addWidget(k, 1);
    lay->addLayout(kpis);

    connect(k1, &ClickableFrame::clicked, this, [this] {
        resetFilters();
        showPage(0);
    });
    connect(k2, &ClickableFrame::clicked, this, [this] {            // -> va sur le doyen
        if (m_animals.isEmpty()) return;
        int old = 0;
        for (int i = 1; i < m_animals.size(); ++i)
            if (m_animals[i].naissance < m_animals[old].naissance) old = i;
        resetFilters();
        showPage(0);
        m_table->selectRow(old);
        showDetail(old);
        m_table->scrollToItem(m_table->item(old, 0));
    });
    connect(k3, &ClickableFrame::clicked, this, [this] { filtrerParStatut(2); });

    // ---- graphique (cercle / courbe) + jauge de santé
    auto *chartCard = new QFrame;
    chartCard->setObjectName("panelCard");
    auto *cl = new QVBoxLayout(chartCard);
    cl->setContentsMargins(18, 14, 18, 12);
    cl->setSpacing(8);
    auto *ch = new QHBoxLayout;
    m_chartTitle = new QLabel("R\u00e9partition par esp\u00e8ce");
    m_chartTitle->setObjectName("cardTitle");
    auto *segCircle = new QPushButton("Cercle");
    auto *segCurve = new QPushButton("Courbe");
    for (QPushButton *b : {segCircle, segCurve}) {
        b->setObjectName("seg");
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
    }
    segCircle->setChecked(true);
    auto *grp = new QButtonGroup(chartCard);
    grp->setExclusive(true);
    grp->addButton(segCircle, 0);
    grp->addButton(segCurve, 1);
    ch->addWidget(m_chartTitle);
    ch->addStretch();
    ch->addWidget(segCircle);
    ch->addWidget(segCurve);
    cl->addLayout(ch);
    m_chartStack = new QStackedWidget;
    m_donut = new DonutChart;
    m_line = new LineChart;
    m_chartStack->addWidget(m_donut);
    m_chartStack->addWidget(m_line);
    cl->addWidget(m_chartStack, 1);
    connect(grp, &QButtonGroup::idClicked, this, [this](int id) {
        m_chartStack->setCurrentIndex(id);
        m_chartTitle->setText(id == 0 ? "R\u00e9partition par esp\u00e8ce" : "Naissances par ann\u00e9e");
    });
    connect(m_donut, &DonutChart::sliceClicked, this, &MainWindow::filtrerParEspece);

    auto *gaugeCard = new QFrame;
    gaugeCard->setObjectName("panelCard");
    auto *gl = new QVBoxLayout(gaugeCard);
    gl->setContentsMargins(18, 14, 18, 14);
    gl->setSpacing(6);
    auto *gt = new QLabel("Score de sant\u00e9 du cheptel");
    gt->setObjectName("cardTitle");
    m_gauge = new GaugeWidget;
    m_gaugeCaption = new QLabel;
    m_gaugeCaption->setAlignment(Qt::AlignCenter);
    m_gaugeCaption->setStyleSheet("font-size:12px; color:#5B706B;");
    gl->addWidget(gt);
    gl->addWidget(m_gauge, 1);
    gl->addWidget(m_gaugeCaption);

    auto *mid = new QHBoxLayout;
    mid->setSpacing(12);
    mid->addWidget(chartCard, 3);
    mid->addWidget(gaugeCard, 2);
    lay->addLayout(mid);

    // ---- analyses intelligentes
    auto *insCard = new QFrame;
    insCard->setObjectName("panelCard");
    auto *il = new QVBoxLayout(insCard);
    il->setContentsMargins(18, 14, 18, 14);
    il->setSpacing(4);
    auto *it = new QLabel("\U0001F4A1  Analyses intelligentes");
    it->setObjectName("cardTitle");
    m_insights = new QLabel;
    m_insights->setTextFormat(Qt::RichText);
    m_insights->setWordWrap(true);
    m_insights->setStyleSheet("font-size:12px; color:#33454B;");
    il->addWidget(it);
    il->addWidget(m_insights);
    lay->addWidget(insCard);

    connect(btnPdf, &QPushButton::clicked, this, &MainWindow::exporterRapportPdf);
    return panel;
}

QWidget *MainWindow::buildStatsPage()
{
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea { background: transparent; }");
    scroll->viewport()->setAutoFillBackground(false);
    auto *container = new QWidget;
    container->setAutoFillBackground(false);
    auto *lay = new QVBoxLayout(container);
    lay->setContentsMargins(0, 0, 4, 10);
    lay->setSpacing(14);
    lay->addWidget(buildStatsPanel());
    lay->addWidget(buildAIAskWidget());
    lay->addStretch();
    scroll->setWidget(container);
    return scroll;
}

// ============================================================ STATISTIQUES : analyses automatiques
QVector<Insight> MainWindow::buildInsights() const
{
    QVector<Insight> out;
    const int total = m_animals.size();
    if (total == 0) return out;
    auto add = [&out](const QString &level, const QString &html) { out.append(Insight{level, html}); };
    auto plural = [](const QString &sp, int n) { return sp.toLower() + (n > 1 ? "s" : ""); };

    QMap<QString, int> cnt;
    QMap<QString, double> sumW;
    QStringList surveillance;
    int seniors = 0, femelles = 0, iOld = 0, iYoung = 0, iHeavy = 0;
    double totalW = 0;
    for (int i = 0; i < total; ++i) {
        const Animal &a = m_animals[i];
        cnt[a.espece] += 1;
        sumW[a.espece] += a.poids;
        totalW += a.poids;
        if (!estEnBonneSante(a)) surveillance << "<b>" + a.nom.toHtmlEscaped() + "</b>";
        if (ageMonths(a.naissance) >= 84) ++seniors;
        if (a.sexe == "Femelle") ++femelles;
        if (a.naissance < m_animals[iOld].naissance) iOld = i;
        if (a.naissance > m_animals[iYoung].naissance) iYoung = i;
        if (a.poids > m_animals[iHeavy].poids) iHeavy = i;
    }

    // 1) espèce dominante
    QString best;
    int bestN = 0;
    for (auto it = cnt.constBegin(); it != cnt.constEnd(); ++it)
        if (it.value() > bestN) { bestN = it.value(); best = it.key(); }
    add("info", QString("Esp\u00e8ce dominante : <b>%1</b>, soit %2 animal(aux) sur %3 (%4&nbsp;%).")
                    .arg(plural(best, 2)).arg(bestN).arg(total).arg(qRound(100.0 * bestN / total)));

    // 2) doyen / plus jeune
    const Animal &old = m_animals[iOld];
    QString t = QString("Doyen : <b>%1</b> (%2, %3)")
                    .arg(old.nom.toHtmlEscaped(), old.espece, ageText(old.naissance));
    if (total > 1) {
        const Animal &young = m_animals[iYoung];
        t += QString(" &middot; Plus jeune : <b>%1</b> (%2, %3)")
                 .arg(young.nom.toHtmlEscaped(), young.espece, ageText(young.naissance));
    }
    add("info", t + ".");

    // 3) poids le plus élevé
    const Animal &heavy = m_animals[iHeavy];
    add("info", QString("Poids le plus \u00e9lev\u00e9 : <b>%1</b> (%2 kg) &middot; moyenne g\u00e9n\u00e9rale : %3 kg.")
                    .arg(heavy.nom.toHtmlEscaped()).arg(heavy.poids, 0, 'f', 1).arg(totalW / total, 0, 'f', 1));

    // 4) écarts de poids par rapport à la moyenne de l'espèce (+25 %)
    int flagged = 0;
    for (const Animal &a : m_animals) {
        const int n = cnt.value(a.espece);
        if (n < 2 || flagged >= 3) continue;
        const double avg = sumW.value(a.espece) / n;
        if (avg > 0 && a.poids > avg * 1.25) {
            ++flagged;
            add("warn", QString("\u00c9cart de poids notable : <b>%1</b> p\u00e8se %2 kg, soit +%3&nbsp;%"
                                " par rapport \u00e0 la moyenne des %4 (%5 kg).")
                            .arg(a.nom.toHtmlEscaped()).arg(a.poids, 0, 'f', 1)
                            .arg(qRound((a.poids / avg - 1.0) * 100.0))
                            .arg(plural(a.espece, 2)).arg(avg, 0, 'f', 1));
        }
    }

    // 5) santé
    if (!surveillance.isEmpty())
        add("warn", QString("%1 animal(aux) sous surveillance : %2.")
                        .arg(surveillance.size()).arg(surveillance.join(", ")));
    else
        add("ok", "Aucun animal sous surveillance : le cheptel est en bonne sant\u00e9.");

    // 6) seniors
    if (seniors > 0)
        add("warn", QString("%1 animal(aux) ont 7 ans ou plus : un bilan g\u00e9riatrique est recommand\u00e9.").arg(seniors));

    // 7) sexe
    add("info", QString("R\u00e9partition par sexe : <b>%1</b> femelle(s) et <b>%2</b> m\u00e2le(s).")
                    .arg(femelles).arg(total - femelles));
    return out;
}

// ============================================================ STATISTIQUES : mise à jour de tout le tableau de bord
void MainWindow::updateDashboard()
{
    const int total = m_animals.size();
    const QDate today = QDate::currentDate();

    QMap<QString, int> bySpecies;
    QMap<int, int> byYear;
    int alertes = 0, femelles = 0, sumMonths = 0, iOld = 0;
    double sumW = 0;
    for (int i = 0; i < total; ++i) {
        const Animal &a = m_animals[i];
        bySpecies[a.espece] += 1;
        byYear[a.naissance.year()] += 1;
        if (!estEnBonneSante(a)) ++alertes;
        if (a.sexe == "Femelle") ++femelles;
        sumMonths += ageMonths(a.naissance);
        sumW += a.poids;
        if (a.naissance < m_animals[iOld].naissance) iOld = i;
    }

    // ---- cartes KPI
    m_kpiTotal->setText(QString::number(total));
    m_kpiAlert->setText(QString::number(alertes));
    m_kpiAlert->setStyleSheet(QString("font-size:28px; font-weight:900; color:%1;")
                                  .arg(alertes > 0 ? "#C0392B" : "#2E9E6B"));
    if (total == 0) {
        m_kpiTotalSub->setText("Aucun animal enregistr\u00e9");
        m_kpiAge->setText("\u2014");
        m_kpiAgeSub->setText("");
        m_kpiAlertSub->setText("");
        m_kpiSex->setText("\u2014");
        m_kpiSexSub->setText("");
    } else {
        m_kpiTotalSub->setText(QString("Poids moyen : %1 kg \u00b7 cliquer pour tout voir").arg(sumW / total, 0, 'f', 1));
        m_kpiAge->setText(QString::number(sumMonths / 12.0 / total, 'f', 1) + " ans");
        m_kpiAgeSub->setText(QString("Doyen : %1 (%2) \u00b7 cliquer pour l'ouvrir")
                                 .arg(m_animals[iOld].nom, ageText(m_animals[iOld].naissance)));
        m_kpiAlertSub->setText(alertes > 0
                                   ? QString::number(qRound(100.0 * alertes / total)) + " % du cheptel \u00b7 cliquer pour filtrer"
                                   : "Tout va bien");
        m_kpiSex->setText(QString("%1 \u2640 \u00b7 %2 \u2642").arg(femelles).arg(total - femelles));
        m_kpiSexSub->setText(QString::number(qRound(100.0 * femelles / total)) + " % de femelles");
    }

    // ---- cercle : répartition par espèce
    QVector<ChartSlice> slices;
    slices.append(ChartSlice{"Chien", "Chiens", double(bySpecies.value("Chien")), QColor("#D97706")});
    slices.append(ChartSlice{"Chat", "Chats", double(bySpecies.value("Chat")), QColor("#2563EB")});
    slices.append(ChartSlice{"Lapin", "Lapins", double(bySpecies.value("Lapin")), QColor("#9333EA")});
    m_donut->setData(slices, total > 1 ? "animaux" : "animal");

    // ---- courbe : naissances par année (les années sans naissance comptent 0)
    QVector<ChartPoint> pts;
    if (!byYear.isEmpty()) {
        const int y0 = byYear.firstKey();
        const int y1 = qMax(byYear.lastKey(), today.year());
        for (int y = y0; y <= y1; ++y)
            pts.append(ChartPoint{QString::number(y), double(byYear.value(y, 0))});
    }
    m_line->setData(pts);

    // ---- jauge : score de santé = % d'animaux en bonne santé
    if (total == 0) {
        m_gauge->setValue(0, false);
        m_gaugeCaption->setText("Aucune donn\u00e9e");
    } else {
        m_gauge->setValue(qRound(100.0 * (total - alertes) / total), true);
        m_gaugeCaption->setText(QString("%1 sur %2 en bonne sant\u00e9").arg(total - alertes).arg(total));
    }

    // ---- analyses intelligentes
    const QVector<Insight> ins = buildInsights();
    if (ins.isEmpty()) {
        m_insights->setText("<i>Ajoutez des animaux pour obtenir des analyses automatiques.</i>");
    } else {
        QString html;
        for (const Insight &in : ins) {
            const QString col = in.level == "warn" ? "#D97706" : in.level == "ok" ? "#2E9E6B" : "#2563EB";
            html += "<p style='margin:5px 0;'><span style='color:" + col + ";'>&#9679;</span>&nbsp; " + in.html + "</p>";
        }
        m_insights->setText(html);
    }
}

// ============================================================ STATISTIQUES : actions de navigation
void MainWindow::filtrerParEspece(const QString &espece)
{
    {
        QSignalBlocker b1(m_search);
        QSignalBlocker b2(m_filterStatut);
        m_search->clear();
        m_filterStatut->setCurrentIndex(0);
    }
    const int idx = m_filterEspece->findText(espece);
    m_filterEspece->setCurrentIndex(idx >= 0 ? idx : 0);   // déclenche applyFilter()
    applyFilter();
    showPage(0);
    statusBar()->showMessage("Liste filtr\u00e9e : " + espece, 4000);
}

void MainWindow::filtrerParStatut(int index)
{
    {
        QSignalBlocker b1(m_search);
        QSignalBlocker b2(m_filterEspece);
        m_search->clear();
        m_filterEspece->setCurrentIndex(0);
    }
    m_filterStatut->setCurrentIndex(index);                // déclenche applyFilter()
    applyFilter();
    showPage(0);
    statusBar()->showMessage(index == 2 ? "Liste filtr\u00e9e : animaux sous surveillance"
                                        : "Filtres r\u00e9initialis\u00e9s", 4000);
}

// ============================================================ STATISTIQUES : export du rapport PDF
void MainWindow::exporterRapportPdf()
{
    if (m_animals.isEmpty()) {
        QMessageBox::information(this, "Rapport", "Ajoutez au moins un animal pour g\u00e9n\u00e9rer un rapport.");
        return;
    }
    const QString def = QDir::homePath() + "/rapport_animaux_"
                        + QDate::currentDate().toString("yyyyMMdd") + ".pdf";
    const QString path = QFileDialog::getSaveFileName(this, "Exporter le rapport statistique", def,
                                                      "Fichier PDF (*.pdf)");
    if (path.isEmpty()) return;

    const int total = m_animals.size();
    int alertes = 0, femelles = 0;
    double sumW = 0;
    for (const Animal &a : m_animals) {
        if (!estEnBonneSante(a)) ++alertes;
        if (a.sexe == "Femelle") ++femelles;
        sumW += a.poids;
    }
    const int score = qRound(100.0 * (total - alertes) / total);

    QTextDocument doc;

    auto kpi = [](const QString &label, const QString &value, const QString &color) {
        return "<td bgcolor='#EAF6F3' align='center'><span style='font-size:8pt; color:#6A7B80;'>" + label
               + "</span><br><b style='font-size:18pt; color:" + color + ";'>" + value + "</b></td>";
    };

    QString h;
    h += "<html><body style=\"font-family:'Segoe UI',Arial,sans-serif; color:#243238; font-size:10pt;\">";
    h += "<h1 style='color:#1F5F54; font-size:20pt; margin-bottom:0;'>Pet Manager &mdash; Rapport statistique</h1>";
    h += "<p style='color:#6A7B80; margin-top:2px;'>G\u00e9n\u00e9r\u00e9 le "
         + QDateTime::currentDateTime().toString("dd/MM/yyyy 'à' HH:mm") + "</p>";

    h += "<table width='100%' cellspacing='6' cellpadding='8'><tr>";
    h += kpi("TOTAL ANIMAUX", QString::number(total), "#2F7F73");
    h += kpi("POIDS MOYEN", QString::number(sumW / total, 'f', 1) + " kg", "#2563EB");
    h += kpi("SOUS SURVEILLANCE", QString::number(alertes), alertes > 0 ? "#C0392B" : "#2E9E6B");
    h += kpi("SCORE DE SANT\u00c9", QString::number(score) + " %", score >= 80 ? "#2E9E6B" : score >= 50 ? "#D97706" : "#C0392B");
    h += "</tr></table>";

    h += "<h2 style='color:#1F5F54;'>R\u00e9partition par esp\u00e8ce</h2>";
    h += "<p><img src='donut.png' width='520'></p>";
    h += "<h2 style='color:#1F5F54;'>Naissances par ann\u00e9e</h2>";
    h += "<p><img src='line.png' width='520'></p>";

    h += "<h2 style='color:#1F5F54;'>Analyses intelligentes</h2><ul>";
    for (const Insight &in : buildInsights()) h += "<li>" + in.html + "</li>";
    h += "</ul>";

    h += "<h2 style='color:#1F5F54;'>Liste des animaux</h2>";
    h += "<table width='100%' cellspacing='0' cellpadding='5'>";
    h += "<tr bgcolor='#2F7F73'>";
    for (const QString &c : {"ID", "Nom", "Esp\u00e8ce", "Race", "Sexe", "\u00c2ge", "Poids", "Statut"})
        h += "<td><b><font color='white'>" + c + "</font></b></td>";
    h += "</tr>";
    int row = 0;
    for (const Animal &a : m_animals) {
        h += QString("<tr bgcolor='%1'>").arg(row++ % 2 ? "#F4FAF9" : "#FFFFFF");
        h += "<td>" + a.id + "</td><td><b>" + a.nom.toHtmlEscaped() + "</b></td><td>" + a.espece + "</td><td>"
             + a.race.toHtmlEscaped() + "</td><td>" + a.sexe + "</td><td>" + ageText(a.naissance) + "</td><td>"
             + QString::number(a.poids, 'f', 1) + " kg</td><td>"
             + (estEnBonneSante(a) ? "<font color='#2E7D52'>" : "<font color='#C0392B'>") + a.statut + "</font></td></tr>";
    }
    h += "</table></body></html>";
    doc.setHtml(h);
    doc.addResource(QTextDocument::ImageResource, QUrl("donut.png"),
                    m_donut->snapshot(QSize(560, 250)).toImage());
    doc.addResource(QTextDocument::ImageResource, QUrl("line.png"),
                    m_line->snapshot(QSize(560, 250)).toImage());

    {
        QPdfWriter pdf(path);
        pdf.setPageSize(QPageSize(QPageSize::A4));
        pdf.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
        pdf.setTitle("Rapport statistique - Pet Manager");
        pdf.setCreator("Pet Manager");
        doc.print(&pdf);
    }                                                      // le fichier est écrit ici
    statusBar()->showMessage("Rapport export\u00e9 : " + path, 6000);
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}


// ============================================================ navigation entre les pages
void MainWindow::showPage(int index)
{
    m_pages->setCurrentIndex(index);
    (index == 0 ? m_tabList : m_tabStats)->setChecked(true);
}

// ============================================================ centre : en-tête + 2 pages
QWidget *MainWindow::buildCenter()
{
    auto *wrapper = new QWidget;
    auto *root = new QVBoxLayout(wrapper);
    root->setContentsMargins(20, 16, 20, 10);
    root->setSpacing(12);

    // ---- en-tête
    auto *head = new QHBoxLayout;
    auto *icon = new QLabel("\U0001F43E");
    icon->setObjectName("headIcon");
    icon->setFixedSize(48, 48);
    icon->setAlignment(Qt::AlignCenter);
    auto *titles = new QVBoxLayout;
    auto *title = new QLabel("Gestion des animaux");
    title->setObjectName("title");
    auto *sub = new QLabel("G\u00e9rez vos animaux et suivez leurs statistiques en temps r\u00e9el");
    sub->setObjectName("subtitle");
    titles->addWidget(title);
    titles->addWidget(sub);

    m_tabList = new QPushButton("Liste des animaux");
    m_tabStats = new QPushButton("Statistiques");
    for (QPushButton *b : {m_tabList, m_tabStats}) {
        b->setObjectName("tab");
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
    }
    m_tabList->setChecked(true);
    auto *tabGrp = new QButtonGroup(wrapper);
    tabGrp->setExclusive(true);
    tabGrp->addButton(m_tabList, 0);
    tabGrp->addButton(m_tabStats, 1);

    auto *btnAdd = new QPushButton("\uff0b Ajouter un animal");
    btnAdd->setObjectName("primary");
    btnAdd->setFixedHeight(42);
    btnAdd->setCursor(Qt::PointingHandCursor);

    head->addWidget(icon);
    head->addLayout(titles);
    head->addSpacing(28);
    head->addWidget(m_tabList);
    head->addWidget(m_tabStats);
    head->addStretch();
    head->addWidget(btnAdd);
    root->addLayout(head);

    // ---- pages
    m_pages = new QStackedWidget;
    m_pages->addWidget(buildListPage());     // 0
    m_pages->addWidget(buildStatsPage());    // 1
    root->addWidget(m_pages, 1);

    connect(tabGrp, &QButtonGroup::idClicked, this, [this](int id) { m_pages->setCurrentIndex(id); });
    connect(btnAdd, &QPushButton::clicked, this, [this] {
        showPage(0);
        clearForm();
        m_fNom->setFocus();
        QTimer::singleShot(0, this, [this] { m_listScroll->ensureWidgetVisible(m_form); });
    });
    return wrapper;
}

// ============================================================ page 1 : liste + formulaire
QWidget *MainWindow::buildListPage()
{
    m_listScroll = new QScrollArea;
    m_listScroll->setWidgetResizable(true);
    m_listScroll->setFrameShape(QFrame::NoFrame);
    m_listScroll->setStyleSheet("QScrollArea { background: transparent; }");
    m_listScroll->viewport()->setAutoFillBackground(false);
    auto *container = new QWidget;
    container->setAutoFillBackground(false);
    auto *lay = new QVBoxLayout(container);
    lay->setContentsMargins(0, 0, 4, 10);
    lay->setSpacing(14);

    // ---- barre de recherche / filtres / tri
    auto *bar = new QHBoxLayout;
    m_search = new QLineEdit;
    m_search->setPlaceholderText("\U0001F50d  Rechercher par NOM ou par ID (ex: ANI001)...");
    m_search->setClearButtonEnabled(true);
    m_search->setMinimumHeight(38);
    m_filterEspece = new QComboBox;
    m_filterEspece->addItems({"Toutes les esp\u00e8ces", "Chien", "Chat", "Lapin"});
    m_filterEspece->setMinimumHeight(38);
    m_filterEspece->setFixedWidth(160);
    m_filterStatut = new QComboBox;
    m_filterStatut->addItems({"Tous les statuts", "En bonne sant\u00e9", "Surveillance"});
    m_filterStatut->setMinimumHeight(38);
    m_filterStatut->setFixedWidth(160);
    auto *btnSort = new QPushButton("\U0001F524  Trier par nom");
    btnSort->setObjectName("secondary");
    btnSort->setMinimumHeight(38);
    btnSort->setCursor(Qt::PointingHandCursor);
    m_btnReset = new QPushButton("\u2715  R\u00e9initialiser");
    m_btnReset->setObjectName("link");
    m_btnReset->setCursor(Qt::PointingHandCursor);
    m_btnReset->setVisible(false);
    bar->addWidget(m_search, 1);
    bar->addWidget(m_filterEspece);
    bar->addWidget(m_filterStatut);
    bar->addWidget(btnSort);
    bar->addWidget(m_btnReset);
    lay->addLayout(bar);

    // ---- tableau
    m_table = new QTableWidget(0, 10);
    m_table->setMinimumHeight(300);
    m_table->setHorizontalHeaderLabels({"ID", "Photo", "Nom", "Esp\u00e8ce", "Race", "Sexe", "\u00c2ge", "Poids", "Statut", "Actions"});
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setShowGrid(false);
    m_table->setFrameShape(QFrame::NoFrame);
    auto *hh = m_table->horizontalHeader();
    hh->setSectionResizeMode(QHeaderView::Stretch);
    hh->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(1, QHeaderView::Fixed);
    hh->setSectionResizeMode(9, QHeaderView::Fixed);
    m_table->setColumnWidth(1, 70);
    m_table->setColumnWidth(9, 90);
    lay->addWidget(m_table);

    m_count = new QLabel;
    m_count->setObjectName("muted");
    lay->addWidget(m_count);

    lay->addWidget(buildForm());
    m_listScroll->setWidget(container);

    connect(btnSort, &QPushButton::clicked, this, &MainWindow::trierParNom);
    connect(m_btnReset, &QPushButton::clicked, this, &MainWindow::resetFilters);
    connect(m_search, &QLineEdit::textChanged, this, &MainWindow::applyFilter);
    connect(m_filterEspece, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyFilter);
    connect(m_filterStatut, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyFilter);
    connect(m_table, &QTableWidget::currentCellChanged, this, [this](int row) { if (row >= 0) showDetail(row); });
    return m_listScroll;
}

// ============================================================ formulaire
QWidget *MainWindow::buildForm()
{
    m_form = new QFrame;
    m_form->setObjectName("card");
    auto *g = new QGridLayout(m_form);
    g->setContentsMargins(16, 10, 16, 12);
    g->setHorizontalSpacing(12);
    g->setVerticalSpacing(4);
    m_formTitle = new QLabel("\U0001F43E  Ajouter un animal");
    m_formTitle->setObjectName("formTitle");
    g->addWidget(m_formTitle, 0, 0, 1, 4);
    auto *photoBtn = new QPushButton;
    photoBtn->setObjectName("photoBox");
    photoBtn->setFixedSize(130, 120);
    auto *pl = new QVBoxLayout(photoBtn);
    m_formPhoto = new QLabel("\U0001F4F7\nAjouter une photo\nJPG, PNG (max 5 Mo)");
    m_formPhoto->setAlignment(Qt::AlignCenter);
    m_formPhoto->setStyleSheet("font-size:10px; color:#4A5A5A; background:transparent;");
    pl->addWidget(m_formPhoto);
    g->addWidget(photoBtn, 1, 0, 5, 1, Qt::AlignTop);
    connect(photoBtn, &QPushButton::clicked, this, &MainWindow::choosePhoto);
    auto lbl = [](const QString &t) {
        auto *l = new QLabel(t);
        l->setObjectName("fieldLabel");
        return l;
    };
    m_fNom = new QLineEdit;   m_fNom->setPlaceholderText("Ex: Luna");
    m_fEspece = new QComboBox; m_fEspece->addItems({"Chien", "Chat", "Lapin"});
    m_fEspece->setCurrentIndex(-1);
    m_fEspece->setPlaceholderText("S\u00e9lectionner une esp\u00e8ce");
    m_fRace = new QLineEdit;  m_fRace->setPlaceholderText("Ex: Golden Retriever");
    m_fSexe = new QComboBox;  m_fSexe->addItems({"Femelle", "M\u00e2le"});
    m_fSexe->setCurrentIndex(-1);
    m_fSexe->setPlaceholderText("S\u00e9lectionner un sexe");
    m_fDate = new QDateEdit(QDate::currentDate());
    m_fDate->setCalendarPopup(true);
    m_fDate->setDisplayFormat("dd/MM/yyyy");
    m_fDate->setMaximumDate(QDate::currentDate());
    m_fPoids = new QDoubleSpinBox;
    m_fPoids->setRange(0, 200); m_fPoids->setDecimals(1); m_fPoids->setSuffix(" kg");
    m_fDesc = new QTextEdit;
    m_fDesc->setPlaceholderText("Informations sur l'animal...");
    m_fDesc->setFixedHeight(52);
    m_fStatut = new QComboBox; m_fStatut->addItems({"En bonne sant\u00e9", "Surveillance"});
    g->addWidget(lbl("Nom *"), 1, 1);          g->addWidget(m_fNom, 2, 1);
    g->addWidget(lbl("Esp\u00e8ce *"), 1, 2);  g->addWidget(m_fEspece, 2, 2);
    g->addWidget(lbl("Race *"), 1, 3);         g->addWidget(m_fRace, 2, 3);
    g->addWidget(lbl("Sexe *"), 3, 1);         g->addWidget(m_fSexe, 4, 1);
    g->addWidget(lbl("Date de naissance *"), 3, 2); g->addWidget(m_fDate, 4, 2);
    g->addWidget(lbl("Poids (kg) *"), 3, 3);   g->addWidget(m_fPoids, 4, 3);
    g->addWidget(lbl("Description"), 5, 1, 1, 3);
    g->addWidget(m_fDesc, 6, 1, 1, 3);
    g->addWidget(lbl("Statut de sant\u00e9"), 7, 1);       g->addWidget(m_fStatut, 8, 1);
    auto *btnCancel = new QPushButton("Annuler");
    btnCancel->setObjectName("secondary");
    m_btnSave = new QPushButton("\U0001F4BE  Enregistrer");
    m_btnSave->setObjectName("primary");
    auto *hb = new QHBoxLayout;
    hb->addStretch();
    hb->addWidget(btnCancel);
    hb->addWidget(m_btnSave);
    g->addLayout(hb, 9, 1, 1, 3);
    g->setColumnStretch(1, 1); g->setColumnStretch(2, 1); g->setColumnStretch(3, 1);
    connect(btnCancel, &QPushButton::clicked, this, &MainWindow::clearForm);
    connect(m_btnSave, &QPushButton::clicked, this, &MainWindow::saveAnimal);
    return m_form;
}

// ============================================================ panneau détail
QWidget *MainWindow::buildDetail()
{
    auto *panel = new QFrame;
    panel->setObjectName("detail");
    panel->setFixedWidth(315);
    auto *lay = new QVBoxLayout(panel);
    lay->setContentsMargins(18, 18, 18, 16);
    lay->setSpacing(8);
    auto *head = new QLabel("FICHE ANIMAL");
    head->setObjectName("detailHead");
    lay->addWidget(head);
    auto *top = new QHBoxLayout;
    m_dPhoto = new QLabel;
    m_dPhoto->setFixedSize(100, 100);
    auto *col = new QVBoxLayout;
    m_dNom = new QLabel; m_dNom->setObjectName("detailName");
    m_dStatut = new QLabel;
    m_dId = new QLabel; m_dId->setObjectName("idChip");
    col->addStretch();
    col->addWidget(m_dNom);
    col->addWidget(m_dStatut, 0, Qt::AlignLeft);
    col->addWidget(m_dId, 0, Qt::AlignLeft);
    col->addStretch();
    top->addWidget(m_dPhoto);
    top->addLayout(col, 1);
    lay->addLayout(top);
    lay->addSpacing(4);
    auto row = [&](const QString &icon, const QString &label, QLabel *&value) {
        auto *h = new QHBoxLayout;
        auto *l = new QLabel(icon + "  " + label + " :");
        l->setObjectName("detailKey");
        value = new QLabel;
        value->setObjectName("detailVal");
        value->setWordWrap(true);
        h->addWidget(l);
        h->addWidget(value, 1);
        lay->addLayout(h);
    };
    row("\U0001F43E", "Esp\u00e8ce", m_dEspece);
    row("\U0001F9B4", "Race", m_dRace);
    row("\u26A5", "Sexe", m_dSexe);
    row("\U0001F4C5", "Date de naissance", m_dNaiss);
    row("\u2696", "Poids", m_dPoids);
    row("\u23F3", "\u00c2ge", m_dAge);
    auto *dt = new QLabel("Description");
    dt->setObjectName("detailSection");
    lay->addWidget(dt);
    m_dDesc = new QLabel;
    m_dDesc->setObjectName("descBox");
    m_dDesc->setWordWrap(true);
    m_dDesc->setMinimumHeight(70);
    m_dDesc->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    lay->addWidget(m_dDesc);
    lay->addStretch();
    auto *btns = new QHBoxLayout;
    auto *bEdit = new QPushButton("\u270e  Modifier");      bEdit->setObjectName("primary");
    auto *bDel  = new QPushButton("\U0001F5D1  Supprimer"); bDel->setObjectName("danger");
    bEdit->setCursor(Qt::PointingHandCursor);
    bDel->setCursor(Qt::PointingHandCursor);
    btns->addWidget(bEdit, 1);
    btns->addWidget(bDel, 1);
    lay->addLayout(btns);
    connect(bEdit, &QPushButton::clicked, this, [this] { int i = requireSelection(); if (i >= 0) editAnimal(m_animals[i].id); });
    connect(bDel, &QPushButton::clicked, this, [this] { int i = requireSelection(); if (i >= 0) deleteAnimal(m_animals[i].id); });
    return panel;
}

int MainWindow::indexOf(const QString &id) const
{
    for (int i = 0; i < m_animals.size(); ++i)
        if (m_animals[i].id == id) return i;
    return -1;
}

int MainWindow::currentIndex() const
{
    int r = m_table->currentRow();
    return (r >= 0 && r < m_animals.size()) ? r : -1;
}

int MainWindow::requireSelection()
{
    int i = currentIndex();
    if (i < 0)
        QMessageBox::information(this, "Aucune s\u00e9lection", "Veuillez d'abord s\u00e9lectionner un animal dans le tableau.");
    return i;
}

// ============================================================ READ : afficher le tableau
void MainWindow::refreshTable()
{
    QSignalBlocker blocker(m_table);
    m_table->setRowCount(0);
    m_table->setRowCount(m_animals.size());
    for (int i = 0; i < m_animals.size(); ++i) {
        const Animal &a = m_animals[i];
        m_table->setRowHeight(i, 58);
        auto item = [](const QString &t) {
            auto *it = new QTableWidgetItem(t);
            it->setFlags(it->flags() & ~Qt::ItemIsEditable);
            return it;
        };
        m_table->setItem(i, 0, item(a.id));
        auto *ph = new QLabel;
        ph->setPixmap(avatar(a, 44));
        ph->setAlignment(Qt::AlignCenter);
        m_table->setCellWidget(i, 1, ph);
        m_table->setItem(i, 2, item(a.nom));
        QString ico = a.espece == "Chien" ? "\U0001F436 " : a.espece == "Chat" ? "\U0001F431 " : "\U0001F430 ";
        m_table->setItem(i, 3, item(ico + a.espece));
        m_table->setItem(i, 4, item(a.race));
        bool f = a.sexe == "Femelle";
        m_table->setCellWidget(i, 5, wrap(badge(a.sexe, f ? "#FBD5D5" : "#D6E8FA", f ? "#C0392B" : "#2C6CB0")));
        m_table->setItem(i, 6, item(ageText(a.naissance)));
        m_table->setItem(i, 7, item(QString::number(a.poids, 'g', 4) + " kg"));
        bool ok = estEnBonneSante(a);
        m_table->setCellWidget(i, 8, wrap(badge(a.statut, ok ? "#D5F0DF" : "#FFF0BF", ok ? "#2E7D52" : "#B8860B")));

        // actions : modifier / supprimer (la consultation se fait en cliquant sur la ligne)
        auto *act = new QWidget;
        auto *al = new QHBoxLayout(act);
        al->setContentsMargins(0, 0, 0, 0);
        al->setSpacing(6);
        auto mkBtn = [&](const QString &t, const QString &obj, const QString &tip) {
            auto *b = new QToolButton;
            b->setText(t);
            b->setObjectName(obj);
            b->setToolTip(tip);
            b->setFixedSize(28, 28);
            b->setCursor(Qt::PointingHandCursor);
            al->addWidget(b);
            return b;
        };
        auto *be = mkBtn("\u270e", "actEdit", "Modifier");
        auto *bd = mkBtn("\U0001F5D1", "actDel", "Supprimer");
        const QString id = a.id;
        connect(be, &QToolButton::clicked, this, [this, id] {
            int k = indexOf(id); if (k >= 0) m_table->selectRow(k);
            editAnimal(id);
        });
        connect(bd, &QToolButton::clicked, this, [this, id] {
            int k = indexOf(id); if (k >= 0) m_table->selectRow(k);
            deleteAnimal(id);
        });
        m_table->setCellWidget(i, 9, act);
    }
    applyFilter();
}

// ============================================================ READ : rechercher / filtrer
void MainWindow::applyFilter()
{
    const QString q = m_search->text().trimmed().toLower();
    const QString esp = m_filterEspece->currentIndex() > 0 ? m_filterEspece->currentText() : QString();
    const int st = m_filterStatut->currentIndex();          // 0 = tous, 1 = bonne santé, 2 = surveillance
    int visible = 0;
    for (int i = 0; i < m_animals.size(); ++i) {
        const Animal &a = m_animals[i];
        const bool ok = estEnBonneSante(a);
        const bool match = (q.isEmpty() || a.nom.toLower().contains(q) || a.id.toLower().contains(q))
                           && (esp.isEmpty() || a.espece == esp)
                           && (st == 0 || (st == 1 && ok) || (st == 2 && !ok));
        m_table->setRowHidden(i, !match);
        if (match) ++visible;
    }
    m_count->setText(visible > 1 ? QString("%1 animaux trouv\u00e9s").arg(visible)
                                 : QString("%1 animal trouv\u00e9").arg(visible));
    m_btnReset->setVisible(!q.isEmpty() || !esp.isEmpty() || st != 0);   // visible seulement si un filtre est actif
}

void MainWindow::resetFilters()
{
    {
        QSignalBlocker b1(m_search);
        QSignalBlocker b2(m_filterEspece);
        QSignalBlocker b3(m_filterStatut);
        m_search->clear();
        m_filterEspece->setCurrentIndex(0);
        m_filterStatut->setCurrentIndex(0);
    }
    applyFilter();
}

// ============================================================ formulaire : remise à zéro (mode ajout)
void MainWindow::clearForm()
{
    m_editId.clear();
    m_photoPath.clear();
    m_formTitle->setText("\U0001F43E  Ajouter un animal");
    m_btnSave->setText("\U0001F4BE  Enregistrer");
    m_formPhoto->setPixmap(QPixmap());
    m_formPhoto->setText("\U0001F4F7\nAjouter une photo\nJPG, PNG (max 5 Mo)");
    m_fNom->clear(); m_fRace->clear();
    m_fEspece->setCurrentIndex(-1); m_fSexe->setCurrentIndex(-1);
    m_fStatut->setCurrentIndex(0);
    m_fDate->setDate(QDate::currentDate());
    m_fPoids->setValue(0);
    m_fDesc->clear();
}

// ============================================================ UPDATE (1/2) : charger l'animal dans le formulaire
void MainWindow::editAnimal(const QString &id)
{
    int i = indexOf(id);
    if (i < 0) return;
    const Animal &a = m_animals[i];
    m_editId = id;
    m_photoPath = a.photo;
    m_formTitle->setText("\u270e  Modifier l'animal " + a.id);
    m_btnSave->setText("\U0001F4BE  Mettre \u00e0 jour");
    m_fNom->setText(a.nom);
    m_fEspece->setCurrentText(a.espece);
    m_fRace->setText(a.race);
    m_fSexe->setCurrentText(a.sexe);
    m_fStatut->setCurrentText(a.statut);
    m_fDate->setDate(a.naissance);
    m_fPoids->setValue(a.poids);
    m_fDesc->setPlainText(a.description);
    QPixmap px;
    if (!a.photo.isEmpty() && px.load(a.photo))
        m_formPhoto->setPixmap(px.scaled(100, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else {
        m_formPhoto->setPixmap(QPixmap());
        m_formPhoto->setText("\U0001F4F7\nAjouter une photo\nJPG, PNG (max 5 Mo)");
    }
    showPage(0);                                             // le formulaire est sur la page "Liste"
    m_fNom->setFocus();
    QTimer::singleShot(0, this, [this] { m_listScroll->ensureWidgetVisible(m_form); });
}

// ============================================================ CREATE + UPDATE (2/2) : enregistrer
void MainWindow::saveAnimal()
{
    // 1) on remplit un objet Animal avec le formulaire
    int idx = m_editId.isEmpty() ? -1 : indexOf(m_editId);   // -1 = ajout, sinon = modification
    Animal a = idx >= 0 ? m_animals[idx] : Animal();
    a.nom = m_fNom->text().trimmed();
    a.espece = m_fEspece->currentIndex() < 0 ? QString() : m_fEspece->currentText();
    a.race = m_fRace->text().trimmed();
    a.sexe = m_fSexe->currentIndex() < 0 ? QString() : m_fSexe->currentText();
    a.statut = m_fStatut->currentText();
    a.naissance = m_fDate->date();
    a.poids = m_fPoids->value();
    a.description = m_fDesc->toPlainText().trimmed();
    a.photo = m_photoPath;

    // 2) contrôle de saisie (code C++ dans la classe Animal)
    const QString err = a.controleSaisie();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Saisie invalide", err);
        return;
    }

    // 3) CREATE ou UPDATE dans Oracle (la requ\u00eate est dans la classe Animal)
    bool ok = (idx >= 0) ? a.modifier() : a.ajouter();
    if (!ok) {
        QMessageBox::critical(this, "Erreur Oracle", Animal::derniereErreur());
        return;
    }

    // 4) on recharge depuis la base et on rafra\u00eechit l'affichage
    chargerAnimaux();
    {
        QSignalBlocker b1(m_search);
        QSignalBlocker b2(m_filterEspece);
        QSignalBlocker b3(m_filterStatut);
        m_search->clear();
        m_filterEspece->setCurrentIndex(0);
        m_filterStatut->setCurrentIndex(0);
    }
    refreshTable();
    updateDashboard();                                       // les graphiques se mettent \u00e0 jour automatiquement
    int sel = indexOf(a.id);
    m_table->selectRow(sel);
    showDetail(sel);
    clearForm();
    statusBar()->showMessage(idx >= 0 ? "Animal modifi\u00e9 : " + a.nom : "Animal ajout\u00e9 : " + a.nom, 4000);
}

void MainWindow::trierParNom()
{
    std::sort(m_animals.begin(), m_animals.end(), [](const Animal &a, const Animal &b) {
        return a.nom.localeAwareCompare(b.nom) < 0;
    });
    refreshTable();
    if (!m_animals.isEmpty()) {
        m_table->selectRow(0);
        showDetail(0);
    }
}

void MainWindow::showDetail(int i)
{
    if (i < 0 || i >= m_animals.size()) return;
    const Animal &a = m_animals[i];
    m_dPhoto->setPixmap(avatar(a, 100));
    m_dNom->setText(a.nom);
    bool ok = a.statut == "En bonne sant\u00e9";
    m_dStatut->setText("\u25cf  " + a.statut);
    m_dStatut->setStyleSheet(QString("background:%1; color:%2; border-radius:11px;"
                                     "padding:3px 10px; font-size:11px; font-weight:600;")
                             .arg(ok ? "#D5F0DF" : "#FFF0BF", ok ? "#2E7D52" : "#B8860B"));
    m_dStatut->show();
    m_dId->setText("ID : " + a.id);
    m_dId->show();
    m_dEspece->setText(a.espece);
    m_dRace->setText(a.race);
    m_dSexe->setText(a.sexe);
    m_dNaiss->setText(a.naissance.toString("dd/MM/yyyy"));
    m_dPoids->setText(QString::number(a.poids, 'g', 4) + " kg");
    m_dAge->setText(ageText(a.naissance));
    m_dDesc->setText(a.description);
}

void MainWindow::clearDetail()
{
    m_dPhoto->setPixmap(QPixmap());
    m_dNom->setText("Aucun animal s\u00e9lectionn\u00e9");
    m_dStatut->clear();
    m_dStatut->setStyleSheet(QString());
    m_dStatut->hide();
    m_dId->clear();
    m_dId->hide();
    for (QLabel *l : {m_dEspece, m_dRace, m_dSexe, m_dNaiss, m_dPoids, m_dAge, m_dDesc})
        l->clear();
}

void MainWindow::choosePhoto()
{
    QString f = QFileDialog::getOpenFileName(this, "Choisir une photo", QString(), "Images (*.png *.jpg *.jpeg)");
    if (f.isEmpty()) return;
    if (QFileInfo(f).size() > 5 * 1024 * 1024) {
        QMessageBox::warning(this, "Photo trop volumineuse", "La photo ne doit pas d\u00e9passer 5 Mo.");
        return;
    }
    QPixmap px(f);
    if (px.isNull()) {
        QMessageBox::warning(this, "Image invalide", "Impossible de charger cette image.");
        return;
    }
    m_photoPath = f;
    m_formPhoto->setPixmap(px.scaled(100, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
void MainWindow::deleteAnimal(const QString &id)
{
    int i = indexOf(id);
    if (i < 0) return;
    auto r = QMessageBox::question(this, "Supprimer", QString("Supprimer %1 (%2) ?").arg(m_animals[i].nom, id));
    if (r != QMessageBox::Yes) return;
    if (!Animal::supprimer(id)) {                            // DELETE dans Oracle (classe Animal)
        QMessageBox::critical(this, "Erreur Oracle", Animal::derniereErreur());
        return;
    }
    if (m_editId == id) clearForm();
    chargerAnimaux();
    refreshTable();
    updateDashboard();
    if (m_animals.isEmpty()) {
        clearDetail();
    } else {
        int k = qMin(i, m_animals.size() - 1);
        m_table->selectRow(k);
        showDetail(k);
    }
}

// ============================================================ style (QSS)
void MainWindow::applyStyle()
{
    setStyleSheet(R"(
#central, QWidget#central { background:#EEF7F5; }
QLabel { background:transparent; color:#243238; }
#sidebar { background:#F4FAF9; border-right:1px solid #DCEBE8; }
#logo { font-size:20px; font-weight:800; }
#footer { color:#2F7F73; font-style:italic; font-size:11px; }
QPushButton#nav { text-align:left; padding:10px 12px; border:none; border-radius:10px;
color:#33454B; font-size:13px; background:transparent; }
QPushButton#nav:hover { background:#E3F1EE; }
QPushButton#nav:checked { background:#CDE8E2; color:#1F5F54; font-weight:700; }
#headIcon { background:#D9EEE9; border-radius:24px; font-size:22px; }
#title { font-size:22px; font-weight:800; color:#1E2B30; }
#subtitle, #muted { color:#6A7B80; font-size:11px; }
#sectionTitle { font-size:16px; font-weight:800; color:#1E2B30; }
#cardTitle { font-size:13px; font-weight:800; color:#1E2B30; }
QPushButton#tab { background:transparent; border:none; border-bottom:3px solid transparent;
padding:8px 14px; color:#5B706B; font-weight:600; font-size:13px; }
QPushButton#tab:hover { color:#1F5F54; }
QPushButton#tab:checked { color:#1F5F54; font-weight:800; border-bottom:3px solid #2F7F73; }
QPushButton#seg { background:#EEF4F3; border:none; border-radius:8px; padding:5px 14px;
color:#5B706B; font-weight:600; }
QPushButton#seg:hover { background:#E1EEEB; }
QPushButton#seg:checked { background:#2F7F73; color:white; }
#aiCard {
background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #EAF6F3, stop:1 #F2FAF8);
border: 1px solid #C5E3DC;
border-radius: 12px;
}
#panelCard, #kpiStatic { background:white; border:1px solid #D9E8E4; border-radius:14px; }
#kpi { background:white; border:1px solid #D9E8E4; border-radius:14px; }
#kpi:hover { background:#F8FCFB; border:1px solid #2F7F73; }
QLineEdit, QComboBox, QDateEdit, QDoubleSpinBox, QTextEdit {
background:white; border:1px solid #D5E3E0; border-radius:8px;
padding:5px 9px; color:#243238; selection-background-color:#BFE3DB; }
QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QDoubleSpinBox:focus, QTextEdit:focus {
border:1px solid #2F7F73; }
QComboBox::drop-down, QDateEdit::drop-down { border:none; width:22px; }
QPushButton#primary { background:#2F7F73; color:white; border:none; border-radius:8px;
padding:8px 16px; font-weight:700; }
QPushButton#primary:hover { background:#276C62; }
QPushButton#secondary { background:#E3F1EE; color:#2F7F73; border:1px solid #CFE5E0;
border-radius:8px; padding:8px 14px; font-weight:600; }
QPushButton#secondary:hover { background:#D3EAE5; }
QPushButton#danger { background:#FBDADA; color:#C0392B; border:none; border-radius:8px;
padding:8px 12px; font-weight:600; }
QPushButton#danger:hover { background:#F6C6C6; }
QPushButton#link { background:transparent; border:none; color:#5B706B; padding:6px 8px; font-weight:600; }
QPushButton#link:hover { color:#C0392B; }
QTableWidget { background:white; border:1px solid #DCEBE8; border-radius:12px; color:#243238; gridline-color:transparent; outline:0; }
QTableWidget::item { border-bottom:1px solid #EEF3F2; padding-left:6px; }
QTableWidget::item:selected { background:#DCF0EC; color:#243238; }
QHeaderView::section { background:white; color:#33454B; font-weight:700; border:none; border-bottom:1px solid #E3ECEA; padding:10px 6px; }
QToolButton#actEdit { background:#E8EEF2; border:none; border-radius:7px; }
QToolButton#actDel  { background:#FBDADA; border:none; border-radius:7px; }
QToolButton:hover { background:#C9E3DD; }
#card { background:white; border:1px solid #DCEBE8; border-radius:12px; }
#formTitle { font-size:13px; font-weight:700; color:#243238; }
#fieldLabel { font-size:10px; font-weight:700; color:#33454B; }
QPushButton#photoBox { background:#F6FBFA; border:2px dashed #BFD9D3; border-radius:10px; }
QPushButton#photoBox:hover { background:#EAF6F3; }
#detail { background:white; border-left:1px solid #DCEBE8; }
#detailHead { font-size:10px; font-weight:800; letter-spacing:1px; color:#2F7F73; }
#detailName { font-size:22px; font-weight:800; color:#1E2B30; }
#idChip { background:#F1F5F5; border:1px solid #E1E8E8; border-radius:8px; padding:3px 10px; font-size:11px; color:#4A5A5A; }
#detailKey { color:#33454B; font-size:12px; }
#detailVal { color:#1E2B30; font-size:12px; font-weight:600; }
#detailSection { font-size:13px; font-weight:800; color:#1E2B30; }
#descBox { background:#EAF4F2; border:1px solid #D5E7E3; border-radius:8px; padding:8px; color:#33454B; font-size:12px; }
QStatusBar { background:#F4FAF9; color:#33454B; border-top:1px solid #DCEBE8; }
)");
}
