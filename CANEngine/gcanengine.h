/**
 1. @file:       gcanengine.h
 2. @Author:     Lsz.
 3. @Date:       2022-06-06
 4. @Brief:      广成can盒驱动
 **/

#ifndef GCANENGINE_H
#define GCANENGINE_H
#include "CANEngine.h"

//广成第三方库
#ifdef __cplusplus
extern "C"
{
#endif
#include <3rdparty/ECANVCI_x64/ECanVci.h>
#ifdef __cplusplus
}
#endif
#include <QMap>
#include <QDebug>
#include <QDateTime>
#include <QThread>
class GCanEnginePrivate;
class GCanEngine :public CANEngine
{
    Q_OBJECT
private:
    void setErr(const QString &device,const QString &errTip);
public:
    GCanEngine();
    ~GCanEngine();

    //获取支持的驱动列
    const QStringList getDeviceNames() const override;
    //获取通道总数
    int getChannelCount()const override;
    //获取支持波特率列表
    const QStringList getBauds() const override;
    //设置设备
    void setDevice(const QString &deviceName,const QString &channel,const QString &baud) override;
    //打开设备
    bool open() override;
    //关闭设备
    bool close() override;
    //发送数据
    void send(canid_t id,int id_type,const canMsg &msg) override;

    GCanEnginePrivate *d_data;
    friend class GCanEnginePrivate;
};

class GCanEnginePrivate : public QThread
{
    Q_OBJECT
private:
    QMap<QString,unsigned int> gcanDeviceTable; //索引号
    QList<QString> gcanBaudTable;    //波特率
    int channalCount;

    //设备信息
    QString deviceName;
    QString baud;
    QString channel;
    QString devicType;
    GCanEngine *p_ptr;

protected:
    void run() override;

    GCanEnginePrivate(GCanEngine *d);
    ~GCanEnginePrivate();

    const QStringList deviceNames() const{ return gcanDeviceTable.keys();}
    const QStringList bauds() const{ return gcanBaudTable ;}
    int channelCount() const{return channalCount;}
    friend class GCanEngine;

};

#endif // GCANENGINE_H
