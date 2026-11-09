#include "animauxpage.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QProgressBar>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTableWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QShowEvent>
#include <QSettings>
#include <QTimer>
#include <algorithm>

// ============================================================ helpers
QString AnimauxPage::ageText(const QDate &d)
{
    QDate t = QDate::currentDate();
    int months = (t.year() - d.year()) * 12 + t.month() - d.month();
    if (t.day() < d.day()) months--;
    if (months < 12) return QString("%1 mois").arg(qMax(0, months));
    int y = months / 12;
    const QString plural = y > 1 ? "s" : "";
    return QString("%1 an%2").arg(y).arg(plural);
}

QPixmap AnimauxPage::avatar(const Animal &a, int size)
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

QLabel *AnimauxPage::badge(const QString &text, const QString &bg, const QString &fg)
{
    auto *l = new QLabel(text);
    l->setStyleSheet(QString("background:%1; color:%2; border-radius:9px;"
                             "padding:3px 10px; font-size:11px; font-weight:600;")
                     .arg(bg, fg));
    return l;
}

QWidget *AnimauxPage::wrap(QWidget *w, Qt::Alignment al)
{
    auto *c = new QWidget;
    auto *l = new QHBoxLayout(c);
    l->setContentsMargins(4, 0, 4, 0);
    l->addWidget(w, 0, al);
    return c;
}

// ============================================================ destructeur
AnimauxPage::~AnimauxPage() = default;

// ============================================================ constructeur
AnimauxPage::AnimauxPage(QWidget *parent, bool avecMenu) : QWidget(parent),
    m_avecMenu(avecMenu)
{
    seedData();

    auto *central = new QWidget;
    central->setObjectName("central");
    auto *root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    if (m_avecMenu)
        root->addWidget(buildSidebar());
    root->addWidget(buildCenter(), 1);
    root->addWidget(buildDetail());

    auto *racine = new QVBoxLayout(this);
    racine->setContentsMargins(0, 0, 0, 0);
    racine->setSpacing(0);
    racine->addWidget(central);

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

int AnimauxPage::nombreParEspece(const QString &espece) const
{
    int n = 0;
    for (const auto &a : m_animals)
        if (a.espece == espece) ++n;
    return n;
}

QStringList AnimauxPage::animauxSousSurveillance() const
{
    QStringList liste;
    for (const auto &a : m_animals)
        if (a.statut != "En bonne sant\u00e9")
            liste.append(QString("%1 (%2) - %3").arg(a.nom, a.espece, a.statut));
    return liste;
}

// L'alerte sant\u00e9 d'origine s'affichait au lancement ; elle s'affiche maintenant
// \u00e0 la premi\u00e8re ouverture de la page Animaux.
void AnimauxPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (m_premiereAffichage) {
        m_premiereAffichage = false;
        // Réglage de la page Paramètres (activé par défaut)
        QSettings reglages("PetManager", "PetManager");
        if (reglages.value("animaux/alerteSante", true).toBool())
            QTimer::singleShot(350, this, &AnimauxPage::verifierSanteAnimaux);
    }
}

void AnimauxPage::seedData()
{
    m_vets = { {"VET001", "Dr. Nessma Tounsi"}, };
    m_owners = {
        {"PR001", "Karim Ben Ali"},
        {"PR002", "Claire Dubois"},
        {"PR003", "Sarra Trabelsi"},
        {"PR004", "Youssef Gharbi"},
        {"PR005", "Amel Jaziri"},
    };
    auto mk = [this](const QString &id, const QString &nom, const QString &esp,
                     const QString &race, const QString &sexe, const QString &statut,
                     const QDate &d, double poids, const QString &desc,
                     const QString &idP, const QString &idV) {
        Animal a;
        a.id = id; a.nom = nom; a.espece = esp; a.race = race; a.sexe = sexe;
        a.statut = statut; a.naissance = d; a.poids = poids; a.description = desc;
        a.idProprio = idP; a.nomProprio = ownerName(idP);
        a.idVet = idV;     a.nomVet = vetName(idV);
        return a;
    };
    m_animals = {
        mk("ANI001", "Luna", "Chien", "Golden Retriever", "Femelle", "En bonne sant\u00e9",
           QDate(2022, 4, 12), 24, "Chienne tr\u00e8s affectueuse et joyeuse.", "PR002", "VET001"),
        mk("ANI002", "Milo", "Chat", "Europ\u00e9en", "M\u00e2le", "En bonne sant\u00e9",
           QDate::currentDate().addYears(-1), 4.2, "Chat calme et c\u00e2lin.", "PR001", "VET001"),
        mk("ANI003", "Max", "Lapin", "Europ\u00e9en", "M\u00e2le", "Surveillance",
           QDate::currentDate().addMonths(-6), 1.8, "Lapin timide, sous traitement l\u00e9ger.", "PR003", "VET001"),
        mk("ANI004", "Bella", "Chien", "Labrador", "Femelle", "En bonne sant\u00e9",
           QDate::currentDate().addYears(-3), 20, "Tr\u00e8s joueuse et dynamique.", "PR004", "VET001"),
        mk("ANI005", "Rocky", "Chat", "British Shorthair", "M\u00e2le", "En bonne sant\u00e9",
           QDate::currentDate().addYears(-2), 5.5, "Ind\u00e9pendant mais adore les caresses.", "PR005", "VET001"),
    };
}

