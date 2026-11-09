#ifndef EMPLOYEDAO_H
#define EMPLOYEDAO_H

#include "employe.h"
#include "databasemanager.h"
#include <QList>
#include <QString>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

class EmployeDAO {
public:
    // CRUD de base
    static bool ajouter(const Employe &employe);
    static bool modifier(const Employe &employe);
    static bool supprimer(int id);
    static Employe obtenirParId(int id);
    static QList<Employe> obtenirTous();

    // Métiers basiques
    static QList<Employe> rechercherParNom(const QString &texte);
    static QList<Employe> trierParPoste();
};

#endif // EMPLOYEDAO_H