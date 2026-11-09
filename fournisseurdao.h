#ifndef FOURNISSEURDAO_H
#define FOURNISSEURDAO_H
#include "fournisseur.h"
#include <QList>
#include <QString>

class FournisseurDAO {
public:
    static bool inserer(const Fournisseur &f);
    static bool modifier(const Fournisseur &f);
    static bool supprimer(int idFournisseur);
    static Fournisseur obtenirParId(int idFournisseur);
    static QList<Fournisseur> obtenirTous();
    static QList<Fournisseur> rechercherParNom(const QString &nom);
};
#endif // FOURNISSEURDAO_H