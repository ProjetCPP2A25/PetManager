#ifndef DIALOGSAUTH_H
#define DIALOGSAUTH_H

#include <QString>

class QWidget;

namespace DialogsAuth {

// E-mail du compte -> code de vérification envoyé par e-mail -> nouveau mot de passe
void motDePasseOublie(QWidget *parent);

// Demande l'ancien mot de passe, puis le nouveau (utilisateur déjà connecté)
void changerMotDePasse(QWidget *parent, const QString &utilisateur);

}

#endif // DIALOGSAUTH_H
