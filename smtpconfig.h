#ifndef SMTPCONFIG_H
#define SMTPCONFIG_H

// =====================================================================
//   Configuration de l'envoi d'e-mails (code "mot de passe oublié")
// =====================================================================
// Exemple avec Gmail :
//   1. Activez la validation en 2 étapes sur le compte Gmail expéditeur
//   2. Créez un "mot de passe d'application" : https://myaccount.google.com/apppasswords
//   3. Collez l'adresse Gmail dans UTILISATEUR et le mot de passe d'application
//      (16 lettres, sans espaces) dans MOT_DE_PASSE
//
// Si UTILISATEUR reste vide, l'application fonctionne en MODE DÉMO :
// le code est affiché directement dans la fenêtre au lieu d'être envoyé.
//
// ⚠️ Ne poussez pas votre vrai mot de passe d'application sur un dépôt GitHub public.

namespace SmtpConfig {
inline const char *SERVEUR        = "smtp.gmail.com";
inline const int   PORT           = 465;            // SSL/TLS direct
inline const char *UTILISATEUR    = "";             // ex. "petmanager.app@gmail.com"
inline const char *MOT_DE_PASSE   = "";             // mot de passe d'application Gmail
inline const char *NOM_EXPEDITEUR = "Pet Manager";
}

#endif // SMTPCONFIG_H
