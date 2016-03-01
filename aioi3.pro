#-------------------------------------------------
#
# Project created by QtCreator 2016-02-26T19:38:22
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = aioi3
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dialogbinarization.cpp \
    dialogotsulocal.cpp \
    dialogquantization.cpp

HEADERS  += mainwindow.h \
    dialogbinarization.h \
    dialogotsulocal.h \
    dialogquantization.h

FORMS    += mainwindow.ui \
    dialogbinarization.ui \
    dialogotsulocal.ui \
    dialogquantization.ui

RESOURCES += \
    resources.qrc
