#ifndef LANGUE_H
#define LANGUE_H

#include <QLocale>
#include <QObject>
#include <QString>

// Gestion de la langue de l'application (Français / English / العربية).
// Les nouvelles pages (connexion, accueil, paramètres) et le menu latéral utilisent Langue::t("cle").
class Langue : public QObject
{
    Q_OBJECT
public:
    enum Code { Francais = 0, Anglais = 1, Arabe = 2 };

    static Langue &instance();
    static QString t(const char *cle);            // texte traduit (renvoie la clé si elle est inconnue)
    static Code courante();
    static void definir(Code code);               // enregistre le choix et prévient toutes les pages
    static QString nomLangue(Code code);          // "Français", "English", "العربية"
    static Qt::LayoutDirection direction();       // RightToLeft pour l'arabe
    static QLocale locale();

signals:
    void langueChangee();

private:
    Langue();
    Code m_code = Francais;
};

#endif // LANGUE_H
