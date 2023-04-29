#include "controlform.h"
#include "ui_controlform.h"
#include <QMessageBox>
QMap<QString,ControlForm*> ControlForm::m_allControlForm = QMap<QString,ControlForm*>();

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
    GearsForm *gearsForm =new GearsForm(name,type,this);
    customControlTable ={
        {"实际档位",gearsForm},
        {"期望挡位",gearsForm},
        {"当前挡位",gearsForm},
        {"手柄触发位置",gearsForm}
    };
    lifePackage_L <<"心跳" <<"生命信号";

    if(customControlTable.contains(name)){ //生成特殊控件
        (*customControlTable.value(name))();
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

    ControlForm::m_allControlForm.insert(name,this);
}



ControlForm::~ControlForm()
{
    delete ui;
}

ControlForm *ControlForm::copy()
{
    return new ControlForm(name,maxValue,type);
}
//!
//! \brief 生成累加生命包
//!
void ControlForm::createLifePackage()
{
    static int cnt =0;
    cnt =(cnt & maxValue) +1; //0-maxValue生命包
    ui->horizontalSlider_2->setValue(cnt);
}

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



