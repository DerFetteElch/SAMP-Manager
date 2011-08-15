#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow),showFile(false){
    ui->setupUi(this);

    createDirDialog=new FileCreateDir();
    connect(createDirDialog,SIGNAL(createDir(QString)),this,SLOT(createDir(QString)));
    connect(createDirDialog,SIGNAL(guiClosed()),this,SLOT(dialogClose()));

    changePasswordDialog=new ChangePassword();
    connect(changePasswordDialog,SIGNAL(changePassword(QString,QString)),this,SLOT(changePassword(QString,QString)));
    connect(changePasswordDialog,SIGNAL(guiClosed()),this,SLOT(dialogClose()));

    fileTransferDialog=new FileTransfer();
    showFileDialog=new ShowFile();
    connect(showFileDialog,SIGNAL(guiClosed()),this,SLOT(dialogClose()));

    comboboxGamemodes<<ui->comboBox_Gamemode1;
    comboboxGamemodes<<ui->comboBox_Gamemode2;
    comboboxGamemodes<<ui->comboBox_Gamemode3;
    comboboxGamemodes<<ui->comboBox_Gamemode4;
    comboboxGamemodes<<ui->comboBox_Gamemode5;
    comboboxGamemodes<<ui->comboBox_Gamemode6;
    comboboxGamemodes<<ui->comboBox_Gamemode7;
    comboboxGamemodes<<ui->comboBox_Gamemode8;

    spinboxGamemodes<<ui->spinBox_Gamemode1;
    spinboxGamemodes<<ui->spinBox_Gamemode2;
    spinboxGamemodes<<ui->spinBox_Gamemode3;
    spinboxGamemodes<<ui->spinBox_Gamemode4;
    spinboxGamemodes<<ui->spinBox_Gamemode5;
    spinboxGamemodes<<ui->spinBox_Gamemode6;
    spinboxGamemodes<<ui->spinBox_Gamemode7;
    spinboxGamemodes<<ui->spinBox_Gamemode8;

    timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(update()));
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::logined(){
    show();

    QByteArray data;
    QDataStream stream(&data,QIODevice::WriteOnly);
    stream<<PACKET_CS_GET_FIRST_DATA;
    emit(send(data));
    timer->start(5000);
}

void MainWindow::update(){
    if(serverList.value(ui->serverList->currentItem(),0)!=0){
        QByteArray data;
        QDataStream out(&data,QIODevice::WriteOnly);
        out<<PACKET_CS_GET_STATUS;
        out<<serverList.value(ui->serverList->currentItem());
        emit(send(data));
    }
}

void MainWindow::dialogClose(){
    setEnabled(true);
}

