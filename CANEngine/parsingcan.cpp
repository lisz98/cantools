#include "parsingcan.h"


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

ParsingCan::ParsingCan()
{

}

ParsingCan::~ParsingCan()
{
    if(msg.data){
        delete msg.data;
        msg.data =0;
    }
}

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
