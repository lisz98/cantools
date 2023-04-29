/**
 1. @file:       controlform.h
 2. @Author:     Lsz.
 3. @Date:       2022-05-06
 4. @Brief:      File Description
 **/
#ifndef CONTROLFORM_H
#define CONTROLFORM_H

#include <QWidget>
#include <QTimer>
#include <QMap>
#include <QPushButton>
namespace Ui {
class ControlForm;
}

class CustomControl{
public:
    virtual void operator()() =0;
    virtual ~CustomControl(){}
};

class ControlForm : public QWidget
{
    Q_OBJECT
private:
    QString name; //控件名称
    int type;
    int maxValue; //最大值

    QStringList lifePackage_L;//用于生成生命包列表，若name是列表中的数据，则生成生命包

    /*用于生成特殊控件*/
    QMap<QString,CustomControl *> customControlTable; //用于生成特殊的控件列表，需要重新继承并实现CustomControl类

public:
    explicit ControlForm(const QString &name,int maxValue,int type,QWidget *parent = nullptr);
    ~ControlForm();

    //按名字获取生成的控件
    static ControlForm* getControlByName(const QString &name) {return m_allControlForm[name];};

    ControlForm* copy();

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
    static QMap<QString,ControlForm*> m_allControlForm;

};


#endif // CONTROLFORM_H
