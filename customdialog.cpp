#include "customdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QKeyEvent>
#include <QApplication>
#include <QScreen>

CustomDialog::CustomDialog(QWidget *parent, Type type, const QString &title, const QString &message, const QString &confirmText, bool isDanger)
    : QDialog(parent), m_type(type), m_confirmed(false)
{
    setupUI();

    QLabel *iconLabel = findChild<QLabel*>("iconBadge");
    QLabel *titleLabel = findChild<QLabel*>("titleLabel");
    QLabel *msgLabel = findChild<QLabel*>("msgLabel");
    QHBoxLayout *btnLayout = findChild<QHBoxLayout*>("btnLayout");

    if (titleLabel) titleLabel->setText(title);
    if (msgLabel) msgLabel->setText(message);

    QString iconText = "";
    QString bgColor = "";
    QString textColor = "#FFFFFF";

    switch (m_type) {
    case Success:
        iconText = QString::fromUtf8("\u2713");
        bgColor = "#28a745";
        break;
    case Warning:
        iconText = "!";
        bgColor = "#fd7e14";
        break;
    case Error:
        iconText = QString::fromUtf8("\u2715");
        bgColor = "#dc3545";
        break;
    case Question:
        iconText = "?";
        bgColor = "#2A8C82";
        break;
    case Info:
    default:
        iconText = "i";
        bgColor = "#17a2b8";
        break;
    }

    if (iconLabel) {
        iconLabel->setText(iconText);
        iconLabel->setStyleSheet(QString("background-color: %1; color: %2; border-radius: 24px; font-size: 24px; font-weight: bold;")
                                     .arg(bgColor, textColor));
    }

    if (btnLayout) {
        btnLayout->addStretch();
        if (m_type == Question) {
            QPushButton *btnCancel = new QPushButton("Annuler");
            btnCancel->setFixedSize(120, 40);
            btnCancel->setStyleSheet("QPushButton { background-color: #FFFFFF; color: #7B8A8A; border: 1.5px solid #DADDDC; border-radius: 8px; font-weight: bold; font-size: 14px; }"
                                     "QPushButton:hover { background-color: #F4F8F8; }"
                                     "QPushButton:pressed { background-color: #E6F2F1; }");
            connect(btnCancel, &QPushButton::clicked, this, [this]() { m_confirmed = false; reject(); });
            btnLayout->addWidget(btnCancel);

            QPushButton *btnConfirm = new QPushButton(confirmText);
            btnConfirm->setFixedSize(120, 40);
            QString confirmColor = isDanger ? "#D9534F" : "#2A8C82";
            btnConfirm->setStyleSheet(QString("QPushButton { background-color: %1; color: #FFFFFF; border: none; border-radius: 8px; font-weight: bold; font-size: 14px; }"
                                              "QPushButton:hover { background-color: %2; }"
                                              "QPushButton:pressed { background-color: %3; }")
                                          .arg(confirmColor, confirmColor + "CC", confirmColor + "99"));
            connect(btnConfirm, &QPushButton::clicked, this, [this]() { m_confirmed = true; accept(); });
            btnLayout->addWidget(btnConfirm);
        } else {
            QPushButton *btnOk = new QPushButton("OK");
            btnOk->setFixedSize(120, 40);
            btnOk->setStyleSheet("QPushButton { background-color: #2A8C82; color: #FFFFFF; border: none; border-radius: 8px; font-weight: bold; font-size: 14px; }"
                                 "QPushButton:hover { background-color: #23766D; }"
                                 "QPushButton:pressed { background-color: #1E635B; }");
            connect(btnOk, &QPushButton::clicked, this, [this]() { m_confirmed = true; accept(); });
            btnLayout->addWidget(btnOk);
        }
        btnLayout->addStretch();
    }

    if (parent) {
        QRect parentRect = parent->geometry();
        move(parentRect.center() - rect().center());
    } else {
        QRect screenRect = QApplication::primaryScreen()->geometry();
        move(screenRect.center() - rect().center());
    }
}

void CustomDialog::setupUI()
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);
    setFixedSize(420, 280);

    QWidget *card = new QWidget(this);
    card->setStyleSheet("background-color: #FFFFFF; border-radius: 14px;");

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 40));
    shadow->setOffset(0, 4);
    card->setGraphicsEffect(shadow);

    QVBoxLayout *mainLayout = new QVBoxLayout(card);
    mainLayout->setContentsMargins(30, 30, 30, 20);
    mainLayout->setSpacing(20);

    QLabel *iconBadge = new QLabel();
    iconBadge->setObjectName("iconBadge");
    iconBadge->setFixedSize(48, 48);
    iconBadge->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(iconBadge, 0, Qt::AlignHCenter);

    QLabel *titleLabel = new QLabel();
    titleLabel->setObjectName("titleLabel");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2D3436;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    QLabel *msgLabel = new QLabel();
    msgLabel->setObjectName("msgLabel");
    msgLabel->setStyleSheet("font-size: 13px; color: #4E6B68;");
    msgLabel->setAlignment(Qt::AlignCenter);
    msgLabel->setWordWrap(true);
    mainLayout->addWidget(msgLabel, 1);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setObjectName("btnLayout");
    btnLayout->setSpacing(12);
    mainLayout->addLayout(btnLayout);

    QVBoxLayout *dialogLayout = new QVBoxLayout(this);
    dialogLayout->setContentsMargins(10, 10, 10, 10);
    dialogLayout->addWidget(card);
}

void CustomDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (m_type == Question) {
            m_confirmed = false;
            reject();
        } else {
            m_confirmed = true;
            accept();
        }
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        m_confirmed = true;
        accept();
    } else {
        QDialog::keyPressEvent(event);
    }
}

void CustomDialog::success(QWidget *parent, const QString &title, const QString &text) {
    CustomDialog d(parent, Success, title, text);
    d.exec();
}

void CustomDialog::warning(QWidget *parent, const QString &title, const QString &text) {
    CustomDialog d(parent, Warning, title, text);
    d.exec();
}

void CustomDialog::error(QWidget *parent, const QString &title, const QString &text) {
    CustomDialog d(parent, Error, title, text);
    d.exec();
}

void CustomDialog::info(QWidget *parent, const QString &title, const QString &text) {
    CustomDialog d(parent, Info, title, text);
    d.exec();
}

bool CustomDialog::confirm(QWidget *parent, const QString &title, const QString &text, const QString &confirmText, bool isDanger) {
    CustomDialog d(parent, Question, title, text, confirmText, isDanger);
    return d.exec() == QDialog::Accepted && d.m_confirmed;
}