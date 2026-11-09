#include "smtpclient.h"
#include "smtpconfig.h"

#include <QDateTime>
#include <QSslSocket>
#include <QStringList>

namespace {

const int DELAI_MS = 15000;

// Lit une réponse SMTP complète (éventuellement sur plusieurs lignes) et renvoie son code
int lireReponse(QSslSocket &socket, QString *texte)
{
    QString tout;
    while (true) {
        while (!socket.canReadLine()) {
            if (!socket.waitForReadyRead(DELAI_MS)) {
                if (texte) *texte = QStringLiteral("Pas de réponse du serveur (%1)").arg(socket.errorString());
                return -1;
            }
        }
        const QString ligne = QString::fromUtf8(socket.readLine()).trimmed();
        tout += ligne + "\n";
        // "250-..." = la réponse continue ; "250 ..." = dernière ligne
        if (ligne.size() < 4 || ligne.at(3) != QLatin1Char('-')) {
            if (texte) *texte = tout.trimmed();
            return ligne.left(3).toInt();
        }
    }
}

bool commande(QSslSocket &socket, const QByteArray &cmd, int codeAttendu, QString *erreur)
{
    socket.write(cmd + "\r\n");
    if (!socket.waitForBytesWritten(DELAI_MS)) {
        if (erreur) *erreur = QStringLiteral("Écriture impossible : %1").arg(socket.errorString());
        return false;
    }
    QString reponse;
    const int code = lireReponse(socket, &reponse);
    if (code != codeAttendu) {
        if (erreur) *erreur = reponse.isEmpty() ? QStringLiteral("Réponse inattendue du serveur") : reponse;
        return false;
    }
    return true;
}

QByteArray encoderEntete(const QString &texte)
{
    return "=?UTF-8?B?" + texte.toUtf8().toBase64() + "?=";
}

} // namespace

bool SmtpClient::estConfigure()
{
    return QString::fromUtf8(SmtpConfig::UTILISATEUR).trimmed().size() > 0
        && QString::fromUtf8(SmtpConfig::MOT_DE_PASSE).trimmed().size() > 0;
}

bool SmtpClient::envoyer(const QString &destinataire, const QString &sujet,
                         const QString &corps, QString *erreur)
{
    if (!estConfigure()) {
        if (erreur) *erreur = QStringLiteral("Aucun compte expéditeur configuré (smtpconfig.h).");
        return false;
    }
    if (!QSslSocket::supportsSsl()) {
        if (erreur) *erreur = QStringLiteral("SSL n'est pas disponible dans cette installation de Qt.");
        return false;
    }

    const QByteArray utilisateur = QByteArray(SmtpConfig::UTILISATEUR).trimmed();
    const QByteArray motDePasse = QByteArray(SmtpConfig::MOT_DE_PASSE).replace(" ", "");

    QSslSocket socket;
    socket.connectToHostEncrypted(QString::fromUtf8(SmtpConfig::SERVEUR), SmtpConfig::PORT);
    if (!socket.waitForEncrypted(DELAI_MS)) {
        if (erreur) *erreur = QStringLiteral("Connexion au serveur impossible : %1").arg(socket.errorString());
        return false;
    }

    QString reponse;
    if (lireReponse(socket, &reponse) != 220) {
        if (erreur) *erreur = reponse;
        return false;
    }

    if (!commande(socket, "EHLO petmanager", 250, erreur)) return false;
    if (!commande(socket, "AUTH LOGIN", 334, erreur)) return false;
    if (!commande(socket, utilisateur.toBase64(), 334, erreur)) return false;
    if (!commande(socket, motDePasse.toBase64(), 235, erreur)) {
        if (erreur) *erreur = QStringLiteral("Authentification refusée (vérifiez l'adresse et le mot de passe d'application).\n") + *erreur;
        return false;
    }
    if (!commande(socket, "MAIL FROM:<" + utilisateur + ">", 250, erreur)) return false;
    if (!commande(socket, "RCPT TO:<" + destinataire.trimmed().toUtf8() + ">", 250, erreur)) return false;
    if (!commande(socket, "DATA", 354, erreur)) return false;

    // Corps : fins de ligne CRLF + "dot-stuffing" (une ligne qui commence par "." est doublée)
    QStringList lignes = QString(corps).replace("\r\n", "\n").split('\n');
    for (QString &l : lignes)
        if (l.startsWith('.'))
            l.prepend('.');

    QByteArray message;
    message += "From: " + encoderEntete(QString::fromUtf8(SmtpConfig::NOM_EXPEDITEUR)) + " <" + utilisateur + ">\r\n";
    message += "To: <" + destinataire.trimmed().toUtf8() + ">\r\n";
    message += "Subject: " + encoderEntete(sujet) + "\r\n";
    message += "Date: " + QDateTime::currentDateTime().toString(Qt::RFC2822Date).toUtf8() + "\r\n";
    message += "MIME-Version: 1.0\r\n";
    message += "Content-Type: text/plain; charset=UTF-8\r\n";
    message += "Content-Transfer-Encoding: 8bit\r\n\r\n";
    message += lignes.join("\r\n").toUtf8();
    message += "\r\n.";

    if (!commande(socket, message, 250, erreur)) return false;
    commande(socket, "QUIT", 221, nullptr);
    socket.disconnectFromHost();
    return true;
}
