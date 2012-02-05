#include "msgcontroller.h"
#include "msghandler.h"

MsgController::MsgController()
{
}

MsgController::~MsgController()
{
    foreach(MsgHandler* h, listObj) {
        h->msgCtrl=0;
    }
}
