#include <QtTest/QtTest>
#include <assert.h>
#include "mainhostvst.h"

class Test_MainHost : public QObject
{
    Q_OBJECT

public:

    Test_MainHost() {}

    int iter;
    MainHostVst *myHost;

private Q_SLOTS:
    void initTestCase() {
//        iter=1000000;
        iter=1000000;
        myHost = new MainHostVst(0);
        myHost->InitThread();
        myHost->SetMainWindow(0);
    }

    void benchcables() {

        MetaInfo info(MetaTypes::object);
        info.data.SetMeta(MetaInfos::ObjType, ObjTypes::Script);
        info.SetContainerId(myHost->hostContainer->ObjId());

        QSharedPointer<Connectables::Object>obj = myHost->objFactory->NewObject(info);

        Connectables::Pin *pinOut = obj->GetPinList(Directions::Output,MediaTypes::Parameter)->listPins.value(0);
        Connectables::Pin *pinIn = obj->GetPinList(Directions::Input,MediaTypes::Parameter)->listPins.value(0);


//        MetaInfo pin1info(MetaTypes::pin);
//        MetaInfo pin2info(MetaTypes::pin);
//        pin1info.SetContainerId(obj->ContainerId());
//        pin2info.SetContainerId(obj->ContainerId());
//        pin1info.data.SetMeta(MetaInfos::Direction,Directions::Output);
//        pin2info.data.SetMeta(MetaInfos::Direction,Directions::Input);
//        pin1info.data.SetMeta(MetaInfos::Media,MediaTypes::Parameter);
//        pin2info.data.SetMeta(MetaInfos::Media,MediaTypes::Parameter);

//        Connectables::ParameterPin pinOut(myHost,pin1info,0.2f);
//        Connectables::ParameterPin pinIn(myHost,pin2info,0.4f);

        Connectables::Cable cab(pinOut->info(),pinIn->info());
        myHost->OnCableAdded(&cab);
        float val=0.1234f;
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                myHost->SendMsg(pinOut->info(),PinMessage::ParameterValue,(void*)&val);
//                pinOut.OnValueChanged();

//            QVERIFY(pinIn.GetValue() == pinOut.GetValue());
        }
    }
};


QTEST_MAIN(Test_MainHost);

#include "test_mainhost.moc"