void MainWindow::sendResponse(QByteArray data){
    QDataStream stream(&data,QIODevice::ReadOnly);
    int id;
    stream>>id;
    qDebug()<<"--->"<<id;

    if(id==PACKET_SC_HELLO){
        int vers;
        stream>>vers;
        if(vers!=SYSTEM_VERSION){
            QMessageBox::critical(this,tr("Error"),tr("The client is outdated!\nPlease contact the admin!"));
            emit(guiClosed());
        }
    }else if(id==PACKET_SC_LOGOUT){
        emit(logout());
    }else if(id==PACKET_SC_GET_FIRST_DATA){
        QString userName;
        stream>>userName;
        ui->accountName->setText(userName);

        serverList.clear();
        ui->serverList->clear();

        QList<QByteArray> dataList;
        stream>>dataList;
        if(dataList.count()==0){
            QTreeWidgetItem* item=new QTreeWidgetItem(QStringList()<<tr("No Server")<<"");
            serverList.insert(item,0);
            ui->serverList->addTopLevelItem(item);
            ui->tabWidget->setCurrentIndex(0);
            //ui->tab->setEnabled(false);
        }else{
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
            if(serverList.count()>0){
                QMapIterator<QTreeWidgetItem*,int> i(serverList);
                i.next();
                ui->serverList->setCurrentItem(i.key());
            }
        }
        QList<QByteArray> dataList2;
        stream>>dataList2;
        if(dataList.count()==0){
            QListWidgetItem* item=new QListWidgetItem(tr("No News"));
            ui->newsList->addItem(item);
        }else{
            for(int i=0;i<dataList2.count();i++){
                QByteArray dataBlock=dataList2.at(i);
                QDataStream dat(&dataBlock,QIODevice::ReadOnly);
                int newsId,time;
                QString headline,text;
                dat>>newsId>>headline>>time>>text;

                QListWidgetItem* item=new QListWidgetItem(headline);
                newsIDs.insert(item,newsId);
                newsHeadline.insert(item,headline);
                newsTime.insert(item,time);
                newsText.insert(item,text);
                ui->newsList->addItem(item);

                if(i==0) ui->newsList->setCurrentItem(item);
            }
        }
        ui->tabWidget->setEnabled(true);
    }else if(id==PACKET_SC_NEW_SERVER){
        int serverId,port;
        stream>>serverId>>port;
        QStringList list;
        list<<QString("%1").arg(serverId);
        list<<QString("%1").arg(port);
        if(serverList.count()==1){
            QMapIterator<QTreeWidgetItem*,int> i(serverList);
            i.next();
            if(i.value()==0){
                serverList.clear();
                ui->serverList->clear();
            }
        }
        QTreeWidgetItem* item=new QTreeWidgetItem(list);
        serverList.insert(item,serverId);
        ui->serverList->addTopLevelItem(item);
        ui->tabWidget->setEnabled(true);
        if(ui->serverList->currentItem()==0){
            ui->serverList->setCurrentItem(item);
        }
    }else if(id==PACKET_SC_CHANGE_SERVER){
        int serverId,port;
        stream>>serverId>>port;
        QMapIterator<QTreeWidgetItem*,int> i(serverList);
        while(i.hasNext()){
            i.next();
            if(i.value()==serverId){
                i.key()->setText(1,QString("%1").arg(port));
                if(i.key()==ui->serverList->currentItem()){
                    QByteArray data;
                    QDataStream out(&data,QIODevice::WriteOnly);
                    out<<PACKET_CS_GET_SERVERINFO;
                    out<<serverId;
                    emit(send(data));
                }
                break;
            }
        }
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
        if(serverList.count()==0){
            QTreeWidgetItem* item=new QTreeWidgetItem(QStringList()<<tr("No Server")<<"");
            serverList.insert(item,0);
            ui->serverList->addTopLevelItem(item);
            ui->tabWidget->setCurrentIndex(0);
            ui->tabWidget->setEnabled(false);
        }
    }else if(id==PACKET_SC_NEW_NEWS){
        int newsId,time;
        QString headline,text;
        stream>>newsId>>headline>>time>>text;

        QListWidgetItem* item=new QListWidgetItem(headline);
        newsIDs.insert(item,newsId);
        newsHeadline.insert(item,headline);
        newsTime.insert(item,time);
        newsText.insert(item,text);

        if(newsIDs.count()==0){
            ui->newsList->clear();
            ui->newsList->addItem(item);
            ui->newsList->setCurrentItem(item);
        }else{
            ui->newsList->insertItem(0,item);
            ui->newsList->setCurrentItem(item);
        }
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
                    QString html;
                    html.append("<center>\n");
                    html.append(QString("  <h1>%1</h1>\n").arg(headline));
                    html.append(QString("  %1\n").arg(QDateTime::fromTime_t(newsTime.value(ui->newsList->currentItem())).toString("dd.MM.yyyy hh:mm:ss")));
                    html.append("</center>\n");
                    html.append("<hr/>\n");
                    html.append(text);
                    ui->news->setHtml(html);
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
                if(ui->newsList->currentItem()==i.key())
                newsIDs.remove(i.key());
                newsHeadline.remove(i.key());
                newsText.remove(i.key());
                newsTime.remove(i.key());
                delete(i.key());
                break;
            }
        }
    }else if(id==PACKET_SC_GET_SERVERINFO){
        int status,port;
        QString owner,serverName,gamemode,map,player;
        stream>>status>>port>>owner>>serverName>>gamemode>>map>>player;
        if(status==1){
            ui->lineEdit_status->setText(tr("Online"));
            ui->startButton->setEnabled(false);
            ui->stopButton->setEnabled(true);
            ui->restartButton->setEnabled(true);
        }else{
            ui->lineEdit_status->setText(tr("Offline"));
            ui->startButton->setEnabled(true);
            ui->stopButton->setEnabled(false);
            ui->restartButton->setEnabled(false);
        }
        ui->lineEdit_Port->setText(QString("%1").arg(port));
        ui->lineEdit_Owner->setText(owner);
        ui->lineEdit_name->setText(serverName);
        ui->lineEdit_Gamemode->setText(gamemode);
        ui->lineEdit_map->setText(map);
        ui->lineEdit_Player->setText(player);
    }else if(id==PACKET_SC_GET_SERVERSETTINGS){

        comboboxGamemodes.at(0)->clear();
        for(int i=1;i<8;i++){
            comboboxGamemodes.at(i)->clear();
            comboboxGamemodes.at(i)->addItem(tr("No Gamemode"));
            comboboxGamemodes.at(i)->setEnabled(false);
            spinboxGamemodes.at(i)->setValue(0);
            spinboxGamemodes.at(i)->setEnabled(false);
        }

        QStringList plugins,filterscripts;
        stream>>gamemodes>>plugins>>filterscripts;

        int announce;
        QString hostname,weburl,rconPassword,serverPassword,mapname;
        stream>>announce>>hostname>>weburl>>rconPassword>>serverPassword>>mapname;

        if(announce==1){
            ui->checkBox_announce->setChecked(true);
        }else{
            ui->checkBox_announce->setChecked(false);
        }
        ui->lineEdit_Hostname->setText(hostname);
        ui->lineEdit_Weburl->setText(weburl);
        ui->lineEdit_RconPassword->setText(rconPassword);
        ui->lineEdit_ServerPassword->setText(serverPassword);
        ui->lineEdit_Mapname->setText(mapname);

        QStringList activeGamemodes;
        QList<int> gamemodeNums;
        stream>>activeGamemodes>>gamemodeNums;

        QStringList tmp=gamemodes;
        for(int i=0;i<8;i++){
            if(activeGamemodes.at(i).isEmpty() || !tmp.contains(activeGamemodes.at(i))){
                QStringList items;
                items<<tr("No Gamemode");
                for(int j=0;j<tmp.count();j++){
                    items<<tmp.at(j);
                }
                comboboxGamemodes.at(i)->clear();
                comboboxGamemodes.at(i)->addItems(items);
                break;
            }else{
                QStringList items;
                items<<tr("No Gamemode");
                for(int j=0;j<tmp.count();j++){
                    items<<tmp.at(j);
                }
                comboboxGamemodes.at(i)->clear();
                comboboxGamemodes.at(i)->addItems(items);
                comboboxGamemodes.at(i)->setCurrentIndex(comboboxGamemodes.at(i)->findText(activeGamemodes.at(i)));
                spinboxGamemodes.at(i)->setValue(gamemodeNums.at(i));
                tmp.removeOne(activeGamemodes.at(i));
                if(i<7){
                    comboboxGamemodes.at(i+1)->setEnabled(true);
                    spinboxGamemodes.at(i+1)->setEnabled(true);
                }
            }
        }


        QMap<QString,int> activeFilterscripts;
        stream>>activeFilterscripts;
        filterscriptList.clear();
        ui->filterscripts->clear();
        for(int i=0;i<filterscripts.count();i++){
            QTreeWidgetItem *item=new QTreeWidgetItem(QStringList()<<""<<filterscripts.at(i));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            if(activeFilterscripts.value(filterscripts.at(i),0)==1){
                item->setCheckState(0,Qt::Checked);
            }else{
                item->setCheckState(0,Qt::Unchecked);
            }
            filterscriptList.insert(item,filterscripts.at(i));
            ui->filterscripts->addTopLevelItem(item);
        }

        QMap<QString,int> activePlugin;
        stream>>activePlugin;
        pluginList.clear();
        ui->plugins->clear();
        for(int i=0;i<plugins.count();i++){
            QTreeWidgetItem *item=new QTreeWidgetItem(QStringList()<<""<<plugins.at(i));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            if(activePlugin.value(plugins.at(i),0)==1){
                item->setCheckState(0,Qt::Checked);
            }else{
                item->setCheckState(0,Qt::Unchecked);
            }
            pluginList.insert(item,plugins.at(i));
            ui->plugins->addTopLevelItem(item);
        }
    }else if(id==PACKET_SC_GET_STATUS){
        int status;
        QString serverName,gamemode,map,player;
        stream>>status>>serverName>>gamemode>>map>>player;
        if(status==1){
            ui->lineEdit_status->setText(tr("Online"));
            ui->startButton->setEnabled(false);
            ui->stopButton->setEnabled(true);
            ui->restartButton->setEnabled(true);
        }else{
            ui->lineEdit_status->setText(tr("Offline"));
            ui->startButton->setEnabled(true);
            ui->stopButton->setEnabled(false);
            ui->restartButton->setEnabled(false);
        }
        ui->lineEdit_name->setText(serverName);
        ui->lineEdit_Gamemode->setText(gamemode);
        ui->lineEdit_map->setText(map);
        ui->lineEdit_Player->setText(player);
    }else if(id==PACKET_SC_CHANGE_PASSWORD){
        changePasswordDialog->close();
    }else if(id==PACKET_SC_CHANGE_PASSWORD_WRONG){
        changePasswordDialog->errorMsg(tr("Wrong Password!"));
    }else if(id==PACKET_SC_FILE_LIST){
        fileNames.clear();
        fileFolder.clear();
        ui->files->clear();
        ui->downloadButton->setEnabled(false);
        ui->editButton->setEnabled(false);

        bool newFolder;
        stream>>newFolder;
        ui->createDirButton->setEnabled(newFolder);
        QList<QByteArray> dataList;
        stream>>dataList;
        for(int i=0;i<dataList.count();i++){
            QByteArray dataBlock=dataList.at(i);
            QDataStream dat(&dataBlock,QIODevice::ReadOnly);
            QString name;
            bool folder;
            int size;
            dat>>name>>folder>>size;
            QTreeWidgetItem* item;
            if(size==0){
                item=new QTreeWidgetItem(QStringList()<<name<<"");
            }else{
                QString sizeString;
                if(size>1024*2){
                    if(size>1024*1024*2){
                        sizeString=QString("%1 MB").arg(size/1024/1024);
                    }else{
                        sizeString=QString("%1 KB").arg(size/1024);
                    }
                }else{
                    sizeString=QString("%1 B").arg(size);
                }
                item=new QTreeWidgetItem(QStringList()<<name<<sizeString);
            }
            fileNames.insert(item,name);
            fileFolder.insert(item,folder);
            ui->files->addTopLevelItem(item);
        }
        bool upload;
        stream>>upload;
        ui->uploadButton->setEnabled(upload);
    }else if(id==PACKET_SC_FILE_CD || id==PACKET_SC_FILE_CREATE_DIR){
        QByteArray data;
        QDataStream out(&data,QIODevice::WriteOnly);
        out<<PACKET_CS_FILE_LIST;
        out<<serverList.value(ui->serverList->currentItem());
        emit(send(data));
    }else if(id==PACKET_SC_FILE_DELETE){
        bool result;
        stream>>result;
        if(result){
            QByteArray dat;
            QDataStream out(&dat,QIODevice::WriteOnly);
            out<<PACKET_CS_FILE_LIST;
            out<<serverList.value(ui->serverList->currentItem());
            emit(send(dat));
        }else{
            ui->statusBar->showMessage(tr("Directory not empty!"),5000);
        }
    }else if(id==PACKET_SC_FILE_GET_FILE){
        int size,parts;
        QString fileName;
        stream>>size>>parts>>fileName;
        dataFile.size=size;
        dataFile.parts=parts;

        QString fn;
        if(showFile){
            fn="tmp";
        }else{
            fn=QFileDialog::getSaveFileName(this,tr("Save As..."),fileName,tr("All Files (*)"));
        }
        if(!fn.isEmpty()){
            fileTransferDialog->setParts(parts);
            fileTransferDialog->show();
            setEnabled(false);

            dataFile.fileName=fn;
            dataFile.file=new QFile(fn);
            if(dataFile.file->open(QIODevice::WriteOnly)){
                QByteArray dat;
                QDataStream out(&dat,QIODevice::WriteOnly);
                out<<PACKET_CS_FILE_GET_FILE_PART;
                out<<serverList.value(ui->serverList->currentItem());
                out<<1;
                emit(send(dat));
            }
        }
    }else if(id==PACKET_SC_FILE_GET_FILE_PART){
        int part;
        QByteArray dat;
        stream>>part>>dat;

        dataFile.file->write(dat);

        fileTransferDialog->setPart(part);
        if(fileTransferDialog->isCanceled()){
            fileTransferDialog->close();
            setEnabled(true);
            dataFile.file->close();
            delete(dataFile.file);
            QFile::remove(dataFile.fileName);

            QByteArray dat;
            QDataStream out(&dat,QIODevice::WriteOnly);
            out<<PACKET_CS_FILE_GET_FILE_COMPLETE;
            emit(send(dat));
        }else{
            if(part<dataFile.parts){
                QByteArray dat;
                QDataStream out(&dat,QIODevice::WriteOnly);
                out<<PACKET_CS_FILE_GET_FILE_PART;
                out<<serverList.value(ui->serverList->currentItem());
                out<<part+1;
                emit(send(dat));
            }else{
                dataFile.file->flush();
                dataFile.file->close();
                fileTransferDialog->close();
                setEnabled(true);
                QByteArray dat;
                QDataStream out(&dat,QIODevice::WriteOnly);
                out<<PACKET_CS_FILE_GET_FILE_COMPLETE;
                emit(send(dat));
                if(showFile){
                    showFileDialog->show();
                    dataFile.file->open(QIODevice::ReadOnly);
                    showFileDialog->setText(QString(dataFile.file->readAll()));
                    dataFile.file->close();
                    setEnabled(false);
                }
                delete(dataFile.file);
            }
        }
    }else if(id==PACKET_SC_FILE_PUT_FILE_PART){
        int part;
        stream>>part;
        fileTransferDialog->setPart(part);
        if(fileTransferDialog->isCanceled()){
            fileTransferDialog->close();
            setEnabled(true);
            dataFile.file->close();
            delete(dataFile.file);

            QByteArray dat;
            QDataStream out(&dat,QIODevice::WriteOnly);
            out<<PACKET_CS_FILE_PUT_FILE_CANCEL;
            out<<serverList.value(ui->serverList->currentItem());
            emit(send(dat));
        }else{
            if(part<=dataFile.parts){
                QByteArray dat;
                QDataStream out(&dat,QIODevice::WriteOnly);
                out<<PACKET_CS_FILE_PUT_FILE_PART;
                out<<serverList.value(ui->serverList->currentItem());
                out<<part;
                QByteArray dat2=dataFile.file->read(1024);
                out<<dat2;
                emit(send(dat));
            }else{
                dataFile.file->close();
                delete(dataFile.file);
                fileTransferDialog->close();
                setEnabled(true);
                QByteArray dat;
                QDataStream out(&dat,QIODevice::WriteOnly);
                out<<PACKET_CS_FILE_PUT_FILE_COMPLETE;
                out<<serverList.value(ui->serverList->currentItem());
                emit(send(dat));
            }
        }
    }else if(id==PACKET_SC_FILE_PUT_FILE_COMPLETE || id==PACKET_SC_FILE_PUT_FILE_FAIL){
        dataFile.file->close();
        delete(dataFile.file);
        fileTransferDialog->close();
        setEnabled(true);
    }else if(id==PACKET_SC_RCON){
        QString dat;
        stream>>dat;
        qDebug()<<"---";
        ui->rconConsole->appendPlainText(dat);
    }
    data.clear();
}

