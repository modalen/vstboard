#ifndef CURSOR_H
#define CURSOR_H

#include "msghandler.h"

class MainHost;
class Cursor : public QObject, public MsgHandler
{
    Q_OBJECT
public:
    Cursor(MainHost *host, float value);
    void ReceiveMsg(const MsgObject &msg);
    inline float GetValue() {return value;}
    void SetValue(float val);

signals:
    void valueChanged(float val);

private:
    float value;
};

#endif // CURSOR_H
