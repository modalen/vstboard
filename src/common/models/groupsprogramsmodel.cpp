#include "groupsprogramsmodel.h"

GroupsProgramsModel::GroupsProgramsModel( MsgController *msgCtrl, QObject *parent ) :
    QStandardItemModel(parent),
    MsgHandler(msgCtrl, FixedObjId::programsManager),
    currentProgColor(Qt::green),
    orderChanged(false),
    currentMidiGroup(0),
    currentMidiProgram(0)
{
    updateTimer.setInterval(20);
    updateTimer.setSingleShot(true);
    connect(&updateTimer, SIGNAL(timeout()),
            this, SLOT(SendUpdateToHost()));
}

void GroupsProgramsModel::SendUpdateToHost()
{
    MsgObject msg(-1,GetIndex());

    if(orderChanged) {
        orderChanged=false;
        msg.prop["actionType"]="fullUpdate";

        int nbGrp = rowCount();
        for(int i=0; i<nbGrp; ++i) {
            QStandardItem *grp = item(i);

            MsgObject msgGrp(GetIndex(),i);
            if(grp) {
                msgGrp.prop["id"]=grp->data().toInt();
                msgGrp.prop["name"]=grp->text();

                int nbPrg = grp->rowCount();
                for(int j=0; j<nbPrg; ++j) {
                    QStandardItem *prg = grp->child(j);

                    MsgObject msgPrg(i,j);
                    if(prg) {
                        msgPrg.prop["id"]=prg->data().toInt();
                        msgPrg.prop["name"]=prg->text();
                    }

                    msgGrp.children << msgPrg;
                }
            }

            msg.children << msgGrp;
        }
    }

    msg.prop["currentGroup"]=currentMidiGroup;
    msg.prop["currentProg"]=currentMidiProgram;

    msgCtrl->SendMsg(msg);
}

void GroupsProgramsModel::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop["actionType"]=="fullUpdate") {
        QStandardItemModel::clear();
        foreach(const MsgObject &msgGrp, msg.children) {
#ifndef QT_NO_DEBUG
            QStandardItem *itemGrp = new QStandardItem( QString("(%1)%2").arg(msgGrp.prop["id"].toInt()).arg(msgGrp.prop["name"].toString()));
#else
            QStandardItem *itemGrp = new QStandardItem(msgGrp.prop["name"].toString());
#endif
            itemGrp->setData(msgGrp.prop["id"]);
            itemGrp->setDragEnabled(true);
            itemGrp->setDropEnabled(false);
            itemGrp->setEditable(true);
//            itemGrp->setCheckState(Qt::Unchecked);

            foreach(const MsgObject &msgPrg, msgGrp.children) {
#ifndef QT_NO_DEBUG
                QStandardItem *itemPrg = new QStandardItem( QString("(%1)%2").arg(msgPrg.prop["id"].toInt()).arg(msgPrg.prop["name"].toString()));
#else
                QStandardItem *itemPrg = new QStandardItem(msgPrg.prop["name"].toString());
#endif
                itemPrg->setData(msgPrg.prop["id"]);
                itemPrg->setDragEnabled(true);
                itemPrg->setDropEnabled(false);
                itemPrg->setEditable(true);
//                itemPrg->setCheckState(Qt::Unchecked);

                itemGrp->appendRow(itemPrg);
            }
            QStandardItemModel::appendRow(itemGrp);
        }
    }

    if(msg.prop.contains("currentGroup")) {
        if(currentGroup.isValid()) {
            itemFromIndex(currentGroup)->setBackground(Qt::transparent);
//            itemFromIndex(currentGroup)->setCheckState(Qt::Unchecked);
        }

        currentMidiGroup = msg.prop["currentGroup"].toUInt();
        QModelIndex idx = index(currentMidiGroup,0);
        if(idx.isValid()) {
            currentGroup=idx;
            emit GroupChanged(idx);
        }

        if(currentGroup.isValid()) {
            itemFromIndex(currentGroup)->setBackground(currentProgColor);
//            itemFromIndex(currentGroup)->setCheckState(Qt::Checked);
        }
    }
    if(msg.prop.contains("currentProg")) {
        if(currentProg.isValid()) {
            itemFromIndex(currentProg)->setBackground(Qt::transparent);
//            itemFromIndex(currentProg)->setCheckState(Qt::Unchecked);
        }

        currentMidiProgram = msg.prop["currentProg"].toUInt();
        if(currentGroup.isValid()) {
            QModelIndex idx = index(currentMidiProgram,0, currentGroup);
            if(idx.isValid()) {
                currentProg=idx;
                emit ProgChanged(idx);
            }
        }

        if(currentProg.isValid()) {
            itemFromIndex(currentProg)->setBackground(currentProgColor);
//            itemFromIndex(currentProg)->setCheckState(Qt::Checked);
        }
    }

    if(msg.prop.contains("promptType")) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText(tr("The %1 has been modified.").arg(msg.prop["promptType"].toString()));
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        MsgObject answer(-1,GetIndex());
        answer.prop["promptAnswer"] = msgBox.exec();
        msgCtrl->SendMsg(answer);
    }

    if(msg.prop.contains("progAutosave")) {
        emit ProgAutosaveChanged( static_cast<Qt::CheckState>(msg.prop["progAutosave"].toInt()) );
    }
    if(msg.prop.contains("groupAutosave")) {
        emit GroupAutosaveChanged( static_cast<Qt::CheckState>(msg.prop["groupAutosave"].toInt()) );
    }

