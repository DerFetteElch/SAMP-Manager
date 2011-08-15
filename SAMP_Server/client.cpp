#include "client.h"

Client::Client(QObject *parent) : QThread(parent){

}
Client::Client(int socketDescriptor, XMLParser* xml, SAMPServer* samp, QObject *parent) : QThread(parent),xml(xml),samp(samp),login(false){
    clientSocket=new QTcpSocket();
    clientSocket->setSocketDescriptor(socketDescriptor);
    cryptKey=new SimpleQtRC5::Key(QString(CRYPT_KEY));
}

void Client::run(){
    connect(clientSocket,SIGNAL(readyRead()),this,SLOT(readyRead()));
    connect(clientSocket,SIGNAL(disconnected()),this,SLOT(disconnected()));
    emit(output(1|4,QString("[%1] New Connection from %2 (ID %3)\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).arg(clientSocket->peerAddress().toString()).arg((int)this)));
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out<<PACKET_SC_HELLO;
    out<<SYSTEM_VERSION;
    send(block);
    exec();
}

void Client::send(QByteArray data){
    SimpleQtRC5::Encryptor e(cryptKey,SimpleQtRC5::RC5_32_32_20,SimpleQtRC5::ModeCFB,SimpleQtRC5::NoChecksum);
    QByteArray cipher;
    SimpleQtRC5::Error er=e.encrypt(data,cipher,false);
    if(er){
        emit(output(2|4,QString("[%1] ERROR Encryption failed (ID %2)\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).arg((int)this)));
    }else{
        clientSocket->write(cipher);
        clientSocket->flush();
        data.clear();
    }
}
QByteArray Client::read(){
    QByteArray data=clientSocket->readAll();
    QByteArray deCrypt;
    SimpleQtRC5::Decryptor d(cryptKey,SimpleQtRC5::RC5_32_32_20,SimpleQtRC5::ModeCFB);
    SimpleQtRC5::Error er=d.decrypt(data,deCrypt,false);
    if(er){
        emit(output(2|4,QString("[%1] ERROR Decryption failed (ID %2)\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).arg((int)this)));
        clientSocket->disconnectFromHost();
        return QByteArray();
    }else{
        return deCrypt;
    }
}

void Client::readyRead(){
    QByteArray readData=read();
    if(readData.isNull()) return;
    QDataStream in(readData);
    int id;
    in>>id;

    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    if(id==PACKET_CS_LOGIN){
        QString name, password;
        in>>name>>password;
        if(xml->getAttribute(QString("data/user/%1").arg(name),"password",QString(""))==QString(QCryptographicHash::hash(password.toAscii(),QCryptographicHash::Md5).toHex())){
            if(xml->getAttribute(QString("data/user/%1").arg(name),"banned",0)==0){
                login=true;
                userName=name;
                userId=xml->getAttribute(QString("data/user/%1").arg(name),"id",0);
                userAdmin=xml->getAttribute(QString("data/user/%1").arg(name),"admin",0);
                out<<PACKET_SC_LOGIN_SUCCESS;
                out<<userAdmin;
                emit(output(1|4,QString("[%1] Login -> Name:%2 (ID %3)\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).arg(name).arg((int)this)));
            }else{
                out<<PACKET_SC_LOGIN_BANNED;
            }
        }else{
            out<<PACKET_SC_LOGIN_FAIL;
        }
    }else if(login){
        if(id==PACKET_CS_LOGOUT){
            login=false;
            userId=0;
            userAdmin=0;
            userName=QString();

            fileManagerPath=QStringList();

            out<<PACKET_SC_LOGOUT;

        }else if(id==PACKET_CS_GET_FIRST_DATA){
            out<<PACKET_SC_GET_FIRST_DATA;
            out<<userName;
            QStringList list=xml->getElements("data/server");
            QList<QByteArray> data;
            for(int i=0;i<list.count();i++){
                if(userAdmin==1 || xml->getAttribute(QString("data/server/%1/owner").arg(list.at(i)),"value",0)==userId){
                    QByteArray dataBlock;
                    QDataStream dat(&dataBlock,QIODevice::WriteOnly);
                    dat<<list.at(i).split("_").at(1).toInt();
                    dat<<xml->getAttribute(QString("data/server/%1/port").arg(list.at(i)),"value",0);
                    data.append(dataBlock);
                }
            }
            out<<data;

            QStringList list2=xml->getElements("data/news");
            QList<QByteArray> data2;
            for(int i=list2.count()-1;i>=0;i--){
                QFile file(QString("news/%1.txt").arg(list2.at(i)));
                if(file.open(QIODevice::ReadOnly)){
                    QByteArray dataBlock;
                    QDataStream dat(&dataBlock,QIODevice::WriteOnly);
                    dat<<list2.at(i).split("_").at(1).toInt();
                    dat<<xml->getAttribute(QString("data/news/%1").arg(list2.at(i)),"headline","");
                    dat<<xml->getAttribute(QString("data/news/%1").arg(list2.at(i)),"time",0);
                    dat<<QString(file.readAll());
                    file.close();
                    data2.append(dataBlock);
                }
            }
            out<<data2;

            out<<userAdmin;
            if(userAdmin==1){
                QStringList list=xml->getElements("data/user");
                QList<QByteArray> data3;
                for(int i=0;i<list.count();i++){
                    QByteArray dataBlock;
                    QDataStream dat(&dataBlock,QIODevice::WriteOnly);
                    dat<<xml->getAttribute(QString("data/user/%1").arg(list.at(i)),"id",0);
                    dat<<list.at(i);
                    dat<<xml->getAttribute(QString("data/user/%1").arg(list.at(i)),"admin",0);
                    data3.append(dataBlock);
                }
                out<<data3;
            }
        }else if(id==PACKET_CS_GET_SERVERINFO){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                SAMPServerInfo info=samp->getServerInfo(serverId);
                out<<PACKET_SC_GET_SERVERINFO;
                if(info.online){
                    out<<1;
                }else{
                    out<<0;
                }
                out<<xml->getAttribute(QString("data/server/server_%1/port").arg(serverId),"value",0);
                out<<getUserNameFromID(xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0));
                out<<info.serverName;
                out<<info.gamemodeName;
                out<<info.mapName;
                out<<QString("%1/%2").arg(info.playerOnline).arg(info.maxPlayer);
            }else{
                out<<PACKET_SC_GET_SERVERINFO_FAIL;
            }
        }else if(id==PACKET_CS_GET_STATUS){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                SAMPServerInfo info=samp->getServerInfo(serverId);
                out<<PACKET_SC_GET_STATUS;
                if(info.online){
                    out<<1;
                }else{
                    out<<0;
                }
                out<<info.serverName;
                out<<info.gamemodeName;
                out<<info.mapName;
                out<<QString("%1/%2").arg(info.playerOnline).arg(info.maxPlayer);
            }
        }else if(id==PACKET_CS_GET_SERVERSETTINGS){
            int serverId;
            in>>serverId;

           if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                out<<PACKET_SC_GET_SERVERSETTINGS;
                out<<samp->getGamemodes(serverId);
                out<<samp->getPlugins(serverId);
                out<<samp->getFilterscripts(serverId);

                out<<xml->getAttribute(QString("data/server/server_%1/config/announce").arg(serverId),"value",0);
                out<<xml->getAttribute(QString("data/server/server_%1/config/hostname").arg(serverId),"value","Server");
                out<<xml->getAttribute(QString("data/server/server_%1/config/weburl").arg(serverId),"value","www");
                out<<xml->getAttribute(QString("data/server/server_%1/config/rconPassword").arg(serverId),"value","");
                out<<xml->getAttribute(QString("data/server/server_%1/config/serverPassword").arg(serverId),"value","");
                out<<xml->getAttribute(QString("data/server/server_%1/config/mapname").arg(serverId),"value","Map");

                QStringList activeGamemodes;
                QList<int> nums;
                for(int i=0;i<8;i++){
                    activeGamemodes<<xml->getAttribute(QString("data/server/server_%1/gamemodes/gm_%2").arg(serverId).arg(i),"name","");
                    nums<<xml->getAttribute(QString("data/server/server_%1/gamemodes/gm_%2").arg(serverId).arg(i),"num",0);
                }
                out<<activeGamemodes;
                out<<nums;

                QStringList list=xml->getElements(QString("data/server/server_%1/filterscripts").arg(serverId));
                QMap<QString,int> activeFilterscripts;
                for(int i=0;i<list.count();i++){
                    activeFilterscripts.insert(list.at(i),xml->getAttribute(QString("data/server/server_%1/filterscripts/%2").arg(serverId).arg(list.at(i)),"value",0));
                }
                out<<activeFilterscripts;

                list=xml->getElements(QString("data/server/server_%1/plugins").arg(serverId));
                QMap<QString,int> activePlugin;
                for(int i=0;i<list.count();i++){
                    activePlugin.insert(list.at(i),xml->getAttribute(QString("data/server/server_%1/plugins/%2").arg(serverId).arg(list.at(i)),"value",0));
                }
                out<<activePlugin;
            }else{
                out<<PACKET_SC_GET_SERVERSETTINGS_FAIL;
            }
        }else if(id==PACKET_CS_CHANGE_SERVERSETTINGS){
            int serverId;
            in>>serverId;
            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                int announce;
                QString hostname,weburl,rconPassword,serverPassword,mapname;
                in>>announce>>hostname>>weburl>>rconPassword>>serverPassword>>mapname;

                xml->setAttribute(QString("data/server/server_%1/config/announce").arg(serverId),"value",announce);
                xml->setAttribute(QString("data/server/server_%1/config/hostname").arg(serverId),"value",hostname);
                xml->setAttribute(QString("data/server/server_%1/config/weburl").arg(serverId),"value",weburl);
                xml->setAttribute(QString("data/server/server_%1/config/rconPassword").arg(serverId),"value",rconPassword);
                xml->setAttribute(QString("data/server/server_%1/config/serverPassword").arg(serverId),"value",serverPassword);
                xml->setAttribute(QString("data/server/server_%1/config/mapname").arg(serverId),"mapname",mapname);

                QStringList activeGamemodes;
                QList<int> gamemodeNums;
                in>>activeGamemodes>>gamemodeNums;
                for(int i=0;i<8;i++){
                    xml->setAttribute(QString("data/server/server_%1/config/gamemodes/gm_%2").arg(serverId).arg(i),"name",activeGamemodes.at(i));
                    xml->setAttribute(QString("data/server/server_%1/config/gamemodes/gm_%2").arg(serverId).arg(i),"num",gamemodeNums.at(i));
                }

                QMap<QString,int> activeFilterscripts;
                QMap<QString,int> activePlugin;
                in>>activeFilterscripts>>activePlugin;
                QMapIterator<QString,int> i(activeFilterscripts);
                while(i.hasNext()){
                    i.next();
                    xml->setAttribute(QString("data/server/server_%1/filterscripts/%2").arg(serverId).arg(i.key()),"value",i.value());
                }
                QMapIterator<QString,int> i2(activePlugin);
                while(i2.hasNext()){
                    i2.next();
                    xml->setAttribute(QString("data/server/server_%1/plugins/%2").arg(serverId).arg(i2.key()),"value",i2.value());
                }
                xml->saveToFile();

                out<<PACKET_SC_CHANGE_SERVERSETTINGS;
            }else{
                out<<PACKET_SC_GET_SERVERSETTINGS_FAIL;
            }
        }else if(id==PACKET_CS_START_SERVER){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                xml->setAttribute(QString("data/server/server_%1/online").arg(serverId),"value",1);
                xml->saveToFile();
                samp->startServer(serverId);
            }
        }else if(id==PACKET_CS_STOP_SERVER){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                xml->setAttribute(QString("data/server/server_%1/online").arg(serverId),"value",0);
                xml->saveToFile();
                samp->stopServer(serverId);
            }
        }else if(id==PACKET_CS_RESTART_SERVER){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                samp->restartServer(serverId);
            }
        }else if(id==PACKET_CS_CHANGE_PASSWORD){
            QString oldPassword,newPassword;
            in>>oldPassword>>newPassword;

            if(userAdmin==1 || xml->getAttribute(QString("data/user/%1").arg(userName),"password",QString(""))==QString(QCryptographicHash::hash(oldPassword.toAscii(),QCryptographicHash::Md5).toHex())){
                xml->setAttribute(QString("data/user/%1").arg(userName),"password",QString(QCryptographicHash::hash(newPassword.toAscii(),QCryptographicHash::Md5).toHex()));
                xml->saveToFile();
                out<<PACKET_SC_CHANGE_PASSWORD;
            }else{
                out<<PACKET_SC_CHANGE_PASSWORD_WRONG;
            }



        }else if(id==PACKET_CS_FILE_LIST){//Filemanager
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                if(fileManagerPath.count()==0){
                    out<<PACKET_SC_FILE_LIST;
                    out<<false;
                    QList<QByteArray> data;
                    QStringList list;
                    list<<"filterscripts"<<"gamemodes"<<"npcmodes"<<"scriptfiles"<<"logs";
                    for(int i=0;i<list.count();i++){
                        QByteArray dat;
                        QDataStream stream(&dat,QIODevice::WriteOnly);
                        stream<<list.at(i);
                        stream<<true;
                        stream<<0;
                        data.append(dat);
                    }
                    QStringList list2;
                    list2<<"mysql_log.txt"<<"server_log.txt"<<"system_log.txt";
                    for(int i=0;i<list2.count();i++){
                        if(QFile::exists(QString("servers/server_%1/%3").arg(serverId).arg(list2.at(i)))){
                            QByteArray dat;
                            QDataStream stream(&dat,QIODevice::WriteOnly);
                            stream<<list2.at(i);
                            stream<<false;
                            stream<<0;
                            data.append(dat);
                        }
                    }
                    out<<data;
                    out<<false;
                }else{
                    out<<PACKET_SC_FILE_LIST;
                    QStringList typList;
                    typList<<"filterscripts"<<"gamemodes"<<"npcmodes"<<"logs";
                    if(fileManagerPath.count()==1 && typList.contains(fileManagerPath.at(0))){
                        out<<false;
                    }else{
                        out<<true;
                    }
                    QDir dir(QString("servers/server_%1").arg(serverId));
                    for(int i=0;i<fileManagerPath.count();i++){
                        dir.cd(fileManagerPath.at(i));
                    }
                    QFileInfoList list=dir.entryInfoList();
                    QList<QByteArray> data;
                    for(int i=0;i<list.count();i++){
                        if(list.at(i).fileName()==".") continue;
                        QByteArray dat;
                        QDataStream stream(&dat,QIODevice::WriteOnly);
                        stream<<list.at(i).fileName();
                        if(list.at(i).isDir()){
                            stream<<true;
                            stream<<0;
                        }else{
                            stream<<false;
                            stream<<(int)list.at(i).size();
                        }
                        data.append(dat);
                    }
                    out<<data;
                    out<<true;
                }
            }
        }else if(id==PACKET_CS_FILE_CD){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                QString name;
                in>>name;
                if(name==".."){
                    if(fileManagerPath.count()>0){
                        fileManagerPath.removeLast();
                    }
                }else{
                    if(fileManagerPath.count()==0){
                        QStringList list;
                        list<<"filterscripts"<<"gamemodes"<<"npcmodes"<<"scriptfiles"<<"logs";
                        if(list.contains(name)){
                            fileManagerPath.append(name);
                        }
                    }else{
                        QDir dir(QString("servers/server_%1").arg(serverId));
                        for(int i=0;i<fileManagerPath.count();i++){
                            dir.cd(fileManagerPath.at(i));
                        }
                        QFileInfoList list=dir.entryInfoList();
                        for(int i=0;i<list.count();i++){
                            if(list.at(i).fileName()==name){
                                if(list.at(i).fileName()==".") continue;
                                fileManagerPath.append(name);
                                break;
                            }
                        }
                    }
                }
                out<<PACKET_SC_FILE_CD;
            }
        }else if(id==PACKET_CS_FILE_BACK){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                if(fileManagerPath.count()!=0){
                    fileManagerPath.removeLast();
                    out<<PACKET_SC_FILE_BACK;
                }
            }
        }else if(id==PACKET_CS_FILE_CREATE_DIR){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                if(fileManagerPath.count()>0){
                    QStringList typList;
                    typList<<"filterscripts"<<"gamemodes"<<"npcmodes"<<"logs";
                    if(!(fileManagerPath.count()==1 && typList.contains(fileManagerPath.at(0)))){
                        QDir dir(QString("servers/server_%1").arg(serverId));
                        for(int i=0;i<fileManagerPath.count();i++){
                            dir.cd(fileManagerPath.at(i));
                        }
                        QString name;
                        in>>name;
                        dir.mkdir(name);
                        out<<PACKET_SC_FILE_CREATE_DIR;
                    }
                }
            }
        }else if(id==PACKET_CS_FILE_DELETE){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                if(fileManagerPath.count()>0){
                    QString name;
                    in>>name;
                    QDir dir(QString("servers/server_%1").arg(serverId));
                    for(int i=0;i<fileManagerPath.count();i++){
                        dir.cd(fileManagerPath.at(i));
                    }
                    QFileInfoList list=dir.entryInfoList();
                    for(int i=0;i<list.count();i++){
                        if(list.at(i).fileName()==name){
                            if(list.at(i).isDir()){
                                out<<PACKET_SC_FILE_DELETE;
                                out<<dir.rmdir(name);
                            }else{
                                out<<PACKET_SC_FILE_DELETE;
                                out<<true;
                                dir.remove(name);
                            }
                            break;
                        }
                    }
                }
            }
        }else if(id==PACKET_CS_FILE_GET_FILE){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                QString name;
                in>>name;
                if(fileManagerPath.count()>0 || (name=="mysql_log.txt" || name=="server_log.txt" || name=="system_log.txt")){
                    if(!name.contains('/')){
                        QString fileName=QString("servers/server_%1/%2/%3").arg(serverId).arg(fileManagerPath.join("/")).arg(name);
                        //qDebug()<<fileName;

                        if(QFile::exists(fileName)){
                            QFileInfo info(fileName);
                            int size=info.size();
                            dataFile.size=size;
                            if(size % 1024==0){
                                dataFile.parts=size/1024;
                            }else{
                                dataFile.parts=size/1024+1;
                            }

                            dataFile.file=new QFile(fileName);
                            if(dataFile.file->open(QIODevice::ReadOnly)){
                                out<<PACKET_SC_FILE_GET_FILE;
                                out<<dataFile.size;
                                out<<dataFile.parts;
                                out<<name;
                                emit(output(1|4,QString("[%1] File-Download: %2 Size %3 (ID %4)\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).arg(fileName).arg(size).arg((int)this)));
                            }else{
                                delete(dataFile.file);
                                out<<PACKET_SC_FILE_GET_FILE_FAIL;
                            }
                        }else{
                            out<<PACKET_SC_FILE_GET_FILE_FAIL;
                        }
                    }else{
                        out<<PACKET_SC_FILE_GET_FILE_FAIL;
                    }
                }
            }
        }else if(id==PACKET_CS_FILE_GET_FILE_PART){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                int part;
                in>>part;
                if(part>0 && part<=dataFile.parts){
                    out<<PACKET_SC_FILE_GET_FILE_PART;
                    out<<part;
                    QByteArray dat=dataFile.file->read(1024);
                    out<<dat;
                }else{
                    out<<PACKET_SC_FILE_GET_FILE_PART_FAIL;
                }
            }else{
                out<<PACKET_SC_FILE_GET_FILE_PART_FAIL;
            }
        }else if(id==PACKET_CS_FILE_GET_FILE_COMPLETE){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                dataFile.file->close();
                delete(dataFile.file);
            }
        }else if(id==PACKET_CS_FILE_PUT_FILE){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                if(fileManagerPath.count()>0){
                    int size,parts;
                    QString name;
                    in>>size>>parts>>name;
                    if(size<(1024*1024*20+5)){//Max 20MB
                        if(!name.contains('/')){
                            QString fileName=QString("servers/server_%1/%2/%3").arg(serverId).arg(fileManagerPath.join("/")).arg(name);

                            dataFile.size=size;
                            dataFile.parts=parts;
                            dataFile.fileName=fileName;
                            dataFile.file=new QFile(fileName);
                            dataFile.file->open(QIODevice::WriteOnly);

                            out<<PACKET_SC_FILE_PUT_FILE_PART;
                            out<<1;
                            emit(output(1|4,QString("[%1] File-Upload: %2 Size %3 (ID %4)\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).arg(fileName).arg(size).arg((int)this)));
                        }else{
                            out<<PACKET_SC_FILE_PUT_FILE_FAIL;
                        }
                    }else{
                        out<<PACKET_SC_FILE_PUT_FILE_FAIL;
                    }
                }
            }
        }else if(id==PACKET_CS_FILE_PUT_FILE_PART){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                int part;
                QByteArray dat;
                in>>part>>dat;

                dataFile.file->write(dat);

                if(part<=dataFile.parts){
                    out<<PACKET_SC_FILE_PUT_FILE_PART;
                    out<<part+1;
                }else{
                    dataFile.file->flush();
                    dataFile.file->close();
                    delete(dataFile.file);
                    out<<PACKET_SC_FILE_PUT_FILE_COMPLETE;
                }
            }
        }else if(id==PACKET_CS_FILE_PUT_FILE_COMPLETE){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                dataFile.file->flush();
                dataFile.file->close();
                delete(dataFile.file);
            }
        }else if(id==PACKET_CS_FILE_PUT_FILE_CANCEL){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                dataFile.file->close();
                delete(dataFile.file);
                QFile::remove(dataFile.fileName);
            }
        }else if(id==PACKET_CS_RCON){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                QString cmd;
                in>>cmd;
                rconData=samp->rconCmd(serverId,cmd);
                if(rconData.count()>0){
                    out<<PACKET_SC_RCON;
                    out<<rconData.at(0);
                    rconData.removeAt(0);


                    /*
                    QString temp;
                    for(int i=0;i<ret.count();i++){
                        if(temp.size()+ret.at(i).size()>128){
                            QByteArray block;
                            QDataStream out(&block,QIODevice::WriteOnly);
                            out<<PACKET_SC_RCON;
                            out<<temp.append("\n");
                            qDebug()<<temp.append("\n");
                            send(block);
                            temp=QString(ret.at(i)).append("\n");
                        }else{
                            temp.append(ret.at(i)).append("\n");
                        }
                    }
                    QByteArray block;
                    QDataStream out(&block,QIODevice::WriteOnly);
                    out<<PACKET_SC_RCON;
                    out<<temp.append("\n");
                    qDebug()<<temp.append("\n");
                    send(block);+
                    return;*/
                }else{
                    out<<PACKET_SC_RCON_FAIL;
                }
            }else{
                out<<PACKET_SC_RCON_FAIL;
            }
        }else if(id==PACKET_CS_RCON_NEXT){
            int serverId;
            in>>serverId;

            if(userAdmin==1 || xml->getAttribute(QString("data/server/server_%1/owner").arg(serverId),"value",0)==userId){
                if(rconData.count()>0){
                    out<<PACKET_SC_RCON;
                    out<<rconData.at(0);
                    rconData.removeAt(0);
                }else{
                    out<<PACKET_SC_RCON_FAIL;
                }
            }else{
                out<<PACKET_SC_RCON_FAIL;
            }


        }else if(userAdmin==1){
            if(id==PACKET_CS_ADMIN_GET_SERVERINFO){///////SERVER
                int serverID;
                in>>serverID;

                out<<PACKET_SC_ADMIN_GET_SERVERINFO;
                out<<xml->getAttribute(QString("data/server/server_%1/owner").arg(serverID),"value",0);
                out<<xml->getAttribute(QString("data/server/server_%1/port").arg(serverID),"value",0);
                out<<xml->getAttribute(QString("data/server/server_%1/maxPlayers").arg(serverID),"value",0);
                out<<xml->getAttribute(QString("data/server/server_%1/maxNPCs").arg(serverID),"value",0);
            }else if(id==PACKET_CS_ADMIN_ADD_SERVER){
                int port,maxPlayers,maxNPCs;
                in>>port>>maxPlayers>>maxNPCs;

                QString userName=getUserNameFromID(userId);
                if(userName.isEmpty()){
                    out<<PACKET_SC_ADMIN_ADD_SERVER_FAIL_OWNER;
                }else if(port<1){
                    out<<PACKET_SC_ADMIN_ADD_SERVER_FAIL_PORT;
                }else if(maxPlayers<2 || maxPlayers>500){
                    out<<PACKET_SC_ADMIN_ADD_SERVER_FAIL_MAXPLAYER;
                }else if(maxNPCs<0 || maxNPCs>500){
                    out<<PACKET_SC_ADMIN_ADD_SERVER_FAIL_MAXNPC;
                }else{
                    QStringList list=xml->getElements("data/server");
                    bool fail=false;
                    for(int i=0;i<list.count();i++){
                        if(xml->getAttribute(QString("data/server/%1/port").arg(list.at(i)),"value",0)==port) fail=true;
                    }
                    if(fail){
                        out<<PACKET_SC_ADMIN_ADD_SERVER_FAIL_PORT_EXISTS;
                    }else{
                        int newServerID=xml->getAttribute("data/data/lastServerID","value",0)+1;
                        xml->setAttribute("data/data/lastServerID","value",newServerID);

                        xml->setAttribute(QString("data/server/server_%1/owner").arg(newServerID),"value",userId);
                        xml->setAttribute(QString("data/server/server_%1/port").arg(newServerID),"value",port);
                        xml->setAttribute(QString("data/server/server_%1/maxPlayers").arg(newServerID),"value",maxPlayers);
                        xml->setAttribute(QString("data/server/server_%1/maxNPCs").arg(newServerID),"value",maxNPCs);

                        xml->setAttribute(QString("data/server/server_%1/config/rconPassword").arg(newServerID),"value",10000+qrand()%89999);
                        xml->saveToFile();

                        samp->newServer(newServerID);

                        emit(output(1|4,QString("[%1] Admin: New Server ID=%2 Port=%3 (ID %4)\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).arg(newServerID).arg(port).arg((int)this)));

                        emit(newServer(newServerID,port,userId));
                        return;
                    }
                }
            }else if(id==PACKET_CS_ADMIN_CHANGE_SERVER){
                int serverID,owner,port,maxPlayers,maxNPCs;
                in>>serverID>>owner>>port>>maxPlayers>>maxNPCs;

                QString userName=getUserNameFromID(owner);
                if(userName.isEmpty()){
                    out<<PACKET_SC_ADMIN_CHANGE_SERVER_FAIL_OWNER;
                }else if(port<1){
                    out<<PACKET_SC_ADMIN_CHANGE_SERVER_FAIL_PORT;
                }else if(maxPlayers<2 || maxPlayers>500){
                    out<<PACKET_SC_ADMIN_CHANGE_SERVER_FAIL_MAXPLAYER;
                }else if(maxNPCs<0 || maxNPCs>500){
                    out<<PACKET_SC_ADMIN_CHANGE_SERVER_FAIL_MAXNPC;
                }else{
                    QStringList list=xml->getElements("data/server");
                    bool fail=false;
                    for(int i=0;i<list.count();i++){
                        if(list.at(i)!=QString("server_%1").arg(serverID) && xml->getAttribute(QString("data/server/%1/port").arg(list.at(i)),"value",0)==port) fail=true;
                    }
                    if(fail){
                        out<<PACKET_SC_ADMIN_CHANGE_SERVER_FAIL_PORT_EXISTS;
                    }else{
                        int oldOwner=xml->getAttribute(QString("data/server/server_%1/owner").arg(serverID),"value",0);
                        xml->setAttribute(QString("data/server/server_%1/owner").arg(serverID),"value",owner);
                        xml->setAttribute(QString("data/server/server_%1/port").arg(serverID),"value",port);
                        xml->setAttribute(QString("data/server/server_%1/maxPlayers").arg(serverID),"value",maxPlayers);
                        xml->setAttribute(QString("data/server/server_%1/maxNPCs").arg(serverID),"value",maxNPCs);
                        xml->saveToFile();

                        emit(output(1|4,QString("[%1] Admin: Change Server ID=%2 Port=%3 (ID %4)\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).arg(serverID).arg(port).arg((int)this)));

                        emit(changeServer(serverID,port,owner,oldOwner));
                        return;
                    }
                }
            }else if(id==PACKET_CS_ADMIN_DELETE_SERVER){
                int serverID;
                in>>serverID;

                if(!xml->existPath(QString("data/server/server_%1").arg(serverID))){
                    out<<PACKET_SC_ADMIN_DELETE_SERVER_FAIL;
                }else{
                    int owner=xml->getAttribute(QString("data/server/server_%1/owner").arg(serverID),"value",0);
                    xml->deleteChild("data/server",QString("server_%1").arg(serverID));
                    xml->saveToFile();
                    emit(output(1|4,QString("[%1] Admin: Delete Server ID=%2 (ID %4)\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).arg(serverID).arg((int)this)));
                    emit(deleteServer(serverID,owner));
                    return;
                }
            }else if(id==PACKET_CS_ADMIN_GET_USERINFO){///////USER
                int userID;
                in>>userID;
                QString userName=getUserNameFromID(userID);

                if(userName.isEmpty()){
                    out<<PACKET_SC_ADMIN_GET_USERINFO_FAIL;
                }else{
                    out<<PACKET_SC_ADMIN_GET_USERINFO;
                    out<<userName;
                    out<<xml->getAttribute(QString("data/user/%1").arg(userName),"admin",0);
                    out<<xml->getAttribute(QString("data/user/%1").arg(userName),"banned",0);
                    out<<(userID==userId);
                }
            }else if(id==PACKET_CS_ADMIN_ADD_USER){
                QString name,password;
                in>>name>>password;

                if(xml->existPath(QString("data/user/%1").arg(name))){
                    out<<PACKET_SC_ADMIN_ADD_USER_FAIL;
                }else{
                    int newUserID=xml->getAttribute("data/data/lastUserID","value",0)+1;
                    xml->setAttribute("data/data/lastUserID","value",newUserID);
                    xml->setAttribute(QString("data/user/%1").arg(name),"password",QString(QCryptographicHash::hash(password.toAscii(),QCryptographicHash::Md5).toHex()));
                    xml->setAttribute(QString("data/user/%1").arg(name),"id",newUserID);
                    xml->setAttribute(QString("data/user/%1").arg(name),"admin",0);
                    xml->saveToFile();

                    emit(output(1|4,QString("[%1] Admin: Add User %2 (ID %3)\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).arg(name).arg((int)this)));

                    emit(newUser(newUserID,name));
                    return;
                }
            }else if(id==PACKET_CS_ADMIN_CHANGE_USER){
                QString password;
                int userID,admin,banned;
                in>>userID>>password>>admin>>banned;

                QString name=getUserNameFromID(userID);

                if(name.isEmpty()){
                    out<<PACKET_SC_ADMIN_CHANGE_USER_FAIL;
                }else{
                    if(admin!=1) admin=0;
                    if(banned!=1) banned=0;

                    if(password!="*****") xml->setAttribute(QString("data/user/%1").arg(name),"password",QString(QCryptographicHash::hash(password.toAscii(),QCryptographicHash::Md5).toHex()));
                    xml->setAttribute(QString("data/user/%1").arg(name),"admin",admin);
                    xml->setAttribute(QString("data/user/%1").arg(name),"banned",banned);
                    xml->saveToFile();

                    emit(output(1|4,QString("[%1] Admin: Change User %2 (ID %3)\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).arg(name).arg((int)this)));

                    emit(changeUser(userID,admin,banned));
                    return;
                }
            }else if(id==PACKET_CS_ADMIN_DELETE_USER){
                int userID;
                in>>userID;

                QString name=getUserNameFromID(userID);

                if(name.isEmpty() || !xml->existPath(QString("data/user/%1").arg(name))){
                    out<<PACKET_SC_ADMIN_DELETE_USER_FAIL;
                }else{
                    xml->deleteChild("data/user",name);
                    xml->saveToFile();

                    emit(output(1|4,QString("[%1] Admin: Delete User %2 (ID %3)\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).arg(name).arg((int)this)));

                    emit(deleteUser(userID));
                    return;
                }
            }else if(id==PACKET_CS_ADMIN_GET_NEWS){//News
                int newsId;
                in>>newsId;

                QFile file(QString("news/news_%1.txt").arg(newsId));
                if(xml->existPath(QString("data/news/%1").arg(newsId)) && file.open(QIODevice::ReadOnly)){
                    out<<PACKET_SC_ADMIN_GET_NEWS;
                    out<<xml->getAttribute(QString("data/news/news_%1").arg(newsId),"headline","");
                    out<<xml->getAttribute(QString("data/news/news_%1").arg(newsId),"time",0);
                    out<<QString(file.readAll());
                    file.close();
                }else{
                    out<<PACKET_SC_ADMIN_GET_NEWS_FAIL;
                }
            }else if(id==PACKET_CS_ADMIN_ADD_NEWS){
                QString headline,text;
                in>>headline>>text;

                int time=QDateTime::currentDateTime().toTime_t();

                int newNewsID=xml->getAttribute("data/data/lastNewsID","value",0)+1;
                xml->setAttribute("data/data/lastNewsID","value",newNewsID);

                QFile file(QString("news/news_%1.txt").arg(newNewsID));
                if(file.open(QIODevice::WriteOnly)){
                    xml->setAttribute(QString("data/news/news_%1").arg(newNewsID),"headline",headline);
                    xml->setAttribute(QString("data/news/news_%1").arg(newNewsID),"time",time);
                    xml->saveToFile();
                    file.write(text.toAscii());
                    file.flush();
                    file.close();

                    emit(output(1|4,QString("[%1] Admin: New News %2 (ID %3)\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).arg(headline).arg((int)this)));

                    emit(newNews(newNewsID,headline,time,text));
                    return;
                }else{
                    out<<PACKET_SC_ADMIN_ADD_NEWS_FAIL;
                }
            }else if(id==PACKET_CS_ADMIN_CHANGE_NEWS){
                QString headline,text;
                int newsId;
                in>>newsId>>headline>>text;

                QFile file(QString("news/news_%1.txt").arg(newsId));
                if(xml->existPath(QString("data/news/news_%1").arg(newsId)) && file.open(QIODevice::WriteOnly)){
                    xml->setAttribute(QString("data/news/news_%1").arg(newsId),"headline",headline);
                    xml->saveToFile();
                    file.write(text.toAscii());
                    file.flush();
                    file.close();

                    emit(output(1|4,QString("[%1] Admin: Change News %2 (ID %3)\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).arg(headline).arg((int)this)));

                    emit(changeNews(newsId,headline,text));
                    return;
                }else{
                    out<<PACKET_SC_ADMIN_CHANGE_NEWS_FAIL;
                }
            }else if(id==PACKET_CS_ADMIN_DELETE_NEWS){
                int newsId;
                in>>newsId;

                if(xml->existPath(QString("data/news/news_%1").arg(newsId))){
                    xml->deleteChild("data/news",QString("news_%1").arg(newsId));
                    xml->saveToFile();
                    QFile::remove(QString("news/news_%1.txt").arg(newsId));

                    emit(output(1|4,QString("[%1] Admin: Delete News (ID %3)\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).arg((int)this)));

                    emit(deleteNews(newsId));
                    return;
                }else{
                    out<<PACKET_SC_ADMIN_CHANGE_NEWS_FAIL;
                }
            }else{
                out<<PACKET_SC_WRONG_PACKET;
            }
        }else{
            out<<PACKET_SC_WRONG_PACKET;
        }
    }else{
        out<<PACKET_SC_WRONG_PACKET;
    }
    clientSocket->readAll();
    send(block);
}

void Client::disconnected(){
    emit(output(1|4,QString("[%1] Disconnection (ID %2)\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).arg((int)this)));
    exit();
}

QString Client::getUserNameFromID(int id){
    if(id<1) return QString();
    QStringList list=xml->getElements("data/user");
    for(int i=0;i<list.count();i++){
        if(xml->getAttribute(QString("data/user/%1").arg(list.at(i)),"id",0)==id) return list.at(i);
    }
    return QString();
}


void Client::onNewServer(int id, int port, int owner){
    if(owner==userId || userAdmin==1){
        QByteArray block;
        QDataStream out(&block,QIODevice::WriteOnly);
        out<<PACKET_SC_NEW_SERVER;
        out<<id;
        out<<port;
        send(block);
    }
}
void Client::onChangeServer(int id, int port, int newOwner, int oldOwner){
    if(newOwner==oldOwner && (newOwner==userId || userAdmin==1)){
        QByteArray block;
        QDataStream out(&block,QIODevice::WriteOnly);
        out<<PACKET_SC_CHANGE_SERVER;
        out<<id;
        out<<port;
        send(block);
    }else{
        if(userAdmin==1){
           QByteArray block;
           QDataStream out(&block,QIODevice::WriteOnly);
           out<<PACKET_SC_CHANGE_SERVER;
           out<<id;
           out<<port;
           send(block);
       }else if(newOwner==userId){
            QByteArray block;
            QDataStream out(&block,QIODevice::WriteOnly);
            out<<PACKET_SC_NEW_SERVER;
            out<<id;
            out<<port;
            send(block);
        }else if(oldOwner==userId){
            QByteArray block;
            QDataStream out(&block,QIODevice::WriteOnly);
            out<<PACKET_SC_DELETE_SERVER;
            out<<id;
            send(block);
        }
    }
}
void Client::onDeleteServer(int id, int owner){
    if(owner==userId || userAdmin==1){
        QByteArray block;
        QDataStream out(&block,QIODevice::WriteOnly);
        out<<PACKET_SC_DELETE_SERVER;
        out<<id;
        send(block);
    }
}

void Client::onNewUser(int id, QString name){
    if(userAdmin==1){
        QByteArray block;
        QDataStream out(&block,QIODevice::WriteOnly);
        out<<PACKET_SC_NEW_USER;
        out<<id;
        out<<name;
        send(block);
    }
}
void Client::onChangeUser(int id, int admin,int banned){
    if(id==userId){
        if(admin==1 || banned==1){
            clientSocket->disconnectFromHost();
        }
    }
    if(userAdmin==1){
        QByteArray block;
        QDataStream out(&block,QIODevice::WriteOnly);
        out<<PACKET_SC_CHANGE_USER;
        out<<id;
        out<<admin;
        send(block);
    }
}
void Client::onDeleteUser(int id){
    if(id==userId){
        clientSocket->disconnectFromHost();
    }else if(userAdmin==1){
        QByteArray block;
        QDataStream out(&block,QIODevice::WriteOnly);
        out<<PACKET_SC_DELETE_USER;
        out<<id;
        send(block);
    }
}

void Client::onNewNews(int id, QString headline, int time, QString text){
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out<<PACKET_SC_NEW_NEWS;
    out<<id;
    out<<headline;
    out<<time;
    out<<text;
    send(block);
}
void Client::onChangeNews(int id, QString headline, QString text){
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out<<PACKET_SC_CHANGE_NEWS;
    out<<id;
    out<<headline;
    out<<text;
    send(block);
}
void Client::onDeleteNews(int id){
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out<<PACKET_SC_DELETE_NEWS;
    out<<id;
    send(block);
}
