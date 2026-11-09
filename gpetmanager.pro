QT += core gui widgets sql

CONFIG += c++17

SOURCES += \
    rendezvous.cpp \
    connexion.cpp \
    main.cpp \
    gpetmanager.cpp

HEADERS += \
    rendezvous.h \
    connexion.h \
    gpetmanager.h

FORMS += \
    gpetmanager.ui

RESOURCES += \
    ressources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target