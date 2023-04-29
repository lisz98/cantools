#ifndef APPDATA_H
#define APPDATA_H

#include <QString>
#include <QApplication>
#include <QIcon>
#include <QFile>
namespace AppData {
    QString version ="v1.00.01";
    QString appName ="驾驶教练评估系统测试软件";
    QString iconPath ="";
    QString organizationName ="上海通用卫星导航有限公司";
    QString StyleSheetPath =":/style.qss";

    inline static void loadingSetting(){
        if(qApp){
            qApp->setOrganizationName(organizationName);
            qApp->setQuitOnLastWindowClosed(true);
            qApp->setApplicationVersion(version);
            qApp->setApplicationDisplayName(appName);
            qApp->setWindowIcon(QIcon(iconPath));
            qApp->setDesktopFileName(appName);

            QFile qss(StyleSheetPath);
            if(qss.open(QIODevice::ReadOnly)){
                qApp->setStyleSheet(qss.readAll());
            }
        }
    }
}

#endif // APPDATA_H