bool MainWindow::event(QEvent *e){
    if(e->type()==QEvent::Close){
        createDirDialog->close();
        fileTransferDialog->close();
        changePasswordDialog->close();
        showFileDialog->close();
        emit(guiClosed());
    }
    return QMainWindow::event(e);
}

void MainWindow::on_tabWidget_currentChanged(int index){
    if(serverList.value(ui->serverList->currentItem(),0)==0) return;
    switch(index){
        case 2:{
            QByteArray data;
            QDataStream out(&data,QIODevice::WriteOnly);
            out<<PACKET_CS_GET_SERVERSETTINGS;
            out<<serverList.value(ui->serverList->currentItem());
            emit(send(data));
            break;
        }case 3:{
            QByteArray data;
            QDataStream out(&data,QIODevice::WriteOnly);
            out<<PACKET_CS_FILE_LIST;
            out<<serverList.value(ui->serverList->currentItem());
            emit(send(data));
            break;
        }
    }
}

void MainWindow::on_serverList_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous){
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    out<<PACKET_CS_GET_SERVERINFO;
    out<<serverList.value(current);
    emit(send(data));
}

void MainWindow::on_accountLogout_clicked(){
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    out<<PACKET_CS_LOGOUT;
    emit(send(data));
}
void MainWindow::on_accountEditPassword_clicked(){
    changePasswordDialog->show();
    setEnabled(false);
}

