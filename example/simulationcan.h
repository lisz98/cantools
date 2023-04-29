#ifndef SIMULATIONCAN_H
#define SIMULATIONCAN_H

#include "controllistwidget.h"
#include <QVector>
#include "can.h"

class simulationCan : public QObject
{
    Q_OBJECT
public:
    simulationCan(CANEngine *engine ,QObject *parent =nullptr);
    ~simulationCan();

    //初始化数据解析包、数据解析函数指针、生成控件列表解析
    void init();
public slots:
    //开始解析
    void strat();
    //停止解析
    void stop();
    //在这进行数据操作,返回的是控件修改的数据
    void dataChanged(int type,QString tableName,QString paramaterName,int value);

private:
    ControlListWidget *control =nullptr; //控件列表
    QMap<QString,ParsingCan*> pads;//数据包索引
    CANEngine *engine =nullptr;//驱动
};

#endif // SIMULATIONCAN_H