QString AnimauxPage::ownerName(const QString &id) const
{
    return m_owners.value(id.trimmed().toUpper(), QStringLiteral("Inconnu"));
}

QString AnimauxPage::vetName(const QString &id) const
{
    return m_vets.value(id.trimmed().toUpper(), QStringLiteral("Inconnu"));
}

// ============================================================ sidebar
QWidget *AnimauxPage::buildSidebar()
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
    const QStringList items = {"Accueil", "  Animaux", "Rendez-vous", "Stock",
                               "commandes", "Fournisseurs", "Employees", "Parametres"};
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
    auto *foot = new QLabel("Des animaux en bonne sant\u00e9,\ndes ma\u00eetres plus heureux ! \U0001F43E");
    foot->setObjectName("footer");
    foot->setAlignment(Qt::AlignCenter);
    foot->setWordWrap(true);
    lay->addWidget(foot);
    return side;
}

// ============================================================ MODULE PETAI ASK
QWidget *AnimauxPage::buildAIAskWidget()
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
    connect(btnAsk, &QPushButton::clicked, this, &AnimauxPage::traiterQuestionIA);
    connect(m_aiInput, &QLineEdit::returnPressed, this, &AnimauxPage::traiterQuestionIA);
    return box;
}

void AnimauxPage::traiterQuestionIA()
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

// ============================================================ BILAN STATISTIQUE INTELLIGENT
QWidget *AnimauxPage::buildStatCard(const QString &title, const QString &icon, QLabel *&valLbl,
                                   QLabel *&pctLbl, QProgressBar *&bar, const QString &color)
{
    auto *card = new QFrame;
    card->setObjectName("statCard");
    auto *lay = new QVBoxLayout(card);
    lay->setContentsMargins(12, 10, 12, 10);
    lay->setSpacing(4);
    auto *top = new QHBoxLayout;
    auto *titleLbl = new QLabel(icon + " " + title);
    titleLbl->setStyleSheet("font-size: 12px; font-weight: 700; color: #4A5A5A;");
    pctLbl = new QLabel("0%");
    pctLbl->setStyleSheet(QString("font-size: 12px; font-weight: 800; color: %1;").arg(color));
    top->addWidget(titleLbl);
    top->addStretch();
    top->addWidget(pctLbl);
    valLbl = new QLabel("0");
    valLbl->setStyleSheet(QString("font-size: 18px; font-weight: 900; color: %1;").arg(color));
    bar = new QProgressBar;
    bar->setFixedHeight(6);
    bar->setTextVisible(false);
    bar->setStyleSheet(QString(
        "QProgressBar { background-color: #E2ECE9; border: none; border-radius: 3px; }"
        "QProgressBar::chunk { background-color: %1; border-radius: 3px; }"
    ).arg(color));
    lay->addLayout(top);
    lay->addWidget(valLbl);
    lay->addWidget(bar);
    return card;
}

