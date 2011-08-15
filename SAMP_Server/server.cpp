#include "server.h"

Server::Server(QObject *parent) : QTcpServer(parent){
    QDir dir(".");
    if(!QFile::exists("news")) dir.mkdir("news");

    if(QFile::exists("data.xml")){
        QDomDocument doc("SAMP_Manager");
        xml=new XMLParser(doc);
        xml->loadFromFile("data.xml");
    }else{
        QDomDocument doc("SAMP_Manager");
        xml=new XMLParser(doc);
        xml->loadFromFile("data.xml");

        int newUserID=1;
        xml->setAttribute("data/data/lastUserID","value",newUserID);
        xml->setAttribute("data/user/admin","password",QString(QCryptographicHash::hash(QString("admin").toAscii(),QCryptographicHash::Md5).toHex()));
        xml->setAttribute("data/user/admin","id",newUserID);
        xml->setAttribute("data/user/admin","admin",1);
        xml->saveToFile();

        qDebug()<<tr("***** NEW ADMIN-ACCOUNT CREATED  *****");
        qDebug()<<tr("***** LOGIN-NAME: \"admin\"        *****");
        qDebug()<<tr("***** PASSWORD:   \"admin\"        *****");
        qDebug()<<tr("***** PLEASE CHANGE THE PASSWORD *****");
    }
    samp=new SAMPServer(xml);

    timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(update()));
    timer->start(60000);

    outStream=new QFile(0);
    outStream->open(1,QIODevice::WriteOnly);
    errStream=new QFile(0);
    errStream->open(2,QIODevice::WriteOnly);
    logStream=new QFile("samp_manager.log");
    logStream->open(QIODevice::Append);
}
Server::~Server(){
    delete(samp);
    delete(timer);
    xml->saveToFile();
    delete(xml);

    outStream->close();
    delete(outStream);
    errStream->close();
    delete(errStream);
    logStream->close();
    delete(logStream);
}

void Server::incomingConnection(int socketDescriptor){
    Client* client=new Client(socketDescriptor,xml,samp);
    connect(client,SIGNAL(finished()),client,SLOT(deleteLater()));

    connect(client,SIGNAL(newServer(int,int,int)),this,SLOT(onNewServer(int,int,int)));
    connect(this,SIGNAL(newServer(int,int,int)),client,SLOT(onNewServer(int,int,int)));

    connect(client,SIGNAL(changeServer(int,int,int,int)),this,SLOT(onChangeServer(int,int,int,int)));
    connect(this,SIGNAL(changeServer(int,int,int,int)),client,SLOT(onChangeServer(int,int,int,int)));

    connect(client,SIGNAL(deleteServer(int,int)),this,SLOT(onDeleteServer(int,int)));
    connect(this,SIGNAL(deleteServer(int,int)),client,SLOT(onDeleteServer(int,int)));


    connect(client,SIGNAL(newUser(int,QString)),this,SLOT(onNewUser(int,QString)));
    connect(this,SIGNAL(newUser(int,QString)),client,SLOT(onNewUser(int,QString)));

    connect(client,SIGNAL(changeUser(int,int,int)),this,SLOT(onChangeUser(int,int,int)));
    connect(this,SIGNAL(changeUser(int,int,int)),client,SLOT(onChangeUser(int,int,int)));

    connect(client,SIGNAL(deleteUser(int)),this,SLOT(onDeleteUser(int)));
    connect(this,SIGNAL(deleteUser(int)),client,SLOT(onDeleteUser(int)));


    connect(client,SIGNAL(newNews(int,QString,int,QString)),this,SLOT(onNewNews(int,QString,int,QString)));
    connect(this,SIGNAL(newNews(int,QString,int,QString)),client,SLOT(onNewNews(int,QString,int,QString)));

    connect(client,SIGNAL(changeNews(int,QString,QString)),this,SLOT(onChangeNews(int,QString,QString)));
    connect(this,SIGNAL(changeNews(int,QString,QString)),client,SLOT(onChangeNews(int,QString,QString)));

    connect(client,SIGNAL(deleteNews(int)),this,SLOT(onDeleteNews(int)));
    connect(this,SIGNAL(deleteNews(int)),client,SLOT(onDeleteNews(int)));

    connect(client,SIGNAL(output(int,QString)),this,SLOT(output(int,QString)));
    client->run();
}

void Server::update(){
    samp->checkServers();
}

void Server::onNewServer(int id, int port, int owner){
    emit(newServer(id,port,owner));
}
void Server::onChangeServer(int id, int port, int newOwner, int oldOwner){
    emit(changeServer(id,port,newOwner,oldOwner));
}
void Server::onDeleteServer(int id, int owner){
    emit(deleteServer(id,owner));
}

void Server::onNewUser(int id, QString name){
    emit(newUser(id,name));
}
void Server::onChangeUser(int id, int admin, int banned){
    emit(changeUser(id,admin,banned));
}
void Server::onDeleteUser(int id){
    emit(deleteUser(id));
}

void Server::onNewNews(int id, QString headline, int time, QString text){
    emit(newNews(id,headline,time,text));
}
void Server::onChangeNews(int id, QString headline, QString text){
    emit(changeNews(id,headline,text));
}
void Server::onDeleteNews(int id){
    emit(deleteNews(id));
}

void Server::output(int streams, QString data){//1 2 4
    if((streams & 1)!=0){
        outStream->write(data.toAscii());
        outStream->flush();
    }
    if((streams & 2)!=0){
        errStream->write(data.toAscii());
        errStream->flush();
    }
    if((streams & 4)!=0){
        logStream->write(data.toAscii());
        logStream->flush();
    }
}
