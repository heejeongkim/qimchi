TEMPLATE = lib
CONFIG += plugin c++11
QT += qml quick

DESTDIR = ../imports/Chilinet
TARGET = $$qtLibraryTarget(chilinetplugin)

HEADERS += \
    chilinetplugin.h \
    chiliclient.h \
    helper.h \
    state.h

SOURCES += \
    chilinetplugin.cpp \
    chiliclient.cpp \
    state.cpp

LIBS += -lzmq

target.path=$$DESTPATH
qmldir.files=$$PWD/qmldir


qmldir.path=$$DESTPATH
INSTALLS += target qmldir

OTHER_FILES += \
    qmldir

# Copy the qmldir file to the same folder as the plugin binary
QMAKE_POST_LINK += $$QMAKE_COPY $$replace($$list($$quote($$PWD/qmldir) $$DESTDIR), /, $$QMAKE_DIR_SEP)