void MainWindow::changePassword(QString oldPassword, QString newPassword){
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    out<<PACKET_CS_CHANGE_PASSWORD;
    out<<oldPassword;
    out<<newPassword;
    emit(send(data));
}

void MainWindow::on_startButton_clicked(){
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    out<<PACKET_CS_START_SERVER;
    out<<serverList.value(ui->serverList->currentItem());
    emit(send(data));
}
void MainWindow::on_stopButton_clicked(){
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    out<<PACKET_CS_STOP_SERVER;
    out<<serverList.value(ui->serverList->currentItem());
    emit(send(data));
}
void MainWindow::on_restartButton_clicked(){
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    out<<PACKET_CS_RESTART_SERVER;
    out<<serverList.value(ui->serverList->currentItem());
    emit(send(data));
}


void MainWindow::on_comboBox_Gamemode1_currentIndexChanged(int index){
    if(index==0){
        for(int i=1;i<8;i++){
            comboboxGamemodes.at(i)->setEnabled(false);
            spinboxGamemodes.at(i)->setEnabled(false);
        }
    }else{
        QStringList tmp=gamemodes;
        tmp.removeOne(comboboxGamemodes.at(0)->currentText());
        comboboxGamemodes.at(1)->clear();
        comboboxGamemodes.at(1)->addItem(tr("No Gamemode"));
        comboboxGamemodes.at(1)->addItems(tmp);
        comboboxGamemodes.at(1)->setEnabled(true);
        spinboxGamemodes.at(1)->setEnabled(true);
    }
}
void MainWindow::on_comboBox_Gamemode2_currentIndexChanged(int index){
    if(index==0){
        for(int i=2;i<8;i++){
            comboboxGamemodes.at(i)->setEnabled(false);
            spinboxGamemodes.at(i)->setEnabled(false);
        }
    }else{
        QStringList tmp=gamemodes;
        for(int i=0;i<2;i++) tmp.removeOne(comboboxGamemodes.at(i)->currentText());
        comboboxGamemodes.at(2)->clear();
        comboboxGamemodes.at(2)->addItem(tr("No Gamemode"));
        comboboxGamemodes.at(2)->addItems(tmp);
        comboboxGamemodes.at(2)->setEnabled(true);
        spinboxGamemodes.at(2)->setEnabled(true);
    }
}
void MainWindow::on_comboBox_Gamemode3_currentIndexChanged(int index){
    if(index==0){
        for(int i=3;i<8;i++){
            comboboxGamemodes.at(i)->setEnabled(false);
            spinboxGamemodes.at(i)->setEnabled(false);
        }
    }else{
        QStringList tmp=gamemodes;
        for(int i=0;i<3;i++) tmp.removeOne(comboboxGamemodes.at(i)->currentText());
        comboboxGamemodes.at(3)->clear();
        comboboxGamemodes.at(3)->addItem(tr("No Gamemode"));
        comboboxGamemodes.at(3)->addItems(tmp);
        comboboxGamemodes.at(3)->setEnabled(true);
        spinboxGamemodes.at(3)->setEnabled(true);
    }
}
void MainWindow::on_comboBox_Gamemode4_currentIndexChanged(int index){
    if(index==0){
        for(int i=4;i<8;i++){
            comboboxGamemodes.at(i)->setEnabled(false);
            spinboxGamemodes.at(i)->setEnabled(false);
        }
    }else{
        QStringList tmp=gamemodes;
        for(int i=0;i<4;i++) tmp.removeOne(comboboxGamemodes.at(i)->currentText());
        comboboxGamemodes.at(4)->clear();
        comboboxGamemodes.at(4)->addItem(tr("No Gamemode"));
        comboboxGamemodes.at(4)->addItems(tmp);
        comboboxGamemodes.at(4)->setEnabled(true);
        spinboxGamemodes.at(4)->setEnabled(true);
    }

}
void MainWindow::on_comboBox_Gamemode5_currentIndexChanged(int index){
    if(index==0){
        for(int i=5;i<8;i++){
            comboboxGamemodes.at(i)->setEnabled(false);
            spinboxGamemodes.at(i)->setEnabled(false);
        }
    }else{
        QStringList tmp=gamemodes;
        for(int i=0;i<5;i++) tmp.removeOne(comboboxGamemodes.at(i)->currentText());
        comboboxGamemodes.at(5)->clear();
        comboboxGamemodes.at(5)->addItem(tr("No Gamemode"));
        comboboxGamemodes.at(5)->addItems(tmp);
        comboboxGamemodes.at(5)->setEnabled(true);
        spinboxGamemodes.at(5)->setEnabled(true);
    }

}
void MainWindow::on_comboBox_Gamemode6_currentIndexChanged(int index){
    if(index==0){
        for(int i=6;i<8;i++){
            comboboxGamemodes.at(i)->setEnabled(false);
            spinboxGamemodes.at(i)->setEnabled(false);
        }
    }else{
        QStringList tmp=gamemodes;
        for(int i=0;i<6;i++) tmp.removeOne(comboboxGamemodes.at(i)->currentText());
        comboboxGamemodes.at(6)->clear();
        comboboxGamemodes.at(6)->addItem(tr("No Gamemode"));
        comboboxGamemodes.at(6)->addItems(tmp);
        comboboxGamemodes.at(6)->setEnabled(true);
        spinboxGamemodes.at(6)->setEnabled(true);
    }

}
void MainWindow::on_comboBox_Gamemode7_currentIndexChanged(int index){
    if(index==0){
        for(int i=7;i<8;i++){
            comboboxGamemodes.at(i)->setEnabled(false);
            spinboxGamemodes.at(i)->setEnabled(false);
        }
    }else{
        QStringList tmp=gamemodes;
        for(int i=0;i<7;i++) tmp.removeOne(comboboxGamemodes.at(i)->currentText());
        comboboxGamemodes.at(7)->clear();
        comboboxGamemodes.at(7)->addItem(tr("No Gamemode"));
        comboboxGamemodes.at(7)->addItems(tmp);
        comboboxGamemodes.at(7)->setEnabled(true);
        spinboxGamemodes.at(7)->setEnabled(true);
    }

}


