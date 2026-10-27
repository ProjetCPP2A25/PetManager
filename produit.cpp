#include "produit.h"
#include <QSqlError>
#include <QSqlDatabase>
#include <QPair>
#include <QSqlRecord>
#include <QVariant>
#include <QObject>
#include <QRegularExpression>
#include <QSettings>
#include <QCoreApplication>
#include <QSslSocket>
#include <QSet>
#include <QHash>
#include <QLocale>
#include <QFile>
#include <QUrl>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>
#include <cstdint>
#include <ctime>
#include <map>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>

// Statut calculé par Oracle (toujours à jour, même quand une date expire)
static const QString SQL_STATUT =
    "CASE WHEN DATEEXPIRATION < TRUNC(SYSDATE) THEN 'Expire' "
    "WHEN QUANTITE = 0 THEN 'Rupture' "
    "WHEN QUANTITE <= SEUILMINIMAL THEN 'Stock bas' "
    "ELSE 'Disponible' END";

// Colonnes lues par depuisRequete() (même ordre partout)
static const QString SQL_COLONNES =
    "IDPRODUIT, NOM, CATEGORIE, QUANTITE, SEUILMINIMAL, PRIXUNITAIRE, "
    "TO_CHAR(DATEEXPIRATION, 'YYYY-MM-DD'), EMPLACEMENT, STATUT";

// Message d'erreur Oracle traduit en français simple (le texte brut du pilote
// ODBC contient des caractères illisibles)
static QString messageErreur(const QSqlError &e)
{
    const QString brut = e.databaseText() + " " + e.driverText();
    if (brut.contains("ORA-00001")) return "Cet identifiant ou ce nom de produit existe déjà.";
    if (brut.contains("ORA-02290")) return "Une valeur ne respecte pas les règles de la base "
                                           "(catégorie, quantité ≥ 0, seuil ≥ 0, prix > 0, statut).";
    if (brut.contains("ORA-02292")) return "Ce produit figure dans au moins une commande (table LIGNE_CMD) : "
                                           "il ne peut pas être supprimé.";
    if (brut.contains("ORA-02291")) return "La valeur de clé étrangère n'existe pas dans la table liée.";
    if (brut.contains("ORA-01400")) return "Un champ obligatoire est vide.";
    if (brut.contains("ORA-12899")) return "Une valeur saisie est trop longue.";
    if (brut.contains("ORA-00942")) return "Table introuvable : exécutez d'abord base_commune_petmanager.sql.";
    static const QRegularExpression ora("ORA-\\d{5}:[^\\[\\n]*");
    const QRegularExpressionMatch m = ora.match(brut);
    return m.hasMatch() ? m.captured(0).trimmed() : QString("Erreur de la base de données.");
}

// =====================================================================
//                         CONSTRUCTEURS
// =====================================================================

Produit::Produit()
    : idProduit(0), quantite(0), seuilMin(0), prixUnitaire(0)
{
}

Produit::Produit(int idProduit, QString nom, QString categorie, int quantite, int seuilMin,
                 double prixUnitaire, QDate dateExpiration, QString emplacement)
{
    this->idProduit = idProduit;
    this->nom = nom.trimmed();
    this->categorie = categorie;
    this->quantite = quantite;
    this->seuilMin = seuilMin;
    this->prixUnitaire = prixUnitaire;
    this->dateExpiration = dateExpiration;
    this->emplacement = emplacement.trimmed();
    this->statut = calculerStatut(quantite, seuilMin, dateExpiration);
}

Produit Produit::depuisRequete(const QSqlQuery &q)
{
    Produit p(q.value(0).toInt(), q.value(1).toString(), q.value(2).toString(),
              q.value(3).toInt(), q.value(4).toInt(), q.value(5).toDouble(),
              QDate::fromString(q.value(6).toString(), "yyyy-MM-dd"),
              q.value(7).toString());
    // le statut affiché est recalculé (il reste juste même si la date a expiré)
    return p;
}

// =====================================================================
//                         CREATE : AJOUTER
// =====================================================================

bool Produit::ajouter()
{
    QSqlQuery query;
    statut = calculerStatut(quantite, seuilMin, dateExpiration);

    // prepare() prend la requête en paramètre pour la préparer à l'exécution
    query.prepare("INSERT INTO STOCK (IDPRODUIT, NOM, CATEGORIE, QUANTITE, SEUILMINIMAL, "
                  "PRIXUNITAIRE, DATEEXPIRATION, EMPLACEMENT, STATUT) "
                  "VALUES (:id, :nom, :categorie, :quantite, :seuil, :prix, "
                  "TO_DATE(:dateexp, 'DD/MM/YYYY'), :emplacement, :statut)");

    // Création des variables liées
    query.bindValue(":id", idProduit);
    query.bindValue(":nom", nom);
    query.bindValue(":categorie", categorie);
    query.bindValue(":quantite", quantite);
    query.bindValue(":seuil", seuilMin);
    query.bindValue(":prix", prixUnitaire);
    query.bindValue(":dateexp", dateExpiration.toString("dd/MM/yyyy"));
    query.bindValue(":emplacement", emplacement);
    query.bindValue(":statut", statut);

    const bool ok = query.exec();   // exec() envoie la requête pour l'exécuter
    erreur = ok ? QString() : messageErreur(query.lastError());
    return ok;
}

// =====================================================================
//                         READ : AFFICHER
// =====================================================================
// Colonnes du modèle :
//  0 ID | 1 Produit | 2 (badge catégorie) | 3 Quantité | 4 Seuil | 5 Expiration
//  6 Emplacement | 7 (badge statut) | 8 (actions) | 9 CATEGORIE | 10 STATUT
//  11 PRIXUNITAIRE                        (9 à 11 : colonnes cachées dans la vue)
QSqlQueryModel *Produit::afficher(const QString &recherche, int tri)
{
    QSqlQueryModel *model = new QSqlQueryModel();

    // Le tri est choisi dans une liste fixe (jamais du texte saisi) -> pas d'injection
    // 0 = aucun tri (ordre des ID), 1 = date d'expiration la plus proche, 2 = la plus lointaine
    static const QStringList ordres = {
        "IDPRODUIT ASC",
        "DATEEXPIRATION ASC, NOM ASC",
        "DATEEXPIRATION DESC, NOM ASC"
    };
    const QString ordre = ordres.value(tri, ordres.first());

    QSqlQuery query;
    query.prepare("SELECT IDPRODUIT, NOM, CAST(NULL AS VARCHAR2(1)) AS BADGE_CAT, QUANTITE, SEUILMINIMAL, "
                  "TO_CHAR(DATEEXPIRATION, 'DD/MM/YYYY') AS EXPIRATION, EMPLACEMENT, "
                  "CAST(NULL AS VARCHAR2(1)) AS BADGE_STATUT, CAST(NULL AS VARCHAR2(1)) AS ACTIONS, "
                  "CATEGORIE, " + SQL_STATUT + " AS STATUT_CALCULE, PRIXUNITAIRE "
                  "FROM STOCK "
                  "WHERE UPPER(NOM) LIKE UPPER(:recherche) "      // recherche par nom
                  "ORDER BY " + ordre);                            // tri par date d'expiration
    query.bindValue(":recherche", "%" + recherche.trimmed() + "%");
    query.exec();
    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Produit"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Catégorie"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Quantité"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Seuil min."));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Expiration"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Emplacement"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Statut"));
    model->setHeaderData(8, Qt::Horizontal, QObject::tr("Actions"));
    return model;
}

// =====================================================================
//                         UPDATE : MODIFIER
// =====================================================================

bool Produit::modifier()
{
    QSqlQuery query;
    statut = calculerStatut(quantite, seuilMin, dateExpiration);

    query.prepare("UPDATE STOCK SET NOM = :nom, CATEGORIE = :categorie, QUANTITE = :quantite, "
                  "SEUILMINIMAL = :seuil, PRIXUNITAIRE = :prix, "
                  "DATEEXPIRATION = TO_DATE(:dateexp, 'DD/MM/YYYY'), EMPLACEMENT = :emplacement, "
                  "STATUT = :statut "
                  "WHERE IDPRODUIT = :id");
    query.bindValue(":nom", nom);
    query.bindValue(":categorie", categorie);
    query.bindValue(":quantite", quantite);
    query.bindValue(":seuil", seuilMin);
    query.bindValue(":prix", prixUnitaire);
    query.bindValue(":dateexp", dateExpiration.toString("dd/MM/yyyy"));
    query.bindValue(":emplacement", emplacement);
    query.bindValue(":statut", statut);
    query.bindValue(":id", idProduit);

    if (!query.exec()) { erreur = messageErreur(query.lastError()); return false; }
    erreur.clear();
    return query.numRowsAffected() != 0;   // 0 ligne = ID inexistant
}

// =====================================================================
//                         DELETE : SUPPRIMER
// =====================================================================

bool Produit::supprimer(int id)
{
    QSqlQuery query;
    QString res = QString::number(id);

    query.prepare("DELETE FROM STOCK WHERE IDPRODUIT = :id");
    query.bindValue(":id", res);

    if (!query.exec()) { erreur = messageErreur(query.lastError()); return false; }
    erreur.clear();
    return query.numRowsAffected() != 0;
}

// =====================================================================
//                         COMPLÉMENTS DU CRUD
// =====================================================================