QWidget *AnimauxPage::buildSmartDashboard()
{
    auto *statPanel = new QFrame;
    statPanel->setObjectName("statPanel");
    auto *statLayout = new QHBoxLayout(statPanel);
    statLayout->setContentsMargins(16, 12, 16, 12);
    statLayout->setSpacing(14);
    auto *totalBox = new QVBoxLayout;
    totalBox->setAlignment(Qt::AlignCenter);
    auto *tTitle = new QLabel("TOTAL ANIMAUX");
    tTitle->setStyleSheet("font-size: 10px; font-weight: 800; color: #2F7F73; letter-spacing: 1px;");
    m_lblTotal = new QLabel("0");
    m_lblTotal->setStyleSheet("font-size: 26px; font-weight: 900; color: #1F5F54;");
    m_lblAvgWeight = new QLabel("Poids moy: 0 kg");
    m_lblAvgWeight->setStyleSheet("font-size: 11px; font-weight: 600; color: #5B706B;");
    totalBox->addWidget(tTitle, 0, Qt::AlignCenter);
    totalBox->addWidget(m_lblTotal, 0, Qt::AlignCenter);
    totalBox->addWidget(m_lblAvgWeight, 0, Qt::AlignCenter);
    auto *sep1 = new QFrame;
    sep1->setFrameShape(QFrame::VLine);
    sep1->setStyleSheet("color: #D2E4E0;");
    QWidget *cardDog    = buildStatCard("Chiens", "\U0001F436", m_lblDogCount, m_lblDogPct, m_barDog, "#D97706");
    QWidget *cardCat    = buildStatCard("Chats", "\U0001F431", m_lblCatCount, m_lblCatPct, m_barCat, "#2563EB");
    QWidget *cardRabbit = buildStatCard("Lapins", "\U0001F430", m_lblRabbitCount, m_lblRabbitPct, m_barRabbit, "#9333EA");
    auto *sep2 = new QFrame;
    sep2->setFrameShape(QFrame::VLine);
    sep2->setStyleSheet("color: #D2E4E0;");
    auto *aiBox = new QVBoxLayout;
    aiBox->setAlignment(Qt::AlignCenter);
    m_btnAlertStatus = new QPushButton("\u25cf Sant\u00e9 OK");
    m_btnAlertStatus->setCursor(Qt::PointingHandCursor);
    m_btnAlertStatus->setStyleSheet(
        "QPushButton { font-size: 12px; font-weight: 800; color: #2E7D52; background: #D5F0DF; border-radius: 8px; padding: 6px 12px; border:none; }"
        "QPushButton:hover { background: #C1EACD; }"
    );
    connect(m_btnAlertStatus, &QPushButton::clicked, this, &AnimauxPage::verifierSanteAnimaux);
    m_btnSmartAnalysis = new QPushButton("\U0001F4A1 Bilan AI");
    m_btnSmartAnalysis->setCursor(Qt::PointingHandCursor);
    m_btnSmartAnalysis->setStyleSheet(
        "QPushButton { font-size: 12px; font-weight: 800; color: #1F5F54; background: #DDEFEB; border-radius: 8px; padding: 6px 12px; border:1px solid #BEE0D8; }"
        "QPushButton:hover { background: #CBE5DF; }"
    );
    connect(m_btnSmartAnalysis, &QPushButton::clicked, this, &AnimauxPage::executerAnalyseIntelligente);
    aiBox->addWidget(m_btnAlertStatus);
    aiBox->addSpacing(4);
    aiBox->addWidget(m_btnSmartAnalysis);
    statLayout->addLayout(totalBox);
    statLayout->addWidget(sep1);
    statLayout->addWidget(cardDog, 1);
    statLayout->addWidget(cardCat, 1);
    statLayout->addWidget(cardRabbit, 1);
    statLayout->addWidget(sep2);
    statLayout->addLayout(aiBox);
    return statPanel;
}

