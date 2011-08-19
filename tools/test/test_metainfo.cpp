#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QByteArray>
#include <assert.h>
#include "objectinfo.h"

class Test_MetaInfo : public QObject
{
    Q_OBJECT

public:

    Test_MetaInfo() {}

    int iter;
    MetaInfo refInfo;
    MetaData refData;

private Q_SLOTS:
    void initTestCase() {
        iter=1000000;
        refInfo.SetType(MetaTypes::object);
        refInfo.data.SetMeta(MetaInfos::Value,0.1234f);
        refInfo.data.SetMeta(MetaInfos::devId,4);

    }

    void benchcanconnect() {
        MetaInfo pin1(MetaTypes::pin);
        MetaInfo pin2(MetaTypes::pin);
        pin1.SetContainerId(1);
        pin2.SetContainerId(1);
        pin1.data.SetMeta(MetaInfos::Direction,1);
        pin2.data.SetMeta(MetaInfos::Direction,2);
        pin1.data.SetMeta(MetaInfos::Media,1);
        pin2.data.SetMeta(MetaInfos::Media,1);

        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                pin1.CanConnectTo(pin2);
        }
    }

    void benchdatawrite() {
        float f=1.123f;
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                refData.SetMeta<float>(MetaInfos::Value,f);
        }
    }
    void benchdataread() {
        float *f;
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                f=refData.GetMeta<float*>(MetaInfos::Value);
        }
    }
    void benchinfowrite() {
        float f=1.123f;
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                refInfo.data.SetMeta(MetaInfos::Value,f);
        }
    }
    void benchinforead() {
        float *f;
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                f=refInfo.data.GetMeta<float*>(MetaInfos::Value);
        }
    }
};


QTEST_MAIN(Test_MetaInfo);

#include "test_metainfo.moc"
