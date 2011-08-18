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

#ifndef MUTEXDEBUG_H
#define MUTEXDEBUG_H

#ifndef QT_NO_DEBUG

#include "precomp.h"
#define DMutex DebugMutex

class DebugMutex : public QMutex
{
public:
    DebugMutex(RecursionMode mode = NonRecursive) :
        QMutex(mode),
        countUsage(0),
        countLocked(0)
    {}

    ~DebugMutex()
    {
        LOGSIMPLE("mutex usage : "<<countUsage<<" locks"<<countLocked);
    }

    void lock()
    {
        ++countUsage;

        if(!QMutex::tryLock()) {
            ++countLocked;
            LOGSIMPLE("wait lock");
            QMutex::lock();
        }
    }

    bool tryLock()
    {
        ++countUsage;

        if(!QMutex::tryLock()) {
            ++countLocked;
            LOGSIMPLE("trylock failed");
            return false;
        }
        return true;
    }

    bool tryLock(int timeout)
    {
        ++countUsage;

        if(!QMutex::tryLock()) {
            ++countLocked;
            LOGSIMPLE("trylock failed");
        }

        if(!QMutex::tryLock(timeout)) {
            LOGSIMPLE("trylock timeout"<<timeout);
            return false;
        }
        return true;
    }

    void unlock()
    {
        QMutex::unlock();
    }

private:
    Q_DISABLE_COPY(DebugMutex)

    int countUsage;
    int countLocked;
};

#else
    #define DMutex QMutex
#endif

#endif // MUTEXDEBUG_H