// ============================================================ centre
QWidget *AnimauxPage::buildCenter()
{
    auto *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; }");
    auto *container = new QWidget;
    auto *lay = new QVBoxLayout(container);
    lay->setContentsMargins(20, 16, 20, 14);
    lay->setSpacing(14);

    auto *head = new QHBoxLayout;
    auto *icon = new QLabel("\U0001F43E");
    icon->setObjectName("headIcon");
    icon->setFixedSize(48, 48);
    icon->setAlignment(Qt::AlignCenter);
    auto *titles = new QVBoxLayout;
    auto *title = new QLabel("Gestion des animaux");
    title->setObjectName("title");
    auto *sub = new QLabel("Ajoutez, consultez et g\u00e9rez vos animaux avec l'assistance IA");
    sub->setObjectName("subtitle");
    titles->addWidget(title);
    titles->addWidget(sub);
    auto *btnAdd = new QPushButton("\uff0b Ajouter un animal");
    btnAdd->setObjectName("primary");
    btnAdd->setFixedHeight(42);
    btnAdd->setCursor(Qt::PointingHandCursor);
    head->addWidget(icon);
    head->addLayout(titles);
    head->addStretch();
    head->addWidget(btnAdd);
    lay->addLayout(head);

    lay->addWidget(buildAIAskWidget());

    auto *bar = new QHBoxLayout;
    m_search = new QLineEdit;
    m_search->setPlaceholderText("\U0001F50d  Rechercher par NOM ou par ID (ex: ANI001)...");
    m_search->setMinimumHeight(38);
    m_filterEspece = new QComboBox;
    m_filterEspece->addItems({"Toutes les esp\u00e8ces", "Chien", "Chat", "Lapin"});
    m_filterEspece->setMinimumHeight(38);
    m_filterEspece->setFixedWidth(160);
    auto *btnSort = new QPushButton("\U0001F524  Trier par nom");
    btnSort->setObjectName("secondary");
    btnSort->setMinimumHeight(38);
    bar->addWidget(m_search, 1);
    bar->addWidget(m_filterEspece);
    bar->addWidget(btnSort);
    lay->addLayout(bar);

    auto *quick = new QHBoxLayout;
    auto addQuick = [&](const QString &txt, const QString &obj) {
        auto *b = new QPushButton(txt);
        b->setObjectName(obj);
        b->setMinimumHeight(34);
        b->setCursor(Qt::PointingHandCursor);
        quick->addWidget(b);
        return b;
    };
    auto *qAdd  = addQuick("\uff0b  Ajouter", "quick");
    auto *qView = addQuick("\U0001F441  Consulter", "quick");
    auto *qEdit = addQuick("\u270e  Modifier", "quick");
    auto *qDel  = addQuick("\U0001F5D1  Supprimer", "quickDanger");
    auto *qSrch = addQuick("\U0001F50d  Rechercher par ID", "quick");
    lay->addLayout(quick);

    m_table = new QTableWidget(0, 10);
    m_table->setMinimumHeight(280);
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
    m_table->setColumnWidth(9, 120);
    lay->addWidget(m_table);

    m_count = new QLabel;
    m_count->setObjectName("muted");
    auto *foot = new QHBoxLayout;
    foot->addWidget(m_count);
    foot->addStretch();
    auto *pg = new QPushButton("1");
    pg->setObjectName("page");
    pg->setFixedSize(34, 30);
    foot->addWidget(pg);
    lay->addLayout(foot);

    lay->addWidget(buildForm());
    lay->addWidget(buildSmartDashboard());
    scrollArea->setWidget(container);

    connect(btnSort, &QPushButton::clicked, this, &AnimauxPage::trierParNom);
    connect(m_search, &QLineEdit::textChanged, this, &AnimauxPage::applyFilter);
    connect(m_filterEspece, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AnimauxPage::applyFilter);
    connect(m_table, &QTableWidget::currentCellChanged, this, [this](int row) { if (row >= 0) showDetail(row); });
    auto focusForm = [this] { clearForm(); m_fNom->setFocus(); };
    connect(btnAdd, &QPushButton::clicked, this, focusForm);
    connect(qAdd, &QPushButton::clicked, this, focusForm);
    connect(qView, &QPushButton::clicked, this, [this] { int i = requireSelection(); if (i >= 0) showDetail(i); });
    connect(qEdit, &QPushButton::clicked, this, [this] { int i = requireSelection(); if (i >= 0) editAnimal(m_animals[i].id); });
    connect(qDel, &QPushButton::clicked, this, [this] { int i = requireSelection(); if (i >= 0) deleteAnimal(m_animals[i].id); });
    connect(qSrch, &QPushButton::clicked, this, [this] { m_search->setFocus(); });
    return scrollArea;
}

// ============================================================ MISE À JOUR DU DASHBOARD
void AnimauxPage::updateDashboard()
{
    int total = m_animals.size();
    m_lblTotal->setText(QString::number(total));
    if (total == 0) {
        m_lblAvgWeight->setText("Poids moy: 0 kg");
        m_lblDogCount->setText("0"); m_lblDogPct->setText("0%"); m_barDog->setValue(0);
        m_lblCatCount->setText("0"); m_lblCatPct->setText("0%"); m_barCat->setValue(0);
        m_lblRabbitCount->setText("0"); m_lblRabbitPct->setText("0%"); m_barRabbit->setValue(0);
        m_btnAlertStatus->setText("\u25cf Aucun animal");
        m_btnAlertStatus->setStyleSheet(
            "QPushButton { font-size: 12px; font-weight: 800; color: #6A7B80; background: #E2ECE9; border-radius: 8px; padding: 6px 12px; border:none; }"
        );
        return;
    }
    int dogs = 0, cats = 0, rabbits = 0, alertes = 0;
    double totalWeight = 0;
    for (const auto &a : m_animals) {
        if (a.espece == "Chien") dogs++;
        else if (a.espece == "Chat") cats++;
        else if (a.espece == "Lapin") rabbits++;
        if (a.statut != "En bonne sant\u00e9") alertes++;
        totalWeight += a.poids;
    }
    double avgWeight = totalWeight / total;
    m_lblAvgWeight->setText(QString("Poids moy: %1 kg").arg(avgWeight, 0, 'f', 1));
    int pDog = qRound((double)dogs / total * 100.0);
    int pCat = qRound((double)cats / total * 100.0);
    int pRabbit = qRound((double)rabbits / total * 100.0);
    m_lblDogCount->setText(QString::number(dogs));
    m_lblDogPct->setText(QString("%1%").arg(pDog));
    m_barDog->setValue(pDog);
    m_lblCatCount->setText(QString::number(cats));
    m_lblCatPct->setText(QString("%1%").arg(pCat));
    m_barCat->setValue(pCat);
    m_lblRabbitCount->setText(QString::number(rabbits));
    m_lblRabbitPct->setText(QString("%1%").arg(pRabbit));
    m_barRabbit->setValue(pRabbit);
    if (alertes > 0) {
        m_btnAlertStatus->setText(QString("\u26a0\ufe0f %1 Alerte(s)").arg(alertes));
        m_btnAlertStatus->setStyleSheet(
            "QPushButton { font-size: 12px; font-weight: 800; color: #C0392B; background: #FBDADA; border-radius: 8px; padding: 6px 12px; border:none; }"
            "QPushButton:hover { background: #F8C2C2; }"
        );
    } else {
        m_btnAlertStatus->setText("\u25cf Sant\u00e9 OK");
        m_btnAlertStatus->setStyleSheet(
            "QPushButton { font-size: 12px; font-weight: 800; color: #2E7D52; background: #D5F0DF; border-radius: 8px; padding: 6px 12px; border:none; }"
            "QPushButton:hover { background: #C1EACD; }"
        );
    }
}

