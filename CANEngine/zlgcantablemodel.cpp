#include "zlgcantablemodel.h"
#include <QDateTime>
//!
//! \brief 构造函数，初始化表头名称
//! \param parent
//!
ZlgCanTableModel::ZlgCanTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    headers << "帧id"<<"时间戳"<<"通道"<<"频率" <<"数量"<<"类型"<<"长度" <<"方向" <<"报文";

}
//!
//! \brief 设置表头
//! \param section
//! \param orientation
//! \param role
//! \return
//!
QVariant ZlgCanTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    if(role == Qt::DisplayRole){
        if(orientation == Qt::Horizontal){
            if (section < 0 || section >= headers.size()){
                return QVariant();
            }
            return headers.at(section);
        }else{
            return QString::number(section +1);
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}
//!
//! \brief 设置模型rowCount
//! \param parent
//! \return
//!
int ZlgCanTableModel::rowCount(const QModelIndex &parent) const
{
    return can_data.count();
}
//!
//! \brief 设置模型columnCount
//! \param parent
//! \return
//!
int ZlgCanTableModel::columnCount(const QModelIndex &parent) const
{
    return headers.count();
}
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
        case 3:
        {
            if(can_data[canId_t].back().orientation == "Rx" && can_data[canId_t].size() >1){
                int f =can_data[canId_t].size()-1;
                auto curT = can_data[canId_t].at(f);
                auto prevT = can_data[canId_t].at(f-1);
                return "Null";
            }
            return can_data[canId_t].back().hz;

        }break;//频率
        case 4: return can_data[canId_t].size(); break;//数量
        case 5: return can_data[canId_t].back().type; break;//类型
        case 6: return can_data[canId_t].back().dlc; break;//长度
        case 7: return can_data[canId_t].back().orientation; break;//方向
        case 8: return byteToHexString(can_data[canId_t].back().data,can_data[canId_t].back().dlc); break;//报文
        }
    }

    return QVariant();
}

Qt::ItemFlags ZlgCanTableModel::flags(const QModelIndex &index) const
{
    return QAbstractTableModel::flags(index) & ~Qt::ItemIsEditable;
}
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
        if(byte[i] <16){
            data +="0";
        }
        data += QString("%1-").arg(byte[i],0,16).toUpper();
    }
    //qDebug() << data;

    return data;
}
