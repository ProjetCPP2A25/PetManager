#include "gpetmanager.h"
#include "connection.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Une seule connexion à la BD (Singleton)
    Connection &c = Connection::createInstance();
    bool test = c.createConnection();   // Établir la connexion

    if (!test) {
        QMessageBox::critical(nullptr, QObject::tr("Base de données"),
                              QObject::tr("Connexion à Oracle échouée.\n%1\n\n"
                                          "Vérifiez la source ODBC « Source_Projet2A », "
                                          "l'utilisateur et le mot de passe (connection.cpp).")
                                  .arg(c.derniereErreur()),
                              QMessageBox::Cancel);
        return 1;
    }

    // L'instanciation de la fenêtre se fait APRÈS la connexion :
    // son constructeur appelle Produit::afficher()
    GPetManager w;
    w.showMaximized();
    QMessageBox::information(nullptr, QObject::tr("Base de données"),
                             QObject::tr("Connexion réussie."), QMessageBox::Ok);
    w.demarrerSurveillance();          // métier 1 : premier contrôle du stock

    const int resultat = a.exec();
    c.closeConnection();
    return resultat;
}