// ============================================================ ANALYSE AI INTELLIGENTE
void AnimauxPage::executerAnalyseIntelligente()
{
    if (m_animals.isEmpty()) {
        QMessageBox::information(this, "\U0001F4A1 PetAI Engine", "Ajoutez des animaux pour g\u00e9n\u00e9rer une analyse d'optimisation.");
        return;
    }
    QStringList recommandations;
    int seniors = 0;
    int sousSurveillance = 0;
    for (const auto &a : m_animals) {
        int months = (QDate::currentDate().year() - a.naissance.year()) * 12 + QDate::currentDate().month() - a.naissance.month();
        if (months >= 84) seniors++;
        if (a.statut != "En bonne sant\u00e9") sousSurveillance++;
    }
    recommandations.append(QString("\U0001F4CA **Statistiques Globales** : %1 animaux g\u00e9r\u00e9s.").arg(m_animals.size()));
    if (sousSurveillance > 0) {
        recommandations.append(QString("\U0001F6A8 **Sant\u00e9** : %1 animal(aux) requi\u00e8rent un suivi m\u00e9dical.").arg(sousSurveillance));
    } else {
        recommandations.append("\u2705 **Sant\u00e9** : Aucun cas critique d\u00e9tect\u00e9.");
    }
    if (seniors > 0) {
        recommandations.append(QString("\U0001F475 **Seniors** : %1 animal(aux) ont plus de 7 ans (bilan g\u00e9riatrique recommand\u00e9).").arg(seniors));
    }
    recommandations.append("\U0001F4A1 **Conseil** : V\u00e9rifiez les informations du docteur v\u00e9t\u00e9rinaire (Dr. Nessma Tounsi) avant toute mise \u00e0 jour.");
    QMessageBox::information(this, "\U0001F4A1 Bilan Intelligent (PetAI)", recommandations.join("\n"));
}

// ============================================================ formulaire
QWidget *AnimauxPage::buildForm()
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
    connect(photoBtn, &QPushButton::clicked, this, &AnimauxPage::choosePhoto);
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
    m_fIdProp = new QLineEdit; m_fIdProp->setPlaceholderText("Ex: PR002");
    m_fIdVet = new QLineEdit;  m_fIdVet->setPlaceholderText("Ex: VET001");
    g->addWidget(lbl("Nom *"), 1, 1);          g->addWidget(m_fNom, 2, 1);
    g->addWidget(lbl("Esp\u00e8ce *"), 1, 2);  g->addWidget(m_fEspece, 2, 2);
    g->addWidget(lbl("Race *"), 1, 3);         g->addWidget(m_fRace, 2, 3);
    g->addWidget(lbl("Sexe *"), 3, 1);         g->addWidget(m_fSexe, 4, 1);
    g->addWidget(lbl("Date de naissance *"), 3, 2); g->addWidget(m_fDate, 4, 2);
    g->addWidget(lbl("Poids (kg) *"), 3, 3);   g->addWidget(m_fPoids, 4, 3);
    g->addWidget(lbl("Description"), 5, 1, 1, 3);
    g->addWidget(m_fDesc, 6, 1, 1, 3);
    g->addWidget(lbl("ID du propri\u00e9taire *"), 7, 1, 1, 1);
    g->addWidget(lbl("ID du docteur *"), 7, 2, 1, 1);
    g->addWidget(m_fIdProp, 8, 1);
    g->addWidget(m_fIdVet, 8, 2);
    auto *btnCancel = new QPushButton("Annuler");
    btnCancel->setObjectName("secondary");
    m_btnSave = new QPushButton("\U0001F4BE  Enregistrer");
    m_btnSave->setObjectName("primary");
    auto *hb = new QHBoxLayout;
    hb->addStretch();
    hb->addWidget(btnCancel);
    hb->addWidget(m_btnSave);
    g->addLayout(hb, 8, 3);
    g->setColumnStretch(1, 1); g->setColumnStretch(2, 1); g->setColumnStretch(3, 1);
    connect(btnCancel, &QPushButton::clicked, this, &AnimauxPage::clearForm);
    connect(m_btnSave, &QPushButton::clicked, this, &AnimauxPage::saveAnimal);
    return m_form;
}

