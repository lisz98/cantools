/**
 1. @file:       controllistwidget.h
 2. @Author:     Lsz.
 3. @Date:       2022-05-06
 4. @Brief:      File Description
 **/

#ifndef CONTROLLISTWIDGET_H
#define CONTROLLISTWIDGET_H

#include <QWidget>
#include <QMap>
#include <QTableWidget>
#include "controlform.h"
#include "parsingcan.h"
class ControlListWidget :public QWidget
{
    Q_OBJECT
public:
    ControlListWidget(QMap<QString,ParsingCan*> pads, int type,CANEngine *engine,QWidget *widget =nullptr);
    ~ControlListWidget();

    //根据pads索引表生成tab控件列表和sub控件列表
    void init();
signals:
    //将控件数据解析后，按照型号，tab，参数名字，修改数据发送信号
    void dataChange(int type,QString tableName,QString paramaterName,int value);
    //void dataChange(QString tableName,QString paramaterName,int value);

public slots:
    //获取子控件列表中数据，并进行解析
    void dataChanged(int value);
    //启动解析
    void start();
    //停止解析
    void stop();
    //按照动作创建解析列表
    void createControlByAction(const QString &actionTitle,const QStringList &actionName);


private:

    typedef QMap<QString,ControlForm*> subControls; //子控件列表
    QMap<QString,subControls> tabControls;//控件列表
    QMap<QString,ParsingCan*> pads; //数据包
    int type;

    CANEngine *engine =nullptr; //can驱动设备
    ParsingThread *parsingThread =nullptr;//解析线程
    QTabWidget *tab =nullptr;

};

#endif // CONTROLLISTWIDGET_H
