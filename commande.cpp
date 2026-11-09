#include "commande.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

/* ##################################################################
 *  PARTIE 1 : MODÈLE (classe purement C++ : entité COMMANDE)
 *  Toutes les requêtes SQL sont ici ; aucune requête dans l'interface.
 * ################################################################## */

/* ==================================================================
 *  CommandeTableModel : mise en forme de l'affichage
 * ================================================================== */
QVariant CommandeTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        const QVariant brut = QSqlQueryModel::data(index, role);
        switch (index.column()) {
        case 0:
            return Commande::formaterId(brut.toInt());
        case 1:
            return Commande::formaterDate(brut.toDate());
        case 2:
            return brut.toDate().isValid() ? Commande::formaterDate(brut.toDate()) : QString("—");
        case 4:
            return QVariant();                    // colonne « Actions » : dessinée par un delegate
        default:
            break;
        }
    }
    return QSqlQueryModel::data(index, role);
}

/* ==================================================================
 *  Commande
 * ================================================================== */
// Constructeur avec paramètres
Commande::Commande(int id, QDate dateCommande, QDate dateLivraison, QString statut,
                   QString nomFournisseur, int quantite)
{
    this->id = id;
    this->dateCommande = dateCommande;
    this->dateLivraison = dateLivraison;
    this->statut = statut;
    this->nomFournisseur = nomFournisseur;
    this->quantite = quantite;
}

// Destructeur : aucune ressource dynamique à libérer (la connexion BD est gérée par le Singleton Connection)
Commande::~Commande() {}

QStringList Commande::statutsDisponibles()
{
    return {"Commandée", "Livrée", "Annulée"};
}

// L'ordre doit correspondre au tableau ORDRES de rechercher()
QStringList Commande::criteresTri()
{
    return {"Aucun tri", "Livraison la plus proche", "Livraison la plus lointaine", "Commande la plus récente"};
}

QString Commande::formaterId(int id)
{
    return QString("C%1").arg(id, 3, 10, QChar('0'));
}

QString Commande::formaterDate(const QDate &d)
{
    return d.toString("dd/MM/yyyy");
}

// Traduit les erreurs Oracle courantes en messages compréhensibles
QString Commande::messageSql(const QString &texteErreur)
{
    if (texteErreur.contains("ORA-00001"))
        return "Une commande avec cet identifiant existe déjà.";
    if (texteErreur.contains("ORA-02291"))
        return "Le fournisseur choisi n'existe pas dans la base.";
    if (texteErreur.contains("ORA-02290"))
        return "Une valeur saisie ne respecte pas les contraintes de la base (statut, dates ou quantité).";
    if (texteErreur.contains("ORA-01400"))
        return "La base exige encore un employé : exécutez sql/commande_modifications.sql "
               "(IDEMPLOYE doit accepter NULL).";
    if (texteErreur.contains("ORA-00904"))
        return "Colonne introuvable (QUANTITE ?) : exécutez sql/commande_modifications.sql.";
    return "Erreur base de données : " + texteErreur;
}

/* ------------------------------------------------------------------
 *  Contrôle de saisie
 * ------------------------------------------------------------------ */
bool Commande::valider(bool creation)
{
    m_erreur.clear();
    Q_UNUSED(creation);

    if (!dateCommande.isValid()) {
        m_erreur = "La date de commande est invalide (format jj/mm/aaaa).";
        return false;
    }
    if (dateLivraison.isValid() && dateLivraison < dateCommande) {
        m_erreur = "La date de livraison ne peut pas être avant la date de commande.";
        return false;
    }

    if (nomFournisseur.trimmed().isEmpty()) {
        m_erreur = "Le nom du fournisseur est obligatoire.";
        return false;
    }
    if (quantite <= 0) {
        m_erreur = "La quantité doit être un nombre entier positif.";
        return false;
    }
    if (statut.isEmpty())
        statut = "Commandée";
    if (!statutsDisponibles().contains(statut)) {
        m_erreur = "Le statut est invalide.";
        return false;
    }

    // Le fournisseur est saisi par son nom : on retrouve son identifiant (clé étrangère)
    QSqlQuery q;
    q.prepare("SELECT IDFOURNISSEUR FROM FOURNISSEUR WHERE UPPER(NOM) = UPPER(:nom)");
    q.bindValue(":nom", nomFournisseur.trimmed());
    if (!q.exec()) {
        m_erreur = messageSql(q.lastError().text());
        return false;
    }
    int nb = 0;
    int trouve = 0;
    while (q.next()) {
        trouve = q.value(0).toInt();
        ++nb;
    }
    if (nb == 0) {
        m_erreur = QString("Aucun fournisseur nommé « %1 » dans la base. "
                           "Ajoutez-le d'abord (module Fournisseurs).").arg(nomFournisseur.trimmed());
        return false;
    }
    if (nb > 1) {
        m_erreur = QString("Plusieurs fournisseurs portent le nom « %1 ».").arg(nomFournisseur.trimmed());
        return false;
    }
    idFournisseur = trouve;
    return true;
}

