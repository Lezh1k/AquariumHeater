TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    src/main.c \
    src/max7219.c

HEADERS += \
    include/commons.h \
    include/max7219.h

INCLUDEPATH += /home/lezh1k/avr8-gnu-toolchain-linux_x86_64/avr/include
INCLUDEPATH += include

DEFINES += __AVR_ATtiny25__

DISTFILES += \
    Makefile