// ============================================================ panneau détail
QWidget *AnimauxPage::buildDetail()
{
    auto *panel = new QFrame;
    panel->setObjectName("detail");
    panel->setFixedWidth(315);
    auto *lay = new QVBoxLayout(panel);
    lay->setContentsMargins(18, 14, 18, 16);
    lay->setSpacing(8);
    auto *back = new QPushButton("\u2190  Retour \u00e0 la liste");
    back->setObjectName("link");
    back->setCursor(Qt::PointingHandCursor);
    lay->addWidget(back, 0, Qt::AlignLeft);
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
    lay->addSpacing(4);
    row("\U0001F464", "ID du propri\u00e9taire", m_dIdProp);
    row("\U0001F464", "Nom du propri\u00e9taire", m_dNomProp);
    row("\U0001FA7A", "ID du docteur", m_dIdVet);
    row("\U0001FA7A", "Nom du docteur", m_dNomVet);
    lay->addStretch();
    auto *btns = new QHBoxLayout;
    auto *bView = new QPushButton("\U0001F441 Consulter"); bView->setObjectName("secondary");
    auto *bEdit = new QPushButton("\u270e Modifier");       bEdit->setObjectName("primary");
    auto *bDel  = new QPushButton("\U0001F5D1 Supprimer"); bDel->setObjectName("danger");
    btns->addWidget(bView); btns->addWidget(bEdit); btns->addWidget(bDel);
    lay->addLayout(btns);
    connect(back, &QPushButton::clicked, this, [this] {
        m_table->clearSelection();
        m_table->setCurrentCell(-1, -1);
        clearDetail();
    });
    connect(bView, &QPushButton::clicked, this, [this] { int i = requireSelection(); if (i >= 0) showDetail(i); });
    connect(bEdit, &QPushButton::clicked, this, [this] { int i = requireSelection(); if (i >= 0) editAnimal(m_animals[i].id); });
    connect(bDel, &QPushButton::clicked, this, [this] { int i = requireSelection(); if (i >= 0) deleteAnimal(m_animals[i].id); });
    return panel;
}

// ============================================================ logique
int AnimauxPage::indexOf(const QString &id) const
{
    for (int i = 0; i < m_animals.size(); ++i)
        if (m_animals[i].id == id) return i;
    return -1;
}

int AnimauxPage::currentIndex() const
{
    int r = m_table->currentRow();
    return (r >= 0 && r < m_animals.size()) ? r : -1;
}

int AnimauxPage::requireSelection()
{
    int i = currentIndex();
    if (i < 0)
        QMessageBox::information(this, "Aucune s\u00e9lection", "Veuillez d'abord s\u00e9lectionner un animal dans le tableau.");
    return i;
}

QString AnimauxPage::nextId() const
{
    int mx = 0;
    for (const auto &a : m_animals) mx = qMax(mx, a.id.mid(3).toInt());
    return QString("ANI%1").arg(mx + 1, 3, 10, QChar('0'));
}

void AnimauxPage::refreshTable()
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
        bool ok = a.statut == "En bonne sant\u00e9";
        m_table->setCellWidget(i, 8, wrap(badge(a.statut, ok ? "#D5F0DF" : "#FFF0BF", ok ? "#2E7D52" : "#B8860B")));
        auto *act = new QWidget;
        auto *al = new QHBoxLayout(act);
        al->setContentsMargins(0, 0, 0, 0);
        al->setSpacing(6);
        auto mkBtn = [&](const QString &t, const QString &obj) {
            auto *b = new QToolButton;
            b->setText(t);
            b->setObjectName(obj);
            b->setFixedSize(28, 28);
            b->setCursor(Qt::PointingHandCursor);
            al->addWidget(b);
            return b;
        };
        auto *bv = mkBtn("\U0001F441", "actView");
        auto *be = mkBtn("\u270e", "actEdit");
        auto *bd = mkBtn("\U0001F5D1", "actDel");
        const QString id = a.id;
        connect(bv, &QToolButton::clicked, this, [this, id] {
            int k = indexOf(id); if (k >= 0) { m_table->selectRow(k); showDetail(k); }
        });
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

