#ifndef CANENGINE_H
#define CANENGINE_H
/**
 1. @file:       CANEngine.h
 2. @Author:     Lsz.
 3. @Date:       2022-05-06
 4. @Brief:      File Description
 **/

#include <QString>
#include <QObject>
#include <QDateTime>

/*
* Controller Area Network Identifier structure
*
* bit 0-28	: CAN identifier (11/29 bit)
* bit 29	: error message frame flag (0 = data frame, 1 = error message)
* bit 30	: remote transmission request flag (1 = rtr frame)
* bit 31	: frame format flag (0 = standard 11 bit, 1 = extended 29 bit)
*/

//!
//! \brief can数据结构体，用于发送和接收can数据
//!
typedef uint canid_t;
struct alignas(8) canMsg{
    QDateTime timestamp;//时间标识
    QString channal;//通道
    int hz;//频率
    QString type;//类型
    QString orientation;//收发方向 Tx Rx
    uchar dlc;//数据长度 8 16
    uchar *data;//报文信息

};

class CANEngine : public QObject
{
    Q_OBJECT
protected:
    QString err; //错误码
signals:
    //发送错误码
    void errString(const QString &err);
    //发送接收的数据
    void readyRead(canid_t id, const canMsg &msg);
public:
    virtual ~CANEngine() {};
    //获取支持的设备列表
    virtual const QStringList getDeviceNames() const =0 ;
    //获取can盒通道总数
    virtual int getChannelCount()const =0;
    //获取波特率列表
    virtual const QStringList getBauds() const =0;
    //设置设备，需要先设置设备才能打开设备
    virtual void setDevice(const QString &deviceName, const QString &channel, const QString &baud) =0;
    //打开设备
    virtual bool open() =0;
    //关闭设备
    virtual bool close() =0;
    //发送数据
    virtual void send(canid_t id,int id_type,const canMsg &msg) =0;
    //获取错误码
    const QString & getErr(){ return err;}

    CANEngine() :err(""){}

};



#endif // CANENGINE_H
