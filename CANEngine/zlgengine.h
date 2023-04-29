/**
 1. @file:       zlgengine.h
 2. @Author:     Lsz.
 3. @Date:       2022-05-06
 4. @Brief:      File Description
 **/

#ifndef ZLGENGINE_H
#define ZLGENGINE_H

#include "CANEngine.h"

//周立功第三方库
#ifdef __cplusplus
extern "C"
{
#endif
#include <3rdparty/zlgcan_x64/canframe.h>
#include <3rdparty/zlgcan_x64/config.h>
#include <3rdparty/zlgcan_x64/typedef.h>
#include <3rdparty/zlgcan_x64/zlgcan.h>
#ifdef __cplusplus
}
#endif
#include <QMap>
#include <QDebug>
#include <QDateTime>
#include <QThread>
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

class ZLGEnginePrivate;
class ZLGEngine :public CANEngine
{
    Q_OBJECT
private:
    //设置错误码
    void setErr(const QString &device,const QString &errTip);

public:
    ZLGEngine();
    ~ZLGEngine();

    //获取支持的驱动列表，返回ZLGEnginePrivate 成员函数ZlgDeviceTable
    const QStringList getDeviceNames() const override;
    //获取通道总数，返回ZLGEnginePrivate 成员函数channelCount
    int getChannelCount()const override;
    //获取支持波特率列表,返回ZLGEnginePrivate 成员函数ZlgBaudTable
    const QStringList getBauds() const override;
    //设置设备
    void setDevice(const QString &deviceName,const QString &channel,const QString &baud) override;
    //打开设备
    bool open() override;
    //关闭设备
    bool close() override;
    //发送数据
    void send(canid_t id,int id_type,const canMsg &msg) override;

    ZLGEnginePrivate *d_data;  //存储了zlgCan的详细信息
    friend class ZLGEnginePrivate;
};

class ZLGEnginePrivate : public QThread
{
    Q_OBJECT
private:
    //zlgCan盒设备启动关闭结构体，详细看二次开发文档
    struct ZLG_CAN_Frame{
        DEVICE_HANDLE dHandle;
        IProperty *property;//波特率
        ZCAN_CHANNEL_INIT_CONFIG cfg;//can配置
        CHANNEL_HANDLE channelHandle;//通道句柄
        ZLG_CAN_Frame(){
            dHandle =0;
            property =0;
            memset((char*)&cfg,0,sizeof (cfg));
            channelHandle =0;
        }
    };
    //启动设备前需要的信息
    ZLG_CAN_Frame frame;//连接zlgcan盒子使用
    QMap<QString,UINT> ZlgDeviceTable; //zlg驱动索引表
    QMap<QString,QString> ZlgBaudTable;//波特率索引表
    int channalCount;//通道总数

    //启动设备后配置的信息
    QString deviceName;//设备名字
    QString baud;//波特率
    QString channel;//当前通道
    ZLGEngine *p_ptr;

    //用于接收数据线程
    void run() override;

    ZLGEnginePrivate(ZLGEngine *d);
    ~ZLGEnginePrivate();

    const QStringList deviceNames() const{ return ZlgDeviceTable.keys();}
    const QStringList bauds() const{ return ZlgBaudTable.keys() ;}
    int channelCount() const{return channalCount;}
    friend class ZLGEngine;
};



#endif // ZLGENGINE_H
