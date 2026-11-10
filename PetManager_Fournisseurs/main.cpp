#include "mainwindow.h"
#include "connection.h"
#include "customdialog.h"
#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QFile>  // <-- ADDED THIS LINE
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setStyle(QStyleFactory::create("Fusion"));
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#F4F8F8"));
    palette.setColor(QPalette::WindowText, QColor("#2D3436"));
    palette.setColor(QPalette::Base, QColor("#FFFFFF"));
    palette.setColor(QPalette::AlternateBase, QColor("#F4F8F8"));
    palette.setColor(QPalette::ToolTipBase, QColor("#FFFFFF"));
    palette.setColor(QPalette::ToolTipText, QColor("#2D3436"));
    palette.setColor(QPalette::Text, QColor("#2D3436"));
    palette.setColor(QPalette::Button, QColor("#FFFFFF"));
    palette.setColor(QPalette::ButtonText, QColor("#2D3436"));
    palette.setColor(QPalette::BrightText, QColor("#FFFFFF"));
    palette.setColor(QPalette::Link, QColor("#2A8C82"));
    palette.setColor(QPalette::Highlight, QColor("#2A8C82"));
    palette.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));
    a.setPalette(palette);

    Connection& c = Connection::createInstance();
    if (!c.createConnection()) {
        QMessageBox::critical(nullptr, "Erreur BD", "Impossible de se connecter à Oracle.\nVérifiez que le service Oracle est bien lancé.");
        return 1;
    }

    QFile themeFile(":/styles/theme.qss");
    if (themeFile.open(QFile::ReadOnly | QFile::Text)) {
        a.setStyleSheet(themeFile.readAll());
        themeFile.close();
    }

    MainWindow w;
    w.show();
    return a.exec();
}