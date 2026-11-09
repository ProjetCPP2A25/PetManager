# Projet Pet Manager - modules Stock + Animaux + Rendez-vous (qmake)
QT += core gui sql network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Accents et emojis du code bien lus avec le compilateur MSVC
msvc: QMAKE_CXXFLAGS += /utf-8

SOURCES += \
    connection.cpp \
    main.cpp \
    gpetmanager.cpp \
    produit.cpp \
    animal.cpp \
    connexion.cpp \
    mainwindow.cpp \
    rendezvous.cpp \
    rendezvousvue.cpp

HEADERS += \
    connection.h \
    gpetmanager.h \
    produit.h \
    animal.h \
    connexion.h \
    mainwindow.h \
    rendezvous.h \
    rendezvousvue.h

FORMS += \
    gpetmanager.ui \
    mainwindow.ui \
    rendezvousvue.ui

RESOURCES += \
    ressources.qrc

DISTFILES += \
    sql/base_commune_petmanager.sql \
    sql/donnees_test.sql \
    sql/donnees_animaux.sql \
    sql/modif_rendezvous.sql
