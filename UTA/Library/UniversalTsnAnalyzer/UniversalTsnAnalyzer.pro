#-------------------------------------------------
#
# Project created by QtCreator 2016-12-14T14:32:07
#
#-------------------------------------------------

QT       += core gui
QT       += serialport
QT       += charts
QT       += widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UniversalTsnAnalyzer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# NPcap
DEFINES += NPCAP
INCLUDEPATH += NpdPack/Include
LIBS += "-L$$_PRO_FILE_PWD_/NpdPack/Lib" -lwpcap -lws2_32

DEFINES += WPCAP
DEFINES += HAVE_REMOTE

INCLUDEPATH += TsnTab \
    HelpMenu


SOURCES += main.cpp \
    Uta_UniversalTsnAnalyzer.cpp \
    TsnTab/Uta_TsnTab.cpp \
    TsnTab/Uta_TsnPrioScreen.cpp \
    HelpMenu/Uta_HelpMenuAbout.cpp

HEADERS  += Uta_UniversalTsnAnalyzer.h \
    TsnTab/Uta_TsnTab.h \
    TsnTab/Uta_TsnPrioScreen.h \
    HelpMenu/Uta_HelpMenuAbout.h

FORMS    += Uta_UniversalTsnAnalyzer.ui \
    TsnTab/Uta_TsnTab.ui \
    TsnTab/Uta_TsnPrioScreen.ui \
    HelpMenu/Uta_HelpMenuAbout.ui

RC_FILE = Uta_UniversalTsnAnalyzer.rc
RESOURCES = Uta_UniversalTsnAnalyzer.qrc

DISTFILES += \
    Uta_UniversalTsnAnalyzer.ico \
    Uta_UniversalTsnAnalyzer.rc
