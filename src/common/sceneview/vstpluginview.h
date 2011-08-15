#ifndef VSTPLUGINVIEW_H
#define VSTPLUGINVIEW_H

#include "connectableobjectview.h"

namespace View {
    class VstPluginView : public ConnectableObjectView
    {
    Q_OBJECT
    public:
        VstPluginView(const MetaInfo &info, MainContainerView * parent = 0);
        void UpdateModelIndex(const MetaInfo &info);
        void SetConfig(ViewConfig *config);

    protected:
        void dragEnterEvent( QGraphicsSceneDragDropEvent *event);
        void dragLeaveEvent( QGraphicsSceneDragDropEvent *event);
//        void dropEvent( QGraphicsSceneDragDropEvent *event);
        QAction *actSaveBank;
        QAction *actSaveBankAs;
        QAction *actSaveProgram;
        QAction *actSaveProgramAs;

    public slots:
        void SaveBankAs();
        void SaveBank();
        void SaveProgramAs();
        void SaveProgram();
        void UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);
        void HighlightStart();
        void HighlightStop();
    };
}

#endif // VSTPLUGINVIEW_H