bool Produit::chargerParId(int id)
{
    QSqlQuery query;
    query.prepare("SELECT " + SQL_COLONNES + " FROM STOCK WHERE IDPRODUIT = :id");
    query.bindValue(":id", id);
    if (!query.exec() || !query.next()) return false;
    *this = depuisRequete(query);
    return true;
}

bool Produit::existe(int id)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM STOCK WHERE IDPRODUIT = :id");
    query.bindValue(":id", id);
    return query.exec() && query.next() && query.value(0).toInt() > 0;
}

bool Produit::nomExiste(const QString &nomProduit, int idExclu)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM STOCK WHERE UPPER(NOM) = UPPER(:nom) AND IDPRODUIT <> :id");
    query.bindValue(":nom", nomProduit.trimmed());
    query.bindValue(":id", idExclu);
    return query.exec() && query.next() && query.value(0).toInt() > 0;
}

int Produit::prochainId()
{
    QSqlQuery query;
    query.prepare("SELECT NVL(MAX(IDPRODUIT), 0) + 1 FROM STOCK");
    if (query.exec() && query.next()) return query.value(0).toInt();
    return 1;
}

// Sortie de stock : la quantité utilisée est retirée (requête UPDATE)
bool Produit::retirerQuantite(int id, int quantiteRetiree)
{
    Produit p;
    if (!p.chargerParId(id) || quantiteRetiree <= 0 || quantiteRetiree > p.getQuantite())
        return false;

    const int nouvelleQuantite = p.getQuantite() - quantiteRetiree;

    // Les deux requêtes forment une seule opération (transaction) :
    // mise à jour du stock + enregistrement de la consommation du jour
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QSqlQuery query;
    query.prepare("UPDATE STOCK SET QUANTITE = :quantite, STATUT = :statut WHERE IDPRODUIT = :id");
    query.bindValue(":quantite", nouvelleQuantite);
    query.bindValue(":statut", calculerStatut(nouvelleQuantite, p.getSeuilMin(), p.getDateExpiration()));
    query.bindValue(":id", id);

    QSqlQuery conso;
    conso.prepare("INSERT INTO CONSOMMATION (IDPRODUIT, QUANTITE, DATECONSO) VALUES (:id, :quantite, SYSDATE)");
    conso.bindValue(":id", id);
    conso.bindValue(":quantite", quantiteRetiree);

    if (!query.exec()) { erreur = messageErreur(query.lastError()); db.rollback(); return false; }
    if (!conso.exec()) { erreur = messageErreur(conso.lastError()); db.rollback(); return false; }
    db.commit();
    return true;
}

// La colonne STATUT est remise à jour (un produit peut expirer d'un jour à l'autre)
bool Produit::actualiserStatuts()
{
    QSqlQuery query;
    query.prepare("UPDATE STOCK SET STATUT = " + SQL_STATUT);
    return query.exec();
}

// Contrôle de saisie (obligatoire dans le code C++)
QStringList Produit::controleSaisie(bool nouveau) const
{
    QStringList erreurs;
    static const QRegularExpression regNom("^[\\p{L}0-9 '()./%+,-]{2,50}$");
    static const QRegularExpression regEmplacement("^[\\p{L}0-9 '()./,-]{2,30}$");

    if (idProduit <= 0)
        erreurs << "L'ID du produit doit être un entier strictement positif.";
    if (!regNom.match(nom).hasMatch())
        erreurs << "Le nom doit contenir 2 à 50 caractères (lettres, chiffres, espaces, ( ) . / % + , -).";
    if (!categories().contains(categorie))
        erreurs << "Veuillez choisir une catégorie.";
    if (quantite < 0)
        erreurs << "La quantité ne peut pas être négative.";
    if (seuilMin < 0)
        erreurs << "Le seuil minimal ne peut pas être négatif.";
    if (prixUnitaire <= 0)
        erreurs << "Le prix unitaire doit être supérieur à 0.";
    if (!dateExpiration.isValid())
        erreurs << "La date d'expiration est invalide.";
    else if (nouveau && dateExpiration <= QDate::currentDate())
        erreurs << "La date d'expiration d'un nouveau produit doit être dans le futur.";
    if (!regEmplacement.match(emplacement).hasMatch())
        erreurs << "L'emplacement doit contenir 2 à 30 caractères (ex : Armoire A - Étagère 2).";
    return erreurs;
}

QString Produit::calculerStatut(int quantite, int seuilMin, const QDate &dateExpiration)
{
    if (dateExpiration.isValid() && dateExpiration < QDate::currentDate()) return "Expire";
    if (quantite == 0)        return "Rupture";
    if (quantite <= seuilMin) return "Stock bas";
    return "Disponible";
}

QString Produit::libelleStatut(const QString &statutBD)
{
    return statutBD == "Expire" ? QString("Expiré") : statutBD;
}

QStringList Produit::categories()
{
    return {"Alimentation", "Médicament", "Équipement"};
}

int Produit::quantiteRecommandee(int quantite, int seuilMin, int facteur)
{
    return std::max(facteur * seuilMin - quantite, 1);
}

// =====================================================================
//                         STATISTIQUES
// =====================================================================

int Produit::nombreProduits()
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM STOCK");
    return (query.exec() && query.next()) ? query.value(0).toInt() : 0;
}

double Produit::valeurStock()
{
    QSqlQuery query;
    query.prepare("SELECT NVL(SUM(QUANTITE * PRIXUNITAIRE), 0) FROM STOCK");
    return (query.exec() && query.next()) ? query.value(0).toDouble() : 0;
}

int Produit::nombreSousSeuil()
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM STOCK WHERE QUANTITE <= SEUILMINIMAL");
    return (query.exec() && query.next()) ? query.value(0).toInt() : 0;
}

// Répartition des produits par catégorie (nombre de produits)
QMap<QString, int> Produit::repartitionParCategorie()
{
    QMap<QString, int> resultat;
    QSqlQuery query;
    query.prepare("SELECT CATEGORIE, COUNT(*) FROM STOCK GROUP BY CATEGORIE");
    if (query.exec())
        while (query.next())
            resultat[query.value(0).toString()] = query.value(1).toInt();
    return resultat;
}

// Fournisseur de chaque produit, d'après le MR commun :
// STOCK -> LIGNE_CMD -> COMMANDE -> FOURNISSEUR (fournisseur de la dernière commande)
// Résultat : IDPRODUIT -> nom du fournisseur
QMap<int, QString> Produit::fournisseurParProduit()
{
    QMap<int, QString> resultat;
    QSqlQuery query;
    query.prepare("SELECT l.IDPRODUIT, f.NOM "
                  "FROM LIGNE_CMD l "
                  "JOIN COMMANDE c ON c.IDCOMMANDE = l.IDCOMMANDE "
                  "JOIN FOURNISSEUR f ON f.IDFOURNISSEUR = c.IDFOURNISSEUR "
                  "ORDER BY c.DATECOMMANDE ASC, c.IDCOMMANDE ASC");
    if (query.exec())
        while (query.next())   // la commande la plus récente écrase les plus anciennes
            resultat[query.value(0).toInt()] = query.value(1).toString();
    return resultat;
}

QList<Produit> Produit::listeTous()
{
    QList<Produit> liste;
    QSqlQuery query;
    query.prepare("SELECT " + SQL_COLONNES + " FROM STOCK ORDER BY IDPRODUIT");
    if (query.exec())
        while (query.next())
            liste.append(depuisRequete(query));
    return liste;
}

// =====================================================================
//          MÉTIER 1 : produits ayant atteint le seuil minimal
// =====================================================================

QList<Produit> Produit::listeSousSeuil()
{
    QList<Produit> liste;
    QSqlQuery query;
    query.prepare("SELECT " + SQL_COLONNES + " FROM STOCK "
                  "WHERE QUANTITE <= SEUILMINIMAL "
                  "ORDER BY CASE WHEN SEUILMINIMAL = 0 THEN 0 ELSE QUANTITE / SEUILMINIMAL END, NOM");
    if (query.exec())
        while (query.next())
            liste.append(depuisRequete(query));
    return liste;
}

// =====================================================================
//          MÉTIER 2 : catalogue des produits utilisables (pour l'IA)
// =====================================================================

QList<Produit> Produit::catalogueDisponible()
{
    QList<Produit> liste;
    QSqlQuery query;
    query.prepare("SELECT " + SQL_COLONNES + " FROM STOCK "
                  "WHERE QUANTITE > 0 AND DATEEXPIRATION >= TRUNC(SYSDATE) "
                  "ORDER BY CATEGORIE, NOM");
    if (query.exec())
        while (query.next())
            liste.append(depuisRequete(query));
    return liste;
}

// =====================================================================
//     GRAPHIQUE : évolution de la consommation (table CONSOMMATION)
// =====================================================================

// Quantité consommée par jour pour un produit, depuis la date « debut »
QMap<QDate, int> Produit::consommationParJour(int id, const QDate &debut)
{
    QMap<QDate, int> resultat;
    QSqlQuery query;
    query.prepare("SELECT TO_CHAR(DATECONSO, 'YYYY-MM-DD'), SUM(QUANTITE) FROM CONSOMMATION "
                  "WHERE IDPRODUIT = :id AND DATECONSO >= TO_DATE(:debut, 'YYYY-MM-DD') "
                  "GROUP BY TO_CHAR(DATECONSO, 'YYYY-MM-DD')");
    query.bindValue(":id", id);
    query.bindValue(":debut", debut.toString("yyyy-MM-dd"));
    if (query.exec())
        while (query.next())
            resultat[QDate::fromString(query.value(0).toString(), "yyyy-MM-dd")] = query.value(1).toInt();
    return resultat;
}

