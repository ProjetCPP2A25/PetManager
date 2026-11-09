#include "rendezvous.h"

#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QTime>
#include <QVariant>

/* ==================================================================
 *  RendezvousTableModel : mise en forme de l'affichage
 * ================================================================== */
QVariant RendezvousTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        const QVariant brut = QSqlQueryModel::data(index, role);
        switch (index.column()) {
        case 0:
            return Rendezvous::formaterId(brut.toInt());
        case 1:
            return Rendezvous::formaterDate(brut.toDate());
        case 6:
            return QVariant();                    // colonne « Actions » : dessinée par un delegate
        default:
            break;
        }
    }
    return QSqlQueryModel::data(index, role);
}

/* ==================================================================
 *  Rendezvous
 * ================================================================== */
// Constructeur avec paramètres
Rendezvous::Rendezvous(int id, QDate date, QString heure, QString statut, QString motif, int idAnimal)
{
    this->id = id;
    this->date = date;
    this->heure = heure;
    this->statut = statut;
    this->motif = motif;
    this->idAnimal = idAnimal;
}

// Destructeur : aucune ressource dynamique à libérer
Rendezvous::~Rendezvous() {}

QStringList Rendezvous::motifsDisponibles()
{
    return {"Consultation", "Vaccination", "Contrôle", "Urgence"};
}

QStringList Rendezvous::statutsDisponibles()
{
    return {"Confirmé", "En attente", "Annulé", "Terminé"};
}

// L'ordre doit correspondre au tableau ORDRES de rechercher()
QStringList Rendezvous::criteresTri()
{
    return {"Aucun tri", "Date la plus proche", "Date la plus lointaine", "Heure"};
}

QString Rendezvous::formaterId(int id)
{
    return QString("RDV%1").arg(id, 3, 10, QChar('0'));
}

QString Rendezvous::formaterDate(const QDate &d)
{
    return d.toString("dd/MM/yyyy");
}

// Traduit les erreurs Oracle courantes en messages compréhensibles
QString Rendezvous::messageSql(const QString &texteErreur)
{
    if (texteErreur.contains("ORA-00001"))
        return "Un rendez-vous avec cet identifiant existe déjà.";
    if (texteErreur.contains("ORA-02291"))
        return "L'animal choisi n'existe pas dans la base.";
    if (texteErreur.contains("ORA-02290"))
        return "Une valeur saisie ne respecte pas les contraintes de la base (statut ou heure).";
    if (texteErreur.contains("ORA-01400"))
        return "La colonne IDEMPLOYE doit accepter NULL : ALTER TABLE RENDEZVOUS MODIFY (IDEMPLOYE NULL);";
    return "Erreur base de données : " + texteErreur;
}

/* ------------------------------------------------------------------
 *  Contrôle de saisie
 * ------------------------------------------------------------------ */
bool Rendezvous::existe(int id)
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM RENDEZVOUS WHERE IDRDV = :id");
    q.bindValue(":id", id);
    return q.exec() && q.next() && q.value(0).toInt() > 0;
}

bool Rendezvous::animalExiste(int idAnimal)
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM ANIMAL WHERE IDANIMAL = :id");
    q.bindValue(":id", idAnimal);
    return q.exec() && q.next() && q.value(0).toInt() > 0;
}

// Un animal (ou un employé, s'il est renseigné) ne peut pas avoir deux rendez-vous au même moment
bool Rendezvous::creneauOccupe() const
{
    if (statut == "Annulé")
        return false;

    QSqlQuery q;
    QString sql = "SELECT COUNT(*) FROM RENDEZVOUS "
                  "WHERE DATERDV = :d AND HEURE = :h AND STATUT <> :annule AND IDRDV <> :id ";
    sql += (idEmploye > 0) ? "AND (IDANIMAL = :a OR IDEMPLOYE = :e)" : "AND IDANIMAL = :a";
    q.prepare(sql);
    q.bindValue(":d", date);
    q.bindValue(":h", heure);
    q.bindValue(":annule", "Annulé");
    q.bindValue(":id", id);
    q.bindValue(":a", idAnimal);
    if (idEmploye > 0)
        q.bindValue(":e", idEmploye);
    return q.exec() && q.next() && q.value(0).toInt() > 0;
}