//    if(msg.prop.contains("name")) {
//        if(msg.prop.contains("prog")) {
//            item( msg.prop["group"].toInt() )->child( msg.prop["prog"].toInt() )->setText( msg.prop["name"].toString() );
//        } else {
//            item( msg.prop["group"].toInt() )->setText( msg.prop["name"].toString() );
//        }
//    }

//    if(msg.prop.contains("removeProg")) {
//        int prg = msg.prop["removeProg"].toInt();
//        int grp = msg.prop["group"].toInt();
//        QModelIndex grpIdx( item(grp)->index() );
//        if(grpIdx.isValid())
//            QStandardItemModel::removeRows(prg,1, grpIdx);
//    }

//    if(msg.prop.contains("removeGroup")) {
//        int grp = msg.prop["removeGroup"].toInt();
//        QStandardItemModel::removeRows(grp,1);
//    }
}

bool GroupsProgramsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    orderChanged=true;
    updateTimer.start();
    return QStandardItemModel::setData(index,value,role);

//    if(!index.isValid())
//        return false;

//    if(role == Qt::EditRole) {

//        MsgObject msg(-1,GetIndex());
//        msg.prop["name"]=value;

//        if(index.parent().isValid()) {
//            //program
//            msg.prop["group"]=index.parent().row();
//            msg.prop["prog"]=index.row();
//        } else {
//            //group
//            msg.prop["group"]=index.row();
//        }

//        msgCtrl->SendMsg(msg);
//        return true;
//    }

//    return true;
}

bool GroupsProgramsModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if(rowCount(parent)==1)
        return false;

//    if(itemFromIndex( parent.child(row,0) )->checkState()==Qt::Checked) {
//        //go to another program
//        QModelIndex idx = index(row+1,0,parent);
//        if(!idx.isValid())
//            idx = index(0,0,parent);

//        UserChangeProg( idx );
//    }

//    if(parent.isValid()) {
//        for(int i=0; i<count; ++i) {
//            MsgObject msg(-1,GetIndex());
//            msg.prop["group"]=parent.row();
//            msg.prop["removeProg"]=row+i;
//            msgCtrl->SendMsg(msg);
//        }
//    } else {
//        for(int i=0; i<count; ++i) {
//            MsgObject msg(-1,GetIndex());
//            msg.prop["removeGroup"]=row+i;
//            msgCtrl->SendMsg(msg);
//        }
//    }

    orderChanged=true;
    updateTimer.start();
    return QStandardItemModel::removeRows(row,count,parent);
}

