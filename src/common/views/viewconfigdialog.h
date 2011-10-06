#ifndef VIEWCONFIGDIALOG_H
#define VIEWCONFIGDIALOG_H

#include <QDialog>

//#include "precomp.h"
#include "mainhost.h"
#include "mainwindow.h"
#include "viewconfig.h"

namespace Ui {
    class ViewConfigDialog;
}

namespace View {

    class ViewConfigDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit ViewConfigDialog(MainHost *myHost, QWidget *parent = 0);
        ~ViewConfigDialog();

    private:
        void InitLists();
        void LoadPreset(const QString &presetName);
        bool UserWantsToUnloadPreset();
        void GetColorFromConf();
        void UpdateSliders();
        void DisableSliders();
        void SaveColor();

        void SaveChanges();
        void DiscardChanges();

        void UpdateProgramsFont();

        QColor currentColor;

        /// pointer to the dialog ui;
        Ui::ViewConfigDialog *ui;

        /// pointer to the MainHost
        MainHost *myHost;

        /// pointer to the config
        ViewConfig *conf;

        /// currently selected group
        ColorGroups::Enum currentGrp;

        /// currently selected color
        Colors::Enum currentCol;

        /// a copy of the original colors, restored if dialog is canceled
        viewConfigPreset backupColors;

        /// save in setup file, oriinal state
        bool backupSaveInSetup;

        /// set to true whan changes are made
        bool modified;

        bool StartUpdate();
        void EndUpdate();
        bool updateInProgress;

    public slots:
        void accept();
        void reject();
        void InitDialog();

    private slots:
        void onPickerColorSelected(const QColor &color);
        void onPickerHueSelected(const QColor &color);
        void on_listPalettes_itemClicked(QListWidgetItem* item);
        void on_listRoles_itemClicked(QListWidgetItem* item);
        void on_checkSavedInSetupFile_clicked(bool checked);
        void on_RedSpinBox_valueChanged(int );
        void on_GreenSpinBox_valueChanged(int );
        void on_BlueSpinBox_valueChanged(int );
        void on_AlphaSpinBox_valueChanged(int );
        void on_listPresets_clicked(const QModelIndex &index);
        void on_addPreset_clicked();
        void on_delPreset_clicked();
        void on_listPresets_itemChanged(QListWidgetItem *item);
        void on_fontProgItalic_clicked(bool checked);
        void on_fontProgStretch_valueChanged(double arg1);
        void on_fontProgSize_valueChanged(double arg1);
        void on_fontProgFamily_currentIndexChanged(const QString &arg1);
        void on_fontProgBold_clicked(bool checked);
    };
}
#endif // VIEWCONFIGDIALOG_H
