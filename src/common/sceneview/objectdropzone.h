#ifndef OBJECTDROPZONE_H
#define OBJECTDROPZONE_H

#include <QGraphicsWidget>
#include "msghandler.h"

namespace View {
    class ObjectDropZone : public QGraphicsWidget, public MsgHandler
    {
        Q_OBJECT
    public:
        explicit ObjectDropZone(MsgController *msgCtrl, int objId, QGraphicsItem *parent, QWidget *parking=0);
        QWidget *GetParking() {return myParking;}

    protected:
        void dragEnterEvent( QGraphicsSceneDragDropEvent *event);
        void dragMoveEvent( QGraphicsSceneDragDropEvent *event);
        void dragLeaveEvent( QGraphicsSceneDragDropEvent *event);
        void dropEvent( QGraphicsSceneDragDropEvent *event);
        bool TranslateMimeData( const QMimeData * data, MsgObject &msg );
        QWidget *myParking;

    signals:
        void ObjectDropped(QGraphicsSceneDragDropEvent *event, MsgObject msg);

    public slots:
        virtual void HighlightStart();
        virtual void HighlightStop();
        void SetParking(QWidget *park) {myParking=park;}
        void UpdateHeight();
    };
}
#endif // OBJECTDROPZONE_H
