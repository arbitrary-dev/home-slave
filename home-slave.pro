#-------------------------------------------------
#
# Project created by QtCreator 2016-11-28T10:05:11
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = home-slave
TEMPLATE = app

INCLUDEPATH += ./inc ./ui
VPATH += ./src ./inc ./ui

SOURCES += main.cpp\
           mainwindow.cpp \
           tasksmodel.cpp \
           tasksdelegate.cpp

HEADERS  += mainwindow.h \
            tasksmodel.h \
            ui_mainwindow.h \
            tasksdelegate.h

FORMS    += mainwindow.ui
