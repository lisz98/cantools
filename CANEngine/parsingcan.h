/**
 1. @file:       parsingcan.h
 2. @Author:     Lsz.
 3. @Date:       2022-05-06
 4. @Brief:      File Description
 **/
#ifndef PARSINGCAN_H
#define PARSINGCAN_H
#include <QObject>
#include<QDateTime>

#include "zlgengine.h"

#include <QTimer>
#include <QMap>
#include <QtMath>


#ifndef UseParsingBaseNotUnion
#define UseParsingBaseNotUnion 1
#endif

#if UseParsingBaseNotUnion
//空基类，用于解析数据
struct ParsingBase{

};
#else

#endif

//线程休眠间隔
#define Interval 10
//参数最大值计算帮助助手
#define MaxHelper(x) qPow(2,x) -1
#if 0
//参数值拷贝 弃用
#define paramaterHelper(_Dst,_Src,paramaterName,value)                      \
{                                                                           \
    static_assert(TraitHelper<decltype(_Dst)>::isPointer,"目标地址不是指针"); \
    static_assert(TraitHelper<decltype(_Src)>::isPointer,"源地址不是指针");   \
    _Src->paramaterName =value;                                             \
    memset(_Dst,0,sizeof (_Src));                                           \
    memcpy((char *)_Dst,(char *)_Src,sizeof(_Src));                         \
}                                                                           \
template <typename T>
struct TraitHelper{
    static const bool isPointer =false;
};

template <typename T>
struct TraitHelper<T *>{
    static const bool isPointer =true;
};
#endif

//解析can数据
class ParsingCan :public QObject
{
    Q_OBJECT
public:
    QString name;//帧名称
    canid_t id;//帧id
    int type;//0:普通帧，1:扩展帧
    canMsg msg; //存储can数据发送消息

    QMap<QString,uint> paramaters;//记录帧内数据 名称，最大值 比如 uchar a:1; //最大值为2

    //使用空基类方式实现指针动态绑定，数据结构体需继承自ParsingBase
#if UseParsingBaseNotUnion
    ParsingBase *data; //解析的数据，与msg中的data数据共享指向同一个地址，通过paramaterFunction动态修改data中的数据实现修改msg中data的数据
    QMap<QString,void(*)(ParsingBase*,int)> paramaterFunction;   //不同参数中使用不同的函数进行解析，这里将函数指针保存在函数索引表中，实现动态调用函数
    void setParamaterData(const QString &paramater,int value);   //数据改变调用该函数使用函数索引表名称和改变值对数据进行修改
#else
    QMap<QString,void(*)(ParsingBase,int)> paramaterFunction;   //修改数据
#endif


    ParsingCan();
    ~ParsingCan();
};

class ParsingThread : public QThread
{
    Q_OBJECT
public:
    ParsingThread(QMap<QString,ParsingCan*> &pads,CANEngine *engine);

    void run() override;

    QMap<QString,ParsingCan*> pads; //解析数据包索引表
    CANEngine *engine; //can驱动
    QMap<QString,int> packFs; //记录数据包频率
};


#endif // PARSINGCAN_H
