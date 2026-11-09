#ifndef CONNEXIONPAGE_H
#define CONNEXIONPAGE_H

#include <QList>
#include <QWidget>
#include <functional>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTimer;

// Écran de connexion affiché dans "Accueil" tant que personne n'est connecté
class ConnexionPage : public QWidget
{
    Q_OBJECT
public:
    explicit ConnexionPage(QWidget *parent = nullptr);

    void reinitialiser();       // vide les champs du formulaire

signals:
    void connexionReussie(const QString &utilisateur);

private slots:
    void tenterConnexion();
    void retraduire();
    void tickBlocage();

private:
    void afficherErreur(const QString &texte);
    void masquerErreur();
    void bloquer(int secondes);

    QList<std::function<void()>> m_traducteurs;

    QLineEdit *m_user = nullptr;
    QLineEdit *m_pass = nullptr;
    QPushButton *m_btnVoir = nullptr;
    QPushButton *m_btnConnexion = nullptr;
    QLabel *m_erreur = nullptr;
    QComboBox *m_langue = nullptr;
    QTimer *m_timerBlocage = nullptr;

    int m_essais = 0;
    int m_secondesBlocage = 0;
};

#endif // CONNEXIONPAGE_H
