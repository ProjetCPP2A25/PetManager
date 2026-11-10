# Projet Pet Manager - module Gestion de stock (qmake)
QT += core gui sql network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Accents et emojis du code bien lus avec le compilateur MSVC
msvc: QMAKE_CXXFLAGS += /utf-8

SOURCES += \
    connection.cpp \
    main.cpp \
    gpetmanager.cpp \
    produit.cpp

HEADERS += \
    connection.h \
    gpetmanager.h \
    produit.h

FORMS += \
    gpetmanager.ui

RESOURCES += \
    ressources.qrc

DISTFILES += \
    sql/base_commune_petmanager.sql \
    sql/donnees_test.sql