void MainWindow::on_saveButton_clicked(){
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    out<<PACKET_CS_CHANGE_SERVERSETTINGS;
    out<<serverList.value(ui->serverList->currentItem());
    if(ui->checkBox_announce->isChecked()){
        out<<1;
    }else{
        out<<0;
    }
    out<<ui->lineEdit_Hostname->text();
    out<<ui->lineEdit_Weburl->text();
    out<<ui->lineEdit_RconPassword->text();
    out<<ui->lineEdit_ServerPassword->text();
    out<<ui->lineEdit_Mapname->text();

    QStringList activeGamemodes;
    QList<int> gamemodeNums;
    for(int i=0;i<8;i++){
        activeGamemodes<<"";
        gamemodeNums<<0;
    }
    for(int i=0;i<8;i++){
        if(comboboxGamemodes.at(i)->currentIndex()==0) break;
        activeGamemodes[i]=comboboxGamemodes.at(i)->currentText();
        gamemodeNums[i]=spinboxGamemodes.at(i)->value();
    }
    out<<activeGamemodes<<gamemodeNums;

    QMap<QString,int> activeFilterscripts;
    QMapIterator<QTreeWidgetItem*,QString> i(filterscriptList);
    while(i.hasNext()){
        i.next();
        if(i.key()->checkState(0)==Qt::Checked){
            activeFilterscripts.insert(i.value(),1);
        }else{
            activeFilterscripts.insert(i.value(),0);
        }
    }
    out<<activeFilterscripts;

    QMap<QString,int> activePlugin;
    QMapIterator<QTreeWidgetItem*,QString> i2(pluginList);
    while(i2.hasNext()){
        i2.next();
        if(i2.key()->checkState(0)==Qt::Checked){
            activePlugin.insert(i2.value(),1);
        }else{
            activePlugin.insert(i2.value(),0);
        }
    }
    out<<activePlugin;
    emit(send(data));
}

