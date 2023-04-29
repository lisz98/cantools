QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp


# 添加子路径
include(./CANEngine/CANEngine.pri)
include(./View/view.pri)
include(./common/common.pri)
include(./example/example.pri)

## 生成文件目录
DESTDIR = $$PWD/bin
MOC_DIR = tmp/moc
RCC_DIR = tmp/rcc
UI_DIR = tmp/ui
OBJECTS_DIR = tmp/obj

TARGET = CanTest



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images/images.qrc \
    qss/qss.qrc

# 周立功第三方库

win32: LIBS += -L$$PWD/3rdparty/zlgcan_x64/ -lzlgcan

INCLUDEPATH += $$PWD/3rdparty/zlgcan_x64
DEPENDPATH += $$PWD/3rdparty/zlgcan_x64

#广成库

win32: LIBS += -L$$PWD/3rdparty/ECANVCI_x64/ -lECanVci64

INCLUDEPATH += $$PWD/3rdparty/ECANVCI_x64
DEPENDPATH += $$PWD/3rdparty/ECANVCI_x64

win32{
    system(windeployqt .\bin\CanTest.exe)
    #zlg库
    system(copy .\3rdparty\zlgcan_x64\zlgcan.lib .\bin\zlgcan.lib)
    system(copy .\3rdparty\zlgcan_x64\zlgcan.dll .\bin\zlgcan.dll)
    system(copy .\3rdparty\zlgcan_x64\zlgcan.dll .\bin\zlgcan.dll)
    #广成库
    system(copy .\3rdparty\ECANVCI_x64\ECanVci64.dll .\bin\ECanVci64.dll)
    system(copy .\3rdparty\ECANVCI_x64\CHUSBDLL64.dll .\bin\CHUSBDLL64.dll)
    system(copy .\3rdparty\ECANVCI_x64\ECanVci64.lib .\bin\ECanVci64.lib)


    system(xcopy .\3rdparty\zlgcan_x64\kerneldlls  .\bin\kerneldlls\ /A /E /Y)

}




INCLUDEPATH += $$PWD/CANEngine
INCLUDEPATH += $$PWD/common
INCLUDEPATH += $$PWD/View
INCLUDEPATH += $$PWD/example








