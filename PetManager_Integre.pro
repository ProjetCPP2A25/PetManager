QT += widgets sql charts printsupport svg network
CONFIG += c++17

# Accents et emojis du code bien lus avec le compilateur MSVC
msvc: QMAKE_CXXFLAGS += /utf-8

SOURCES += \
    employe.cpp \
    employedao.cpp \
    employespage.cpp \
    langue.cpp \
    dialogsauth.cpp \
    smtpclient.cpp \
    connexionpage.cpp \
    authmanager.cpp \
    accueilpage.cpp \
    animauxpage.cpp \
    parametrespage.cpp \
    customdialog.cpp \
    databasemanager.cpp \
    fournisseur.cpp \
    fournisseurdao.cpp \
    main.cpp \
    mainwindow.cpp \
    mockcommandesprovider.cpp \
    supplierrecommender.cpp \
    badgedelegate.cpp \
    icondelegate.cpp \
    graphiques.cpp

HEADERS += \
    employe.h \
    employedao.h \
    employespage.h \
    langue.h \
    dialogsauth.h \
    smtpclient.h \
    smtpconfig.h \
    connexionpage.h \
    authmanager.h \
    accueilpage.h \
    animauxpage.h \
    parametrespage.h \
    customdialog.h \
    databasemanager.h \
    fournisseur.h \
    fournisseurdao.h \
    mainwindow.h \
    mockcommandesprovider.h \
    supplierrecommender.h \
    badgedelegate.h \
    icondelegate.h \
    graphiques.h

FORMS += mainwindow.ui \
    employespage.ui
RESOURCES += resources.qrc \
    ressources.qrc \
    employes.qrc
