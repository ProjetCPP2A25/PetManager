#ifndef SUPPLIERRECOMMENDER_H
#define SUPPLIERRECOMMENDER_H
#include "fournisseur.h"
#include <QList>
#include <QString>
struct Recommendation { int idFournisseur; QString nom; int score; QString reason; };
class SupplierRecommender {
public:
    static QList<Recommendation> getTop3(const QString &productType, const QList<Fournisseur> &suppliers);
};
#endif