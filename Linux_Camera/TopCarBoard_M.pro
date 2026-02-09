QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwidget.cpp \
    UVC/processImage.cpp \
    UVC/videodevice.cpp \
    QMapDemo/gps_modul.cpp \
    QMapDemo/multidemo.cpp \
    QMapDemo/src/QMapControl/Geometry.cpp \
    QMapDemo/src/QMapControl/GeometryLineString.cpp \
    QMapDemo/src/QMapControl/GeometryPoint.cpp \
    QMapDemo/src/QMapControl/GeometryPointArrow.cpp \
    QMapDemo/src/QMapControl/GeometryPointCircle.cpp \
    QMapDemo/src/QMapControl/GeometryPointImage.cpp \
    QMapDemo/src/QMapControl/GeometryPointImageScaled.cpp \
    QMapDemo/src/QMapControl/GeometryPointShape.cpp \
    QMapDemo/src/QMapControl/GeometryPointShapeScaled.cpp \
    QMapDemo/src/QMapControl/GeometryPolygon.cpp \
    QMapDemo/src/QMapControl/GeometryPolygonImage.cpp \
    QMapDemo/src/QMapControl/GeometryWidget.cpp \
    QMapDemo/src/QMapControl/GPS_Position.cpp \
    QMapDemo/src/QMapControl/ImageManager.cpp \
    QMapDemo/src/QMapControl/Layer.cpp \
    QMapDemo/src/QMapControl/LayerGeometry.cpp \
    QMapDemo/src/QMapControl/LayerMapAdapter.cpp \
    QMapDemo/src/QMapControl/MapAdapter.cpp \
    QMapDemo/src/QMapControl/MapAdapterBing.cpp \
    QMapDemo/src/QMapControl/MapAdapterGoogle.cpp \
    QMapDemo/src/QMapControl/MapAdapterOSM.cpp \
    QMapDemo/src/QMapControl/MapAdapterOTM.cpp \
    QMapDemo/src/QMapControl/MapAdapterTile.cpp \
    QMapDemo/src/QMapControl/MapAdapterWMS.cpp \
    QMapDemo/src/QMapControl/MapAdapterYahoo.cpp \
    QMapDemo/src/QMapControl/NetworkManager.cpp \
    QMapDemo/src/QMapControl/Projection.cpp \
    QMapDemo/src/QMapControl/ProjectionEquirectangular.cpp \
    QMapDemo/src/QMapControl/ProjectionSphericalMercator.cpp \
    QMapDemo/src/QMapControl/QMapControl.cpp \
    QMapDemo/src/QMapControl/QProgressIndicator.cpp \
    GPSProcess/GPSProcess.cpp \
    IMUProcess/IMUProcess.cpp \
    Weather/mainwindow.cpp \
    Weather/sysTrayIcon.cpp \
    Dashboard/widget.cpp

HEADERS += \
    mainwidget.h \
    UVC/processImage.h \
    UVC/videodevice.h \
    QMapDemo/gps_modul.h \
    QMapDemo/multidemo.h \
    QMapDemo/src/QMapControl/Geometry.h \
    QMapDemo/src/QMapControl/GeometryLineString.h \
    QMapDemo/src/QMapControl/GeometryPoint.h \
    QMapDemo/src/QMapControl/GeometryPointArrow.h \
    QMapDemo/src/QMapControl/GeometryPointCircle.h \
    QMapDemo/src/QMapControl/GeometryPointImage.h \
    QMapDemo/src/QMapControl/GeometryPointImageScaled.h \
    QMapDemo/src/QMapControl/GeometryPointShape.h \
    QMapDemo/src/QMapControl/GeometryPointShapeScaled.h \
    QMapDemo/src/QMapControl/GeometryPolygon.h \
    QMapDemo/src/QMapControl/GeometryPolygonImage.h \
    QMapDemo/src/QMapControl/GeometryWidget.h \
    QMapDemo/src/QMapControl/GPS_Position.h \
    QMapDemo/src/QMapControl/ImageManager.h \
    QMapDemo/src/QMapControl/Layer.h \
    QMapDemo/src/QMapControl/LayerGeometry.h \
    QMapDemo/src/QMapControl/LayerMapAdapter.h \
    QMapDemo/src/QMapControl/MapAdapter.h \
    QMapDemo/src/QMapControl/MapAdapterBing.h \
    QMapDemo/src/QMapControl/MapAdapterGoogle.h \
    QMapDemo/src/QMapControl/MapAdapterOSM.h \
    QMapDemo/src/QMapControl/MapAdapterOTM.h \
    QMapDemo/src/QMapControl/MapAdapterTile.h \
    QMapDemo/src/QMapControl/MapAdapterWMS.h \
    QMapDemo/src/QMapControl/MapAdapterYahoo.h \
    QMapDemo/src/QMapControl/NetworkManager.h \
    QMapDemo/src/QMapControl/Point.h \
    QMapDemo/src/QMapControl/Projection.h \
    QMapDemo/src/QMapControl/ProjectionEquirectangular.h \
    QMapDemo/src/QMapControl/ProjectionSphericalMercator.h \
    QMapDemo/src/QMapControl/QMapControl.h \
    QMapDemo/src/QMapControl/qmapcontrol_global.h \
    QMapDemo/src/QMapControl/QProgressIndicator.h \
    QMapDemo/src/QMapControl/QuadTreeContainer.h \
    IMUProcess/IMUProcess.h \
    GPSProcess/GPSProcess.h \
    Weather/mainwindow.h \
    Weather/sysTrayIcon.h \
    Weather/weatherdata.h \
    Weather/weathertool.h \
    Dashboard/widget.h

FORMS += \
    Weather/mainwindow.ui \
    Dashboard/widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Weather/main.qrc \
    Dashboard/res.qrc
