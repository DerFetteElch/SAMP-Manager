#include "adminwindow.h"
#include "ui_adminwindow.h"

AdminWindow::AdminWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::AdminWindow){
    ui->setupUi(this);

    addServerDialog=new AdminAddServer();
    connect(addServerDialog,SIGNAL(addServer(int,int,int)),this,SLOT(addServer(int,int,int)));
    connect(addServerDialog,SIGNAL(guiClosed()),this,SLOT(dialogClose()));

    addUserDialog=new AdminAddUser();
    connect(addUserDialog,SIGNAL(addUser(QString,QString)),this,SLOT(addUser(QString,QString)));
    connect(addUserDialog,SIGNAL(guiClosed()),this,SLOT(dialogClose()));

    addNewsDialog=new AdminAddNews();
    connect(addNewsDialog,SIGNAL(addNews(QString,QString)),this,SLOT(addNews(QString,QString)));
    connect(addNewsDialog,SIGNAL(guiClosed()),this,SLOT(dialogClose()));
}

AdminWindow::~AdminWindow(){
    delete ui;
}

void AdminWindow::sendResponse(QByteArray data){
    if(isHidden()) return;
    QDataStream stream(&data,QIODevice::ReadOnly);
    int id;
    stream>>id;
    if(id==PACKET_SC_GET_FIRST_DATA){
        QString userName;
        stream>>userName;

        serverList.clear();
        ui->serverList->clear();
        userList.clear();
        userNames.clear();
        userIds.clear();
        ui->userList->clear();

        QList<QByteArray> dataList;
        stream>>dataList;
        for(int i=0;i<dataList.count();i++){
            QByteArray dataBlock=dataList.at(i);
            QDataStream dat(&dataBlock,QIODevice::ReadOnly);
            int serverId,port;
            dat>>serverId>>port;
            QStringList list;
            list<<QString("%1").arg(serverId);
            list<<QString("%1").arg(port);
            QTreeWidgetItem* item=new QTreeWidgetItem(list);
            serverList.insert(item,serverId);
            ui->serverList->addTopLevelItem(item);
        }
        QList<QByteArray> dataList2;
        stream>>dataList2;
        for(int i=0;i<dataList2.count();i++){
            QByteArray dataBlock=dataList2.at(i);
            QDataStream dat(&dataBlock,QIODevice::ReadOnly);
            int newsId,time;
            QString headline,text;
            dat>>newsId>>headline>>time>>text;

            QListWidgetItem* item=new QListWidgetItem(headline);
            newsIDs.insert(item,newsId);
            newsHeadline.insert(item,headline);
            newsText.insert(item,text);
            ui->newsList->addItem(item);

            if(i==0) ui->newsList->setCurrentItem(item);
        }

        int admin;
        stream>>admin;
        if(admin==1){
            QList<QByteArray> dataList2;
            stream>>dataList2;
            for(int i=0;i<dataList2.count();i++){
                QByteArray dataBlock=dataList2.at(i);
                QDataStream dat(&dataBlock,QIODevice::ReadOnly);
                int userId,admin;
                QString name;
                dat>>userId>>name>>admin;
                ui->serverOwner->addItem(name);
                userNames.insert(userId,name);
                userIds.insert(name,userId);
                QStringList list;
                list<<QString("%1").arg(userId);
                list<<name;
                if(admin==1){
                    list<<tr("Yes");
                }else{
                    list<<tr("No");
                }
                QTreeWidgetItem* item=new QTreeWidgetItem(list);
                userList.insert(item,userId);
                ui->userList->addTopLevelItem(item);
            }
        }else{
            close();
        }
    }else if(id==PACKET_SC_ADMIN_ADD_SERVER_FAIL_OWNER){
        addServerDialog->errorMsg(tr("Owner Fail!"));
    }else if(id==PACKET_SC_ADMIN_ADD_SERVER_FAIL_PORT){
        addServerDialog->errorMsg(tr("Port Fail!"));
    }else if(id==PACKET_SC_ADMIN_ADD_SERVER_FAIL_MAXPLAYER){
        addServerDialog->errorMsg(tr("Max Players Fail!"));
    }else if(id==PACKET_SC_ADMIN_ADD_SERVER_FAIL_MAXNPC){
        addServerDialog->errorMsg(tr("Max NPCs Fail!"));
    }else if(id==PACKET_SC_ADMIN_ADD_SERVER_FAIL_PORT_EXISTS){
        addServerDialog->errorMsg(tr("Port exists!"));
    }else if(id==PACKET_SC_NEW_SERVER){
        int serverId,port;
        stream>>serverId>>port;
        QStringList list;
        list<<QString("%1").arg(serverId);
        list<<QString("%1").arg(port);
        QTreeWidgetItem* item=new QTreeWidgetItem(list);
        serverList.insert(item,serverId);
        ui->serverList->addTopLevelItem(item);
        if(addServerDialog->isAddServer()) addServerDialog->close();
    }else if(id==PACKET_SC_ADMIN_GET_SERVERINFO){
        int owner,port,maxPlayers,maxNPCs;
        stream>>owner>>port>>maxPlayers>>maxNPCs;

        ui->serverOwner->setCurrentIndex(ui->serverOwner->findText(userNames.value(owner)));
        ui->serverPort->setValue(port);
        ui->serverMaxPlayers->setValue(maxPlayers);
        ui->serverMaxNPCs->setValue(maxNPCs);
    }else if(id==PACKET_SC_CHANGE_SERVER){
        int serverId,port;
        stream>>serverId>>port;
        QMapIterator<QTreeWidgetItem*,int> i(serverList);
        while(i.hasNext()){
            i.next();
            if(i.value()==serverId){
                i.key()->setText(1,QString("%1").arg(port));
                break;
            }
        }
    }else if(id==PACKET_SC_ADMIN_CHANGE_SERVER_FAIL_OWNER){
        ui->statusbar->showMessage(tr("Server-Edit Owner Fail!"),5000);
    }else if(id==PACKET_SC_ADMIN_CHANGE_SERVER_FAIL_PORT){
        ui->statusbar->showMessage(tr("Server-Edit Port Fail!"),5000);
    }else if(id==PACKET_SC_ADMIN_CHANGE_SERVER_FAIL_MAXPLAYER){
        ui->statusbar->showMessage(tr("Server-Edit Max Players Fail!"),5000);
    }else if(id==PACKET_SC_ADMIN_CHANGE_SERVER_FAIL_MAXNPC){
        ui->statusbar->showMessage(tr("Server-Edit Max NPCs Fail!"),5000);
    }else if(id==PACKET_SC_ADMIN_CHANGE_SERVER_FAIL_PORT_EXISTS){
        ui->statusbar->showMessage(tr("Server-Edit Port exists!"),5000);
    }else if(id==PACKET_SC_DELETE_SERVER){
        int serverId;
        stream>>serverId;
        QMapIterator<QTreeWidgetItem*,int> i(serverList);
        while(i.hasNext()){
            i.next();
            if(i.value()==serverId){
                ui->serverList->takeTopLevelItem(ui->serverList->indexOfTopLevelItem(i.key()));
                serverList.remove(i.key());
                delete(i.key());
                break;
            }
        }
    }else if(id==PACKET_SC_ADMIN_ADD_USER_FAIL){
        addUserDialog->errorMsg(tr("Username exists!"));
    }else if(id==PACKET_SC_NEW_USER){
        int userId;
        QString name;
        stream>>userId>>name;
        ui->serverOwner->addItem(name);
        userNames.insert(userId,name);
        userIds.insert(name,userId);
        QStringList list;
        list<<QString("%1").arg(userId);
        list<<QString("%1").arg(name);
        list<<tr("No");
        QTreeWidgetItem* item=new QTreeWidgetItem(list);
        userList.insert(item,userId);
        ui->userList->addTopLevelItem(item);
        if(addUserDialog->isAddUser()) addUserDialog->close();
    }else if(id==PACKET_CS_ADMIN_GET_USERINFO){
        int admin,banned;
        QString name;
        bool self;
        stream>>name>>admin>>banned>>self;
        ui->userName->setText(name);
        ui->userPassword->setText("*****");
        if(admin==1){
            ui->userAdmin->setChecked(true);
        }else{
            ui->userAdmin->setChecked(false);
        }
        if(banned==1){
            ui->userBanned->setChecked(true);
        }else{
            ui->userBanned->setChecked(false);
        }
        if(self){
            ui->editUserButton->setEnabled(false);
            ui->deleteUserButton->setEnabled(false);
        }else{
            ui->editUserButton->setEnabled(true);
            ui->deleteUserButton->setEnabled(true);
        }
    }else if(id==PACKET_SC_CHANGE_USER){
        int userId,admin;
        stream>>userId>>admin;
        QMapIterator<QTreeWidgetItem*,int> i(userList);
        while(i.hasNext()){
            i.next();
            if(i.value()==userId){
                if(admin==1){
                    i.key()->setText(2,tr("Yes"));
                }else{
                    i.key()->setText(2,tr("No"));
                }
                break;
            }
        }
    }else if(id==PACKET_SC_DELETE_USER){
        int userId;
        stream>>userId;
        QMapIterator<QTreeWidgetItem*,int> i(userList);
        while(i.hasNext()){
            i.next();
            if(i.value()==userId){
                ui->userList->takeTopLevelItem(ui->userList->indexOfTopLevelItem(i.key()));
                userList.remove(i.key());
                delete(i.key());
                break;
            }
        }
        userIds.remove(userNames.value(userId));
        userNames.remove(userId);
        ui->serverOwner->clear();

        QMapIterator<int,QString> j(userNames);
        while(j.hasNext()){
            j.next();
            ui->serverOwner->addItem(j.value());
        }
    }else if(id==PACKET_SC_NEW_NEWS){
        int newsId,time;
        QString headline,text;
        stream>>newsId>>headline>>time>>text;
        QListWidgetItem* item=new QListWidgetItem(headline);
        newsIDs.insert(item,newsId);
        newsHeadline.insert(item,headline);
        newsText.insert(item,text);
        ui->newsList->insertItem(0,item);

        if(addNewsDialog->isAddNews()) addNewsDialog->close();
    }else if(id==PACKET_SC_CHANGE_NEWS){
        int newsId;
        QString headline,text;
        stream>>newsId>>headline>>text;
        QMapIterator<QListWidgetItem*,int> i(newsIDs);
        while(i.hasNext()){
            i.next();
            if(i.value()==newsId){
                i.key()->setText(headline);
                newsHeadline.insert(i.key(),headline);
                newsText.insert(i.key(),text);
                if(ui->newsList->currentItem()==i.key()){
                    ui->newsHeadline->setText(headline);
                    ui->newsText->setPlainText(text);
                }
                break;
            }
        }
    }else if(id==PACKET_SC_DELETE_NEWS){
        int newsId;
        stream>>newsId;
        QMapIterator<QListWidgetItem*,int> i(newsIDs);
        while(i.hasNext()){
            i.next();
            if(i.value()==newsId){
                ui->newsList->removeItemWidget(i.key());
                newsIDs.remove(i.key());
                newsHeadline.remove(i.key());
                newsText.remove(i.key());
                delete(i.key());
                break;
            }
        }
    }
}

