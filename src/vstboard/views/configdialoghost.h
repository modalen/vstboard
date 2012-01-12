#ifndef CONFIGDIALOGHOST_H
#define CONFIGDIALOGHOST_H

#include "views/configdialog.h"

class ConfigDialogHost : public ConfigDialog
{
public:
    ConfigDialogHost(Settings *settings,MainHost *myHost=0, QWidget *parent = 0);

public slots:
    void accept();
};

#endif // CONFIGDIALOGHOST_H
