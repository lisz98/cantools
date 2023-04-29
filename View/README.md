
**视图类解析**
--------------------------------------------

* [controlform.cpp](controlform.cpp)


1. 自定义控件

```cpp
//!
//! \brief 自定义控件
//!
class GearsForm : public CustomControl{
    Q_GADGET
private:
    QList<QPushButton*> btns;//特使控件按钮列表
    ControlForm *form =nullptr;//父控件指针
    QString name;//名称
    int type;//类型
public:
    //仿函数，必须实现，通过这个来创建新的控件样式
    void operator()() override{
        createGearsForm();
    }

    //生成控件列表
    void createGearsForm(){
        QMap<QString,uchar> actions;

        if(name !="手柄触发位置"){
            if(type ==0){
                actions.insert("空挡",0x4E);
                actions.insert("1挡",0x31);
                actions.insert("2挡",0x32);
                actions.insert("3挡",0x33);
                actions.insert("4挡",0x34);
                actions.insert("5挡",0x35);
                actions.insert("6挡",0x36);
                actions.insert("PT挡",0x50);
                actions.insert("倒挡R",0x52);
                actions.insert("D挡",0x44);
            }else{
                actions.insert("空挡",0x4E);
                actions.insert("1挡",0x31);
                actions.insert("2挡",0x32);
                actions.insert("3挡",0x33);
                actions.insert("4挡",0x34);
                actions.insert("5挡",0x35);
                actions.insert("6挡",0x36);
                actions.insert("7挡",0x50);
                actions.insert("倒挡R",0x52);
                actions.insert("未知挡位",0xE9);
            }
        }else{
            actions.insert("前进D",0x7d);
            actions.insert("H档",0xE2);
            actions.insert("空档",0xF1);
            actions.insert("倒档R1、R2",0xF0);
            actions.insert("+档",0xE6);
            actions.insert("-档",0xE7);
            actions.insert("中心转向PT档",0xE5);
            actions.insert("+、-档中间位",0xE8);
            actions.insert("未知档位",0xE9);
        }
        QVBoxLayout *v =new QVBoxLayout(form);
        QGroupBox *g =new QGroupBox;
        g->setMaximumHeight(60);
        g->setTitle(name);
        QHBoxLayout *h =new QHBoxLayout(g);
        v->addWidget(g);
        foreach (auto action, actions) {
            QPushButton *btn =new QPushButton(actions.key(action));
            btn->setObjectName(actions.key(action));
            btns.append(btn);
            h->addWidget(btn,Qt::AlignmentFlag::AlignBottom);
            btn->connect(btn,&QPushButton::clicked,[=](){
                form->dataChange((int)action);
                for (int i = 0; i < btns.count(); ++i) {
                    btns.at(i)->setStyleSheet("background-color: ");
                }
                btn->setStyleSheet("background-color:green");});
        }
    }


    GearsForm(const QString &name,int type,ControlForm *form) :CustomControl(),form(form) ,name(name),type(type){}
    ~GearsForm() override{}
};
```

2. 控件构造函数，根据不同的名称会生成不同的效果

```cpp
//!
//! \brief 控件构造函数，根据不同的名称会生成不同的效果
//! \param 控件名称 "实际档位" <<"期望档位" <<"当前档位" <<"手柄触发位置"会生成特殊的按钮控件 ，"心跳" <<"生命信号"会生成自动累加的生命包
//! \param 控件数据可设置的最大值
//! \param 类型 
//! \param parent
//!
ControlForm::ControlForm(const QString &name,int maxValue,int type,QWidget *parent) :
    QWidget(parent),name(name),type(type),maxValue(maxValue),
    ui(new Ui::ControlForm)
{
    gears_l << "实际档位" <<"期望档位" <<"当前档位" <<"手柄触发位置";
    lifePackage_L <<"心跳" <<"生命信号";
    
    if(gears_l.contains(name)){ //生成特殊控件
        createGearsForm();
    }else{ //生成普通控件
        ui->setupUi(this);
        ui->label_2->setNum(0); 
        ui->spinBox_4->setValue(maxValue);
        ui->horizontalSlider_2->setRange(0,maxValue);
        ui->groupBox_2->setTitle(name);
        QObject::connect(ui->horizontalSlider_2,&QSlider::valueChanged,[&](int value){ //数据改变时发送数据
            ui->label_2->setNum(value);
            emit dataChange(value);
        });
    }

    //生成10ms累加1的生命包
    if(lifePackage_L.contains(name)){
        ui->groupBox_2->setDisabled(true);
        QTimer *t =new QTimer;
        connect(t,&QTimer::timeout,this,&ControlForm::createLifePackage);
        t->start(10);

    }
}
```

3. 生成累加生命包

```cpp
//!
//! \brief 生成累加生命包
//!
void ControlForm::createLifePackage()
{
    static int cnt =0;
    cnt =(cnt & maxValue) +1; //0-maxValue生命包
    ui->horizontalSlider_2->setValue(cnt);
}
```

4. 重新设置最大值最小值