void AnimauxPage::applyFilter()
{
    const QString q = m_search->text().trimmed().toLower();
    const QString esp = m_filterEspece->currentIndex() > 0 ? m_filterEspece->currentText() : "";
    int visible = 0;
    for (int i = 0; i < m_animals.size(); ++i) {
        const Animal &a = m_animals[i];
        bool match = (q.isEmpty() || a.nom.toLower().contains(q) || a.id.toLower().contains(q))
                     && (esp.isEmpty() || a.espece == esp);
        m_table->setRowHidden(i, !match);
        if (match) ++visible;
    }
    const QString plural = visible > 1 ? "s" : "";
    m_count->setText(QString("%1 animal%2 trouv%2").arg(visible).arg(plural));
}

void AnimauxPage::trierParNom()
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

void AnimauxPage::showDetail(int i)
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
    m_dIdProp->setText(a.idProprio);
    m_dNomProp->setText(a.nomProprio);
    m_dIdVet->setText(a.idVet);
    m_dNomVet->setText(a.nomVet);
}

void AnimauxPage::clearDetail()
{
    m_dPhoto->setPixmap(QPixmap());
    m_dNom->setText("Aucun animal s\u00e9lectionn\u00e9");
    m_dStatut->clear();
    m_dStatut->setStyleSheet(QString());
    m_dStatut->hide();
    m_dId->clear();
    m_dId->hide();
    for (QLabel *l : {m_dEspece, m_dRace, m_dSexe, m_dNaiss, m_dPoids, m_dAge, m_dDesc, m_dIdProp, m_dNomProp, m_dIdVet, m_dNomVet})
        l->clear();
}

void AnimauxPage::choosePhoto()
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

void AnimauxPage::clearForm()
{
    m_editId.clear();
    m_photoPath.clear();
    m_formTitle->setText("\U0001F43E  Ajouter un animal");
    m_btnSave->setText("\U0001F4BE  Enregistrer");
    m_formPhoto->setPixmap(QPixmap());
    m_formPhoto->setText("\U0001F4F7\nAjouter une photo\nJPG, PNG (max 5 Mo)");
    m_fNom->clear(); m_fRace->clear(); m_fIdProp->clear(); m_fIdVet->clear();
    m_fEspece->setCurrentIndex(-1); m_fSexe->setCurrentIndex(-1);
    m_fDate->setDate(QDate::currentDate());
    m_fPoids->setValue(0);
    m_fDesc->clear();
}

