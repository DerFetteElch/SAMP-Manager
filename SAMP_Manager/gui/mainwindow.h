#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../packets.h"

#include <QString>
#include <QStringList>
#include <QDataStream>
#include <QTreeWidgetItem>
#include <QComboBox>
#include <QSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QTimer>
#include <QListWidgetItem>
#include <QDateTime>

#include "gui/subgui/filecreatedir.h"
#include "gui/subgui/filetransfer.h"
#include "gui/subgui/changepassword.h"
#include "gui/subgui/showfile.h"

#include <QDebug>

namespace Ui {
    class MainWindow;
}

class FileData{
    public:
        int size;
        int parts;
        QString fileName;
        QFile* file;
};

class MainWindow : public QMainWindow{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

    public slots:
        void logined();
        void sendResponse(QByteArray data);

    signals:
        void guiClosed();
        void logout();
        void send(QByteArray);
        void versionError();

    private:
        Ui::MainWindow *ui;

        FileCreateDir* createDirDialog;
        FileTransfer* fileTransferDialog;
        ChangePassword* changePasswordDialog;
        ShowFile* showFileDialog;

        QMap<QTreeWidgetItem*,int> serverList;

        QList<QComboBox*> comboboxGamemodes;
        QList<QSpinBox*> spinboxGamemodes;

        QStringList gamemodes;

        QMap<QTreeWidgetItem*,QString> filterscriptList;
        QMap<QTreeWidgetItem*,QString> pluginList;

        QMap<QListWidgetItem*,int> newsIDs;
        QMap<QListWidgetItem*,QString> newsHeadline;
        QMap<QListWidgetItem*,int> newsTime;
        QMap<QListWidgetItem*,QString> newsText;


        QMap<QTreeWidgetItem*,QString> fileNames;
        QMap<QTreeWidgetItem*,bool> fileFolder;

        FileData dataFile;
        bool showFile;

        QTimer *timer;

    private slots:
        void update();
        void dialogClose();

        void on_tabWidget_currentChanged(int index);

        void on_serverList_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

        void on_accountLogout_clicked();
        void on_accountEditPassword_clicked();

        void on_startButton_clicked();
        void on_stopButton_clicked();
        void on_restartButton_clicked();

        void on_comboBox_Gamemode1_currentIndexChanged(int index);
        void on_comboBox_Gamemode2_currentIndexChanged(int index);
        void on_comboBox_Gamemode3_currentIndexChanged(int index);
        void on_comboBox_Gamemode4_currentIndexChanged(int index);
        void on_comboBox_Gamemode5_currentIndexChanged(int index);
        void on_comboBox_Gamemode6_currentIndexChanged(int index);
        void on_comboBox_Gamemode7_currentIndexChanged(int index);

        void on_saveButton_clicked();

        void changePassword(QString oldPassword,QString newPassword);

        void on_files_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

        void createDir(QString name);
        void on_files_doubleClicked(const QModelIndex &index);
        void on_deleteButton_clicked();

        void on_createDirButton_clicked();
        void on_downloadButton_clicked();

        void on_uploadButton_clicked();
        void on_editButton_clicked();

        void on_sendButton_clicked();

        void on_newsList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    protected:
        bool event(QEvent* e);
};

#endif // MAINWINDOW_H
