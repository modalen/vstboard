/******************************************************************************
#    Copyright 2010 Rapha�l Fran�ois
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
******************************************************************************/

#include "renderer.h"
#include "pathsolver.h"
#include "connectables/object.h"
#include "solvernode.h"

Renderer::Renderer(QObject *parent)
    : QThread(parent)
{

}

Renderer::~Renderer()
{
    Clear();
}

void Renderer::Clear()
{
    QMutexLocker mutx(&mutexRender);
    renderSteps.clear();
}

void Renderer::OnNewRenderingOrder(orderedNodes *newSteps)
{
    QMutexLocker mutx(&mutexRender);
    renderSteps.clear();

    orderedNodes::iterator i = newSteps->begin();
    while (i != newSteps->end()) {
        renderSteps.insert(i.key(),i.value()->objectPtr);
        ++i;
    }
}

void Renderer::StartRender()
{
    QMutexLocker mutx(&mutexRender);

    if(renderSteps.isEmpty())
        return;

    //reset counters
    orderedObjects::iterator i = renderSteps.begin();
    while (i != renderSteps.end()) {
        if(!i.value().isNull()) {
            QSharedPointer<Connectables::Object>objPtr = i.value().toStrongRef();
            if(!objPtr->GetSleep())
                objPtr->NewRenderLoop();
        } else {
            debug("Renderer::StartRender obj deleted")
        }
        ++i;
    }

    //render each node
    i = renderSteps.begin();
    while (i != renderSteps.end()) {
        QSharedPointer<Connectables::Object>objPtr = i.value().toStrongRef();
        if(!objPtr.isNull()) {
            if(!objPtr->GetSleep())
                objPtr->Render();
        }
        ++i;
    }
}

void Renderer::run()
{
    exec();
}