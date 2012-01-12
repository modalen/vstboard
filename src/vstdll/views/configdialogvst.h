#ifndef CONFIGDIALOGVST_H
#define CONFIGDIALOGVST_H

#include "views/configdialog.h"

class ConfigDialogVst : public ConfigDialog
{
public:
    ConfigDialogVst(Settings *settings, MainHost *myHost=0, QWidget *parent = 0);
};

#endif // CONFIGDIALOGVST_H
