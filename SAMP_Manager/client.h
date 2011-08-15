#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>

#include "gui/login.h"
#include "gui/mainwindow.h"
#include "gui/adminwindow.h"

#include <QMessageBox>
#include <QString>
#include <QCryptographicHash>

#include "crypt/simpleqtrc5.h"

class Client : public QObject{
    Q_OBJECT
    public:
        Client(QObject *parent = 0);
        void connectToHost(QString host,int port);

    private:
        QTcpSocket* socket;
        SimpleQtRC5::Key* cryptKey;

        Login* login;
        MainWindow* mainWindow;
        AdminWindow* adminWindow;

    signals:
        void readyRead(QByteArray);

    private slots:
        void connectedToServer();
        void readyRead();
        void send(QByteArray data);

        void onGuiClosed();
        void loginSuccess(int admin);
        void onLogout();

        void error(QAbstractSocket::SocketError socketError);
};

#endif // CLIENT_H
