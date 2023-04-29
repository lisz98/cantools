#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include "gcanengine.h"
//!
//! \brief 构造函数
//! \param parent
//!
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),engine(nullptr),simulationCana(nullptr)
{
    ui->setupUi(this);
    //注册自定义类型到元对象中，用于信号与槽
    qRegisterMetaType<canid_t>("canid_t");
    qRegisterMetaType<canMsg>("canMsg");
    init();


}

MainWindow::~MainWindow()
{
    if(simulationCana){
        delete  simulationCana;
    }

    if(engine){
        delete  engine;
    }

    delete ui;

}
//!
//! \brief 初始化can驱动设备
//!
void MainWindow::init()
{
    //初始化can驱动设备
//    engine =new ZLGEngine;

    engine =new GCanEngine;


    simulationCana =new simulationCan(engine);

    //初始化控件
    ui->comboBox->addItems(engine->getDeviceNames());
    ui->comboBox_2->addItems(engine->getBauds());
    ui->pushButton_2->setDisabled(false);
    ui->comboBox_4->addItem("练习");
    for(int i =1;i <engine->getChannelCount()+1;i++){
        ui->comboBox_3->addItem(QString::number(i-1));
    }

    //初始化模型
    model = new ZlgCanTableModel;
    ui->tableView->setModel(model);



    //槽函数
    connect(ui->pushButton,&QPushButton::clicked,this,&MainWindow::on_clicked); //打开设备
    connect(engine,&CANEngine::errString,this,&MainWindow::err);//错误信息
    connect(engine,&ZLGEngine::readyRead, //can盒收到的信息
            model,static_cast<void(ZlgCanTableModel::*)(canid_t,const canMsg&)>(&ZlgCanTableModel::insertRow));
}

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
        if(simulationCana){
            simulationCana->stop();
        }
    }
}

void MainWindow::on_clicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());

    if(btn->text() =="打开"){
        engine->setDevice(ui->comboBox->currentText(),ui->comboBox_3->currentText(),ui->comboBox_2->currentText());
        if(!engine->open()){
            return;
        }
        ui->pushButton_2->setEnabled(true);
        btn->setText("关闭");
    }else{
        if(!engine->close()){
            return;
        }
        ui->pushButton_2->setEnabled(false);
        btn->setText("打开");
    }

}


void MainWindow::on_pushButton_2_clicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());

    if(btn->text() =="打开"){
        simulationCana->strat();
        btn->setText("关闭");
    }else{
        ui->comboBox_4->setEnabled(true);
        btn->setText("打开");
    }
}

