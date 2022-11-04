#-------------------------------------------------
#
# Project created by QtCreator 2021-11-05T15:52:48
#
#-------------------------------------------------

QT       += core gui serialport network#QLibrary

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia

TARGET = TSRF_UI
TEMPLATE = app

INCLUDEPATH += include
LIBS += -lcurl -lusbgx

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    gpiotools.cpp \
    starmedlogo.cpp \
    setup.cpp \
    sysparm.cpp \
    gpiobutton.cpp \
    audiooutput.cpp \
    sysmessage.cpp \
    rfoutputmodes.cpp \
    outputthread.cpp \
    outputmode.cpp \
    workerthread.cpp \
    monitoringtask.cpp \
    sha256_software.cpp \
    securedic.cpp \
    ds28e25_ds28e22_ds28e15.cpp \
    1wire_ds2465.cpp \
    calibration.cpp \
    file.cpp \
    mrfa_rf_table.c \
    monitoringthread.cpp \
    usbotg.cpp

HEADERS  += mainwindow.h \
    include.h \
    gpiotools.h \
    global.h \
    starmedlogo.h \
    setup.h \
    sysparm.h \
    gpiobutton.h \
    audiooutput.h \
    sysmessage.h \
    rfoutputmodes.h \
    outputthread.h \
    outputmode.h \
    workerthread.h \
    monitoringtask.h \
    sha256_software.h \
    securedic.h \
    ds28e25_ds28e22_ds28e15.h \
    1wire_ds2465.h \
    calibration.h \
    file.h \
    mrfa_rf_table.h \
    monitoringthread.h \
    usbotg.h

FORMS    += mainwindow.ui \
    starmedlogo.ui \
    setup.ui \
    sysmessage.ui \
    calibration.ui

RESOURCES += \
    image.qrc \
    fonts.qrc \
    sound.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../starmed/buildroot/output-buildroot-2017.02.5/build/libusbgx-2e3d43ee098ed928d1baa61ce791ce9ff4788c5a/src/.libs/release/ -lusbgx
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../starmed/buildroot/output-buildroot-2017.02.5/build/libusbgx-2e3d43ee098ed928d1baa61ce791ce9ff4788c5a/src/.libs/debug/ -lusbgx
else:unix: LIBS += -L$$PWD/../starmed/buildroot/output-buildroot-2017.02.5/build/libusbgx-2e3d43ee098ed928d1baa61ce791ce9ff4788c5a/src/.libs/ -lusbgx

INCLUDEPATH += $$PWD/include/usbg
DEPENDPATH += $$PWD/include/usbg