void MainWindow::on_createDirButton_clicked(){
    createDirDialog->show();
    setEnabled(false);
}
void MainWindow::createDir(QString name){
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    out<<PACKET_CS_FILE_CREATE_DIR;
    out<<serverList.value(ui->serverList->currentItem());
    out<<name;
    emit(send(data));
}

void MainWindow::on_files_doubleClicked(const QModelIndex &index){
    QTreeWidgetItem* item=ui->files->currentItem();
    if(fileFolder.value(item)){
        QByteArray data;
        QDataStream out(&data,QIODevice::WriteOnly);
        out<<PACKET_CS_FILE_CD;
        out<<serverList.value(ui->serverList->currentItem());
        out<<fileNames.value(item);
        emit(send(data));
    }
}
void MainWindow::on_deleteButton_clicked(){
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    out<<PACKET_CS_FILE_DELETE;
    out<<serverList.value(ui->serverList->currentItem());
    out<<fileNames.value(ui->files->currentItem());
    emit(send(data));
}
void MainWindow::on_downloadButton_clicked(){
    showFile=false;
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    out<<PACKET_CS_FILE_GET_FILE;
    out<<serverList.value(ui->serverList->currentItem());
    out<<fileNames.value(ui->files->currentItem());
    emit(send(data));
}

