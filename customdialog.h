#ifndef CUSTOMDIALOG_H
#define CUSTOMDIALOG_H

#include <QDialog>
#include <QString>

class CustomDialog : public QDialog
{
    Q_OBJECT

public:
    enum Type { Success, Warning, Error, Question, Info };

    explicit CustomDialog(QWidget *parent = nullptr, Type type = Info,
                          const QString &title = "", const QString &message = "",
                          const QString &confirmText = "OK", bool isDanger = false);

    static void success(QWidget *parent, const QString &title, const QString &text);
    static void warning(QWidget *parent, const QString &title, const QString &text);
    static void error(QWidget *parent, const QString &title, const QString &text);
    static void info(QWidget *parent, const QString &title, const QString &text);
    static bool confirm(QWidget *parent, const QString &title, const QString &text,
                        const QString &confirmText = "Oui", bool isDanger = false);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupUI();
    Type m_type;
    bool m_confirmed;
};

#endif // CUSTOMDIALOG_H