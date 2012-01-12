#include "configdialogvst.h"
#include "../common/ui_configdialog.h"
#include "mainhost.h"

ConfigDialogVst::ConfigDialogVst(Settings *settings, MainHost *myHost, QWidget *parent) :
    ConfigDialog(settings, myHost, parent)
{
    //ui->groupSampleFormat->setVisible(false);
    ui->sampleRate->setVisible(false);
    ui->labelsampleRate->setVisible(false);
}
