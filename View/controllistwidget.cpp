#include "controllistwidget.h"
#include <QGridLayout>
#include <QTabWidget>
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
//!
//! \brief 析构函数
//!
ControlListWidget::~ControlListWidget()
{
#if 0
    foreach(auto subControl ,tabControls){
        foreach(auto control , subControl){
            disconnect(control,&ControlForm::dataChange,this,&ControlListWidget::dataChanged);
        }
    }
#endif

    if(parsingThread->isRunning()){
        parsingThread->terminate();
        parsingThread->quit();
        delete parsingThread;
        parsingThread =nullptr;
    }
}

//!
//! \brief 按照解析包列表生成控件列表
//!
void ControlListWidget::init()
{

    if(pads.isEmpty()){
        return;
    }

    tab =new QTabWidget;
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
//!
//! \brief 槽函数，解析数据
//! \param 修改数据
//!
void ControlListWidget::dataChanged(int value)
{
    QString objStr =sender()->objectName(); //获取控件的名称

    const QString tabName = objStr.split(":").at(0); //解析包名称
    const QString paramater = objStr.split(":").at(1); //控件名称

    emit dataChange(type,tabName,paramater,value); //将解析的数据发送出去

}
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
//!
//! \brief 创建动作列表
//! \param actionTitle
//! \param actionName
//!
void ControlListWidget::createControlByAction(const QString &actionTitle, const QStringList &actionName)
{
    QWidget *w = new QWidget;
    QGridLayout *layout = new QGridLayout;
    w->setLayout(layout);
    subControls subControl;

    for (int i=0;i <actionName.size() ;i++ ) {
       auto control  = ControlForm::getControlByName(actionName.at(i));
       auto cc = control->copy();
       subControl.insert(actionName.at(i),cc);
       connect(cc,&ControlForm::dataChange,control,&ControlForm::dataChange);
    }

    const QStringList & keys =subControl.keys();
    for(int i =0; i <keys.count();++i){
        layout->addWidget(subControl[keys.at(i)],i / 5,i %5);
    }

    tabControls.insert(actionTitle,subControl);

    tab->addTab(w,actionTitle);

}

