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

#ifndef DEBUGSEMAPHORE_H
#define DEBUGSEMAPHORE_H

#ifndef DEBUG_SEMAPHORE
    #define DSemaphore QSemaphore
    #define SET_SEMAPHORE_NAME(SEM,NAME);
#else

#include <QDebug>
#include "precomp.h"
#define DSemaphore DebugSemaphore
#define SET_SEMAPHORE_NAME(SEM,NAME); SEM.name=NAME;

class DebugSemaphore : public QSemaphore
{
public:
    DebugSemaphore(int n=0) :
        QSemaphore(n),
        countUsage(0),
        countLocked(0),
        countLockedTime(0),
        maxLockedTime(0)
    {
        name=QString("%1").arg((long)this,16);
    }

    ~DebugSemaphore()
    {
#ifdef DEBUG_SEMAPHORE_REPORT_UNUSED
        if(countUsage==0) {
            qDebug()<<name<<"unused semaphore";
        }
#endif
        if(countUsage!=0) {
            qDebug()<<name<<"semaphore usage:"<<countUsage
                      <<"locks:"<<countLocked
                      <<"time locked:"<<countLockedTime
                      <<"maxTimeLock:"<<maxLockedTime;
        }
    }

    bool tryAcquire(int n, int timeout)
    {
        ++countUsage;

        if(available()>=n) {
            QSemaphore::acquire(n);
            return true;
        }

        ++countLocked;

        elapsed.restart();
        bool res=QSemaphore::tryAcquire(n,timeout);
        qint64 t = elapsed.elapsed();

        countLockedTime+=t;

        if(!res) {
            qDebug()<<name<<"timeout"<<timeout<<t;
        } else {
            if(t>maxLockedTime)
                maxLockedTime=t;
        }

        return res;
    }

    QString name;

private:
    Q_DISABLE_COPY(DebugSemaphore)

    int countUsage;
    int countLocked;
    qint64 countLockedTime;
    qint64 maxLockedTime;
    QElapsedTimer elapsed;
};
#endif

#endif // DEBUGSEMAPHORE_H
