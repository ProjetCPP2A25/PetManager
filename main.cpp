#include "connexion.h"
#include "gpetmanager.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Une seule connexion à la BD (Singleton), établie AVANT la fenêtre
    Connexion &cx = Connexion::createInstance();
    if (!cx.createConnection()) {
        QMessageBox::critical(nullptr, QObject::tr("Base de données"),
                              QObject::tr("Connexion à la base de données échouée.\n%1\n\n"
                                          "Vérifiez la source ODBC, l'utilisateur et le mot de passe "
                                          "(connexion.cpp).").arg(cx.derniereErreur()),
                              QMessageBox::Cancel);
        return 1;
    }

    // L'instanciation de la fenêtre se fait APRÈS la connexion :
    // son constructeur appelle Commande::afficher()
    GPetManager w;
    w.showMaximized();

    const int resultat = a.exec();
    cx.closeConnection();
    return resultat;
}