bool GroupsProgramsModel::insertRows(int row, int count, const QModelIndex &parent)
{
    orderChanged=true;
    updateTimer.start();
    return QStandardItemModel::insertRows(row,count,parent);
}

QMimeData * GroupsProgramsModel::mimeData ( const QModelIndexList & indexes ) const
{
    QMimeData *mime = QStandardItemModel::mimeData(indexes);

//    QByteArray b;
//    QDataStream s(&b, QIODevice::WriteOnly);

//    foreach(const QModelIndex &idx, indexes) {
//        s << itemFromIndex(idx);
//    }
    if(indexes.first().parent().isValid())
        mime->setData(MIMETYPE_PROGRAM,QByteArray());
    else
        mime->setData(MIMETYPE_GROUP,QByteArray());

    return mime;
}

void GroupsProgramsModel::UserChangeProg(const QModelIndex &idx)
{
    MsgObject msg(-1,GetIndex());
    if(idx.parent().isValid())
        msg.prop["currentProg"]=idx.row();
    else
        msg.prop["currentGroup"]=idx.row();
    msgCtrl->SendMsg(msg);

//    if(idx.parent().isValid())
//        currentMidiProgram = idx.row();
//    else
//        currentMidiGroup = idx.row();
//    updateTimer.start();
}

//void GroupsProgramsModel::UserRemoveGroup(const QModelIndexList &grp)
//{
//    QVariantList lstGrp;
//    foreach(const QModelIndex &idx, grp) {
//        lstGrp << idx.row();
//    }
//    MsgObject msg(-1,GetIndex());
//    msg.prop["removeGroup"]=QVariant::fromValue(lstGrp);
//    msgCtrl->SendMsg(msg);
//    updateTimer.start();
//}

//void GroupsProgramsModel::UserRemoveProg(const QModelIndexList &prg)
//{
//    QVariantList lstPrg;
//    foreach(const QModelIndex &idx, prg) {
//        lstPrg << idx.row();
//    }

//    MsgObject msg(-1,GetIndex());
//    msg.prop["removeProg"]=QVariant::fromValue(lstPrg);
//    msgCtrl->SendMsg(msg);
//    updateTimer.start();
//}

//void GroupsProgramsModel::UserAddGroup(const int row)
//{
//    MsgObject msg(-1,GetIndex());
//    msg.prop["addGroup"]=row;
//    msgCtrl->SendMsg(msg);
//}

//void GroupsProgramsModel::UserAddProgram(const int row)
//{
//    MsgObject msg(-1,GetIndex());
//    msg.prop["addProg"]=row;
//    msgCtrl->SendMsg(msg);
//}

void GroupsProgramsModel::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(groupId==ColorGroups::Programs && colorId==Colors::HighlightBackground) {
        currentProgColor = color;
        if(currentGroup.isValid())
            itemFromIndex(currentGroup)->setBackground(currentProgColor);
        if(currentProg.isValid())
            itemFromIndex(currentProg)->setBackground(currentProgColor);
    }
}

QStringList GroupsProgramsModel::mimeTypes () const
{
    return QStandardItemModel::mimeTypes() << MIMETYPE_PROGRAM << MIMETYPE_GROUP;
}

void GroupsProgramsModel::UserChangeProgAutosave(const Qt::CheckState state)
{
    MsgObject msg(-1,GetIndex());
    msg.prop["progAutosave"]=state;
    msgCtrl->SendMsg(msg);
}

void GroupsProgramsModel::UserChangeGroupAutosave(const Qt::CheckState state)
{
    MsgObject msg(-1,GetIndex());
    msg.prop["groupAutosave"]=state;
    msgCtrl->SendMsg(msg);
}

void GroupsProgramsModel::Update()
{
    MsgObject msg(-1,GetIndex());
    msg.prop["fullUpdate"]=1;
    msgCtrl->SendMsg(msg);
}
