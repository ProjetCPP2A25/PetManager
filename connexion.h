#ifndef CONNEXION_H
#define CONNEXION_H

#include <QSqlDatabase>
#include <QString>


class Connexion
{
public:
    static Connexion &createInstance();
    bool createConnection();
    void closeConnection();
    QString derniereErreur() const;

private:
    QSqlDatabase db;

    Connexion();
    ~Connexion();
    Connexion(const Connexion &) = delete;
    Connexion &operator=(const Connexion &) = delete;
};

#endif // CONNEXION_H