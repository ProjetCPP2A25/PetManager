#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include <QString>

// Petit client SMTP (SSL, AUTH LOGIN) pour envoyer un e-mail texte.
namespace SmtpClient {

// true si un compte expéditeur est renseigné dans smtpconfig.h
bool estConfigure();

// Envoie un e-mail. Renvoie false et remplit "erreur" en cas d'échec.
bool envoyer(const QString &destinataire, const QString &sujet,
             const QString &corps, QString *erreur = nullptr);

}

#endif // SMTPCLIENT_H
