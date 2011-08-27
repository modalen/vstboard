/**************************************************************************
#    Copyright 2010-2011 Rapha�l Fran�ois
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

#include "mainhost.h"
#include "pathsolver.h"
#include "globals.h"
#include "projectfile/projectfile.h"
#include "connectables/objectfactory.h"
#include "connectables/buffer.h"

PathSolver::PathSolver(MainHost *parent) :
    QObject(parent),
    myHost(parent)
{
    connect(this,SIGNAL(GlobalDelayChanged(long)),
            myHost,SIGNAL(DelayChanged(long)));
}

PathSolver::~PathSolver()
{
    Clear();
}

void PathSolver::Clear()
{
    mutex.lock();
    foreach(SolverNode* line, listNodes) {
        delete line;
    }
    listNodes.clear();
    mutex.unlock();

}

void PathSolver::Resolve(hashCables cables, Renderer *renderer)
{
    Clear();

    if(cables.size()==0) {
        return;
    }

    mutex.lock();

    listCables = cables;

    CreateNodes();
    PutParentsInNodes();

    int cpt=0;

    UnwrapLoops();

    ResetDelays();
    cpt=0;
    while(AddDelays() && cpt<100) { ++cpt; }
    cpt=0;
    while(SynchronizeAudioOutputs() && cpt<100) { ++cpt; }

    cpt=0;
    while(ChainNodes() && cpt<100) { ++cpt; }

    RemoveUnusedNodes();

    SetMinAndMaxStep();
    renderer->OnNewRenderingOrder(listNodes);
    mutex.unlock();
}



void PathSolver::GetListPinsConnectedTo(ConnectionInfo out, QList<ConnectionInfo> &list)
{
    hashCables::const_iterator i = listCables.constFind(out);
    while (i != listCables.constEnd()  && i.key() == out) {
        list << i.value();
        ++i;
    }
}


/*!
  Create nodes for each objects
  */
void PathSolver::CreateNodes()
{
    const Connectables::hashObjects listObjects = myHost->objFactory->GetListObjects();
    Connectables::hashObjects::const_iterator i = listObjects.constBegin();
    while(i!=listObjects.constEnd()) {
        QSharedPointer<Connectables::Object> objPtr = i.value();

        //don't add parked objects
        if(!objPtr.isNull() && !objPtr->parked) {
//            if(objPtr->info().nodeType!=NodeType::bridge && objPtr->info().nodeType!=NodeType::container) {
            if( objPtr->info().nodeType!=NodeType::container) {
                SolverNode *node = new SolverNode();
                listNodes << node;
                node->listOfObj << objPtr;
                objPtr->SetSolverNode(node);
            }
        }
        ++i;
    }
}

/*!
  List all childs and parents in each node
  */
void PathSolver::PutParentsInNodes()
{
    foreach(SolverNode *node,listNodes) {
        foreach(QSharedPointer<Connectables::Object> parent,GetListParents(node->listOfObj.first())) {
            if(!parent.isNull())// && parent->info().nodeType!=NodeType::bridge)
                node->AddParent( parent->GetSolverNode() );
        }
//        foreach(QSharedPointer<Connectables::Object> child,GetListChildren(node->listOfObj.last())) {
//            if(!child.isNull())// && child->info().nodeType!=NodeType::bridge)
//                node->listChilds << child->GetSolverNode();
//        }
    }

//    foreach(SolverNode *node,listNodes) {
//        //no parents, no childs : don't render it
//        if(node->listChilds.isEmpty() && node->listParents.isEmpty()) {
//            listNodes.removeAll(node);
//            delete node;
//        }
//    }
}

/*!
    Group nodes with only one parent or child
    \return true if a node was grouped
    */
bool PathSolver::ChainNodes()
{
    foreach(SolverNode* node, listNodes) {
        if(node->listChilds.count() == 1) {
            SolverNode *childNode = node->listChilds.first();
            if( childNode->listParents.count() == 1) {
                if(childNode->MergeWithParentNode()) {
                    listNodes.removeAll(childNode);
                    delete childNode;
                    return true;
                }
            }
        }
    }

    return false;
}

