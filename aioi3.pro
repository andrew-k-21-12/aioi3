#-------------------------------------------------
#
# Project created by QtCreator 2016-02-26T19:38:22
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TemplateMatching
TEMPLATE = app

CONFIG += c++11

macx {
    INCLUDEPATH += /usr/local/opt/opencv3/include
    CONFIG += link_pkgconfig
    # PKGCONFIG += opencv
}

LIBS += `pkg-config opencv --libs`

SOURCES += main.cpp

HEADERS  += mainwindow.h \
    resultdialog.h \
    tangenthistsearch.h \
    textures.h \
    sketchsearch.h \
    skeletonbuilder.h

FORMS    += mainwindow.ui \
    resultdialog.ui
