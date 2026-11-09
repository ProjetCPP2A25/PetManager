#include "supplierrecommender.h"
#include "mockcommandesprovider.h"
#include <algorithm>
#include <QtGlobal>

QList<Recommendation> SupplierRecommender::getTop3(const QString &productType, const QList<Fournisseur> &suppliers) {
    QList<Recommendation> recs;
    for (const Fournisseur &f : suppliers) {
        // Note: Filtering for "Actif" is now done in mainwindow.cpp before calling this
        if (!productType.isEmpty() && f.typeProduit() != productType) continue;

        int reliability = MockCommandesProvider::calculateReliability(f.idFournisseur());
        int priceScore = 100 - (int)(f.prixUnitaire() / 10); if (priceScore < 0) priceScore = 0;
        int qtyScore = f.quantite() / 10; if (qtyScore > 100) qtyScore = 100;
        int totalScore = (int)((reliability * 0.4) + (priceScore * 0.3) + (qtyScore * 0.3));
        recs.append({f.idFournisseur(), f.nom(), totalScore, "Fiabilité: " + QString::number(reliability) + "%"});
    }
    std::sort(recs.begin(), recs.end(), [](const Recommendation &a, const Recommendation &b) { return a.score > b.score; });
    if (recs.size() > 3) return recs.mid(0, 3);
    return recs;
}