void PathSolver::RemoveUnusedNodes()
{
    foreach(SolverNode *node, listNodes) {
        bool onlyBridges=true;
        foreach(QSharedPointer<Connectables::Object>objPtr,node->listOfObj) {
            if(objPtr->info().nodeType!=NodeType::bridge) {
                onlyBridges=false;
                break;
            }
        }
        if(onlyBridges) {
            foreach(SolverNode *child, node->listChilds) {
                child->listParents.removeAll(node);
                child->listParents << node->listParents;
            }
            foreach(SolverNode *parent, node->listParents) {
                parent->listChilds.removeAll(node);
                parent->listChilds << node->listChilds;
            }
            listNodes.removeAll(node);
            delete node;
        }
    }
}

void PathSolver::UnwrapLoops()
{
    //unwrap loops
    foreach(SolverNode *node,listNodes) {
        foreach(SolverNode *n,listNodes) {
            n->ResetLoopFlags();
        }

        //find loop
        QList<SolverNode*>loop;
        if(node->DetectLoopback(loop)) {

            //find the node close to a root
            QList<SolverNode*>listStarts = ListOfGoodStarts(loop);

            SolverNode* bestStart=0;

            //we have a winner
            if(listStarts.size()==1)
                bestStart = listStarts.first();

            //it's a draw... try to find the best one
            if(listStarts.size()>1)
                listStarts = BestStartsInAList(loop,listStarts);

            //now we have a winner
            if(listStarts.size()==1)
                bestStart = listStarts.first();

            //a draw, again. i don't know what else to check : take the first in the list
            if(listStarts.size()>1)
                bestStart = listStarts.first();

            if(bestStart) {
                //opened the loop : no more feedback
                QList<SolverNode*>openedLoop = OpenLoop(loop, bestStart);

                //create a copy of the loop
                QList<SolverNode*>copiedLoop = CopyNodesChain(openedLoop);

                //insert the copied loop before the original one
                openedLoop.first()->ReconnectParentsTo(copiedLoop.first());
                copiedLoop.last()->AddChild(openedLoop.first());

            } else {
                Q_ASSERT(false);
                //bestEnd not found : wtf ?
            }
        }
    }
}

/*!
  Remove delays
  */
void PathSolver::ResetDelays()
{
    foreach(SolverNode *node, listNodes) {
        QSharedPointer<Connectables::Object>obj = node->listOfObj.last();
        if(obj->info().objType == ObjType::Buffer) {
            obj.staticCast<Connectables::Buffer>()->SetDelay(0);
        }
    }
}

/*!
  Add delays at the end of nodes to keep in sync
  */
bool PathSolver::AddDelays()
{
    LOG("ADD DELAYS");
    foreach(SolverNode *node, listNodes) {
        if(!node->IsRoot())
            continue;
        node->UpdateInitialDelay();
    }

    foreach(SolverNode *node, listNodes) {
        long delay = node->GetParentMaxDelay();
        if(SynchronizeParentNodes(node,delay))
            return true;
    }
    return false;
}

bool PathSolver::SynchronizeParentNodes(SolverNode *node, long targetDelay)
{
    foreach(SolverNode *parent, node->listParents) {
        long delayToAdd = targetDelay - parent->totalDelayAtOutput;

        if(delayToAdd>0) {
            LOG("add delay"<<delayToAdd<<parent->listOfObj.first().toStrongRef()->objectName()<<node->listOfObj.first().toStrongRef()->objectName());
            QSharedPointer<Connectables::Object>obj = parent->listOfObj.last().toStrongRef();
            if(obj->info().objType==ObjType::Buffer) {

                obj.staticCast<Connectables::Buffer>()->SetDelay( delayToAdd+obj->initialDelay );
                return true;
            } else {
                CreateDelayNode(parent, node, delayToAdd);
                return true;
            }
        }
    }
    return false;
}