void AnimauxPage::editAnimal(const QString &id)
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
    m_fDate->setDate(a.naissance);
    m_fPoids->setValue(a.poids);
    m_fDesc->setPlainText(a.description);
    m_fIdProp->setText(a.idProprio);
    m_fIdVet->setText(a.idVet);
    QPixmap px;
    if (!a.photo.isEmpty() && px.load(a.photo))
        m_formPhoto->setPixmap(px.scaled(100, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else {
        m_formPhoto->setPixmap(QPixmap());
        m_formPhoto->setText("\U0001F4F7\nAjouter une photo\nJPG, PNG (max 5 Mo)");
    }
    m_fNom->setFocus();
}

void AnimauxPage::saveAnimal()
{
    if (m_fNom->text().trimmed().isEmpty() || m_fEspece->currentIndex() < 0
        || m_fRace->text().trimmed().isEmpty() || m_fSexe->currentIndex() < 0
        || m_fIdProp->text().trimmed().isEmpty() || m_fIdVet->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champs obligatoires", "Veuillez remplir tous les champs marqu\u00e9s d'un *.");
        return;
    }
    if (m_fPoids->value() <= 0) {
        QMessageBox::warning(this, "Poids invalide", "Le poids doit \u00eatre sup\u00e9rieur \u00e0 0.");
        return;
    }
    int idx = m_editId.isEmpty() ? -1 : indexOf(m_editId);
    Animal a = idx >= 0 ? m_animals[idx] : Animal();
    if (idx < 0) {
        a.id = nextId();
        a.statut = "En bonne sant\u00e9";
    }
    a.nom = m_fNom->text().trimmed();
    a.espece = m_fEspece->currentText();
    a.race = m_fRace->text().trimmed();
    a.sexe = m_fSexe->currentText();
    a.naissance = m_fDate->date();
    a.poids = m_fPoids->value();
    a.description = m_fDesc->toPlainText().trimmed();
    a.idProprio = m_fIdProp->text().trimmed().toUpper();
    a.nomProprio = ownerName(a.idProprio);
    a.idVet = m_fIdVet->text().trimmed().toUpper();
    a.nomVet = vetName(a.idVet);
    a.photo = m_photoPath;
    if (idx >= 0) m_animals[idx] = a; else m_animals.append(a);
    {
        QSignalBlocker b1(m_search);
        QSignalBlocker b2(m_filterEspece);
        m_search->clear();
        m_filterEspece->setCurrentIndex(0);
    }
    refreshTable();
    updateDashboard();
    int sel = indexOf(a.id);
    m_table->selectRow(sel);
    showDetail(sel);
    clearForm();
}

void AnimauxPage::deleteAnimal(const QString &id)
{
    int i = indexOf(id);
    if (i < 0) return;
    auto r = QMessageBox::question(this, "Supprimer", QString("Supprimer %1 (%2) ?").arg(m_animals[i].nom, id));
    if (r != QMessageBox::Yes) return;
    if (m_editId == id) clearForm();
    m_animals.removeAt(i);
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

void AnimauxPage::verifierSanteAnimaux()
{
    QStringList alertes;
    for (const auto &a : m_animals) {
        if (a.statut != "En bonne sant\u00e9") {
            alertes.append(QString("\u2022 %1 (%2) - Statut: %3").arg(a.nom, a.espece, a.statut));
        }
    }
    if (!alertes.isEmpty()) {
        QMessageBox::warning(this, "\u26a0\ufe0f Alerte Sant\u00e9 Animaux",
                             "Attention : Les animaux suivants n\u00e9cessitent une prise en charge m\u00e9dicale :\n" + alertes.join("\n"));
    } else {
        QMessageBox::information(this, "\u25cf \u00c9tat de Sant\u00e9 Global",
                                 "Tous les animaux sont actuellement enregistr\u00e9s en bonne sant\u00e9 !");
    }
}

// ============================================================ style (QSS)
void AnimauxPage::applyStyle()
{
    setStyleSheet(R"(
* { font-size:9pt; }
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
#aiCard {
background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #EAF6F3, stop:1 #F2FAF8);
border: 1px solid #C5E3DC;
border-radius: 12px;
}
#statPanel {
background: white;
border: 1px solid #D1E4E0;
border-radius: 14px;
}
#statCard {
background: #F8FCFB;
border: 1px solid #E2ECE9;
border-radius: 10px;
}
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
QPushButton#quick { background:#DDEFEB; color:#1F5F54; border:none; border-radius:10px; font-weight:600; }
QPushButton#quick:hover { background:#CBE5DF; }
QPushButton#quickDanger { background:#FBE2E2; color:#C0392B; border:none; border-radius:10px; font-weight:600; }
QPushButton#quickDanger:hover { background:#F7CFCF; }
QPushButton#page { background:#2F7F73; color:white; border:none; border-radius:8px; font-weight:700; }
QPushButton#link { background:transparent; border:none; color:#33454B; }
QPushButton#link:hover { color:#2F7F73; }
QTableWidget { background:white; border:1px solid #DCEBE8; border-radius:12px; color:#243238; gridline-color:transparent; outline:0; }
QTableWidget::item { border-bottom:1px solid #EEF3F2; padding-left:6px; }
QTableWidget::item:selected { background:#DCF0EC; color:#243238; }
QHeaderView::section { background:white; color:#33454B; font-weight:700; border:none; border-bottom:1px solid #E3ECEA; padding:10px 6px; }
QToolButton#actView { background:#DDEFEB; border:none; border-radius:7px; }
QToolButton#actEdit { background:#E8EEF2; border:none; border-radius:7px; }
QToolButton#actDel  { background:#FBDADA; border:none; border-radius:7px; }
QToolButton:hover { background:#C9E3DD; }
#card { background:white; border:1px solid #DCEBE8; border-radius:12px; }
#formTitle { font-size:13px; font-weight:700; color:#243238; }
#fieldLabel { font-size:10px; font-weight:700; color:#33454B; }
QPushButton#photoBox { background:#F6FBFA; border:2px dashed #BFD9D3; border-radius:10px; }
QPushButton#photoBox:hover { background:#EAF6F3; }
#detail { background:white; border-left:1px solid #DCEBE8; }
#detailName { font-size:22px; font-weight:800; color:#1E2B30; }
#idChip { background:#F1F5F5; border:1px solid #E1E8E8; border-radius:8px; padding:3px 10px; font-size:11px; color:#4A5A5A; }
#detailKey { color:#33454B; font-size:12px; }
#detailVal { color:#1E2B30; font-size:12px; font-weight:600; }
#detailSection { font-size:13px; font-weight:800; color:#1E2B30; }
#descBox { background:#EAF4F2; border:1px solid #D5E7E3; border-radius:8px; padding:8px; color:#33454B; font-size:12px; }
)");
}