void MainWindow::on_uploadButton_clicked(){
    QString fn=QFileDialog::getOpenFileName(this,tr("Open File"),QString(),tr("All Files (*)"));
    if(!fn.isEmpty()){
        QFileInfo info(fn);
        int size=info.size();
        if(size>=1024*1024*20){
            QMessageBox::critical(this,tr("Error"),tr("File is too large! (20MB max)"));
        }else{
            dataFile.size=size;
            if(size % 1024==0){
                dataFile.parts=size/1024;
            }else{
                dataFile.parts=size/1024+1;
            }
            dataFile.file=new QFile(fn);

            if(dataFile.file->open(QIODevice::ReadOnly)){
                QByteArray data;
                QDataStream out(&data,QIODevice::WriteOnly);
                out<<PACKET_CS_FILE_PUT_FILE;
                out<<serverList.value(ui->serverList->currentItem());
                out<<dataFile.size;
                out<<dataFile.parts;
                out<<info.fileName();
                emit(send(data));

                fileTransferDialog->setParts(dataFile.parts);
                fileTransferDialog->show();
                setEnabled(false);
            }else{
                delete(dataFile.file);
            }
        }
    }
}

void MainWindow::on_editButton_clicked(){
    showFile=true;
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    out<<PACKET_CS_FILE_GET_FILE;
    out<<serverList.value(ui->serverList->currentItem());
    out<<fileNames.value(ui->files->currentItem());
    emit(send(data));
}

void MainWindow::on_files_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous){
    bool file=!fileFolder.value(current);
    ui->downloadButton->setEnabled(file);
    ui->editButton->setEnabled(file);
}

void MainWindow::on_sendButton_clicked(){
    if(!ui->rconCommand->text().isEmpty()){
        QByteArray data;
        QDataStream out(&data,QIODevice::WriteOnly);
        out<<PACKET_CS_RCON;
        out<<serverList.value(ui->serverList->currentItem());
        out<<ui->rconCommand->text();
        ui->rconCommand->setText("");
        emit(send(data));
    }
}


void MainWindow::on_newsList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous){
    QString html;
    html.append("<center>\n");
    html.append(QString("  <h1>%1</h1>\n").arg(newsHeadline.value(current)));
    html.append(QString("  %1\n").arg(QDateTime::fromTime_t(newsTime.value(current)).toString("dd.MM.yyyy hh:mm:ss")));
    html.append("</center>\n");
    html.append("<hr/>\n");
    html.append(newsText.value(current));
    ui->news->setHtml(html);
}
