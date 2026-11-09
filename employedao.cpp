#include "employedao.h"

bool EmployeDAO::ajouter(const Employe &employe) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare("INSERT INTO employe (nom, prenom, email, telephone, poste, salaire, dateEmbauche, statut) "
                  "VALUES (:nom, :prenom, :email, :telephone, :poste, :salaire, :dateEmbauche, :statut)");
    query.bindValue(":nom", employe.getNom());
    query.bindValue(":prenom", employe.getPrenom());
    query.bindValue(":email", employe.getEmail());
    query.bindValue(":telephone", employe.getTelephone());
    query.bindValue(":poste", employe.getPoste());
    query.bindValue(":salaire", employe.getSalaire());
    query.bindValue(":dateEmbauche", employe.getDateEmbauche().toString("yyyy-MM-dd"));
    query.bindValue(":statut", employe.getStatut());

    if (!query.exec()) {
        qDebug() << "Erreur ajout employé:" << query.lastError().text();
        return false;
    }
    return true;
}

bool EmployeDAO::modifier(const Employe &employe) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare("UPDATE employe SET nom=:nom, prenom=:prenom, email=:email, telephone=:telephone, "
                  "poste=:poste, salaire=:salaire, dateEmbauche=:dateEmbauche, statut=:statut WHERE id=:id");
    query.bindValue(":nom", employe.getNom());
    query.bindValue(":prenom", employe.getPrenom());
    query.bindValue(":email", employe.getEmail());
    query.bindValue(":telephone", employe.getTelephone());
    query.bindValue(":poste", employe.getPoste());
    query.bindValue(":salaire", employe.getSalaire());
    query.bindValue(":dateEmbauche", employe.getDateEmbauche().toString("yyyy-MM-dd"));
    query.bindValue(":statut", employe.getStatut());
    query.bindValue(":id", employe.getId());

    if (!query.exec()) {
        qDebug() << "Erreur modification employé:" << query.lastError().text();
        return false;
    }
    return true;
}

bool EmployeDAO::supprimer(int id) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare("DELETE FROM employe WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur suppression employé:" << query.lastError().text();
        return false;
    }
    return true;
}

Employe EmployeDAO::obtenirParId(int id) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare("SELECT * FROM employe WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return Employe(
            query.value("id").toInt(),
            query.value("nom").toString(),
            query.value("prenom").toString(),
            query.value("email").toString(),
            query.value("telephone").toString(),
            query.value("poste").toString(),
            query.value("salaire").toDouble(),
            QDate::fromString(query.value("dateEmbauche").toString(), "yyyy-MM-dd"),
            query.value("statut").toString()
            );
    }
    return Employe(); // Retourne un employé vide si non trouvé
}

QList<Employe> EmployeDAO::obtenirTous() {
    QList<Employe> liste;
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare("SELECT * FROM employe ORDER BY id DESC");

    if (query.exec()) {
        while (query.next()) {
            liste.append(Employe(
                query.value("id").toInt(),
                query.value("nom").toString(),
                query.value("prenom").toString(),
                query.value("email").toString(),
                query.value("telephone").toString(),
                query.value("poste").toString(),
                query.value("salaire").toDouble(),
                QDate::fromString(query.value("dateEmbauche").toString(), "yyyy-MM-dd"),
                query.value("statut").toString()
                ));
        }
    } else {
        qDebug() << "Erreur récupération employés:" << query.lastError().text();
    }
    return liste;
}

QList<Employe> EmployeDAO::rechercherParNom(const QString &texte) {
    QList<Employe> liste;
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    QString recherche = "%" + texte + "%";
    query.prepare("SELECT * FROM employe WHERE nom LIKE :texte OR prenom LIKE :texte ORDER BY nom");
    query.bindValue(":texte", recherche);

    if (query.exec()) {
        while (query.next()) {
            liste.append(Employe(
                query.value("id").toInt(),
                query.value("nom").toString(),
                query.value("prenom").toString(),
                query.value("email").toString(),
                query.value("telephone").toString(),
                query.value("poste").toString(),
                query.value("salaire").toDouble(),
                QDate::fromString(query.value("dateEmbauche").toString(), "yyyy-MM-dd"),
                query.value("statut").toString()
                ));
        }
    }
    return liste;
}

QList<Employe> EmployeDAO::trierParPoste() {
    QList<Employe> liste;
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare("SELECT * FROM employe ORDER BY poste ASC, nom ASC");

    if (query.exec()) {
        while (query.next()) {
            liste.append(Employe(
                query.value("id").toInt(),
                query.value("nom").toString(),
                query.value("prenom").toString(),
                query.value("email").toString(),
                query.value("telephone").toString(),
                query.value("poste").toString(),
                query.value("salaire").toDouble(),
                QDate::fromString(query.value("dateEmbauche").toString(), "yyyy-MM-dd"),
                query.value("statut").toString()
                ));
        }
    }
    return liste;
}