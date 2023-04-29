
**Can驱动类**
------------------------------------------------


* [zlgengine.cpp](zlgengine.cpp)

1. 构造函数

```cpp
ZLGEngine::ZLGEngine()
    :CANEngine()
{
    d_data =new ZLGEnginePrivate(this); //生成与驱动相关的数据和接收线程
    qRegisterMetaType<canid_t>("canid_t"); //注册canid_t类型到源对象,用于使用信号
}
```

2. 配置设备信息

```cpp
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
```

3. 打开设备

```cpp
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
```

4. 接收数据

```cpp
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
                memset((char*)msg.data,0,sizeof(data->frame.data));
                memcpy((char*)&msg.data,(char*)data->frame.data,sizeof(data->frame.data));

                emit p_ptr->readyRead(data->frame.can_id,msg);
            }
            count --;
        }
        msleep(20);
    }
}
```

5. 发送数据

```cpp
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
   if(ZCAN_Transmit(d_data->frame.channelHandle,&zcan_data,1) !=1){
       setErr(d_data->deviceName,QString("帧0x%1发送失败").arg(QString::number(id,16)));
       return;
   }
   
   
   canMsg msg_t =msg;
   msg_t.channal =d_data->channel;

   //发送数据到model中
   emit readyRead(id,msg_t);
}
```

6. 关闭驱动设备

```cpp
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
```

7. 初始化设备索引表、波特率索引表，通道总数

```cpp
//!
//! \brief 初始化设备索引表、波特率索引表，通道总数
//! \param d
//!
#define Key(x) #x
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
```

* [parsingcan.cpp](parsingcan.cpp)

1. 根据参数名称动态调用函数指针索引表修改参数数值

```cpp
//!
//! \brief 根据参数名称动态调用函数指针索引表修改参数数值
//! \param 参数
//! \param 修改值
//!
void ParsingCan::setParamaterData(const QString &paramater, int value)
{
    //异常处理
    if(paramater.isEmpty() || paramaterFunction.isEmpty()){
        return;
    }

    if(value < 0 || value > paramaters[paramater]){
        return;
    }

    //调用paramaterFunction函数指针索引表中函数，需要自己实现函数
    (*paramaterFunction[paramater])(data,value);
}
```

2. can数据解析线程

```cpp
//!
//! \brief can数据解析线程
//! \param 解析包
//! \param can驱动
//!
ParsingThread::ParsingThread(QMap<QString, ParsingCan *> &pads, CANEngine *engine)
    :pads(pads),engine(engine)
{
    //记录数据包频率
    for(auto pad :pads){
        packFs.insert(pad->name,pad->msg.hz);
    }
}
```
3. can数据解析，根据数据包不同的频率，来进行发送数据，模拟数据频率

```cpp
//!
//! \brief can数据解析，根据数据包不同的频率，来进行发送数据，模拟数据频率
//!
void ParsingThread::run()
{

    while(true){
        //从数据包中获取数据列表
        auto datas =pads.values();

        //按照Interval对数据频率进行更新检查，如果到达数据频率则发送数据
        for(int i=0;i <datas.size();++i){
            auto data =datas[i];
            int &hz = data->msg.hz;
            hz -= Interval;
            if(hz <=0){
                hz = packFs[data->name];
                engine->send(data->id,data->type,data->msg);
            }
        }
        msleep(Interval);
    }
}
```


* [zlgcantablemodel.cpp](zlgcantablemodel.cpp)

1. 构造函数，初始化表头名称

```cpp
//!
//! \brief 构造函数，初始化表头名称
//! \param parent
//!
ZlgCanTableModel::ZlgCanTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    headers << "帧id"<<"时间戳"<<"通道"<<"频率" <<"数量"<<"类型"<<"长度" <<"方向" <<"报文";

}
```

2. 设置表格数据

```cpp
//!
//! \brief 设置表格数据
//! \param index
//! \param role
//! \return
//!
QVariant ZlgCanTableModel::data(const QModelIndex &index, int role) const
{
    int col =index.column();
    int row =index.row();
    if(col <0 | col > columnCount() | row <0 | row >rowCount()){
        return QVariant();
    }

    if(role ==Qt::DisplayRole){

        auto canId_t =can_data.keys()[row]; //从存储了收发的数据的索引表中获取canid

        switch (col) {
        case 0: return  QString("0x%1").arg(canId_t,0,16).toUpper();  break; //帧id
        case 1: return can_data[canId_t].back().timestamp; break;//时间戳
        case 2: return can_data[canId_t].back().channal; break;//通道
        case 3: return can_data[canId_t].back().hz == -1
                    ? can_data[canId_t].back().timestamp.msecsTo((can_data[canId_t].rbegin()+1)->timestamp): can_data[canId_t].back().hz; break;//频率
        case 4: return can_data[canId_t].size(); break;//数量
        case 5: return can_data[canId_t].back().type; break;//类型
        case 6: return can_data[canId_t].back().dlc; break;//长度
        case 7: return can_data[canId_t].back().orientation; break;//方向
        case 8: return byteToHexString(can_data[canId_t].back().data,can_data[canId_t].back().dlc); break;//报文
        }
    }

    return QVariant();
}
```

3. 唯一对外接口，通过这个来设置数据

```cpp
//!
//! \brief 唯一对外接口，通过这个来设置数据
//! \param id
//! \param msg
//!
void ZlgCanTableModel::insertRow(canid_t id,const canMsg &msg)
{
    beginResetModel();
    can_data[id].push_back(msg);
    endResetModel();
}
```

4. 辅助函数，用于将BYTE类型转换成QString类型显示

```cpp
//!
//! \brief 辅助函数，用于将BYTE类型转换成QString类型显示
//! \param byte
//! \param 记录byte的数据长度,也可使用(sizeof(byte) /sizeof(byte[0]))计算
//! \return
//!
QVariant ZlgCanTableModel::byteToHexString(BYTE *byte,int length) const
{
    if(!byte){
        return QVariant();
    }
    QString data;
    for(int i=0;i <length/*(sizeof(byte) /sizeof(byte[0]))*/;i++){
        if(byte[i] <=16){
            data +="0";
        }
        data += QString("%1-").arg(byte[i],0,16).toUpper();
    }
    qDebug() << data;

    return data;
}
```