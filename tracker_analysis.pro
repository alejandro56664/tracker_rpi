greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
qtHaveModule(printsupport): QT += printsupport

TEMPLATE = app

target.path = /home/pi/tracker_analysis
TARGET = tracker_analysis
INSTALLS += target

SOURCES += main.cpp\
    segment.cpp \
    track.cpp \
    videowindow.cpp \
    qcustomplot.cpp \
    mainwindow.cpp \
    plotwindow.cpp \
    tracklist.cpp \
    videocore.cpp \

HEADERS  += \
    cvimagewidget.h \
    segment.h \
    track.h \
    videowindow.h \
    qcustomplot.h \
    mainwindow.h \
    plotwindow.h \
    tracklist.h \
    videocore.h \

QT += quick \
      widgets\
    printsupport\
    core gui


INCLUDEPATH += /mnt/rasp-pi-rootfs/usr/local/qt5pi/include/

for(deploymentfolder, DEPLOYMENTFOLDERS) {
    item = item$${deploymentfolder}
    itemsources = $${item}.sources
    $$itemsources = $$eval($${deploymentfolder}.source)
    itempath = $${item}.path
    $$itempath= $$eval($${deploymentfolder}.target)
    export($$itemsources)
    export($$itempath)
    DEPLOYMENT += $$item
}

installPrefix = /home/pi/$${TARGET}

for(deploymentfolder, DEPLOYMENTFOLDERS) {
    item = item$${deploymentfolder}
    itemfiles = $${item}.files
    $$itemfiles = $$eval($${deploymentfolder}.source)
    itempath = $${item}.path
    $$itempath = $${installPrefix}/$$eval($${deploymentfolder}.target)
    export($$itemfiles)
    export($$itempath)
    INSTALLS += $$item
}

unix:!macx: LIBS += -L$$PWD/../../../../mnt/rasp-pi-rootfs/usr/local/lib/ -lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_objdetect -lopencv_ts -lopencv_video


INCLUDEPATH += $$PWD/../../../../mnt/rasp-pi-rootfs/usr/local/include
DEPENDPATH += $$PWD/../../../../mnt/rasp-pi-rootfs/usr/local/include

