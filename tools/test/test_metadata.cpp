/**************************************************************************
#    Copyright 2010-2011 Raphaël François
#    Contact : ctrlbrk76@gmail.com
#
#    This file is part of VstBoard.
#
#    VstBoard is free software: you can redistribute it and/or modify
#    it under the terms of the under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    VstBoard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    under the terms of the GNU Lesser General Public License for more details.
#
#    You should have received a copy of the under the terms of the GNU Lesser General Public License
#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QByteArray>
#include <assert.h>
#include "objectinfo.h"

class Test_MetaData : public QObject
{
    Q_OBJECT

public:

    Test_MetaData() {}

    int iter;

    MetaData refData;
    int refInt;
    QString refString;
    float refFloat;
    bool refBool;

    QByteArray refArray;

private Q_SLOTS:
    void initTestCase() {
        iter=10;
        refInt = 321;
        refFloat = 0.1234f;
        refString = "test";
        refBool = true;
        refData=MetaData();
        refArray.resize(0);
    }

    void writestr() {
        refData.SetMeta<QString>(MetaInfos::devName,refString);

        QString *p;
        p=refData.GetMeta<QString*>(MetaInfos::devName);
        QVERIFY(p!=0);
        QVERIFY(*p==refString);

        QString i;
        i = refData.GetMetaData<QString>(MetaInfos::devName);
        QVERIFY(i==refString);
    }

    void writeint() {
        refData.SetMeta<int>(MetaInfos::devId,refInt);

        int *p;
        p=refData.GetMeta<int*>(MetaInfos::devId);
        QVERIFY(p!=0);
        QVERIFY(*p==refInt);

        int i;
        i = refData.GetMetaData<int>(MetaInfos::devId);
        QVERIFY(i==refInt);
    }

    void writefloat() {
        refData.SetMeta<float>(MetaInfos::Value,refFloat);

        float *p;
        p=refData.GetMeta<float*>(MetaInfos::Value);
        QVERIFY(p!=0);
        QVERIFY(*p==refFloat);

        float i;
        i = refData.GetMetaData<float>(MetaInfos::Value);
        QVERIFY(i==refFloat);
    }

    void writebool() {
        refBool=true;
        refData.SetMeta<bool>(MetaInfos::Dirty,refBool);

        bool *p;
        p=refData.GetMeta<bool*>(MetaInfos::Dirty);
        QVERIFY(p!=0);
        QVERIFY(*p==refBool);

        bool i;
        i = refData.GetMetaData<bool>(MetaInfos::Dirty);
        QVERIFY(i==refBool);

        refBool=false;
        refData.SetMeta<bool>(MetaInfos::Dirty,refBool);

        bool *p2;
        p2=refData.GetMeta<bool*>(MetaInfos::Dirty);
        QVERIFY(p2!=0);
        QVERIFY(*p2==refBool);

        bool i2;
        i2 = refData.GetMetaData<bool>(MetaInfos::Dirty);
        QVERIFY(i2==refBool);
    }

    void writestring() {
        refData.SetMeta<QString>(MetaInfos::devName,refString);

        QString *p;
        p=refData.GetMeta<QString*>(MetaInfos::devName);
        QVERIFY(p!=0);
        QVERIFY(*p==refString);

        QString i;
        i = refData.GetMetaData<QString>(MetaInfos::devName);
        QVERIFY(i==refString);
    }

    void testconst() {
        int i,j;
        i = refData.GetMetaData<int>(MetaInfos::devId);
        ++i;
        j = refData.GetMetaData<int>(MetaInfos::devId);
        QVERIFY(i==j+1);
    }

    void copy() {
        MetaData copy=refData;

        QVERIFY(copy==refData);
        QVERIFY(&copy!=&refData);

        int i;
        i = copy.GetMetaData<int>(MetaInfos::devId);
        QVERIFY(i==refInt);

        float f;
        f = copy.GetMetaData<float>(MetaInfos::Value);
        QVERIFY(f==refFloat);

        bool b;
        b = copy.GetMetaData<bool>(MetaInfos::Dirty);
        QVERIFY(b==refBool);

        QString s;
        s = copy.GetMetaData<QString>(MetaInfos::devName);
        QVERIFY(s==refString);

        MetaData copy2=copy;
        copy=MetaData();
        QVERIFY(copy2==refData);

    }

    void stream() {
        MetaData copy;
        QDataStream w(&refArray,QIODevice::WriteOnly);
        w << refData;
        QDataStream r(&refArray,QIODevice::ReadOnly);
        r >> copy;

        QVERIFY(copy==refData);
        QVERIFY(&copy!=&refData);

        int i;
        i = copy.GetMetaData<int>(MetaInfos::devId);
        QVERIFY(i==refInt);

        float f;
        f = copy.GetMetaData<float>(MetaInfos::Value);
        QVERIFY(f==refFloat);

        bool b;
        b = copy.GetMetaData<bool>(MetaInfos::Dirty);
        QVERIFY(b==refBool);

        QString s;
        s = copy.GetMetaData<QString>(MetaInfos::devName);
        QVERIFY(s==refString);
    }

    void benchwriteint() {
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                refData.SetMeta<int>(MetaInfos::devId,refInt);
        }
    }
    void benchreadint() {
        int j;
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                j=refData.GetMetaData<int>(MetaInfos::devId);
        }
    }
    void benchwritefloat() {
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                refData.SetMeta<float>(MetaInfos::Value,refFloat);
        }
    }
    void benchreadfloat() {
        float j;
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                j=refData.GetMetaData<float>(MetaInfos::Value);
        }
    }
    void benchwritebool() {
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                refData.SetMeta<bool>(MetaInfos::Dirty,refBool);
        }
    }
    void benchreadbool() {
        bool j;
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                j=refData.GetMetaData<bool>(MetaInfos::Dirty);
        }
    }
    void benchwritestring() {
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                refData.SetMeta<QString>(MetaInfos::devName,refString);
        }
    }
    void benchreadstring() {
        QString j;
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                j=refData.GetMetaData<QString>(MetaInfos::devName);
        }
    }
    void benchreadstringpointer() {
        QString *j;
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                j=refData.GetMeta<QString*>(MetaInfos::devName);
        }
    }
    void benchcopy() {
        MetaData copy;
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                copy=refData;
        }
    }
    void benchstreamwrite() {
        QDataStream w(&refArray,QIODevice::WriteOnly);
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                w << refData;
        }
    }
    void benchstreamread() {
        MetaData copy;
        QDataStream r(&refArray,QIODevice::ReadOnly);
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                r >> copy;
        }
    }
    void benchdel() {
        MetaData copy=refData;
        QBENCHMARK {
            for (int i=0; i<iter; ++i)
                copy.DelMeta(MetaInfos::devId);
        }
    }

};

//QTEST_MAIN(Test_MetaData);

#include "test_metadata.moc"