void AdminWindow::on_tabWidget_currentChanged(int index){
    //ui->tabWidget
}

void AdminWindow::dialogClose(){
    setEnabled(true);
}

void AdminWindow::on_addServerButton_clicked(){
    addServerDialog->show();
    setEnabled(false);
}
void AdminWindow::addServer(int port, int maxPlayers, int maxNPCs){
    QByteArray data;
    QDataStream stream(&data,QIODevice::WriteOnly);
    stream<<PACKET_CS_ADMIN_ADD_SERVER;
    stream<<port;
    stream<<maxPlayers;
    stream<<maxNPCs;
    emit(send(data));
}

void AdminWindow::on_serverList_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous){
    int serverId=serverList.value(current);

    QByteArray data;
    QDataStream stream(&data,QIODevice::WriteOnly);
    stream<<PACKET_CS_ADMIN_GET_SERVERINFO;
    stream<<serverId;
    emit(send(data));
}

void AdminWindow::on_editServerButton_clicked(){
    QByteArray data;
    QDataStream stream(&data,QIODevice::WriteOnly);
    stream<<PACKET_CS_ADMIN_CHANGE_SERVER;
    stream<<serverList.value(ui->serverList->currentItem());
    stream<<userIds.value(ui->serverOwner->currentText());
    stream<<ui->serverPort->value();
    stream<<ui->serverMaxPlayers->value();
    stream<<ui->serverMaxNPCs->value();
    emit(send(data));
}
void AdminWindow::on_deleteServerButton_clicked(){
    QByteArray data;
    QDataStream stream(&data,QIODevice::WriteOnly);
    stream<<PACKET_CS_ADMIN_DELETE_SERVER;
    stream<<serverList.value(ui->serverList->currentItem());
    emit(send(data));
}


