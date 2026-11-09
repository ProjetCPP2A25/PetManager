#ifndef MOCKCOMMANDESPROVIDER_H
#define MOCKCOMMANDESPROVIDER_H
#include <QList>
struct MockCommande { int idFournisseur; int totalDeliveries; int onTimeDeliveries; int problems; };
class MockCommandesProvider {
public:
    static QList<MockCommande> getMockData();
    static int calculateReliability(int idFournisseur);
};
#endif