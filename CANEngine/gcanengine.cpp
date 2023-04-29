#include "gcanengine.h"

void GCanEngine::setErr(const QString &device, const QString &errTip)
{
    err =QString("设备%1:%2").arg(device).arg(errTip);
    emit errString(err);
}

GCanEngine::GCanEngine()
{
    d_data =new GCanEnginePrivate(this); //生成与驱动相关的数据和接收线程
    qRegisterMetaType<canid_t>("canid_t"); //注册canid_t类型到源对象,用于使用信号
}

GCanEngine::~GCanEngine()
{
    delete d_data;
}

const QStringList GCanEngine::getDeviceNames() const
{
    return d_data->deviceNames();
}

int GCanEngine::getChannelCount() const
{
    return d_data->channelCount();
}

const QStringList GCanEngine::getBauds() const
{
    return d_data->bauds();
}

void GCanEngine::setDevice(const QString &deviceName, const QString &channel, const QString &baud)
{
    d_data->deviceName =deviceName;
    d_data->channel =channel;
    d_data->baud =baud;
    d_data->devicType = d_data->gcanDeviceTable.value(deviceName);
}

bool GCanEngine::open()
{
    const QString &deviceName =d_data->deviceName; //设备名
    const QString &baud = d_data->baud; //波特率
    const QString &channel =d_data->channel; //通道
    int deviceType =d_data->devicType.toInt();

    ERR_INFO err;

    if(deviceName.isEmpty() || baud.isEmpty() || channel.isEmpty()){
        setErr("","设备名、波特率、通道未设置");
        return false;
    }

    if(OpenDevice(deviceType,0,0) ==STATUS_ERR){
        QThread::msleep(500);
        if(ReadErrInfo(deviceType,0,channel.toInt(),&err) !=STATUS_ERR){
            setErr(deviceName,QString::number(err.ErrCode,16));
        }
        return false;
    }

    INIT_CONFIG init_config;
    init_config.Filter=0;
    init_config.Mode =0;
    init_config.AccCode=0x000000;//验收码
    init_config.AccMask=0xFFFFFF;//屏蔽码
    QString b = baud;
    b.remove("kbps");
    switch (b.toInt()) {
    case 5:{
        init_config.Timing0 = 0xBF;
        init_config.Timing1 =0xFF;
    }break; //5kbps
    case 10:{
        init_config.Timing0 = 0x31;
        init_config.Timing1 =0x1C;
    }break;
    case 20:{
        init_config.Timing0 = 0x18;
        init_config.Timing1 =0x1C;
    }break;
    case 40:{
        init_config.Timing0 = 0x87;
        init_config.Timing1 =0xFF;
    }break;
    case 50:{
        init_config.Timing0 = 0x09;
        init_config.Timing1 =0x1C;
    }break;
    case 80:{
        init_config.Timing0 = 0x83;
        init_config.Timing1 =0xFF;
    }break;
    case 100:{
        init_config.Timing0 = 0x04;
        init_config.Timing1 =0x1C;
    }break;
    case 125:{
        init_config.Timing0 = 0x03;
        init_config.Timing1 =0x1C;
    }break;
    case 200:{
        init_config.Timing0 = 0x81;
        init_config.Timing1 =0xFA;
    }break;
    case 250:{
        init_config.Timing0 = 0x01;
        init_config.Timing1 =0x1C;
    }break;
    case 400:{
        init_config.Timing0 = 0x80;
        init_config.Timing1 =0xFA;
    }break;
    case 500:{
        init_config.Timing0 = 0x00;
        init_config.Timing1 =0x1C;
    }break;
    case 666:{
        init_config.Timing0 = 0x80;
        init_config.Timing1 =0xB6;
    }break;
    case 800:{
        init_config.Timing0 = 0x00;
        init_config.Timing1 =0x16;
    }break;
    case 1000:{
        init_config.Timing0 = 0x00;
        init_config.Timing1 =0x14;
    }break;
    }

    if(InitCAN(deviceType,0,channel.toInt(),&init_config) ==STATUS_ERR){
        setErr(deviceName,"初始化失败");
        close();
        return false;
    }


    if(StartCAN(deviceType,0,channel.toInt()) == STATUS_ERR){
        setErr(deviceName,"打开失败");
        close();
        return false;
    }

    d_data->start();
    return true;
}

bool GCanEngine::close()
{
    if(!CloseDevice(d_data->devicType.toInt(),0)){
        setErr(d_data->deviceName,"关闭错误");
        return false;
    }
    return true;
}

void GCanEngine::send(canid_t id, int id_type, const canMsg &msg)
{
    int channel =d_data->channel.toInt(); //通道
    int deviceType =d_data->devicType.toInt();

    CAN_OBJ data;
    ZeroMemory(&data,sizeof(data));
    data.ID = id;
    data.SendType =0;
    data.RemoteFlag =0;
    data.ExternFlag =id_type;
    data.DataLen =msg.dlc;
    memcpy((char*)data.Data,(char*)msg.data,sizeof (msg.data));

    int ret =Transmit(deviceType,0,channel,&data,1);

    qDebug() << data.Data[0] << "---" <<ret;


    if(ret >0){
        canMsg msg_t =msg;
        msg_t.channal =d_data->channel;
        emit readyRead(id,msg_t);
    }
    else{
        //setErr(d_data->deviceName,QString("帧0x%1发送失败").arg(QString::number(id,16)));
        return;
    }
}
//!
//! \brief 数据接收
//!
void GCanEnginePrivate::run()
{
    int type =devicType.toInt();
    int cnt =channel.toInt();
    while(true){

        bool ok;
        ERR_INFO err;
        const int d_size =100;
        CAN_OBJ cdata[d_size] ={};

        int count = GetReceiveNum(type,0,cnt);

        while(count >0){
            uint rcount = Receive(type,0,cnt,cdata,d_size,10);
            if(rcount == 0xFFFFFFFF){
                if(ReadErrInfo(type,0,cnt,&err) != STATUS_ERR)
                {
                    p_ptr->setErr(deviceName,QString::number(err.ErrCode,16));
                }
            }

            if(rcount >0){
                canMsg msg;
                msg.timestamp =QDateTime::currentDateTime();//时间戳
                msg.channal =channel;//通道
                msg.hz =cdata->TimeStamp;//频率
                msg.type ="CAN";//类型
                msg.orientation ="Rx";
                msg.dlc =cdata->DataLen;
                msg.data = new uchar[sizeof(cdata->Data)];
                memcpy((char*)(msg.data),(char*)cdata->Data,sizeof(cdata->Data));

                emit p_ptr->readyRead(cdata->ID,msg);
            }
            count--;
        }
//        msleep(20);
    }
}

GCanEnginePrivate::GCanEnginePrivate(GCanEngine *d)
    :p_ptr(d)
{

    gcanDeviceTable ={
        {"GCAN_USBCAN1",USBCAN1},
        {"GCAN_USBCAN2",USBCAN2}
    };

    gcanBaudTable ={
        "250kbps",
        "125kbps",
        "1000kbps",
        "800kbps",
        "500kbps",
        "100kbps",
        "50kbps",
        "20kbps",
        "10kbps"

    };

    channalCount =1;
}

GCanEnginePrivate::~GCanEnginePrivate()
{

}