void AdminWindow::on_userList_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous){
    int userId=userList.value(current);

    QByteArray data;
    QDataStream stream(&data,QIODevice::WriteOnly);
    stream<<PACKET_CS_ADMIN_GET_USERINFO;
    stream<<userId;
    emit(send(data));
}
void AdminWindow::on_addUserButton_clicked(){
    addUserDialog->show();
    setEnabled(false);
}
void AdminWindow::on_editUserButton_clicked(){
    QByteArray data;
    QDataStream stream(&data,QIODevice::WriteOnly);
    stream<<PACKET_CS_ADMIN_CHANGE_USER;
    stream<<userList.value(ui->userList->currentItem());
    stream<<ui->userPassword->text();
    if(ui->userAdmin->isChecked()){
        stream<<1;
    }else{
        stream<<0;
    }
    if(ui->userBanned->isChecked()){
        stream<<1;
    }else{
        stream<<0;
    }
    emit(send(data));
}
void AdminWindow::on_deleteUserButton_clicked(){
    QByteArray data;
    QDataStream stream(&data,QIODevice::WriteOnly);
    stream<<PACKET_SC_ADMIN_DELETE_USER;
    stream<<userList.value(ui->userList->currentItem());
    emit(send(data));
}
void AdminWindow::addUser(QString name, QString password){
    QByteArray data;
    QDataStream stream(&data,QIODevice::WriteOnly);
    stream<<PACKET_SC_ADMIN_ADD_USER;
    stream<<name;
    stream<<password;
    emit(send(data));
}