bool PathSolver::SynchronizeAudioOutputs()
{
    LOG("SYNC AUDIO");
    foreach(SolverNode *node, listNodes) {
        if(!node->IsRoot())
            continue;
        node->UpdateInitialDelay();
    }

    //get the maximum delay at audio out
    long newDelay=0L;
    foreach(SolverNode *node, listNodes) {
        if(node->totalDelayAtOutput>newDelay) {
            //only for nodes containing audio output
            foreach(QSharedPointer<Connectables::Object>obj, node->listOfObj) {
                if(obj->info().objType==ObjType::AudioInterfaceOut)
                    newDelay=node->totalDelayAtOutput;
            }
        }
    }

    //set an equal delay on all outputs
    foreach(SolverNode *node, listNodes) {
        bool isOutput=false;
        foreach(QSharedPointer<Connectables::Object>obj, node->listOfObj) {
            if(obj->info().objType==ObjType::AudioInterfaceOut) {
//                if(node->totalDelayAtOutput<newDelay)
                    isOutput=true;
            }
        }
        if(isOutput) {
            if(SynchronizeParentNodes(node,newDelay)) {
                return true;
            }
        }
    }

    //report new delay to the host
    if(newDelay!=globalDelay) {
        globalDelay=newDelay;
        emit GlobalDelayChanged(globalDelay);
    }

    return false;
}

/*!
  Insert a delay node between node and chilnode
  */
void PathSolver::CreateDelayNode(SolverNode *node, SolverNode *childNode, long delay)
{
    QSharedPointer<Connectables::Object>outObj = node->listOfObj.last();
    QSharedPointer<Connectables::Object>inObj = childNode->listOfObj.first();
    QSharedPointer<Connectables::Container>container = myHost->objFactory->GetObjectFromId(outObj->GetContainerId()).staticCast<Connectables::Container>();


    node->RemoveChild(childNode);
    childNode->totalDelayAtOutput+=delay;
//    LOG("add obj"<<delayObj->objectName());

    //find affected cables
    foreach(Connectables::Pin* outPin, outObj->GetListAudioPinOut()->listPins) {
        ConnectionInfo outInfo = outPin->GetConnectionInfo();
        QList<ConnectionInfo>lstIn;
        GetListPinsConnectedTo(outInfo, lstIn);
        foreach(ConnectionInfo inInfo, lstIn) {
            if(inObj->GetIndex() == inInfo.objId) {
//                //found a cable !
//                LOG("add delay"<<delay<<outObj->objectName()<<inObj->objectName());
                SolverNode *newNode = new SolverNode();
                node->AddChild(newNode);
                newNode->AddChild(childNode);
                listNodes<<newNode;
                ObjectInfo info;
                info.nodeType = NodeType::object;
                info.objType = ObjType::Buffer;
                info.initDelay = delay;
                QSharedPointer<Connectables::Buffer>delayObj = myHost->objFactory->NewObject(info).staticCast<Connectables::Buffer>();
                newNode->listOfObj << delayObj;
                container->UserAddObject(delayObj);
                delayObj->SetSolverNode(node);
                container->UserAddCable(outInfo, delayObj->GetListAudioPinIn()->GetPin(0)->GetConnectionInfo());
                container->UserAddCable(delayObj->GetListAudioPinOut()->GetPin(0)->GetConnectionInfo(), inInfo);
                container->UserRemoveCable(outInfo, inInfo);
            }
        }
    }

    //can be a bridge too
    foreach(Connectables::Pin* outPin, outObj->GetListBridgePinOut()->listPins) {
        ConnectionInfo outInfo = outPin->GetConnectionInfo();
        QList<ConnectionInfo>lstIn;
        GetListPinsConnectedTo(outInfo, lstIn);
        foreach(ConnectionInfo inInfo, lstIn) {
            if(inObj->GetIndex() == inInfo.objId) {
//                //found a cable !
//                LOG("add delay"<<delay<<outObj->objectName()<<inObj->objectName());
                SolverNode *newNode = new SolverNode();
                node->AddChild(newNode);
                newNode->AddChild(childNode);
                listNodes<<newNode;
                ObjectInfo info;
                info.nodeType = NodeType::object;
                info.objType = ObjType::Buffer;
                info.initDelay = delay;
                QSharedPointer<Connectables::Buffer>delayObj = myHost->objFactory->NewObject(info).staticCast<Connectables::Buffer>();
                newNode->listOfObj << delayObj;
                container->UserAddObject(delayObj);
                delayObj->SetSolverNode(node);
                container->UserAddCable(outInfo, delayObj->GetListAudioPinIn()->GetPin(0)->GetConnectionInfo());
                container->UserAddCable(delayObj->GetListAudioPinOut()->GetPin(0)->GetConnectionInfo(), inInfo);
                container->UserRemoveCable(outInfo, inInfo);
            }
        }
    }
}

