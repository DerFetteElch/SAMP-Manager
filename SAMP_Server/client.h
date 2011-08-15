#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QtNetwork>
#include <QThread>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QCryptographicHash>

#include "samp/sampserver.h"
#include "xmlparser.h"
#include "../packets.h"

#include "crypt/simpleqtrc5.h"

//#include <QDebug>

class FileData{
    public:
        int size;
        int parts;
        QString fileName;
        QFile* file;
};

class Client : public QThread{
    Q_OBJECT
    public:
        Client(QObject *parent = 0);
        Client(int socketDescriptor,XMLParser* xml,SAMPServer* samp,QObject *parent = 0);

        void run();

    private:
        QTcpSocket *clientSocket;
        void send(QByteArray data);
        QByteArray read();
        SimpleQtRC5::Key* cryptKey;

        XMLParser* xml;
        SAMPServer* samp;

        bool login;
        int userId;
        int userAdmin;
        QString userName;

        QString getUserNameFromID(int id);

        QStringList fileManagerPath;

        FileData dataFile;

    private slots:
        void readyRead();
        void disconnected();

        void onNewServer(int id,int port,int owner);
        void onChangeServer(int id,int port,int newOwner,int oldOwner);
        void onDeleteServer(int id,int owner);

        void onNewUser(int id,QString name);
        void onChangeUser(int id,int admin,int banned);
        void onDeleteUser(int id);

        void onNewNews(int id,QString headline,int time,QString text);
        void onChangeNews(int id,QString headline,QString text);
        void onDeleteNews(int id);

    signals:
        void newServer(int,int,int);
        void changeServer(int,int,int,int);
        void deleteServer(int,int);

        void newUser(int,QString);
        void changeUser(int,int,int);
        void deleteUser(int);

        void newNews(int,QString,int,QString);
        void changeNews(int,QString,QString);
        void deleteNews(int);

        void output(int,QString);
};

#endif // CLIENT_H
