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

#ifndef DEBUGREADWRITELOCK_H
#define DEBUGREADWRITELOCK_H

#ifndef DEBUG_READWRITELOCK
    #define DReadWriteLock QReadWriteLock
    #define SET_READWRITELOCK_NAME(RWLOCK,NAME);
#else

#include <QDebug>
#include "precomp.h"
#define DReadWriteLock DebugReadWriteLock
#define SET_READWRITELOCK_NAME(RWLOCK,NAME); RWLOCK.name=NAME;


class DebugReadWriteLock : public QReadWriteLock
{
public:
    DebugReadWriteLock(RecursionMode recursionMode=NonRecursive) :
        QReadWriteLock(recursionMode),
        countUsage(0),
        countLocked(0),
        countLockedTime(0),
        maxLockedTime(0)
    {
        name=QString("%1").arg((long)this,16);
    }

    ~DebugReadWriteLock()
    {
#ifdef DEBUG_READWRITELOCK_REPORT_UNUSED
        if(countUsage==0) {
            qDebug()<<name<<"unused lock";
        }
#endif
        if(countUsage!=0) {
            qDebug()<<name<<"lock usage:"<<countUsage
                      <<"locks:"<<countLocked
                      <<"time locked:"<<countLockedTime
                      <<"maxTimeLock:"<<maxLockedTime;
        }
    }

    void lockForRead()
    {
        ++countUsage;

        if(!QReadWriteLock::tryLockForRead()) {
            ++countLocked;
            qDebug()<<name<<"lockForRead locked";

            elapsed.restart();
            QReadWriteLock::lockForRead();
            qint64 t = elapsed.elapsed();

            countLockedTime+=t;
            if(t>maxLockedTime)
                maxLockedTime=t;
        }
    }

    bool tryLockForRead()
    {
        ++countUsage;

        if(!QReadWriteLock::tryLockForRead()) {
            ++countLocked;
            qDebug()<<name<<"tryLockForRead failed";
            return false;
        }
        return true;
    }
    bool tryLockForRead(int timeout)
    {
        ++countUsage;

        if(QReadWriteLock::tryLockForRead()) {
            return true;
        }

        ++countLocked;
        qDebug()<<name<<"tryLockForRead locked";


        elapsed.restart();
        bool res=QReadWriteLock::tryLockForRead(timeout);
        qint64 t = elapsed.elapsed();

        countLockedTime+=t;

        if(!res) {
            qDebug()<<name<<"tryLockForRead timeout"<<timeout<<t;
        } else {
            if(t>maxLockedTime)
                maxLockedTime=t;
        }

        return res;
    }

    void lockForWrite()
    {
        ++countUsage;

        if(!QReadWriteLock::tryLockForWrite()) {
            ++countLocked;
            qDebug()<<name<<"lockForWrite locked";

            elapsed.restart();
            QReadWriteLock::lockForWrite();
            qint64 t = elapsed.elapsed();

            countLockedTime+=t;
            if(t>maxLockedTime)
                maxLockedTime=t;
        }
    }

    bool tryLockForWrite()
    {
        ++countUsage;

        if(!QReadWriteLock::tryLockForWrite()) {
            ++countLocked;
            qDebug()<<name<<"tryLockForWrite failed";
            return false;
        }
        return true;
    }

    bool tryLockForWrite(int timeout)
    {
        ++countUsage;

        if(QReadWriteLock::tryLockForWrite()) {
            return true;
        }

        ++countLocked;
        qDebug()<<name<<"tryLockForWrite locked";


        elapsed.restart();
        bool res=QReadWriteLock::tryLockForWrite(timeout);
        qint64 t = elapsed.elapsed();

        countLockedTime+=t;

        if(!res) {
            qDebug()<<name<<"tryLockForWrite timeout"<<timeout<<t;
        } else {
            if(t>maxLockedTime)
                maxLockedTime=t;
        }

        return res;
    }

    void unlock()
    {
        QReadWriteLock::unlock();
    }

    QString name;

private:
    Q_DISABLE_COPY(DebugReadWriteLock)

    int countUsage;
    int countLocked;
    qint64 countLockedTime;
    qint64 maxLockedTime;
    QElapsedTimer elapsed;
};

#endif

#endif // DEBUGREADWRITELOCK_H