/* ------------------------------------------------------------------
 *  CREATE
 * ------------------------------------------------------------------ */
int Commande::prochainId()
{
    QSqlQuery q;
    if (q.exec("SELECT COALESCE(MAX(IDCOMMANDE), 0) + 1 FROM COMMANDE") && q.next())
        return q.value(0).toInt();
    return 1;
}

bool Commande::existe(int id)
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM COMMANDE WHERE IDCOMMANDE = :id");
    q.bindValue(":id", id);
    return q.exec() && q.next() && q.value(0).toInt() > 0;
}

bool Commande::ajouter()
{
    if (!valider(true))
        return false;

    if (id <= 0)
        id = prochainId();                 // l'ID est attribué automatiquement

    QSqlQuery q;
    q.prepare("INSERT INTO COMMANDE (IDCOMMANDE, DATECOMMANDE, DATELIVRAISON, STATUT, IDFOURNISSEUR, QUANTITE) "
              "VALUES (:id, :dc, :dl, :statut, :fournisseur, :quantite)");
    q.bindValue(":id", id);
    q.bindValue(":dc", dateCommande);
    q.bindValue(":dl", dateLivraison.isValid() ? QVariant(dateLivraison)
                                               : QVariant(QMetaType::fromType<QDate>()));   // NULL si vide
    q.bindValue(":statut", statut);
    q.bindValue(":fournisseur", idFournisseur);
    q.bindValue(":quantite", quantite);

    if (!q.exec()) {
        m_erreur = messageSql(q.lastError().text());
        return false;
    }
    return true;
}

/* ------------------------------------------------------------------
 *  UPDATE
 * ------------------------------------------------------------------ */
bool Commande::modifier()
{
    if (id <= 0) {
        m_erreur = "Aucune commande sélectionnée.";
        return false;
    }
    if (!valider(false))
        return false;

    QSqlQuery q;
    q.prepare("UPDATE COMMANDE SET DATECOMMANDE = :dc, DATELIVRAISON = :dl, STATUT = :statut, "
              "IDFOURNISSEUR = :fournisseur, QUANTITE = :quantite WHERE IDCOMMANDE = :id");
    q.bindValue(":dc", dateCommande);
    q.bindValue(":dl", dateLivraison.isValid() ? QVariant(dateLivraison)
                                               : QVariant(QMetaType::fromType<QDate>()));
    q.bindValue(":statut", statut);
    q.bindValue(":fournisseur", idFournisseur);
    q.bindValue(":quantite", quantite);
    q.bindValue(":id", id);

    if (!q.exec()) {
        m_erreur = messageSql(q.lastError().text());
        return false;
    }
    if (q.numRowsAffected() == 0) {
        m_erreur = "Commande introuvable (elle a peut-être été supprimée).";
        return false;
    }
    return true;
}

/* ------------------------------------------------------------------
 *  DELETE
 * ------------------------------------------------------------------ */
bool Commande::supprimer(int id)
{
    m_erreur.clear();
    QSqlQuery q;
    q.prepare("DELETE FROM COMMANDE WHERE IDCOMMANDE = :id");
    q.bindValue(":id", id);

    if (!q.exec()) {
        m_erreur = messageSql(q.lastError().text());
        return false;
    }
    if (q.numRowsAffected() == 0) {
        m_erreur = "Commande introuvable (elle a peut-être déjà été supprimée).";
        return false;
    }
    return true;
}