bool AdminWindow::event(QEvent *e){
    if(e->type()==QEvent::Close){
        addServerDialog->close();
        addUserDialog->close();
        addNewsDialog->close();
        emit(guiClosed());
    }
    return QMainWindow::event(e);
}

void AdminWindow::on_newsList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous){
    ui->newsHeadline->setText(newsHeadline.value(current,""));
    ui->newsText->setPlainText(newsText.value(current,""));
}
void AdminWindow::on_addNewsButton_clicked(){
    addNewsDialog->show();
    setEnabled(false);
}
void AdminWindow::on_editNewsButton_clicked(){
    QByteArray data;
    QDataStream stream(&data,QIODevice::WriteOnly);
    stream<<PACKET_CS_ADMIN_CHANGE_NEWS;
    stream<<newsIDs.value(ui->newsList->currentItem(),0);
    stream<<ui->newsHeadline->text();
    stream<<ui->newsText->toPlainText();
    emit(send(data));
}
void AdminWindow::on_deleteNewsButton_clicked(){
    QByteArray data;
    QDataStream stream(&data,QIODevice::WriteOnly);
    stream<<PACKET_CS_ADMIN_DELETE_NEWS;
    stream<<newsIDs.value(ui->newsList->currentItem(),0);
    emit(send(data));
}
void AdminWindow::addNews(QString headline, QString text){
    QByteArray data;
    QDataStream stream(&data,QIODevice::WriteOnly);
    stream<<PACKET_CS_ADMIN_ADD_NEWS;
    stream<<headline;
    stream<<text;
    emit(send(data));
}
