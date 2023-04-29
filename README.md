

***can盒二次开发程序***

1. 实现自定义结构体数据发送
2. 可自由控制结构体数据数值
3. 可按照位域控制数据
4. 仅支持Win下使用
5. 仅支持几个周立功can盒
6. 新增支持gcan盒

----------------------------------------------------

# 代码目录

- [代码目录](#代码目录)
- [CAN驱动类](#can驱动类)
  - [CAN驱动抽象类](#can驱动抽象类)
  - [ZLGCAN驱动类](#zlgcan驱动类)
  - [解析CAN数据类](#解析can数据类)
- [通用类](#通用类)
  - [App配置类](#app配置类)
- [视图类](#视图类)
  - [CAN数据控件类](#can数据控件类)
  - [CAN数据控件列表类](#can数据控件列表类)
  - [主窗口类](#主窗口类)
  - [CAN数据简单表格模型类](#can数据简单表格模型类)
- [示例](#示例)
  - [CAN数据模拟解析包类](#can数据模拟解析包类)


# CAN驱动类

**用于加载CAN驱动，实现交互, 当前仅用于ZLGcan盒驱动，但是公司CAN_4E_U好像有问题有时候能连接但有时候不能连接**

**具体实现[can驱动实现](CANEngine/README.md)**

## CAN驱动抽象类

1. canMsg : 用于发送和接收can数据的结构体
    1. timestamp:记录can数据时间戳，并没有获取zlgCan盒内部发送的时间戳，而是使用Qt内部时间戳
    2. channal:记录can盒通道
    3. hz:记录发送或接收频率，其中接收频率can盒中没有发出，是自己计算的
    4. type:记录can类型，分为CAN和CANFD,本例子中仅使用了CAN
    5. orientation:记录收发方向 Tx 为发送 ，Rx 为接收
    6. dlc:记录数据长度
    7. data:报文数据

```cpp
typedef uint canid_t;
struct alignas(8) canMsg{
    QDateTime timestamp;//时间标识
    QString channal;//通道
    int hz;//频率
    QString type;//类型
    QString orientation;//收发方向 0:收 1：发
    uchar dlc;//数据长度 8 16
    uchar *data;//报文信息

};
```

2. CANEngine : 用于定义CAN驱动启动、关闭、接收、发送数据、错误码

```cpp
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
```

## ZLGCAN驱动类

1. 周立功第三方库

```cpp
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
```

2. ZLGEngine : 实现CANEngine父类功能，并将数据存储在ZLGEnginePrivate中

```cpp
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
```

2. ZLGEnginePrivate : 存储了zlg驱动设备启动需要的信息，和进行获取连接后的消息

```cpp
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
    const QStringList bauds() const{ return ZlgBaudTable.keys();}
    int channelCount() const{return channalCount;}
    friend class ZLGEngine;
};
```

## 解析CAN数据类

1. 宏
    1. Interval :线程休眠间隔
    2. MaxHelper(x) :参数最大值计算助手，例如 uchar data:4 MaxHelper(1) 返回最大值16 即data的范围为0-16
    3. UseParsingBaseNotUnion : 使用空基类方式或联合体方式来解析数据，建议使用空基类方式
2. 类
    1. ParsingCan :用于解析can数据的类，ParsingThread线程中使用，每个需要解析的包都应该使用它生成

    ```cpp
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
        //使用联合体方式共享指针
        union ParsingBase{

        }data;

        QMap<QString,void(*)(ParsingBase,int)> paramaterFunction;   //修改数据
    #endif


        ParsingCan();
        ~ParsingCan();
    };

    ```

    1. ParsingThread :解析线程，数据的解析在run函数中，并调用CANEngine的send方法发送数据

    ```cpp
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
    ```

# 通用类

**当前仅用于配置App版本、名称等信息**

## App配置类

```cpp
namespace AppData {
    QString version ="v1.00.01";
    QString appName ="canTestTool by:lsz qq:57452466";
    QString iconPath ="";
    QString organizationName ="lsz qq:57452466";
    QString StyleSheetPath =":/style.qss";

    inline static void loadingSetting(){
        if(qApp){
            qApp->setOrganizationName(organizationName);
            qApp->setQuitOnLastWindowClosed(true);
            qApp->setApplicationVersion(version);
            qApp->setApplicationDisplayName(appName);
            qApp->setWindowIcon(QIcon(iconPath));
            qApp->setDesktopFileName(appName);

            QFile qss(StyleSheetPath);
            if(qss.open(QIODevice::ReadOnly)){
                qApp->setStyleSheet(qss.readAll());
            }
        }
    }
}
```

# 视图类

**用于生成显示数据表格、操作数据控件**

**具体实现[视图类实现](View/README.md)**

## CAN数据控件类

**用于生成两种不同形式的操控数据控件，通过控件的名称来判断，如果需要其他样式的控件可以自行添加**

1. 仿函数用于自定义控件样式，改变控件行为

```cpp
class CustomControl{
public:
    virtual void operator()() =0;
    virtual ~CustomControl(){}
};
```

2. 生成控件
```cpp
class ControlForm : public QWidget
{
    Q_OBJECT
private:
    QString name; //控件名称
    int type; //控件型号
    int maxValue; //最大值

    QStringList lifePackage_L;//用于生成生命包列表，若name是列表中的数据，则生成生命包

    /*用于生成特殊控件*/
    QMap<QString,CustomControl *> customControlTable; //预留用于生成不同的空间列表

public:
    explicit ControlForm(const QString &name,int maxValue,int type,QWidget *parent = nullptr);
    ~ControlForm();
signals:
    //发送修改的数据
    void dataChange(int);

public slots:
    //生成生命包
    void createLifePackage();

private slots:
    //重新设置最大值最小值
    void on_pushButton_2_clicked();

private:
    Ui::ControlForm *ui;
};
```

## CAN数据控件列表类

**CAN数据控件列表，通过这个类显示并管理CAN数据控件**

```cpp
class ControlListWidget :public QWidget
{
    Q_OBJECT
public:
    ControlListWidget(QMap<QString,ParsingCan*> pads, int type,QWidget *widget =nullptr);
    ~ControlListWidget();

    //根据pads索引表生成tab控件列表和sub控件列表
    void init();
signals:
    //将控件数据解析后，按照型号，tab，参数名字，修改数据发送信号
    void dataChange(int type,QString tableName,QString paramaterName,int value);
    //void dataChange(QString tableName,QString paramaterName,int value);

public slots:
    //获取子控件列表中数据，并进行解析
    void dataChanged(int value);
    //启动解析
    void start();
    //停止解析
    void stop();

private:

    typedef QMap<QString,ControlForm*> subControls; //子控件列表
    QMap<QString,subControls> tabControls;//控件列表
    QMap<QString,ParsingCan*> pads; //数据包
    int type; 

    CANEngine *engine =nullptr; //can驱动设备
    ParsingThread *parsingThread =nullptr;//解析线程

};
```

## 主窗口类

**SimulationCan_ 是主要工作类，其他都是辅助类**

```cpp
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //初始化can驱动、表格模型、模拟can数据包
    void init();

private slots:
    //打开CAN设备
    void on_clicked();
    //生成控件列表，必须打开成功can设备才能使用
    void on_pushButton_2_clicked();
    //can连接错误码
    void err(const QString &err);


private:
    Ui::MainWindow *ui;
    CANEngine *engine; //can设备，这里使用的是zlgcan的驱动
    ZlgCanTableModel *model ;//can数据表格模型
    SimulationCan_ *simulationCan_;//模拟can数据
    QMessageBox box;//错误信息提示框

};
```


## CAN数据简单表格模型类

** 这是一个简单模型，存储了收和发的数据，若需要做数据分析可以将数据成员can_data保存下来,只有一个insertRow做为添加数据到模型中的接口**

```cpp
class ZlgCanTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ZlgCanTableModel(QObject *parent = nullptr);

    // 返回行列名称
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    //返回行列数
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    //表格数据
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    //返回标志
    Qt::ItemFlags flags(const QModelIndex &index) const override;

public slots:
    //添加数据
    void insertRow(canid_t id,const canMsg & msg);

    QVariant byteToHexString(BYTE *byte) const;


private:


    QMap<canid_t,std::vector<canMsg>> can_data; //存储了收发的数据，用 Tx或Rx区分
    QStringList headers; //表格头
};
```

# 示例

**自定义数据的生成在这里!!!**

**具体实现[示例具体实现](example/README.md)**

## CAN数据模拟解析包类

```cpp
class SimulationCan : public QObject
{
    Q_OBJECT
public:
    explicit SimulationCan(CANEngine *engine,QObject *widget =nullptr);
    ~SimulationCan();

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
```


