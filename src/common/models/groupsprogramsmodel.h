#ifndef GROUPSPROGRAMSMODEL_H
#define GROUPSPROGRAMSMODEL_H

#include "precomp.h"
#include "msghandler.h"
#include "views/viewconfig.h"

#define MIMETYPE_GROUP QLatin1String("application/x-groupsdata")
#define MIMETYPE_PROGRAM QLatin1String("application/x-programsdata")

class GroupsProgramsModel : public QStandardItemModel, public MsgHandler
{
    Q_OBJECT
public:
    GroupsProgramsModel( MsgController *msgCtrl, QObject *parent=0 );
    void ReceiveMsg(const MsgObject &msg);

    QStringList mimeTypes () const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    bool removeRows(int row, int count, const QModelIndex &parent);
    bool insertRows(int row, int count, const QModelIndex &parent);
    QMimeData * mimeData ( const QModelIndexList & indexes ) const;

private:
    QPersistentModelIndex currentGroup;
    QPersistentModelIndex currentProg;
    QBrush currentProgColor;
    QTimer updateTimer;
    bool orderChanged;
    quint16 currentMidiGroup;
    quint16 currentMidiProgram;

signals:
    void GroupChanged(const QModelIndex &idx);
    void ProgChanged(const QModelIndex &idx);

public slots:
    void UserChangeProg(const QModelIndex &idx);
//    void UserRemoveGroup(const QModelIndexList &grp);
//    void UserRemoveProg(const QModelIndexList &prg);
//    void UserAddGroup(const int row);
//    void UserAddProgram(const int row);
    void UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);

private slots:
    void SendUpdateToHost();
};

#endif // GROUPSPROGRAMSMODEL_H
