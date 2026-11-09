#include "mockcommandesprovider.h"
#include <QtGlobal>

QList<MockCommande> MockCommandesProvider::getMockData() {
    return { {1, 10, 9, 0}, {2, 10, 7, 2}, {3, 10, 5, 4} };
}

int MockCommandesProvider::calculateReliability(int idFournisseur) {
    QList<MockCommande> data = getMockData();
    for (const MockCommande &m : data) {
        if (m.idFournisseur == idFournisseur) {
            if (m.totalDeliveries == 0) return 50;
            int score = (m.onTimeDeliveries * 100 / m.totalDeliveries) - (m.problems * 5);

            if (score < 0) {
                score = 0;
            }
            if (score > 100) {
                score = 100;
            }
            return score;
        }
    }
    return 50;
}