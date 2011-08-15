#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QtNetwork>
#include <QTimer>
#include <QFile>

#include "xmlparser.h"
#include "client.h"
#include "samp/sampserver.h"


class Server : public QTcpServer{
    Q_OBJECT
    public:
        Server(QObject *parent = 0);
        ~Server();

    protected:
        void incomingConnection(int socketDescriptor);

    private:
        XMLParser* xml;
        SAMPServer* samp;

        QTimer* timer;

        QFile* outStream;
        QFile* errStream;
        QFile* logStream;

    private slots:
        void update();

        void onNewServer(int id,int port,int owner);
        void onChangeServer(int id,int port,int newOwner,int oldOwner);
        void onDeleteServer(int id,int owner);

        void onNewUser(int id,QString name);
        void onChangeUser(int id,int admin,int banned);
        void onDeleteUser(int id);

        void onNewNews(int id,QString headline,int time,QString text);
        void onChangeNews(int id,QString headline,QString text);
        void onDeleteNews(int id);

        void output(int streams,QString data);

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
};

#endif // SERVER_H
