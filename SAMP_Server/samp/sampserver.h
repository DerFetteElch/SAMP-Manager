#ifndef SAMPSERVER_H
#define SAMPSERVER_H

#include <QDir>
#include <QFile>
#include <QString>
#include <QUdpSocket>
#include <QThread>

#include "xmlparser.h"

class Sleep : public QThread{
    Q_OBJECT
    public:
        static void sleep(unsigned long time){
            QThread::sleep(time);
        }
};


class SAMPServerInfo{
    public:
        SAMPServerInfo(){
            online=false;
            playerOnline=0;
            maxPlayer=0;
        }
        bool online;
        int playerOnline;
        int maxPlayer;
        QString serverName;
        QString gamemodeName;
        QString mapName;
};

class SAMPServer{
    public:
        SAMPServer();
        SAMPServer(XMLParser * xml);

        bool newServer(int serverId);

        bool startServer(int serverId);
        bool stopServer(int serverId);
        bool restartServer(int serverId);

        SAMPServerInfo getServerInfo(int serverId);
        QStringList rconCmd(int serverId,QString cmd);

        QStringList getGamemodes(int serverId);
        QStringList getFilterscripts(int serverId);
        QStringList getPlugins(int serverId);

        QString getServerLog(int serverId);
        QString getMysqlLog(int serverId);
        QString getSystemLog(int serverId);

        void checkServer(int serverId);
        void checkServers();

    private:
        void writeServerCfg(int serverId);

        XMLParser * xml;
};

#endif // SAMPSERVER_H