/*!
  Find the rendering window, minimum and maximum rendering step
  */
void PathSolver::SetMinAndMaxStep()
{
    int maxStep=0;
    foreach(SolverNode *node, listNodes) {
        if(!node->IsRoot())
            continue;
        maxStep = std::max(maxStep, node->SetMinRenderOrder( 0 ));
    }

    foreach(SolverNode *node, listNodes) {
        if(!node->IsTail())
            continue;
        node->SetMaxRenderOrder(maxStep);
    }
}

//return a list of good starts by looking at the nodes close to a root
QList<SolverNode*> PathSolver::ListOfGoodStarts(const QList<SolverNode*>&loop)
{
    QList<SolverNode*> bestStarts;
    int minStep=99999;

    foreach(SolverNode *node, loop) {
        foreach(SolverNode *n,listNodes) {
            n->ResetLoopFlags();
        }
        int steps=0;

        if(node->DistanceFromRoot(steps)) {
            //detect equality
            if(steps==minStep) {
                bestStarts << node;
            }
            //this is a new best
            if(steps<minStep) {
                bestStarts.clear();
                minStep=steps;
                bestStarts << node;
            }
        }
    }

    return bestStarts;
}

//return the best start of a loop by finding the ending node close to a tail
QList<SolverNode*> PathSolver::BestStartsInAList(const QList<SolverNode*>&loop, const QList<SolverNode*>&possibleStarts)
{
    QList<SolverNode*> bestStarts;
    int minStep=99999;

    foreach(SolverNode *triedStart, possibleStarts) {
        //which node would be the end one in this case
        SolverNode *triedEnd;
        if(triedStart == loop.first())
            triedEnd = loop.last();
        else {
            triedEnd = loop.at( loop.indexOf(triedStart)-1 );
        }

        foreach(SolverNode *n,listNodes) {
            n->ResetLoopFlags();
        }
        int steps=0;

        if(triedEnd->DistanceFromTail(steps)) {
            if(steps==minStep) {
                bestStarts << triedStart;
            }
            if(steps<minStep) {
                bestStarts.clear();
                bestStarts << triedStart;
                minStep=steps;
            }
        }
    }

    return bestStarts;
}

//copy a chain of nodes
QList<SolverNode*> PathSolver::CopyNodesChain(const QList<SolverNode*>&chain)
{
    QList<SolverNode*>copiedChain;
    SolverNode *parentNode=0;
    foreach(SolverNode *node, chain) {
        SolverNode *copiedNode = new SolverNode();
        listNodes << copiedNode;
        copiedNode->listOfObj = node->listOfObj;

        if(parentNode)
            parentNode->AddChild(copiedNode);

        parentNode = copiedNode;
        copiedChain << copiedNode;
    }
    return copiedChain;
}