// Les « nombre » produits les plus consommés depuis « debut » : (ID, nom)
QList<QPair<int, QString>> Produit::produitsLesPlusConsommes(const QDate &debut, int nombre)
{
    QList<QPair<int, QString>> resultat;
    QSqlQuery query;
    query.prepare("SELECT c.IDPRODUIT, s.NOM, SUM(c.QUANTITE) AS TOTAL "
                  "FROM CONSOMMATION c JOIN STOCK s ON s.IDPRODUIT = c.IDPRODUIT "
                  "WHERE c.DATECONSO >= TO_DATE(:debut, 'YYYY-MM-DD') "
                  "GROUP BY c.IDPRODUIT, s.NOM ORDER BY TOTAL DESC");
    query.bindValue(":debut", debut.toString("yyyy-MM-dd"));
    if (query.exec())
        while (query.next() && resultat.size() < nombre)
            resultat.append(qMakePair(query.value(0).toInt(), query.value(1).toString()));
    return resultat;
}

// #####################################################################
//
//      MÉTIER BASIQUE : FICHE DE COMMANDE DE RÉAPPROVISIONNEMENT (EXCEL)
//
//  Un fichier .xlsx est une archive ZIP de fichiers XML : les fonctions
//  ci-dessous écrivent ces fichiers puis l'archive (aucune bibliothèque
//  externe). Elles sont internes à produit.cpp.
// #####################################################################

