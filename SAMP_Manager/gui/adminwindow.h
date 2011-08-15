#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QMainWindow>
#include "../packets.h"

#include <QDataStream>
#include <QString>
#include <QStringList>
#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include <QMap>

#include "gui/subgui/adminaddserver.h"
#include "gui/subgui/adminadduser.h"
#include "gui/subgui/adminaddnews.h"

//#include <QDebug>

namespace Ui {
    class AdminWindow;
}

class AdminWindow : public QMainWindow{
    Q_OBJECT

    public:
        explicit AdminWindow(QWidget *parent = 0);
        ~AdminWindow();

    public slots:
        void sendResponse(QByteArray data);

    signals:
        void guiClosed();
        void send(QByteArray);

    private slots:
        void on_tabWidget_currentChanged(int index);

        void dialogClose();

        void on_serverList_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
        void on_editServerButton_clicked();
        void on_deleteServerButton_clicked();
        void on_addServerButton_clicked();

        void addServer(int port, int maxPlayers, int maxNPCs);

        void on_userList_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
        void on_addUserButton_clicked();
        void on_editUserButton_clicked();
        void on_deleteUserButton_clicked();

        void addUser(QString name,QString password);

        void on_newsList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
        void on_addNewsButton_clicked();
        void on_editNewsButton_clicked();
        void on_deleteNewsButton_clicked();

        void addNews(QString headline,QString text);

    private:
        Ui::AdminWindow *ui;

        AdminAddServer* addServerDialog;
        AdminAddUser* addUserDialog;
        AdminAddNews* addNewsDialog;

        QMap<QTreeWidgetItem*,int> serverList;

        QMap<QTreeWidgetItem*,int> userList;
        QMap<int,QString> userNames;
        QMap<QString,int> userIds;

        QMap<QListWidgetItem*,int> newsIDs;
        QMap<QListWidgetItem*,QString> newsHeadline;
        QMap<QListWidgetItem*,QString> newsText;

    protected:
        bool event(QEvent* e);
};

#endif // ADMINWINDOW_H
