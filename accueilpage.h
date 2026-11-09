#ifndef ACCUEILPAGE_H
#define ACCUEILPAGE_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <functional>

class QLabel;
class QFrame;
class QPushButton;
class QVBoxLayout;

// Chiffres affichés sur l'accueil (remplis par MainWindow à partir des 3 modules)
struct AccueilDonnees {
    // Animaux
    int animaux = 0;
    int chiens = 0;
    int chats = 0;
    int lapins = 0;
    int animauxSurveillance = 0;
    // Stock
    int produits = 0;
    int ruptures = 0;
    int stockBas = 0;
    int expires = 0;
    int commandesEnAttente = 0;
    // Fournisseurs
    int fournisseurs = 0;
    int fournisseursActifs = 0;
    // Employés
    int employes = 0;
    int employesActifs = 0;
    // Liste "À surveiller"
    QStringList alertes;
};

// Tableau de bord affiché après la connexion
class AccueilPage : public QWidget
{
    Q_OBJECT
public:
    enum Module { Animaux = 0, Stock = 1, Fournisseurs = 2, Employes = 3, Parametres = 4 };

    explicit AccueilPage(QWidget *parent = nullptr);

    void setDonnees(const AccueilDonnees &d);
    void actualiserUtilisateur();     // nom, rôle et avatar de l'utilisateur connecté + salutation

signals:
    void naviguerVers(int module);    // clic sur un bouton "Accès rapide"
    void deconnexionDemandee();

private slots:
    void retraduire();

private:
    QFrame *creerKpi(const QString &icone, const char *cleTitre, QLabel *&valeur, QLabel *&detail);

    QList<std::function<void()>> m_traducteurs;
    AccueilDonnees m_donnees;

    QLabel *m_salutation = nullptr;
    QLabel *m_avatar = nullptr;
    QLabel *m_nomUser = nullptr;
    QLabel *m_roleUser = nullptr;
    QLabel *m_valeur[4] = {nullptr, nullptr, nullptr, nullptr};
    QLabel *m_detail[4] = {nullptr, nullptr, nullptr, nullptr};
    QVBoxLayout *m_listeAlertes = nullptr;
    QLabel *m_resume = nullptr;
    QLabel *m_astuce = nullptr;
};

#endif // ACCUEILPAGE_H
