
#include "zlgengine.h"
#include <QStringList>
#include <QDebug>
#define Key(x) #x



ZLGEngine::ZLGEngine()
    :CANEngine()
{
    d_data =new ZLGEnginePrivate(this); //生成与驱动相关的数据和接收线程
    qRegisterMetaType<canid_t>("canid_t"); //注册canid_t类型到源对象,用于使用信号
}

ZLGEngine::~ZLGEngine()
{
    delete d_data;
}

const QStringList ZLGEngine::getBauds() const
{
    return  d_data->bauds();
}

const QStringList ZLGEngine::getDeviceNames() const
{
    return d_data->deviceNames();
}

int ZLGEngine::getChannelCount() const
{
    return d_data->channelCount();
}
//!
//! \brief ZLGEngine::setErr
//! \param 驱动名称
//! \param 错误信息
//!
void ZLGEngine::setErr(const QString &device, const QString &errTip)
{
    err =QString("设备%1:%2").arg(device).arg(errTip);
    emit errString(err);
}
//!
//! \brief ZLGEngine::open
//! \return 打开驱动设备，成功返回true，不成功会发送错误码信号
//!
bool ZLGEngine::open()
{
    const QString &deviceName =d_data->deviceName; //设备名
    const QString &baud = d_data->baud; //波特率
    const QString &channel =d_data->channel; //通道
    auto &frame =d_data->frame;//zcan_frame结构体用于连接can盒

    if(deviceName.isEmpty() || baud.isEmpty() || channel.isEmpty()){
        setErr("","设备名、波特率、通道未设置");
        return false;
    }

    /*打开设备*/
    frame.dHandle = ZCAN_OpenDevice(d_data->ZlgDeviceTable[deviceName],0,0);

    if(frame.dHandle == INVALID_DEVICE_HANDLE){
        setErr(deviceName,"打开失败");
        return false;
    }

    /*配置波特率*/
    frame.property =GetIProperty(frame.dHandle);

    if(frame.property ==nullptr){
        setErr(deviceName,"波特率获取失败");
        return false;
    }


    if(frame.property->SetValue(QString("%1/baud_rate").arg(channel).toStdString().data(),
                                d_data->ZlgBaudTable[baud].toStdString().data())  != STATUS_OK){
        setErr(deviceName,QString("通道%1波特率%2设置失败").arg(channel).arg(baud));
        return false;
    }

    /*初始化can配置*/
    memset(&frame.cfg,0,sizeof (frame.cfg));
    frame.cfg.can_type =TYPE_CAN;
    frame.cfg.can.filter=0;               //0 双滤波 1 单滤波
    frame.cfg.can.mode =0;               //0 正常模式、1 只听模式
    frame.cfg.can.acc_code =0;           //SJA1000的帧过滤验收码，对经过屏蔽码过滤为“有关位”进行匹配，全部匹配成功后，此报文可以被接收，否则不接收。推荐设置为0。
    frame.cfg.can.acc_mask =0xffffffff; //SJA1000的帧过滤屏蔽码，对接收的CAN帧ID进行过滤，位为0的是“有关位”，位为1的是“无关位”。推荐设置为0xFFFFFFFF，即全部接收。

    /*初始化cfg*/
    frame.channelHandle =ZCAN_InitCAN(frame.dHandle,channel.toUInt(),&frame.cfg);

    if(INVALID_CHANNEL_HANDLE == frame.channelHandle){
        setErr(deviceName,QString("通道%1初始化失败").arg(channel));
        return false;
    }

    /*启动通道*/
    if(ZCAN_StartCAN(frame.channelHandle) != STATUS_OK){
        setErr(deviceName,QString("通道%1启动失败").arg(channel));
        return false;
    }

    /*开启can消息接收线程*/
    d_data->start();

    return true;
}
//!
//! \brief ZLGEngine::setDevice
//! \param 设备名称
//! \param 通道
//! \param 波特率
//!
void ZLGEngine::setDevice(const QString &deviceName, const QString &channel, const QString &baud)
{
    d_data->deviceName =deviceName;
    d_data->channel =channel;
    d_data->baud =baud;
}
//!
//! \brief 发送数据到can盒
//! \param 帧id
//! \param 帧类型
//! \param 帧内数据
//!
void ZLGEngine::send(canid_t id, int id_type, const canMsg &msg)
{
    ZCAN_Transmit_Data zcan_data;
    memset(&zcan_data, 0, sizeof(zcan_data));
    //第31位(最高位)代表扩展帧标志，=0表示标准帧，=1代表扩展帧，宏IS_EFF可获取该标志；
    //第30位代表远程帧标志，=0表示数据帧，=1表示远程帧，宏IS_RTR可获取该标志；
    //第29位代表错误帧标准，=0表示CAN帧，=1表示错误帧，目前只能设置为0；
   zcan_data.frame.can_id = MAKE_CAN_ID(id,id_type,0,0);
   zcan_data.frame.can_dlc =msg.dlc;
   zcan_data.transmit_type =(msg.type =="CAN" ? 0:1);
   memcpy(zcan_data.frame.data, &msg.data, sizeof(msg.data));

   //发送数据函数
//   if(ZCAN_Transmit(d_data->frame.channelHandle,&zcan_data,1) !=1){
//       setErr(d_data->deviceName,QString("帧0x%1发送失败").arg(QString::number(id,16)));
//       return;
//   }


   canMsg msg_t =msg;
   msg_t.channal =d_data->channel;

   //发送数据到model中
   emit readyRead(id,msg_t);
}
//!
//! \brief 关闭驱动设备
//! \return 成功关闭返回true
//!
bool ZLGEngine::close()
{
    /*释放波特率*/
    if(d_data->frame.property != nullptr){
        if(ReleaseIProperty(d_data->frame.property) !=STATUS_OK)
        {
            setErr(d_data->deviceName,"波特率释放失败");
            return false;
        }
    }

    /*关闭设备*/
    if(ZCAN_CloseDevice(d_data->frame.dHandle) != STATUS_OK){
        setErr(d_data->deviceName,"关闭设备失败");
        return false;
    }

    return true;
}