/* ------------------------------------------------------------------
 *  READ : liste
 * ------------------------------------------------------------------ */
QSqlQueryModel *Commande::afficher() const
{
    return rechercher(QString(), QString(), 0);         // liste complète
}

/* ------------------------------------------------------------------
 *  MÉTIERS : recherche par ID, filtre de statut, tri
 * ------------------------------------------------------------------ */
QSqlQueryModel *Commande::rechercher(const QString &recherche, const QString &statutFiltre, int tri) const
{
    // Le tri est choisi dans une liste fixe (jamais dans le texte saisi) : pas d'injection SQL
    static const char *ORDRES[] = {
        "c.IDCOMMANDE ASC",
        "c.DATELIVRAISON ASC NULLS LAST, c.IDCOMMANDE ASC",
        "c.DATELIVRAISON DESC NULLS LAST, c.IDCOMMANDE ASC",
        "c.DATECOMMANDE DESC, c.IDCOMMANDE DESC"
    };
    const int nbOrdres = int(sizeof(ORDRES) / sizeof(ORDRES[0]));
    const int ordre = (tri >= 0 && tri < nbOrdres) ? tri : 0;

    // « C001 » ou « 1 » : on garde les chiffres
    const QString texte = recherche.trimmed();
    QString chiffres;
    for (const QChar &ch : texte)
        if (ch.isDigit())
            chiffres += ch;

    QStringList conditions;
    if (!texte.isEmpty())
        conditions << (chiffres.isEmpty() ? QString("1 = 0")
                                          : QString("CAST(c.IDCOMMANDE AS VARCHAR2(20)) LIKE :idc"));
    const bool filtreStatut = !statutFiltre.isEmpty() && statutFiltre != "Tous";
    if (filtreStatut)
        conditions << "c.STATUT = :statut";

    QString sql = "SELECT c.IDCOMMANDE, c.DATECOMMANDE, c.DATELIVRAISON, c.STATUT, 0 AS ACTIONS FROM COMMANDE c";
    if (!conditions.isEmpty())
        sql += " WHERE " + conditions.join(" AND ");
    sql += QString(" ORDER BY %1").arg(ORDRES[ordre]);

    QSqlQuery q;
    q.prepare(sql);
    if (!chiffres.isEmpty())
        q.bindValue(":idc", "%" + QString::number(chiffres.toInt()) + "%");
    if (filtreStatut)
        q.bindValue(":statut", statutFiltre);
    q.exec();

    auto *model = new CommandeTableModel;
    model->setQuery(std::move(q));
    if (model->lastError().isValid())
        m_erreur = "Erreur base de données : " + model->lastError().text();

    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Date commande");
    model->setHeaderData(2, Qt::Horizontal, "Date livraison");
    model->setHeaderData(3, Qt::Horizontal, "Statut");
    model->setHeaderData(4, Qt::Horizontal, "Actions");
    return model;
}

/* ------------------------------------------------------------------
 *  READ : une seule commande (détails, formulaire de modification)
 * ------------------------------------------------------------------ */
bool Commande::charger(int idCommande)
{
    QSqlQuery q;
    q.prepare("SELECT c.IDCOMMANDE, c.DATECOMMANDE, c.DATELIVRAISON, c.STATUT, c.MONTANTTOTAL, "
              "c.IDFOURNISSEUR, c.IDEMPLOYE, c.QUANTITE, f.NOM "
              "FROM COMMANDE c LEFT JOIN FOURNISSEUR f ON f.IDFOURNISSEUR = c.IDFOURNISSEUR "
              "WHERE c.IDCOMMANDE = :id");
    q.bindValue(":id", idCommande);
    if (!q.exec()) {
        m_erreur = messageSql(q.lastError().text());
        return false;
    }
    if (!q.next()) {
        m_erreur = "Commande introuvable.";
        return false;
    }
    this->id = q.value(0).toInt();
    this->dateCommande = q.value(1).toDate();
    this->dateLivraison = q.value(2).toDate();
    this->statut = q.value(3).toString();
    this->montantTotal = q.value(4).toDouble();
    this->idFournisseur = q.value(5).toInt();
    this->idEmploye = q.value(6).toInt();
    this->quantite = q.value(7).toInt();
    this->nomFournisseur = q.value(8).toString();
    return true;
}