bool Rendezvous::valider(bool creation)
{
    m_erreur.clear();

    if (id <= 0) {
        m_erreur = "L'identifiant du rendez-vous est obligatoire (nombre entier positif).";
        return false;
    }
    if (creation && existe(id)) {
        m_erreur = "Un rendez-vous avec cet identifiant existe déjà.";
        return false;
    }
    if (!date.isValid()) {
        m_erreur = "La date du rendez-vous est invalide.";
        return false;
    }
    static const QRegularExpression reHeure("^([01]\\d|2[0-3]):[0-5]\\d$");
    if (!reHeure.match(heure).hasMatch()) {
        m_erreur = "L'heure est invalide (format HH:mm).";
        return false;
    }
    if (creation) {
        if (date < QDate::currentDate()) {
            m_erreur = "La date d'un nouveau rendez-vous ne peut pas être dans le passé.";
            return false;
        }
        if (date == QDate::currentDate() && QTime::fromString(heure, "HH:mm") < QTime::currentTime()) {
            m_erreur = "Cette heure est déjà passée aujourd'hui.";
            return false;
        }
    }
    if (idAnimal <= 0) {
        m_erreur = "Saisissez l'ID de l'animal (nombre entier positif).";
        return false;
    }
    if (!animalExiste(idAnimal)) {
        m_erreur = QString("Aucun animal avec l'ID %1 dans la base.").arg(idAnimal);
        return false;
    }
    if (motif.trimmed().isEmpty() || motif.size() > 200) {
        m_erreur = "Le motif est obligatoire (200 caractères au maximum).";
        return false;
    }
    if (!statutsDisponibles().contains(statut)) {
        m_erreur = "Le statut est invalide.";
        return false;
    }
    if (creneauOccupe()) {
        m_erreur = "Cet animal a déjà un rendez-vous à cette date et à cette heure.";
        return false;
    }
    return true;
}

/* ------------------------------------------------------------------
 *  CREATE
 * ------------------------------------------------------------------ */
int Rendezvous::prochainId()
{
    QSqlQuery q;
    if (q.exec("SELECT COALESCE(MAX(IDRDV), 0) + 1 FROM RENDEZVOUS") && q.next())
        return q.value(0).toInt();
    return 1;
}

bool Rendezvous::ajouter()
{
    if (!valider(true))
        return false;

    QSqlQuery q;
    q.prepare("INSERT INTO RENDEZVOUS (IDRDV, DATERDV, HEURE, STATUT, MOTIF, IDANIMAL, IDEMPLOYE) "
              "VALUES (:id, :date, :heure, :statut, :motif, :animal, :employe)");
    q.bindValue(":id", id);
    q.bindValue(":date", date);
    q.bindValue(":heure", heure);
    q.bindValue(":statut", statut);
    q.bindValue(":motif", motif.trimmed());
    q.bindValue(":animal", idAnimal);
    q.bindValue(":employe", idEmploye > 0 ? QVariant(idEmploye)
                                          : QVariant(QMetaType::fromType<int>()));   // NULL si non renseigné

    if (!q.exec()) {
        m_erreur = messageSql(q.lastError().text());
        return false;
    }
    return true;
}

/* ------------------------------------------------------------------
 *  UPDATE
 * ------------------------------------------------------------------ */
bool Rendezvous::modifier()
{
    if (id <= 0) {
        m_erreur = "Aucun rendez-vous sélectionné.";
        return false;
    }
    if (!valider(false))
        return false;

    QSqlQuery q;
    QString sql = "UPDATE RENDEZVOUS SET DATERDV = :date, HEURE = :heure, STATUT = :statut, "
                  "MOTIF = :motif, IDANIMAL = :animal";
    if (idEmploye > 0)
        sql += ", IDEMPLOYE = :employe";             // sinon l'employé existant reste inchangé
    sql += " WHERE IDRDV = :id";
    q.prepare(sql);
    q.bindValue(":date", date);
    q.bindValue(":heure", heure);
    q.bindValue(":statut", statut);
    q.bindValue(":motif", motif.trimmed());
    q.bindValue(":animal", idAnimal);
    if (idEmploye > 0)
        q.bindValue(":employe", idEmploye);
    q.bindValue(":id", id);

    if (!q.exec()) {
        m_erreur = messageSql(q.lastError().text());
        return false;
    }
    if (q.numRowsAffected() == 0) {
        m_erreur = "Rendez-vous introuvable (il a peut-être été supprimé).";
        return false;
    }
    return true;
}

/* ------------------------------------------------------------------
 *  DELETE
 * ------------------------------------------------------------------ */
bool Rendezvous::supprimer(int idRdv)
{
    m_erreur.clear();
    QSqlQuery q;
    q.prepare("DELETE FROM RENDEZVOUS WHERE IDRDV = :id");
    q.bindValue(":id", idRdv);

    if (!q.exec()) {
        m_erreur = messageSql(q.lastError().text());
        return false;
    }
    if (q.numRowsAffected() == 0) {
        m_erreur = "Rendez-vous introuvable (il a peut-être déjà été supprimé).";
        return false;
    }
    return true;
}

