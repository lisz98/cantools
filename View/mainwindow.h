/**
 1. @file:       mainwindow.h
 2. @Author:     Lsz.
 3. @Date:       2022-05-06
 4. @Brief:      File Description
 **/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "zlgengine.h"
#include "zlgcantablemodel.h"
#include "controlform.h"
#include "simulationcan.h"
#include <QMessageBox>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //初始化can驱动、表格模型、模拟can数据包
    void init();

private slots:
    //打开CAN设备
    void on_clicked();
    //生成控件列表，必须打开成功can设备才能使用
    void on_pushButton_2_clicked();
    //can连接错误码
    void err(const QString &err);


private:
    Ui::MainWindow *ui;
    CANEngine *engine; //can设备，这里使用的是zlgcan的驱动
    ZlgCanTableModel *model ;//can数据表格模型
    simulationCan *simulationCana;//模拟can数据
    QMessageBox box;//错误信息提示框

};
#endif // MAINWINDOW_H
