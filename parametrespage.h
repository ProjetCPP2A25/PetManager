#ifndef PARAMETRESPAGE_H
#define PARAMETRESPAGE_H

#include <QList>
#include <QString>
#include <QWidget>
#include <functional>

class QCheckBox;
class QComboBox;
class QFrame;
class QLabel;
class QLineEdit;
class QVBoxLayout;

// Page "Paramètres" : compte, langue, démarrage, alertes… (réglages enregistrés avec QSettings)
class ParametresPage : public QWidget
{
    Q_OBJECT
public:
    explicit ParametresPage(QWidget *parent = nullptr);

    void actualiserCompte();                 // affiche l'utilisateur connecté

    // Lecture des réglages (utilisable depuis n'importe quel module)
    static QString nomCentre();
    static QString pageDemarrage();          // "Accueil", "Animaux", "Stock" ou "Fournisseurs"
    static bool alerteSanteAnimaux();

signals:
    void reglagesEnregistres();
    void deconnexionDemandee();

private slots:
    void enregistrer();
    void reinitialiser();
    void ouvrirDossierBase();
    void retraduire();

private:
    void chargerDansFormulaire();
    QFrame *creerCarte(const char *cleTitre, const QString &emoji, QVBoxLayout *&contenu);

    QList<std::function<void()>> m_traducteurs;

    QLabel *m_avatar = nullptr;
    QLabel *m_nomCompte = nullptr;
    QLabel *m_roleCompte = nullptr;
    QComboBox *m_langue = nullptr;
    QLineEdit *m_centre = nullptr;
    QComboBox *m_demarrage = nullptr;
    QCheckBox *m_alerteSante = nullptr;
};

#endif // PARAMETRESPAGE_H