```cpp
//!
//! \brief 重新设置最大值最小值
//!
void ControlForm::on_pushButton_2_clicked()
{
    int min =ui->spinBox_3->text().toInt();
    int max =ui->spinBox_4->text().toInt();

    if(min <0 || min > max || max >maxValue){
        QMessageBox::critical(nullptr,"设置失败",QString("%1的范围为0~%2").arg(name).arg(maxValue));
        ui->spinBox_3->setValue(0);
        ui->spinBox_4->setValue(maxValue);
        return;
    }

    ui->horizontalSlider_2->setRange(min,max);
}
```

* [controllistwidget.cpp](controllistwidget.cpp)

1. 构造函数

```cpp
//!
//! \brief ControlListWidget::ControlListWidget
//! \param 解析包列表
//! \param 类型
//! \param can设备
//! \param widget
//!
ControlListWidget::ControlListWidget(QMap<QString,ParsingCan*> pads,int type,CANEngine *engine ,QWidget *widget)
        :QWidget(widget),pads(pads),type(type),engine(engine)
{
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
    init();
}
```

2. 按照解析包列表生成控件列表

```cpp
//!
//! \brief 按照解析包列表生成控件列表
//!
void ControlListWidget::init()
{

    if(pads.isEmpty()){
        return;
    }

    QTabWidget *tab =new QTabWidget;
    QVBoxLayout *v =new QVBoxLayout(this);
    v->addWidget(tab);

    Q_FOREACH(auto can , pads ){
        //遍历生成控件
        QWidget *w = new QWidget;
        QGridLayout *layout = new QGridLayout;
        w->setLayout(layout);
        subControls subControl;
        for(auto itr =can->paramaters.begin();itr != can->paramaters.end();++itr){
            ControlForm *control =new ControlForm(itr.key(),itr.value(),type);
            control->setObjectName(can->name+":"+itr.key());
            subControl.insert(itr.key(),control);

            connect(control,&ControlForm::dataChange,this,&ControlListWidget::dataChanged);
        }
        //生成布局


        const QStringList & keys =subControl.keys();
        for(int i =0; i <keys.count();++i){
            layout->addWidget(subControl[keys.at(i)],i / 5,i %5);
        }

        tabControls.insert(can->name,subControl);

        tab->addTab(w,can->name);
    }

    //生成解析线程
    parsingThread =new ParsingThread(pads,engine);
}
```

3. 槽函数，解析数据

```cpp
void ControlListWidget::dataChanged(int value)
{
    QString objStr =sender()->objectName(); //获取控件的名称

    const QString tabName = objStr.split(":").at(0); //解析包名称
    const QString paramater = objStr.split(":").at(1); //控件名称

    emit dataChange(type,tabName,paramater,value); //将解析的数据发送出去

}
```

4. 启动解析线程和停止解析线程

```cpp
//!
//! \brief 启动解析线程
//!
void ControlListWidget::start()
{
    parsingThread->start();
}
//!
//! \brief 停止解析线程
//!
void ControlListWidget::stop()
{
    if(parsingThread->isRunning()){
        parsingThread->terminate();
        parsingThread->quit();
    }

}
```

* [mainwindow.cpp](mainwindow.cpp)

1. 构造函数

```cpp
//!
//! \brief 构造函数
//! \param parent
//!
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),engine(nullptr),simulationCan_(nullptr)
{
    ui->setupUi(this);
    //注册自定义类型到元对象中，用于信号与槽
    qRegisterMetaType<canid_t>("canid_t");
    qRegisterMetaType<canMsg>("canMsg");
    init();


}
```

1. 初始化can驱动设备，初始化控件，初始化zlg数据模型

```cpp
//!
//! \brief 初始化can驱动设备
//!
void MainWindow::init()
{
    //初始化can驱动设备
    engine =new ZLGEngine;

    //初始化模拟解析
    simulationCan_ =new SimulationCan_(engine);

    //初始化控件
    ui->comboBox->addItems(engine->getDeviceNames());
    ui->comboBox_2->addItems(engine->getBauds());
    ui->pushButton_2->setDisabled(false);
    ui->comboBox_4->addItem("练习");
    for(int i =1;i <engine->getChannelCount()+1;i++){
        ui->comboBox_3->addItem(QString::number(i));
    }

    //初始化模型
    model = new ZlgCanTableModel;
    ui->tableView->setModel(model);



    //槽函数
    connect(ui->pushButton,&QPushButton::clicked,this,&MainWindow::on_clicked); //打开设备
    connect(engine,&CANEngine::errString,this,&MainWindow::err);//错误信息
    connect(static_cast<ZLGEngine*>(engine),&ZLGEngine::readyRead, //can盒收到的信息
            model,static_cast<void(ZlgCanTableModel::*)(canid_t,const canMsg&)>(&ZlgCanTableModel::insertRow));
}
```

3. 显示can驱动连接报错

```cpp
//!
//! \brief 显示can驱动连接报错
//! \param 错误码
//!
void MainWindow::err(const QString &err)
{
    box.setWindowTitle("错误!!!");
    box.setText(err);
    //QMessageBox::warning(this,,);
    if(box.exec() ==QMessageBox::Ok){
        if(simulationCan_){
            simulationCan_->stop();
        }
    }
}
```