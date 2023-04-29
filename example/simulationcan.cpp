#include "simulationcan.h"

simulationCan::simulationCan(CANEngine *engine, QObject *parent)
    :engine(engine),QObject(parent)
{
    init();
}

simulationCan::~simulationCan()
{
    if(!pads.isEmpty()){
        foreach(auto pad,pads){
            delete pad;
            pad =nullptr;
        }
        pads.clear();
    }
}

void simulationCan::init()
{
    if(true)
    {
        ParsingCan *pad =new ParsingCan;
        CMD_XXXXX *cmd =new CMD_XXXXX;
        memset((char*)cmd,0,sizeof (cmd));
        pad->msg.data =(BYTE*)cmd;   //用于发送数据，这个函数指针是与cmd共享的，指向CMD结构体
        pad->name = "演示";
        pad->id = 0x123; //id 为0x123
        pad->type = 0; //配置为普通帧
        pad->paramaters ={ //记录相应的参数的最大值，例如uchar smxh:8 则MaxHelper(8) 返回255，即最大值为0-255
            {"a",MaxHelper(8)},
            {"b",MaxHelper(4)},
            {"c",MaxHelper(1)},
            {"h",MaxHelper(16)}
        };
     //使用空基类的方式实现数据动态绑定
#if UseParsingBaseNotUnion
        pad->data = reinterpret_cast<ParsingBase*>(cmd); //记录数据，数据在这里进行改动，因为pad->msg.data中指针指向cmd，所以pad->data和pad->msg.data指向同一块内存地址
        pad->paramaterFunction ={//关键操作,这里的函数指针提供给解析类，必须实现
            {"a",[](ParsingBase *d,int value){ reinterpret_cast<CMD_XXXXX*>(d)->a = value; }}, //参数名：发动机油压，函数指针：实现发布信息到发动机油压时调用此函数对数值进行修改
            {"b",[](ParsingBase *d,int value){ reinterpret_cast<CMD_XXXXX*>(d)->b = value;}},
            {"c",[](ParsingBase *d,int value){ reinterpret_cast<CMD_XXXXX*>(d)->c = value;}},
            {"h",[](ParsingBase *d,int value){ reinterpret_cast<CMD_XXXXX*>(d)->h = value;}}
        };
#endif
        pad->msg.hz =20;//频率
        pad->msg.type ="CAN";//can类型
        pad->msg.dlc =8;//can长度
        pad->msg.orientation = "Tx";//Tx为发送方向
        pad->msg.timestamp = QDateTime::currentDateTime(); //获取当前时间戳
        pads.insert(pad->name,pad);//关键操作，将数据添加到解析列表中
    }
}

void simulationCan::strat()
{
    if(!control){
        control = new ControlListWidget(pads,0,engine);
        connect(control,&ControlListWidget::dataChange,this,&simulationCan::dataChanged);

    }
    control->start();
    control->show();
}

void simulationCan::stop()
{
    if(control){
        control->stop();
        disconnect(control,&ControlListWidget::dataChange,this,&simulationCan::dataChanged);
        delete  control;
        control =nullptr;
    }
}

void simulationCan::dataChanged(int type, QString tableName, QString paramaterName, int value)
{
    if(type ==1 || tableName.isEmpty() || paramaterName.isEmpty()){
        return;
    }

    //调用相应的数据包中的线程解析函数，对数据进行动态修改
    pads[tableName]->setParamaterData(paramaterName,value);
}