//!
//! \brief can数据接收线程
//!
void ZLGEnginePrivate::run()
{
    while(true){
        //zlgcan数据结构体
        ZCAN_Receive_Data data[100] ={};
        int count =ZCAN_GetReceiveNum(frame.channelHandle,0); //判断can盒中是否有数据
        while (count >0) {
            int rcount = ZCAN_Receive(frame.channelHandle, data, 100, 10);//通道句柄，数据，数据长度。缓存
            if(rcount >0){
                canMsg msg;
                msg.timestamp =QDateTime::currentDateTime();//时间戳
                msg.channal =channel;//通道
                msg.hz =-1;//频率
                msg.type =frame.cfg.can_type ==0?"CAN":"CANFD";//类型
                msg.orientation ="Rx";
                msg.dlc =data->frame.can_dlc;
                msg.data = new uchar[sizeof(data->frame.data)];
                memcpy((char*)msg.data,(char*)data->frame.data,sizeof(data->frame.data));

                emit p_ptr->readyRead(data->frame.can_id,msg);
            }
            count --;
        }
        msleep(20);
    }
}
//!
//! \brief 初始化设备索引表、波特率索引表，通道总数
//! \param d
//!
ZLGEnginePrivate::ZLGEnginePrivate(ZLGEngine *d)
    :p_ptr(d)
{
    /*设备支持索引表*/
    ZlgDeviceTable ={
        {Key(ZCAN_PCI5121),ZCAN_PCI5121},
        {Key(ZCAN_PCI9810),ZCAN_PCI9810},
        {Key(ZCAN_USBCAN1),ZCAN_USBCAN1},
        {Key(ZCAN_USBCAN2),ZCAN_USBCAN2},
        {Key(ZCAN_PCI9820),ZCAN_PCI9820},
        {Key(ZCAN_CAN232),ZCAN_CAN232},
        {Key(ZCAN_PCI5110),ZCAN_PCI5110},
        {Key(ZCAN_CANLITE),ZCAN_CANLITE},
        {Key(ZCAN_ISA9620),ZCAN_ISA9620},
        {Key(ZCAN_ISA5420),ZCAN_ISA5420},
        {Key(ZCAN_PC104CAN),ZCAN_PC104CAN},
        {Key(ZCAN_CANETUDP),ZCAN_CANETUDP},
        {Key(ZCAN_CANETE),ZCAN_CANETE},
        {Key(ZCAN_DNP9810),ZCAN_DNP9810},
        {Key(ZCAN_PCI9840),ZCAN_PCI9840},
        {Key(ZCAN_PC104CAN2),ZCAN_PC104CAN2},
        {Key(ZCAN_PCI9820I),ZCAN_PCI9820I},
        {Key(ZCAN_CANETTCP),ZCAN_CANETTCP},
        {Key(ZCAN_PCIE_9220),ZCAN_PCIE_9220},
        {Key(ZCAN_PCI5010U),ZCAN_PCI5010U},
        {Key(ZCAN_USBCAN_E_U),ZCAN_USBCAN_E_U},
        {Key(ZCAN_USBCAN_2E_U),ZCAN_USBCAN_2E_U},
        {Key(ZCAN_PCI5020U),ZCAN_PCI5020U},
        {Key(ZCAN_EG20T_CAN),ZCAN_EG20T_CAN},
        {Key(ZCAN_PCIE9221),ZCAN_PCIE9221},
        {Key(ZCAN_WIFICAN_TCP),ZCAN_WIFICAN_TCP},
        {Key(ZCAN_WIFICAN_UDP),ZCAN_WIFICAN_UDP},
        {Key(ZCAN_PCIe9120),ZCAN_PCIe9120},
        {Key(ZCAN_PCIe9110),ZCAN_PCIe9110},
        {Key(ZCAN_PCIe9140),ZCAN_PCIe9140},
        {Key(ZCAN_USBCAN_4E_U),ZCAN_USBCAN_4E_U},
        {Key(ZCAN_CANDTU_200UR),ZCAN_CANDTU_200UR},
        {Key(ZCAN_CANDTU_MINI),ZCAN_CANDTU_MINI},
        {Key(ZCAN_USBCAN_8E_U),ZCAN_USBCAN_8E_U},
        {Key(ZCAN_CANREPLAY),ZCAN_CANREPLAY},
        {Key(ZCAN_CANDTU_NET),ZCAN_CANDTU_NET},
        {Key(ZCAN_CANDTU_100UR),ZCAN_CANDTU_100UR},
        {Key(ZCAN_PCIE_CANFD_100U),ZCAN_PCIE_CANFD_100U},
        {Key(ZCAN_PCIE_CANFD_200U),ZCAN_PCIE_CANFD_200U},
        {Key(ZCAN_PCIE_CANFD_400U),ZCAN_PCIE_CANFD_400U},
        {Key(ZCAN_USBCANFD_200U),ZCAN_USBCANFD_200U},
        {Key(ZCAN_USBCANFD_100U),ZCAN_USBCANFD_100U},
        {Key(ZCAN_USBCANFD_MINI),ZCAN_USBCANFD_MINI},
        {Key(ZCAN_CANFDCOM_100IE),ZCAN_CANFDCOM_100IE},
        {Key(ZCAN_CANSCOPE),ZCAN_CANSCOPE},
        {Key(ZCAN_CLOUD),ZCAN_CLOUD},
        {Key(ZCAN_CANDTU_NET_400),ZCAN_CANDTU_NET_400},
        {Key(ZCAN_CANFDNET_TCP),ZCAN_CANFDNET_TCP},
        {Key(ZCAN_CANFDNET_UDP),ZCAN_CANFDNET_UDP},
        {Key(ZCAN_CANFDWIFI_TCP),ZCAN_CANFDWIFI_TCP},
        {Key(ZCAN_CANFDWIFI_UDP),ZCAN_CANFDWIFI_TCP}
    };
    /*波特率索引表*/
    ZlgBaudTable ={
        {Key(250kbps),Key(250000)},
        {Key(125kbps),Key(125000)},
        {Key(1000kbps),Key(1000000)},
        {Key(800kbps),Key(800000)},
        {Key(500kbps),Key(500000)},
        {Key(100kbps),Key(100000)},
        {Key(50kbps),Key(50000)},
        {Key(20kbps),Key(20000)},
        {Key(10kbps),Key(10000)},

    };

    //通道总数
    channalCount =4;
}

ZLGEnginePrivate::~ZLGEnginePrivate()
{
    if(frame.dHandle && frame.property){
        p_ptr->close();

    }
}