//cut a loop before the selected start
QList<SolverNode*> PathSolver::OpenLoop(const QList<SolverNode*>&loop, SolverNode* startingNode)
{
    QList<SolverNode*>destLoop;

    //rotate the loop, it must start with startingNode
    int pos=loop.indexOf(startingNode);
    for(int i=0;i<loop.size();i++) {
        if(pos==loop.size())
            pos=0;
        destLoop << loop.at(pos);
        ++pos;
    }

    //remove connections between last and first to break the feedback
    destLoop.first()->RemoveParent(destLoop.last());

    return destLoop;
}


//get the objects connected to the input pins
QList< QSharedPointer<Connectables::Object> >PathSolver::GetListParents( QSharedPointer<Connectables::Object> objPtr)
{
    QList< QSharedPointer<Connectables::Object> >listParents;

    hashCables::iterator i = listCables.begin();
    while (i != listCables.end()) {
        QSharedPointer<Connectables::Object> parentPtr = myHost->objFactory->GetObjectFromId(i.key().objId);
        if(!parentPtr.isNull()) {
            if(i.value().objId == objPtr->GetIndex()) {
                if(!listParents.contains(parentPtr)) {
                    listParents << parentPtr;
//                    if(parentPtr->info().nodeType == NodeType::bridge)
//                        GetListParentsOfBridgePin(i.key(),listParents);
                }
            }
        }
        ++i;
    }

    return listParents;
}

//void PathSolver::GetListParentsOfBridgePin(const ConnectionInfo &info, QList< QSharedPointer<Connectables::Object> > &listParents)
//{
//    hashCables::iterator i = listCables.begin();
//    while (i != listCables.end()) {
//        QSharedPointer<Connectables::Object> parentPtr = myHost->objFactory->GetObjectFromId(i.key().objId);
//        if(!parentPtr.isNull()) {
//            if(i.value().objId == info.objId && i.value().pinNumber == info.pinNumber && i.value().type == info.type) {
//                if(!listParents.contains(parentPtr)) {
//                    listParents << parentPtr;
//                    if(parentPtr->info().nodeType == NodeType::bridge)
//                        GetListParentsOfBridgePin(i.key(),listParents);
//                }
//            }
//        }
//        ++i;
//    }
//}

//get the objects connected to the output pins
QList< QSharedPointer<Connectables::Object> >PathSolver::GetListChildren( QSharedPointer<Connectables::Object> objPtr)
{
    QList< QSharedPointer<Connectables::Object> >listChildren;

    hashCables::iterator i = listCables.begin();
    while (i != listCables.end()) {
        QSharedPointer<Connectables::Object> childPtr = myHost->objFactory->GetObjectFromId(i.value().objId);
        if(!childPtr.isNull()) {
            if(i.key().objId == objPtr->GetIndex()) {
                if(!listChildren.contains(childPtr)) {
                    listChildren << childPtr;
//                    if(childPtr->info().nodeType == NodeType::bridge)
//                        GetListChildrenOfBridgePin(i.value(),listChildren);
                }
            }
        }
        ++i;
    }

    return listChildren;
}

//void PathSolver::GetListChildrenOfBridgePin(const ConnectionInfo &info, QList< QSharedPointer<Connectables::Object> > &listChildren)
//{
//    hashCables::iterator i = listCables.begin();
//    while (i != listCables.end()) {
//        QSharedPointer<Connectables::Object> childPtr = myHost->objFactory->GetObjectFromId(i.value().objId);
//        if(!childPtr.isNull()) {
//            if(i.key().objId == info.objId && i.key().pinNumber == info.pinNumber && i.key().type == info.type) {
//                if(!listChildren.contains(childPtr)) {
//                    listChildren << childPtr;
//                    if(childPtr->info().nodeType == NodeType::bridge) {
//                        GetListChildrenOfBridgePin(i.value(),listChildren);
//                    }
//                }
//            }
//        }
//        ++i;
//    }
//}