namespace {

// Styles de cellule (index dans styles.xml)
enum StyleXl { Normal = 0, Titre, SousTitre, Libelle, Valeur, Entete, Texte, Entier,
               Montant, Section, SousTotalLibelle, SousTotalMontant, TotalLibelle,
               TotalMontant, Note, EntierFort };

struct CelluleXl {
    int type = 0;            // -1 vide stylée, 0 texte, 1 nombre, 2 formule
    std::string texte;
    double valeur = 0;
    int style = 0;
};

// Contenu de la feuille (données uniquement)
struct FeuilleXl {
    std::map<int, std::map<int, CelluleXl>> cellules;
    std::map<int, double> largeurs;
    std::map<int, double> hauteurs;
    std::vector<std::string> fusions;
};

std::string xlLettreColonne(int colonne)
{
    std::string s;
    while (colonne > 0) {
        const int r = (colonne - 1) % 26;
        s.insert(s.begin(), char('A' + r));
        colonne = (colonne - 1) / 26;
    }
    return s;
}

std::string xlReference(int ligne, int colonne)      // (1,1) -> "A1"
{
    return xlLettreColonne(colonne) + std::to_string(ligne);
}

void xlTexte(FeuilleXl &f, int ligne, int colonne, const std::string &valeur, int style)
{
    CelluleXl c;
    c.type = 0;
    c.texte = valeur;
    c.style = style;
    f.cellules[ligne][colonne] = c;
}

void xlNombre(FeuilleXl &f, int ligne, int colonne, double valeur, int style)
{
    CelluleXl c;
    c.type = 1;
    c.valeur = valeur;
    c.style = style;
    f.cellules[ligne][colonne] = c;
}

void xlFormule(FeuilleXl &f, int ligne, int colonne, const std::string &formule, double valeurCalculee, int style)
{
    CelluleXl c;
    c.type = 2;
    c.texte = formule;
    c.valeur = valeurCalculee;
    c.style = style;
    f.cellules[ligne][colonne] = c;
}

// Les cellules fusionnées reçoivent toutes le même style (bordures et fond continus)
void xlFusionner(FeuilleXl &f, int l1, int c1, int l2, int c2, int style)
{
    f.fusions.push_back(xlReference(l1, c1) + ":" + xlReference(l2, c2));
    for (int l = l1; l <= l2; ++l)
        for (int c = c1; c <= c2; ++c) {
            auto it = f.cellules[l].find(c);
            if (it == f.cellules[l].end()) {
                CelluleXl vide;
                vide.type = -1;
                vide.style = style;
                f.cellules[l][c] = vide;
            } else if (!(l == l1 && c == c1)) {
                it->second.style = style;
            }
        }
}

std::string xlEchapper(const std::string &s)
{
    std::string r;
    for (unsigned char ch : s) {
        switch (ch) {
        case '&': r += "&amp;"; break;
        case '<': r += "&lt;"; break;
        case '>': r += "&gt;"; break;
        case '"': r += "&quot;"; break;
        default:
            if (ch < 0x20 && ch != '\t' && ch != '\n' && ch != '\r') r += ' ';
            else r += char(ch);
        }
    }
    return r;
}

std::string xlNombreXml(double v)
{
    std::ostringstream o;
    o.imbue(std::locale::classic());
    o << std::setprecision(15) << v;
    return o.str();
}

std::string xlXmlFeuille(const FeuilleXl &f)
{
    int derniereLigne = 1, derniereColonne = 1;
    for (const auto &l : f.cellules) {
        derniereLigne = std::max(derniereLigne, l.first);
        for (const auto &c : l.second) derniereColonne = std::max(derniereColonne, c.first);
    }

    std::ostringstream x;
    x << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
      << "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
         "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
      << "<sheetPr><pageSetUpPr fitToPage=\"1\"/></sheetPr>"
      << "<dimension ref=\"A1:" << xlReference(derniereLigne, derniereColonne) << "\"/>"
      << "<sheetViews><sheetView showGridLines=\"0\" workbookViewId=\"0\"/></sheetViews>"
      << "<sheetFormatPr defaultRowHeight=\"15\"/>";

    if (!f.largeurs.empty()) {
        x << "<cols>";
        for (const auto &l : f.largeurs)
            x << "<col min=\"" << l.first << "\" max=\"" << l.first << "\" width=\""
              << xlNombreXml(l.second) << "\" customWidth=\"1\"/>";
        x << "</cols>";
    }

    x << "<sheetData>";
    for (const auto &l : f.cellules) {
        x << "<row r=\"" << l.first << "\"";
        auto h = f.hauteurs.find(l.first);
        if (h != f.hauteurs.end()) x << " ht=\"" << xlNombreXml(h->second) << "\" customHeight=\"1\"";
        x << ">";
        for (const auto &c : l.second) {
            const CelluleXl &cel = c.second;
            const std::string ref = xlReference(l.first, c.first);
            if (cel.type == -1) {
                x << "<c r=\"" << ref << "\" s=\"" << cel.style << "\"/>";
            } else if (cel.type == 0) {
                x << "<c r=\"" << ref << "\" s=\"" << cel.style << "\" t=\"inlineStr\"><is><t xml:space=\"preserve\">"
                  << xlEchapper(cel.texte) << "</t></is></c>";
            } else if (cel.type == 1) {
                x << "<c r=\"" << ref << "\" s=\"" << cel.style << "\"><v>" << xlNombreXml(cel.valeur) << "</v></c>";
            } else {
                x << "<c r=\"" << ref << "\" s=\"" << cel.style << "\"><f>" << xlEchapper(cel.texte)
                  << "</f><v>" << xlNombreXml(cel.valeur) << "</v></c>";
            }
        }
        x << "</row>";
    }
    x << "</sheetData>";

    if (!f.fusions.empty()) {
        x << "<mergeCells count=\"" << f.fusions.size() << "\">";
        for (const std::string &fu : f.fusions) x << "<mergeCell ref=\"" << fu << "\"/>";
        x << "</mergeCells>";
    }
    x << "<pageMargins left=\"0.5\" right=\"0.5\" top=\"0.6\" bottom=\"0.6\" header=\"0.3\" footer=\"0.3\"/>"
      << "<pageSetup paperSize=\"9\" orientation=\"landscape\" fitToWidth=\"1\" fitToHeight=\"0\"/>"
      << "</worksheet>";
    return x.str();
}

// Index des styles (cellXfs) = valeurs de l'énumération StyleXl
std::string xlXmlStyles()
{
    return
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
        "<numFmts count=\"1\"><numFmt numFmtId=\"164\" formatCode=\"#,##0.000&quot; TND&quot;\"/></numFmts>"
        "<fonts count=\"6\">"
        "<font><sz val=\"11\"/><color rgb=\"FF1E3A3A\"/><name val=\"Calibri\"/><family val=\"2\"/></font>"
        "<font><b/><sz val=\"11\"/><color rgb=\"FF17363A\"/><name val=\"Calibri\"/><family val=\"2\"/></font>"
        "<font><b/><sz val=\"16\"/><color rgb=\"FFFFFFFF\"/><name val=\"Calibri\"/><family val=\"2\"/></font>"
        "<font><b/><sz val=\"11\"/><color rgb=\"FFFFFFFF\"/><name val=\"Calibri\"/><family val=\"2\"/></font>"
        "<font><i/><sz val=\"9\"/><color rgb=\"FF6B8080\"/><name val=\"Calibri\"/><family val=\"2\"/></font>"
        "<font><b/><sz val=\"12\"/><color rgb=\"FF1F7A6E\"/><name val=\"Calibri\"/><family val=\"2\"/></font>"
        "</fonts>"
        "<fills count=\"5\">"
        "<fill><patternFill patternType=\"none\"/></fill>"
        "<fill><patternFill patternType=\"gray125\"/></fill>"
        "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FF2A8C86\"/><bgColor indexed=\"64\"/></patternFill></fill>"
        "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FFE4F2F1\"/><bgColor indexed=\"64\"/></patternFill></fill>"
        "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FFF7FAFA\"/><bgColor indexed=\"64\"/></patternFill></fill>"
        "</fills>"
        "<borders count=\"2\">"
        "<border><left/><right/><top/><bottom/><diagonal/></border>"
        "<border><left style=\"thin\"><color rgb=\"FFC5D6D5\"/></left><right style=\"thin\"><color rgb=\"FFC5D6D5\"/></right>"
        "<top style=\"thin\"><color rgb=\"FFC5D6D5\"/></top><bottom style=\"thin\"><color rgb=\"FFC5D6D5\"/></bottom><diagonal/></border>"
        "</borders>"
        "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
        "<cellXfs count=\"16\">"
        "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"
        "<xf numFmtId=\"0\" fontId=\"2\" fillId=\"2\" borderId=\"0\" xfId=\"0\" applyFont=\"1\" applyFill=\"1\" applyAlignment=\"1\"><alignment horizontal=\"left\" vertical=\"center\" indent=\"1\"/></xf>"
        "<xf numFmtId=\"0\" fontId=\"4\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyFont=\"1\" applyAlignment=\"1\"><alignment vertical=\"center\"/></xf>"
        "<xf numFmtId=\"0\" fontId=\"1\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyFont=\"1\" applyAlignment=\"1\"><alignment vertical=\"center\"/></xf>"
        "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyAlignment=\"1\"><alignment horizontal=\"left\" vertical=\"center\" wrapText=\"1\"/></xf>"
        "<xf numFmtId=\"0\" fontId=\"3\" fillId=\"2\" borderId=\"1\" xfId=\"0\" applyFont=\"1\" applyFill=\"1\" applyBorder=\"1\" applyAlignment=\"1\"><alignment horizontal=\"center\" vertical=\"center\" wrapText=\"1\"/></xf>"
        "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"1\" xfId=\"0\" applyBorder=\"1\" applyAlignment=\"1\"><alignment vertical=\"center\" wrapText=\"1\"/></xf>"
        "<xf numFmtId=\"1\" fontId=\"0\" fillId=\"0\" borderId=\"1\" xfId=\"0\" applyNumberFormat=\"1\" applyBorder=\"1\" applyAlignment=\"1\"><alignment horizontal=\"center\" vertical=\"center\"/></xf>"
        "<xf numFmtId=\"164\" fontId=\"0\" fillId=\"0\" borderId=\"1\" xfId=\"0\" applyNumberFormat=\"1\" applyBorder=\"1\" applyAlignment=\"1\"><alignment vertical=\"center\"/></xf>"
        "<xf numFmtId=\"0\" fontId=\"5\" fillId=\"3\" borderId=\"1\" xfId=\"0\" applyFont=\"1\" applyFill=\"1\" applyBorder=\"1\" applyAlignment=\"1\"><alignment vertical=\"center\" indent=\"1\"/></xf>"
        "<xf numFmtId=\"0\" fontId=\"1\" fillId=\"4\" borderId=\"1\" xfId=\"0\" applyFont=\"1\" applyFill=\"1\" applyBorder=\"1\" applyAlignment=\"1\"><alignment horizontal=\"right\" vertical=\"center\"/></xf>"
        "<xf numFmtId=\"164\" fontId=\"1\" fillId=\"4\" borderId=\"1\" xfId=\"0\" applyNumberFormat=\"1\" applyFont=\"1\" applyFill=\"1\" applyBorder=\"1\" applyAlignment=\"1\"><alignment vertical=\"center\"/></xf>"
        "<xf numFmtId=\"0\" fontId=\"3\" fillId=\"2\" borderId=\"1\" xfId=\"0\" applyFont=\"1\" applyFill=\"1\" applyBorder=\"1\" applyAlignment=\"1\"><alignment horizontal=\"right\" vertical=\"center\"/></xf>"
        "<xf numFmtId=\"164\" fontId=\"3\" fillId=\"2\" borderId=\"1\" xfId=\"0\" applyNumberFormat=\"1\" applyFont=\"1\" applyFill=\"1\" applyBorder=\"1\" applyAlignment=\"1\"><alignment vertical=\"center\"/></xf>"
        "<xf numFmtId=\"0\" fontId=\"4\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyFont=\"1\" applyAlignment=\"1\"><alignment vertical=\"top\" wrapText=\"1\"/></xf>"
        "<xf numFmtId=\"1\" fontId=\"5\" fillId=\"3\" borderId=\"1\" xfId=\"0\" applyNumberFormat=\"1\" applyFont=\"1\" applyFill=\"1\" applyBorder=\"1\" applyAlignment=\"1\"><alignment horizontal=\"center\" vertical=\"center\"/></xf>"
        "</cellXfs>"
        "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>"
        "</styleSheet>";
}

uint32_t xlCrc32(const std::string &donnees)
{
    static uint32_t table[256];
    static bool initialisee = false;
    if (!initialisee) {
        for (uint32_t i = 0; i < 256; ++i) {
            uint32_t c = i;
            for (int k = 0; k < 8; ++k) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            table[i] = c;
        }
        initialisee = true;
    }
    uint32_t crc = 0xFFFFFFFFu;
    for (unsigned char b : donnees) crc = table[(crc ^ b) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}

void xlEcrire16(std::string &s, uint16_t v) { s += char(v & 0xFF); s += char(v >> 8); }
void xlEcrire32(std::string &s, uint32_t v)
{
    for (int i = 0; i < 4; ++i) s += char((v >> (8 * i)) & 0xFF);
}

// Contenu complet du fichier .xlsx (archive ZIP « stockée »)
std::string xlArchive(const FeuilleXl &f, const std::string &nomFeuille)
{
    const std::vector<std::pair<std::string, std::string>> fichiers = {
        {"[Content_Types].xml",
         "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
         "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
         "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
         "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
         "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
         "<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
         "<Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>"
         "</Types>"},
        {"_rels/.rels",
         "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
         "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
         "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>"
         "</Relationships>"},
        {"xl/workbook.xml",
         "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
         "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
         "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
         "<sheets><sheet name=\"" + xlEchapper(nomFeuille) + "\" sheetId=\"1\" r:id=\"rId1\"/></sheets>"
         "</workbook>"},
        {"xl/_rels/workbook.xml.rels",
         "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
         "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
         "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet1.xml\"/>"
         "<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" Target=\"styles.xml\"/>"
         "</Relationships>"},
        {"xl/styles.xml", xlXmlStyles()},
        {"xl/worksheets/sheet1.xml", xlXmlFeuille(f)}
    };

    // Date et heure au format DOS (exigé par le format ZIP)
    const std::time_t maintenant = std::time(nullptr);
    const std::tm *t = std::localtime(&maintenant);
    const uint16_t heureDos = uint16_t((t->tm_hour << 11) | (t->tm_min << 5) | (t->tm_sec / 2));
    const uint16_t dateDos  = uint16_t(((t->tm_year - 80) << 9) | ((t->tm_mon + 1) << 5) | t->tm_mday);

    std::string zip, repertoire;
    for (const auto &fi : fichiers) {
        const std::string &nom = fi.first;
        const std::string &contenu = fi.second;
        const uint32_t crc = xlCrc32(contenu);
        const uint32_t taille = uint32_t(contenu.size());
        const uint32_t position = uint32_t(zip.size());

        // En-tête local
        xlEcrire32(zip, 0x04034b50);
        xlEcrire16(zip, 20); xlEcrire16(zip, 0x0800);    // version, drapeau UTF-8
        xlEcrire16(zip, 0);                               // méthode : stockée
        xlEcrire16(zip, heureDos); xlEcrire16(zip, dateDos);
        xlEcrire32(zip, crc); xlEcrire32(zip, taille); xlEcrire32(zip, taille);
        xlEcrire16(zip, uint16_t(nom.size())); xlEcrire16(zip, 0);
        zip += nom;
        zip += contenu;

        // Entrée du répertoire central
        xlEcrire32(repertoire, 0x02014b50);
        xlEcrire16(repertoire, 20); xlEcrire16(repertoire, 20); xlEcrire16(repertoire, 0x0800);
        xlEcrire16(repertoire, 0);
        xlEcrire16(repertoire, heureDos); xlEcrire16(repertoire, dateDos);
        xlEcrire32(repertoire, crc); xlEcrire32(repertoire, taille); xlEcrire32(repertoire, taille);
        xlEcrire16(repertoire, uint16_t(nom.size()));
        xlEcrire16(repertoire, 0); xlEcrire16(repertoire, 0); xlEcrire16(repertoire, 0); xlEcrire16(repertoire, 0);
        xlEcrire32(repertoire, 0);
        xlEcrire32(repertoire, position);
        repertoire += nom;
    }
    const uint32_t debutRepertoire = uint32_t(zip.size());
    zip += repertoire;

    // Fin du répertoire central
    xlEcrire32(zip, 0x06054b50);
    xlEcrire16(zip, 0); xlEcrire16(zip, 0);
    xlEcrire16(zip, uint16_t(fichiers.size())); xlEcrire16(zip, uint16_t(fichiers.size()));
    xlEcrire32(zip, uint32_t(repertoire.size()));
    xlEcrire32(zip, debutRepertoire);
    xlEcrire16(zip, 0);
    return zip;
}

} // fin des outils internes Excel

QString Produit::numeroCommande()
{
    return "CMD-" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmm");
}

// Mise en page : en-tête, un bloc par fournisseur, sous-totaux, total général, signatures.
// Colonnes : A Réf. | B Produit | C Catégorie | D Emplacement | E Stock |
//            F Seuil | G Qté à commander | H Prix unitaire | I Montant
bool Produit::genererFicheCommande(const QString &chemin, const QList<Produit> &produits,
                                   const QMap<int, QString> &fournisseurs, int facteur,
                                   const QString &numero, QString &erreurFiche)
{
    if (produits.isEmpty()) {
        erreurFiche = "Aucun produit n'a atteint son seuil minimal.";
        return false;
    }

    FeuilleXl xl;
    const int derniereColonne = 9;
    const double largeursCol[] = {8, 34, 15, 24, 12, 11, 16, 16, 18};
    for (int c = 1; c <= derniereColonne; ++c) xl.largeurs[c] = largeursCol[c - 1];

    // ---- En-tête ----
    const std::string date = QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm").toStdString();
    const std::string livraison = QDate::currentDate().addDays(3).toString("dd/MM/yyyy").toStdString();
    const std::string regle = QString("Quantité à commander = %1 × seuil minimal − stock actuel (minimum 1)")
                                  .arg(facteur).toStdString();

    xlTexte(xl, 1, 1, "PET MANAGER  -  Fiche de commande de réapprovisionnement", Titre);
    xlFusionner(xl, 1, 1, 1, derniereColonne, Titre);
    xl.hauteurs[1] = 32;
    xlTexte(xl, 2, 1, "Smart Pet Care Center  ·  Module Gestion de stock  ·  Document généré automatiquement", SousTitre);
    xlFusionner(xl, 2, 1, 2, derniereColonne, SousTitre);

    xlTexte(xl, 4, 1, "N° de commande :", Libelle);        xlFusionner(xl, 4, 1, 4, 2, Libelle);
    xlTexte(xl, 4, 3, numero.toStdString(), Valeur);        xlFusionner(xl, 4, 3, 4, 4, Valeur);
    xlTexte(xl, 4, 6, "Date :", Libelle);                   xlFusionner(xl, 4, 6, 4, 7, Libelle);
    xlTexte(xl, 4, 8, date, Valeur);                        xlFusionner(xl, 4, 8, 4, 9, Valeur);

    xlTexte(xl, 5, 1, "Émise par :", Libelle);              xlFusionner(xl, 5, 1, 5, 2, Libelle);
    xlTexte(xl, 5, 3, "Responsable du stock", Valeur);      xlFusionner(xl, 5, 3, 5, 4, Valeur);
    xlTexte(xl, 5, 6, "Livraison souhaitée :", Libelle);    xlFusionner(xl, 5, 6, 5, 7, Libelle);
    xlTexte(xl, 5, 8, livraison, Valeur);                   xlFusionner(xl, 5, 8, 5, 9, Valeur);

    xlTexte(xl, 6, 1, "Calcul des quantités :", Libelle);   xlFusionner(xl, 6, 1, 6, 2, Libelle);
    xlTexte(xl, 6, 3, regle, Valeur);                       xlFusionner(xl, 6, 3, 6, derniereColonne, Valeur);

    // ---- Tableau ----
    int ligne = 8;
    const char *titres[] = {"Réf.", "Produit", "Catégorie", "Emplacement", "Stock actuel",
                            "Seuil min.", "Qté à commander", "Prix unitaire", "Montant"};
    for (int c = 1; c <= derniereColonne; ++c) xlTexte(xl, ligne, c, titres[c - 1], Entete);
    xl.hauteurs[ligne] = 30;
    ++ligne;

    // Regroupement par fournisseur (ordre alphabétique, « non renseigné » à la fin)
    std::map<std::string, std::vector<Produit>> parFournisseur;
    for (const Produit &p : produits)
        parFournisseur[fournisseurs.value(p.getIdProduit()).toStdString()].push_back(p);
    std::vector<std::string> ordre;
    for (const auto &g : parFournisseur) ordre.push_back(g.first);
    std::stable_sort(ordre.begin(), ordre.end(), [](const std::string &a, const std::string &b) {
        const bool aVide = a.empty(), bVide = b.empty();
        if (aVide != bVide) return bVide;
        return a < b;
    });

    std::vector<std::string> sousTotaux;
    double totalGeneral = 0;
    int totalQuantites = 0;
    for (const std::string &nomFournisseur : ordre) {
        const std::vector<Produit> &groupe = parFournisseur[nomFournisseur];
        xlTexte(xl, ligne, 1, "Fournisseur : " + (nomFournisseur.empty() ? std::string("non renseigné")
                                                                         : nomFournisseur), Section);
        xlFusionner(xl, ligne, 1, ligne, derniereColonne, Section);
        xl.hauteurs[ligne] = 22;
        ++ligne;

        const int debut = ligne;
        double sousTotal = 0;
        for (const Produit &p : groupe) {
            const int qte = quantiteRecommandee(p.getQuantite(), p.getSeuilMin(), facteur);
            const double montant = qte * p.getPrixUnitaire();
            sousTotal += montant;
            totalQuantites += qte;
            xlNombre(xl, ligne, 1, p.getIdProduit(), Entier);
            xlTexte(xl, ligne, 2, p.getNom().toStdString(), Texte);
            xlTexte(xl, ligne, 3, p.getCategorie().toStdString(), Texte);
            xlTexte(xl, ligne, 4, p.getEmplacement().toStdString(), Texte);
            xlNombre(xl, ligne, 5, p.getQuantite(), Entier);
            xlNombre(xl, ligne, 6, p.getSeuilMin(), Entier);
            xlNombre(xl, ligne, 7, qte, EntierFort);
            xlNombre(xl, ligne, 8, p.getPrixUnitaire(), Montant);
            xlFormule(xl, ligne, 9, xlReference(ligne, 7) + "*" + xlReference(ligne, 8), montant, Montant);
            xl.hauteurs[ligne] = 20;
            ++ligne;
        }
        xlTexte(xl, ligne, 1, "Sous-total " + (nomFournisseur.empty() ? std::string("(fournisseur non renseigné)")
                                                                      : nomFournisseur), SousTotalLibelle);
        xlFusionner(xl, ligne, 1, ligne, derniereColonne - 1, SousTotalLibelle);
        xlFormule(xl, ligne, derniereColonne,
                  "SUM(" + xlReference(debut, 9) + ":" + xlReference(ligne - 1, 9) + ")",
                  sousTotal, SousTotalMontant);
        sousTotaux.push_back(xlReference(ligne, 9));
        totalGeneral += sousTotal;
        ligne += 2;
    }

    // ---- Total général ----
    std::string somme;
    for (size_t i = 0; i < sousTotaux.size(); ++i) somme += (i ? "," : "") + sousTotaux[i];
    xlTexte(xl, ligne, 1, "TOTAL GÉNÉRAL  (" + std::to_string(produits.size()) + " produit(s), "
                              + std::to_string(totalQuantites) + " unité(s))", TotalLibelle);
    xlFusionner(xl, ligne, 1, ligne, derniereColonne - 1, TotalLibelle);
    xlFormule(xl, ligne, derniereColonne, somme.empty() ? "0" : "SUM(" + somme + ")", totalGeneral, TotalMontant);
    xl.hauteurs[ligne] = 24;
    ligne += 3;

    // ---- Signatures ----
    xlTexte(xl, ligne, 1, "Visa du responsable du stock :", Libelle);
    xlFusionner(xl, ligne, 1, ligne, 3, Libelle);
    xlTexte(xl, ligne, 6, "Cachet et signature du fournisseur :", Libelle);
    xlFusionner(xl, ligne, 6, ligne, 9, Libelle);
    ligne += 4;
    xlTexte(xl, ligne, 1, "Fiche générée automatiquement par Pet Manager à partir des produits ayant atteint "
                          "leur seuil minimal. À imprimer ou à envoyer au fournisseur.", Note);
    xlFusionner(xl, ligne, 1, ligne, derniereColonne, Note);

    // ---- Écriture du fichier (QFile gère les chemins avec accents sous Windows) ----
    const std::string contenu = xlArchive(xl, "Fiche de commande");
    QFile fichier(chemin);
    if (!fichier.open(QIODevice::WriteOnly)) {
        erreurFiche = "Impossible de créer le fichier (est-il déjà ouvert dans Excel ?).";
        return false;
    }
    fichier.write(contenu.data(), qint64(contenu.size()));
    fichier.close();
    return true;
}

// #####################################################################
//
//      MÉTIER 1 : ALERTE AUTOMATIQUE DE RÉAPPROVISIONNEMENT PAR E-MAIL
//  - repère les produits qui ont atteint le seuil minimal (listeSousSeuil()),
//  - n'alerte qu'une fois par produit tant qu'il reste sous le seuil,
//  - construit et envoie l'e-mail au responsable du stock (SMTP + TLS),
//  - garde un historique des envois.
//  Paramètres et historique : fichier petmanager.ini à côté de l'exécutable.
// #####################################################################

QString Produit::fichierParametres()
{
    return QCoreApplication::applicationDirPath() + "/petmanager.ini";
}

// ---------------- Paramètres ----------------

ConfigEmail Produit::configurationEmail()
{
    QSettings s(fichierParametres(), QSettings::IniFormat);
    ConfigEmail c;
    c.serveur      = s.value("email/serveur", "smtp.gmail.com").toString();
    c.port         = s.value("email/port", 465).toInt();
    c.utilisateur  = s.value("email/utilisateur").toString();
    c.motDePasse   = s.value("email/motDePasse").toString();
    c.destinataire = s.value("email/destinataire").toString();
    return c;
}

bool Produit::configurationComplete(const ConfigEmail &config)
{
    return !config.serveur.trimmed().isEmpty() && config.port > 0 && config.utilisateur.contains('@')
           && !config.motDePasse.isEmpty() && config.destinataire.contains('@');
}

bool Produit::envoiAutomatique()
{
    return QSettings(fichierParametres(), QSettings::IniFormat).value("email/automatique", true).toBool();
}

int Produit::intervalleMinutes()
{
    return QSettings(fichierParametres(), QSettings::IniFormat).value("email/intervalle", 30).toInt();
}

int Produit::facteurStockCible()
{
    return QSettings(fichierParametres(), QSettings::IniFormat).value("email/facteur", 2).toInt();
}

QString Produit::cleApiIA()
{
    return QSettings(fichierParametres(), QSettings::IniFormat).value("ia/cle").toString();
}

QString Produit::modeleIA()
{
    return QSettings(fichierParametres(), QSettings::IniFormat).value("ia/modele", "gemini-flash-latest").toString();
}

void Produit::enregistrerParametres(const ConfigEmail &config, bool automatique, int intervalle,
                                    int facteur, const QString &cleIA, const QString &modele)
{
    QSettings s(fichierParametres(), QSettings::IniFormat);
    s.setValue("email/serveur", config.serveur.trimmed());
    s.setValue("email/port", config.port);
    s.setValue("email/utilisateur", config.utilisateur.trimmed());
    s.setValue("email/motDePasse", config.motDePasse);
    s.setValue("email/destinataire", config.destinataire.trimmed());
    s.setValue("email/automatique", automatique);
    s.setValue("email/intervalle", intervalle);
    s.setValue("email/facteur", facteur);
    s.setValue("ia/cle", cleIA.trimmed());
    s.setValue("ia/modele", modele.trimmed());
}

void Produit::enregistrerSurveillance(int intervalle, int facteur)
{
    QSettings s(fichierParametres(), QSettings::IniFormat);
    s.setValue("email/intervalle", intervalle);
    s.setValue("email/facteur", facteur);
}

// ---------------- Surveillance ----------------

// Produits déjà signalés : "id|date ISO"
static QMap<int, QDateTime> lireSuivi(const QString &chemin)
{
    QMap<int, QDateTime> suivi;
    const QStringList lignes = QSettings(chemin, QSettings::IniFormat).value("suivi/signales").toStringList();
    for (const QString &l : lignes) {
        const QStringList morceaux = l.split('|');
        if (morceaux.size() == 2)
            suivi[morceaux[0].toInt()] = QDateTime::fromString(morceaux[1], Qt::ISODate);
    }
    return suivi;
}

static void ecrireSuivi(const QString &chemin, const QMap<int, QDateTime> &suivi)
{
    QStringList lignes;
    for (auto it = suivi.cbegin(); it != suivi.cend(); ++it)
        lignes << QString::number(it.key()) + "|" + it.value().toString(Qt::ISODate);
    QSettings(chemin, QSettings::IniFormat).setValue("suivi/signales", lignes);
}

void Produit::actualiserSuivi(const QList<Produit> &sousSeuil)
{
    QSet<int> ids;
    for (const Produit &p : sousSeuil) ids.insert(p.getIdProduit());

    QMap<int, QDateTime> suivi = lireSuivi(fichierParametres());
    for (auto it = suivi.begin(); it != suivi.end(); ) {
        if (!ids.contains(it.key())) it = suivi.erase(it);   // revenu au-dessus du seuil
        else ++it;
    }
    ecrireSuivi(fichierParametres(), suivi);
}

QList<Produit> Produit::produitsNonSignales(const QList<Produit> &sousSeuil)
{
    const QMap<int, QDateTime> suivi = lireSuivi(fichierParametres());
    QList<Produit> resultat;
    for (const Produit &p : sousSeuil)
        if (!suivi.contains(p.getIdProduit())) resultat.append(p);
    return resultat;
}

QDateTime Produit::derniereAlerte(int idProduit)
{
    return lireSuivi(fichierParametres()).value(idProduit);
}

// ---------------- Envoi de l'alerte ----------------

QString Produit::construireHtmlAlerte(const QList<Produit> &produits, const QMap<int, QString> &fournisseurs)
{
    const int facteur = facteurStockCible();
    QString lignes;
    double total = 0;
    for (const Produit &p : produits) {
        const int qte = quantiteRecommandee(p.getQuantite(), p.getSeuilMin(), facteur);
        total += qte * p.getPrixUnitaire();
        const QString couleur = p.getQuantite() == 0 ? "#C62828" : "#E08A00";
        lignes += QString("<tr>"
                          "<td style='padding:8px;border-bottom:1px solid #E0E8E8'>%1</td>"
                          "<td style='padding:8px;border-bottom:1px solid #E0E8E8'><b>%2</b><br>"
                          "<span style='color:#6B8080;font-size:12px'>%3 · %4</span></td>"
                          "<td style='padding:8px;border-bottom:1px solid #E0E8E8;color:%5'><b>%6</b></td>"
                          "<td style='padding:8px;border-bottom:1px solid #E0E8E8'>%7</td>"
                          "<td style='padding:8px;border-bottom:1px solid #E0E8E8;color:#1F7A6E'><b>%8</b></td>"
                          "<td style='padding:8px;border-bottom:1px solid #E0E8E8'>%9</td>"
                          "</tr>")
                      .arg(p.getIdProduit())
                      .arg(p.getNom().toHtmlEscaped(), p.getCategorie().toHtmlEscaped(),
                           p.getEmplacement().toHtmlEscaped(), couleur)
                      .arg(p.getQuantite())
                      .arg(p.getSeuilMin())
                      .arg(qte)
                      .arg(fournisseurs.value(p.getIdProduit(), "-").toHtmlEscaped());
    }

    return QString(
        "<html><body style='font-family:Segoe UI,Arial,sans-serif;color:#1E3A3A;background:#F4F8F8;padding:20px'>"
        "<div style='max-width:760px;margin:auto;background:white;border:1px solid #E0E8E8;border-radius:10px'>"
        "<div style='background:#2A8C86;color:white;padding:16px 20px;border-radius:10px 10px 0 0'>"
        "<b style='font-size:18px'>Pet Manager – Alerte de réapprovisionnement</b></div>"
        "<div style='padding:20px'>"
        "<p>Bonjour,</p>"
        "<p>La surveillance automatique du stock a détecté <b>%1 produit(s)</b> ayant atteint "
        "ou dépassé vers le bas leur <b>seuil minimal</b>. Un réapprovisionnement est recommandé "
        "pour éviter une rupture.</p>"
        "<table style='border-collapse:collapse;width:100%;font-size:13px'>"
        "<tr style='background:#F7FAFA;text-align:left'>"
        "<th style='padding:8px'>ID</th><th style='padding:8px'>Produit</th>"
        "<th style='padding:8px'>Qté restante</th><th style='padding:8px'>Seuil min.</th>"
        "<th style='padding:8px'>Qté à commander</th><th style='padding:8px'>Fournisseur</th></tr>"
        "%2</table>"
        "<p style='margin-top:16px'>Montant estimé de la commande : <b>%3 TND</b><br>"
        "<span style='color:#6B8080;font-size:12px'>Quantité recommandée = %4 × seuil minimal − quantité restante.</span></p>"
        "<p style='color:#6B8080;font-size:12px'>Message envoyé automatiquement par Pet Manager le %5.</p>"
        "</div></div></body></html>")
        .arg(produits.size())
        .arg(lignes)
        .arg(QLocale(QLocale::French).toString(total, 'f', 3))
        .arg(facteur)
        .arg(QDateTime::currentDateTime().toString("dd/MM/yyyy à HH:mm"));
}

bool Produit::envoyerAlerte(const QList<Produit> &produits, const QMap<int, QString> &fournisseurs,
                            bool automatique, QString &erreurEnvoi)
{
    if (produits.isEmpty()) {
        erreurEnvoi = "Aucun produit n'a atteint son seuil minimal.";
        return false;
    }
    const ConfigEmail config = configurationEmail();
    const QString sujet = produits.size() == 1
        ? "[Pet Manager] Alerte réapprovisionnement : " + produits.first().getNom()
        : QString("[Pet Manager] Alerte réapprovisionnement : %1 produits sous le seuil").arg(produits.size());

    const bool ok = envoyerEmail(config, sujet, construireHtmlAlerte(produits, fournisseurs), erreurEnvoi);

    QStringList noms;
    for (const Produit &p : produits) noms << p.getNom();
    EntreeHistorique e;
    e.date = QDateTime::currentDateTime();
    e.destinataire = config.destinataire;
    e.produits = noms.join(", ");
    e.automatique = automatique;
    e.succes = ok;
    e.detail = ok ? QString("Envoyé") : erreurEnvoi;
    ajouterHistorique(e);

    if (ok) {   // ces produits ne seront plus signalés tant qu'ils restent sous le seuil
        QMap<int, QDateTime> suivi = lireSuivi(fichierParametres());
        for (const Produit &p : produits) suivi[p.getIdProduit()] = e.date;
        ecrireSuivi(fichierParametres(), suivi);
    }
    return ok;
}

bool Produit::envoyerTest(const ConfigEmail &config, QString &erreurEnvoi)
{
    return envoyerEmail(config, "[Pet Manager] Test de l'alerte e-mail",
                        "<html><body style='font-family:Segoe UI,Arial,sans-serif'>"
                        "<h3 style='color:#2A8C86'>Pet Manager</h3>"
                        "<p>La configuration de l'alerte de réapprovisionnement fonctionne : "
                        "vous recevrez ici les alertes du stock.</p></body></html>", erreurEnvoi);
}

// Client SMTP (QSslSocket, chiffrement TLS)
bool Produit::envoyerEmail(const ConfigEmail &config, const QString &sujet,
                           const QString &html, QString &erreurEnvoi)
{
    if (!configurationComplete(config)) {
        erreurEnvoi = "Paramètres e-mail incomplets (serveur, expéditeur, mot de passe, responsable).";
        return false;
    }
    if (!QSslSocket::supportsSsl()) {
        erreurEnvoi = "Le chiffrement TLS n'est pas disponible dans cette installation de Qt.";
        return false;
    }

    QStringList destinataires;
    for (const QString &d : config.destinataire.split(QRegularExpression("[;,]"), Qt::SkipEmptyParts))
        if (d.trimmed().contains('@')) destinataires << d.trimmed();

    const int delai = 15000;
    QSslSocket socket;

    // Lit la réponse du serveur (éventuellement sur plusieurs lignes "250-...")
    auto lire = [&](int codeAttendu) -> bool {
        QByteArray reponse;
        while (true) {
            if (!socket.canReadLine() && !socket.waitForReadyRead(delai)) {
                erreurEnvoi = "Le serveur SMTP ne répond pas (" + socket.errorString() + ").";
                return false;
            }
            while (socket.canReadLine()) {
                const QByteArray ligne = socket.readLine();
                reponse += ligne;
                if (ligne.size() >= 4 && ligne.at(3) == ' ') {      // dernière ligne
                    const int code = ligne.left(3).toInt();
                    if (code / 100 != codeAttendu / 100) {
                        erreurEnvoi = "Refus du serveur SMTP : " + QString::fromUtf8(reponse).trimmed();
                        if (code == 535)
                            erreurEnvoi += "\nVérifiez l'adresse et le mot de passe d'application.";
                        return false;
                    }
                    return true;
                }
            }
        }
    };
    auto commande = [&](const QByteArray &texte, int codeAttendu) -> bool {
        socket.write(texte + "\r\n");
        socket.flush();
        return lire(codeAttendu);
    };

    if (config.port == 465) {                       // SSL dès la connexion
        socket.connectToHostEncrypted(config.serveur, quint16(config.port));
        if (!socket.waitForEncrypted(delai)) {
            erreurEnvoi = "Connexion sécurisée impossible : " + socket.errorString();
            return false;
        }
        if (!lire(220)) return false;
    } else {                                        // 587 : connexion puis STARTTLS
        socket.connectToHost(config.serveur, quint16(config.port));
        if (!socket.waitForConnected(delai)) {
            erreurEnvoi = "Connexion au serveur impossible : " + socket.errorString();
            return false;
        }
        if (!lire(220) || !commande("EHLO petmanager", 250) || !commande("STARTTLS", 220)) return false;
        socket.startClientEncryption();
        if (!socket.waitForEncrypted(delai)) {
            erreurEnvoi = "Chiffrement STARTTLS impossible : " + socket.errorString();
            return false;
        }
    }

    if (!commande("EHLO petmanager", 250)) return false;
    if (!commande("AUTH LOGIN", 334)) return false;
    if (!commande(config.utilisateur.trimmed().toUtf8().toBase64(), 334)) return false;
    if (!commande(config.motDePasse.toUtf8().toBase64(), 235)) return false;
    if (!commande("MAIL FROM:<" + config.utilisateur.trimmed().toUtf8() + ">", 250)) return false;
    for (const QString &d : destinataires)
        if (!commande("RCPT TO:<" + d.toUtf8() + ">", 250)) return false;
    if (!commande("DATA", 354)) return false;

    // Corps HTML encodé en base64 (accents conservés), lignes de 76 caractères
    const QByteArray corps64 = html.toUtf8().toBase64();
    QByteArray corps;
    for (int i = 0; i < corps64.size(); i += 76)
        corps += corps64.mid(i, 76) + "\r\n";

    QByteArray message;
    message += "From: Pet Manager <" + config.utilisateur.trimmed().toUtf8() + ">\r\n";
    message += "To: " + destinataires.join(", ").toUtf8() + "\r\n";
    message += "Subject: =?UTF-8?B?" + sujet.toUtf8().toBase64() + "?=\r\n";
    message += "Date: " + QDateTime::currentDateTime().toString(Qt::RFC2822Date).toUtf8() + "\r\n";
    message += "MIME-Version: 1.0\r\n";
    message += "Content-Type: text/html; charset=UTF-8\r\n";
    message += "Content-Transfer-Encoding: base64\r\n\r\n";
    message += corps;
    message += ".";                                  // fin du message
    if (!commande(message, 250)) return false;

    socket.write("QUIT\r\n");
    socket.flush();
    socket.waitForBytesWritten(2000);
    socket.disconnectFromHost();
    erreurEnvoi.clear();
    return true;
}

// ---------------- Historique ----------------

QList<EntreeHistorique> Produit::historiqueAlertes()
{
    QList<EntreeHistorique> liste;
    QSettings s(fichierParametres(), QSettings::IniFormat);
    const int n = s.beginReadArray("historique");
    for (int i = 0; i < n; ++i) {
        s.setArrayIndex(i);
        EntreeHistorique e;
        e.date         = QDateTime::fromString(s.value("date").toString(), Qt::ISODate);
        e.destinataire = s.value("destinataire").toString();
        e.produits     = s.value("produits").toString();
        e.automatique  = s.value("automatique").toBool();
        e.succes       = s.value("succes").toBool();
        e.detail       = s.value("detail").toString();
        liste.append(e);
    }
    s.endArray();
    return liste;
}

void Produit::ajouterHistorique(const EntreeHistorique &entree)
{
    QList<EntreeHistorique> liste = historiqueAlertes();
    liste.prepend(entree);                     // la plus récente en premier
    while (liste.size() > 100) liste.removeLast();

    QSettings s(fichierParametres(), QSettings::IniFormat);
    s.remove("historique");
    s.beginWriteArray("historique", int(liste.size()));
    for (int i = 0; i < liste.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("date", liste[i].date.toString(Qt::ISODate));
        s.setValue("destinataire", liste[i].destinataire);
        s.setValue("produits", liste[i].produits);
        s.setValue("automatique", liste[i].automatique);
        s.setValue("succes", liste[i].succes);
        s.setValue("detail", liste[i].detail);
    }
    s.endArray();
}

void Produit::viderHistoriqueAlertes()
{
    QSettings(fichierParametres(), QSettings::IniFormat).remove("historique");
}

// #####################################################################
//
//      MÉTIER 2 : SUGGESTION INTELLIGENTE DE PRODUITS ALTERNATIFS (IA)
//  L'IA (Google Gemini) reçoit le besoin de l'utilisateur et le catalogue
//  des produits réellement disponibles dans la table STOCK. Elle renvoie au
//  maximum 3 alternatives justifiées. Sans clé API ou sans Internet, un
//  moteur local (mots-clés + catégorie) prend le relais.
// #####################################################################

static const QString CONSEIL_LOCAL =
    "Suggestions calculées par le moteur local (mots-clés et catégorie). "
    "Ajoutez une clé API Gemini dans « Paramètres des métiers » pour une analyse "
    "intelligente de la fonction et de l'usage des produits.";

bool Produit::iaConfiguree()
{
    return !cleApiIA().trimmed().isEmpty();
}

QList<SuggestionIA> Produit::suggererAlternatives(const QString &besoin, const QString &categorieBesoin,
                                                  int idExclu, QString &conseil, bool &viaIA,
                                                  QString &avertissement)
{
    // Catalogue des produits utilisables, lu dans la table STOCK
    const QList<Produit> catalogue = catalogueDisponible();
    viaIA = false;
    avertissement.clear();

    const QString cle = cleApiIA().trimmed();
    if (cle.isEmpty()) {
        conseil = CONSEIL_LOCAL;
        return suggestionsLocales(besoin, categorieBesoin, idExclu, catalogue);
    }
    QString modele = modeleIA().trimmed();
    if (modele.isEmpty()) modele = "gemini-flash-latest";

    // Requête envoyée à l'API Gemini
    const QJsonObject partie{{"text", construirePromptIA(besoin, categorieBesoin, idExclu, catalogue)}};
    const QJsonObject message{{"role", "user"}, {"parts", QJsonArray{partie}}};
    const QJsonObject generation{{"temperature", 0.2}, {"responseMimeType", "application/json"}};
    const QJsonObject corps{{"contents", QJsonArray{message}}, {"generationConfig", generation}};

    QNetworkRequest requete(QUrl("https://generativelanguage.googleapis.com/v1beta/models/"
                                 + modele + ":generateContent"));
    requete.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    requete.setRawHeader("x-goog-api-key", cle.toUtf8());
    requete.setTransferTimeout(30000);   // 30 s maximum

    // Attente de la réponse : la boucle locale garde l'interface affichée
    QNetworkAccessManager reseau;
    QNetworkReply *reponse = reseau.post(requete, QJsonDocument(corps).toJson(QJsonDocument::Compact));
    QEventLoop boucle;
    QObject::connect(reponse, &QNetworkReply::finished, &boucle, &QEventLoop::quit);
    if (!reponse->isFinished())
        boucle.exec();

    const QByteArray donnees = reponse->readAll();
    QString erreurIA;
    if (reponse->error() == QNetworkReply::NoError) {
        QList<SuggestionIA> suggestions;
        if (analyserReponseIA(donnees, catalogue, idExclu, suggestions, conseil, erreurIA)) {
            viaIA = true;
            return suggestions;
        }
    } else {
        // Message d'erreur renvoyé par l'API (clé invalide, quota, modèle...)
        const QJsonObject err = QJsonDocument::fromJson(donnees).object().value("error").toObject();
        erreurIA = err.value("message").toString();
        if (erreurIA.isEmpty()) erreurIA = reponse->errorString();
    }
    avertissement = erreurIA;
    conseil = CONSEIL_LOCAL;
    return suggestionsLocales(besoin, categorieBesoin, idExclu, catalogue);
}

QString Produit::construirePromptIA(const QString &besoin, const QString &categorieBesoin,
                                    int idExclu, const QList<Produit> &catalogue)
{
    QString lignes;
    int n = 0;
    for (const Produit &p : catalogue) {
        if (p.getIdProduit() == idExclu) continue;
        if (++n > 200) break;
        lignes += QString("%1 | %2 | %3 | %4 | %5 | %6\n")
                      .arg(p.getIdProduit()).arg(p.getNom(), p.getCategorie())
                      .arg(p.getQuantite())
                      .arg(p.getDateExpiration().toString("dd/MM/yyyy"), p.getEmplacement());
    }
    if (lignes.isEmpty()) lignes = "(aucun produit disponible)\n";

    return QString(
        "Tu es l'assistant de gestion de stock d'un centre de soins vétérinaires (Pet Manager).\n"
        "Un utilisateur recherche le produit suivant, ou ne peut pas l'utiliser car il est indisponible :\n"
        "« %1 » (catégorie connue : %2)\n\n"
        "Catalogue des produits RÉELLEMENT disponibles en stock "
        "(ID | nom | catégorie | quantité | date d'expiration | emplacement) :\n%3\n"
        "Consignes :\n"
        "1. Analyse la fonction, l'usage et les caractéristiques du produit recherché "
        "(espèce visée, forme, rôle médical ou pratique).\n"
        "2. Choisis au maximum 3 produits du catalogue qui peuvent le remplacer de façon pertinente, "
        "même s'ils appartiennent à une autre catégorie.\n"
        "3. N'invente aucun produit : utilise uniquement les ID du catalogue ci-dessus%4.\n"
        "4. Pour chaque alternative, explique en une ou deux phrases pourquoi elle convient (\"raison\"), "
        "puis ses avantages et précautions d'utilisation (\"avantages\") ; pour un médicament, rappelle "
        "que l'avis du vétérinaire reste nécessaire.\n"
        "5. Classe les alternatives de la plus pertinente à la moins pertinente. Si aucune n'est "
        "pertinente, renvoie une liste vide et explique pourquoi dans \"conseil\".\n"
        "Réponds UNIQUEMENT avec un JSON valide de la forme :\n"
        "{\"alternatives\":[{\"id\":12,\"pertinence\":85,\"raison\":\"...\",\"avantages\":\"...\"}],"
        "\"conseil\":\"...\"}\n"
        "\"pertinence\" est un entier de 0 à 100. Rédige tout en français.")
        .arg(besoin.trimmed(),
             categorieBesoin.isEmpty() ? QString("non précisée") : categorieBesoin,
             lignes,
             idExclu > 0 ? QString(" (n'inclus pas le produit ID %1)").arg(idExclu) : QString());
}

bool Produit::analyserReponseIA(const QByteArray &json, const QList<Produit> &catalogue, int idExclu,
                                QList<SuggestionIA> &suggestions, QString &conseil, QString &erreurIA)
{
    // 1) Texte produit par le modèle
    const QJsonObject racine = QJsonDocument::fromJson(json).object();
    const QJsonArray candidats = racine.value("candidates").toArray();
    if (candidats.isEmpty()) { erreurIA = "réponse vide de l'IA"; return false; }

    QString texte;
    const QJsonArray parties = candidats.first().toObject().value("content").toObject()
                                   .value("parts").toArray();
    for (const QJsonValue &v : parties) {
        const QJsonObject partie = v.toObject();
        if (!partie.value("thought").toBool())
            texte += partie.value("text").toString();
    }

    // 2) On isole l'objet JSON (au cas où il serait entouré de ```json ... ```)
    const int debut = texte.indexOf('{');
    const int fin = texte.lastIndexOf('}');
    if (debut < 0 || fin <= debut) { erreurIA = "format de réponse inattendu"; return false; }
    const QJsonObject resultat = QJsonDocument::fromJson(texte.mid(debut, fin - debut + 1).toUtf8()).object();
    if (resultat.isEmpty()) { erreurIA = "JSON de l'IA illisible"; return false; }

    // 3) Seuls les produits présents dans le catalogue sont acceptés (pas d'invention)
    QHash<int, Produit> parId;
    for (const Produit &p : catalogue) parId.insert(p.getIdProduit(), p);

    QSet<int> dejaVus;
    const QJsonArray alternatives = resultat.value("alternatives").toArray();
    for (const QJsonValue &v : alternatives) {
        const QJsonObject o = v.toObject();
        const int id = o.value("id").toVariant().toInt();
        if (id == idExclu || !parId.contains(id) || dejaVus.contains(id)) continue;
        dejaVus.insert(id);

        const Produit p = parId.value(id);
        SuggestionIA s;
        s.idProduit = id;
        s.nom = p.getNom();
        s.categorie = p.getCategorie();
        s.quantite = p.getQuantite();
        s.pertinence = std::clamp(o.value("pertinence").toVariant().toInt(), 0, 100);
        s.raison = o.value("raison").toString();
        s.avantages = o.value("avantages").toString();
        suggestions.append(s);
    }
    std::stable_sort(suggestions.begin(), suggestions.end(),
                     [](const SuggestionIA &a, const SuggestionIA &b) { return a.pertinence > b.pertinence; });
    while (suggestions.size() > 3) suggestions.removeLast();

    conseil = resultat.value("conseil").toString();
    return true;
}

// ---------------- Moteur local de secours (sans clé API / sans Internet) ----------------

// "Vermifuge Chat 10mg" -> {"vermifuge", "chat", "10mg"}  (sans accents)
static QSet<QString> motsCles(const QString &texte)
{
    static const QSet<QString> motsVides = {"pour", "avec", "les", "des", "une", "sans", "aux",
                                            "par", "sur", "dans", "produit", "boite"};
    QString t = texte.toLower().normalized(QString::NormalizationForm_D);
    QString propre;
    for (const QChar &c : t) {
        if (c.category() == QChar::Mark_NonSpacing) continue;
        propre += c.isLetterOrNumber() ? c : QChar(' ');
    }
    QSet<QString> mots;
    for (const QString &m : propre.split(' ', Qt::SkipEmptyParts))
        if (m.length() >= 3 && !motsVides.contains(m)) mots.insert(m);
    return mots;
}

QList<SuggestionIA> Produit::suggestionsLocales(const QString &besoin, const QString &categorieBesoin,
                                                int idExclu, const QList<Produit> &catalogue)
{
    const QSet<QString> motsBesoin = motsCles(besoin);
    QList<SuggestionIA> liste;

    for (const Produit &p : catalogue) {
        if (p.getIdProduit() == idExclu) continue;

        int score = 0;
        QStringList raisons;
        if (!categorieBesoin.isEmpty() && p.getCategorie() == categorieBesoin) {
            score += 35;
            raisons << "même catégorie (" + categorieBesoin + ")";
        }
        QStringList communs;
        const QSet<QString> motsProduit = motsCles(p.getNom() + " " + p.getCategorie());
        for (const QString &m : motsBesoin)
            for (const QString &mp : motsProduit)
                if (mp == m || (m.length() >= 5 && mp.startsWith(m.left(5)))) { communs << m; break; }
        if (!communs.isEmpty()) {
            score += std::min(25 * int(communs.size()), 55);
            raisons << "mots en commun : " + communs.join(", ");
        }
        if (score < 30) continue;                             // pas assez proche
        if (p.getQuantite() > p.getSeuilMin()) score += 10;   // stock confortable

        SuggestionIA s;
        s.idProduit = p.getIdProduit();
        s.nom = p.getNom();
        s.categorie = p.getCategorie();
        s.quantite = p.getQuantite();
        s.pertinence = std::min(score, 95);
        s.raison = "Proposé car " + raisons.join(" et ") + ".";
        s.avantages = QString("Disponible immédiatement : %1 unité(s), expire le %2, emplacement : %3.")
                          .arg(p.getQuantite())
                          .arg(p.getDateExpiration().toString("dd/MM/yyyy"), p.getEmplacement());
        liste.append(s);
    }
    std::stable_sort(liste.begin(), liste.end(),
                     [](const SuggestionIA &a, const SuggestionIA &b) { return a.pertinence > b.pertinence; });
    while (liste.size() > 3) liste.removeLast();
    return liste;
}