/* ------------------------------------------------------------------
 *  READ : liste
 * ------------------------------------------------------------------ */
QSqlQueryModel *Rendezvous::afficher() const
{
    return rechercher(QString(), QString(), 0);         // liste complète
}

/* ------------------------------------------------------------------
 *  MÉTIERS : recherche par ID, filtre de statut, tri
 * ------------------------------------------------------------------ */
QSqlQueryModel *Rendezvous::rechercher(const QString &recherche, const QString &statutFiltre, int tri) const
{
    // Le tri est choisi dans une liste fixe (jamais dans le texte saisi) : pas d'injection SQL
    static const char *ORDRES[] = {
        "r.IDRDV ASC",
        "r.DATERDV ASC, r.HEURE ASC, r.IDRDV ASC",
        "r.DATERDV DESC, r.HEURE DESC, r.IDRDV ASC",
        "r.HEURE ASC, r.DATERDV ASC, r.IDRDV ASC"
    };
    const int nbOrdres = int(sizeof(ORDRES) / sizeof(ORDRES[0]));
    const int ordre = (tri >= 0 && tri < nbOrdres) ? tri : 0;

    // « RDV001 » ou « 1 » : on garde les chiffres
    const QString texte = recherche.trimmed();
    QString chiffres;
    for (const QChar &ch : texte)
        if (ch.isDigit())
            chiffres += ch;

    QStringList conditions;
    if (!texte.isEmpty())
        conditions << (chiffres.isEmpty() ? QString("1 = 0")
                                          : QString("CAST(r.IDRDV AS VARCHAR2(20)) LIKE :idr"));
    const bool filtreStatut = !statutFiltre.isEmpty() && statutFiltre != "Tous";
    if (filtreStatut)
        conditions << "r.STATUT = :statut";

    QString sql = "SELECT r.IDRDV, r.DATERDV, r.HEURE, r.IDANIMAL, r.MOTIF, r.STATUT, 0 AS ACTIONS FROM RENDEZVOUS r";
    if (!conditions.isEmpty())
        sql += " WHERE " + conditions.join(" AND ");
    sql += QString(" ORDER BY %1").arg(ORDRES[ordre]);

    QSqlQuery q;
    q.prepare(sql);
    if (!chiffres.isEmpty())
        q.bindValue(":idr", "%" + QString::number(chiffres.toInt()) + "%");
    if (filtreStatut)
        q.bindValue(":statut", statutFiltre);
    q.exec();

    auto *model = new RendezvousTableModel;
    model->setQuery(std::move(q));
    if (model->lastError().isValid())
        m_erreur = "Erreur base de données : " + model->lastError().text();

    model->setHeaderData(0, Qt::Horizontal, "ID RDV");
    model->setHeaderData(1, Qt::Horizontal, "Date");
    model->setHeaderData(2, Qt::Horizontal, "Heure");
    model->setHeaderData(3, Qt::Horizontal, "ID Animal");
    model->setHeaderData(4, Qt::Horizontal, "Motif");
    model->setHeaderData(5, Qt::Horizontal, "Statut");
    model->setHeaderData(6, Qt::Horizontal, "Actions");
    return model;
}

/* ------------------------------------------------------------------
 *  READ : un seul rendez-vous (détails, formulaire de modification)
 * ------------------------------------------------------------------ */
bool Rendezvous::charger(int idRdv)
{
    QSqlQuery q;
    q.prepare("SELECT r.IDRDV, r.DATERDV, r.HEURE, r.STATUT, r.MOTIF, r.IDANIMAL, r.IDEMPLOYE, a.NOM, a.ESPECE "
              "FROM RENDEZVOUS r LEFT JOIN ANIMAL a ON a.IDANIMAL = r.IDANIMAL "
              "WHERE r.IDRDV = :id");
    q.bindValue(":id", idRdv);
    if (!q.exec() || !q.next()) {
        m_erreur = "Rendez-vous introuvable.";
        return false;
    }
    this->id = q.value(0).toInt();
    this->date = q.value(1).toDate();
    this->heure = q.value(2).toString();
    this->statut = q.value(3).toString();
    this->motif = q.value(4).toString();
    this->idAnimal = q.value(5).toInt();
    this->idEmploye = q.value(6).toInt();
    const QString nom = q.value(7).toString();
    const QString espece = q.value(8).toString();
    this->nomAnimal = nom.isEmpty() ? QString("—")
                                    : (espece.isEmpty() ? nom : nom + " (" + espece + ")");
    return true;
}