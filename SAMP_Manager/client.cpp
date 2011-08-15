#include "client.h"

Client::Client(QObject *parent) : QObject(parent){
    socket=new QTcpSocket(this);
    connect(socket,SIGNAL(connected()),this,SLOT(connectedToServer()));
    connect(socket,SIGNAL(readyRead()),this,SLOT(readyRead()));
    connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error(QAbstractSocket::SocketError)));

    login=new Login();
    mainWindow=new MainWindow();
    adminWindow=new AdminWindow();

    connect(mainWindow,SIGNAL(guiClosed()),this,SLOT(onGuiClosed()));
    connect(adminWindow,SIGNAL(guiClosed()),this,SLOT(onGuiClosed()));


    connect(this,SIGNAL(readyRead(QByteArray)),login,SLOT(loginResponse(QByteArray)));
    connect(login,SIGNAL(send(QByteArray)),this,SLOT(send(QByteArray)));
    connect(login,SIGNAL(loginSuccess(int)),this,SLOT(loginSuccess(int)));

    connect(this,SIGNAL(readyRead(QByteArray)),mainWindow,SLOT(sendResponse(QByteArray)));
    connect(mainWindow,SIGNAL(send(QByteArray)),this,SLOT(send(QByteArray)));
    connect(mainWindow,SIGNAL(logout()),this,SLOT(onLogout()));

    connect(this,SIGNAL(readyRead(QByteArray)),adminWindow,SLOT(sendResponse(QByteArray)));
    connect(adminWindow,SIGNAL(send(QByteArray)),this,SLOT(send(QByteArray)));

    cryptKey=new SimpleQtRC5::Key(QString(CRYPT_KEY));
}

void Client::connectToHost(QString host, int port){
    socket->connectToHost(host,port);
}
void Client::connectedToServer(){
    login->show();
}
void Client::readyRead(){
    QByteArray data=socket->readAll();
    QByteArray deCrypt;
    SimpleQtRC5::Decryptor d(cryptKey,SimpleQtRC5::RC5_32_32_20,SimpleQtRC5::ModeCFB);
    SimpleQtRC5::Error er=d.decrypt(data,deCrypt,false);
    if(er){
        onGuiClosed();
    }else{
        emit(readyRead(deCrypt));
    }
}
void Client::send(QByteArray data){
    socket->readAll();
    SimpleQtRC5::Encryptor e(cryptKey,SimpleQtRC5::RC5_32_32_20,SimpleQtRC5::ModeCFB,SimpleQtRC5::NoChecksum);
    QByteArray cipher;
    SimpleQtRC5::Error er=e.encrypt(data,cipher,false);
    if(er){
        onGuiClosed();
    }else{
        socket->write(cipher);
        socket->flush();
        data.clear();
    }
}

void Client::loginSuccess(int admin){
    login->hide();
    mainWindow->logined();
    if(admin==1){
        adminWindow->show();
    }
}

void Client::onGuiClosed(){
    login->close();
    mainWindow->close();
    adminWindow->close();
}
void Client::onLogout(){
    mainWindow->close();
    adminWindow->close();
    login->show();
}


void Client::error(QAbstractSocket::SocketError socketError){
    login->close();
    mainWindow->close();
    adminWindow->close();
    if(socketError==QAbstractSocket::ConnectionRefusedError){
        QMessageBox::critical(0,tr("Socket Error"),tr("Connection refused"));
    }else if(socketError==QAbstractSocket::RemoteHostClosedError){
        QMessageBox::critical(0,tr("Socket Error"),tr("Host closed Connection"));
    }else{
        QMessageBox::critical(0,tr("Socket Error"),QString(tr("Socket Error:%1")).arg(socketError));
    }
}
