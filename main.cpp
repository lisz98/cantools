/******************************************************************************
 * Copyright 2022 xxx Co., Ltd.
 * All right reserved. See COPYRIGHT for detailed Information.
 *
 * @file       main.cpp
 * @brief      CAN盒测试程序
 *
 * @author     lsz
 * @date       2022/05/06
 * @history
 *****************************************************************************/
#include "mainwindow.h"

#include <QApplication>

#include "CANEngine/zlgengine.h"
#include "common/appdata.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(images);
    Q_INIT_RESOURCE(qss);
    QApplication a(argc, argv);

    AppData::loadingSetting();

    MainWindow w;
    w.show();
    return a.exec();
}
