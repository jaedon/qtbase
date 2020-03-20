TARGET = qeglfs-brcmstb-integration

QT += core-private gui-private eglfsdeviceintegration-private

INCLUDEPATH += $$PWD/../../api
CONFIG += egl

LIBS += -lv3dplatform

# Avoid X11 header collision, use generic EGL native types
DEFINES += QT_EGL_NO_X11

SOURCES += $$PWD/qeglfsbrcmstbmain.cpp \
           $$PWD/qeglfsbrcmstbintegration.cpp

HEADERS += $$PWD/qeglfsbrcmstbintegration.h

OTHER_FILES += $$PWD/eglfs_brcmstb.json

PLUGIN_TYPE = egldeviceintegrations
PLUGIN_CLASS_NAME = QEglFSBrcmSTBIntegrationPlugin
load(qt_plugin)
