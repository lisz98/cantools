/**
 1. @file:       zlgcantablemodel.h
 2. @Author:     Lsz.
 3. @Date:       2022-05-06
 4. @Brief:      File Description
 **/

#ifndef ZLGCANTABLEMODEL_H
#define ZLGCANTABLEMODEL_H

#include <QAbstractItemModel>
#include <QDateTime>
#include <QString>
#include <QByteArray>
#include <QSet>

#include "CANEngine/zlgengine.h"
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

    QVariant byteToHexString(BYTE *byte,int length) const;


private:


    QMap<canid_t,std::vector<canMsg>> can_data; //存储了收发的数据，用 Tx或Rx区分
    QStringList headers; //表格头
};

#endif // ZLGCANTABLEMODEL_H
