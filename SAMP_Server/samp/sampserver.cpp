#include "sampserver.h"

SAMPServer::SAMPServer(){

}

SAMPServer::SAMPServer(XMLParser * xml) : xml(xml){

}

bool SAMPServer::newServer(int serverId){
    QDir dir(".");
    if(!QFile::exists("servers")) dir.mkdir("servers");
    if(QFile::exists(QString("servers/server_%1").arg(serverId))){
        return false;
    }else{
        dir.mkdir(QString("servers/server_%1").arg(serverId));
        dir.mkdir(QString("servers/server_%1/gamemodes").arg(serverId));
        dir.mkdir(QString("servers/server_%1/filterscripts").arg(serverId));
        dir.mkdir(QString("servers/server_%1/npcmodes").arg(serverId));
        dir.mkdir(QString("servers/server_%1/npcmodes/recordings").arg(serverId));
        dir.mkdir(QString("servers/server_%1/plugins").arg(serverId));
        dir.mkdir(QString("servers/server_%1/scriptfiles").arg(serverId));
        dir.mkdir(QString("servers/server_%1/logs").arg(serverId));
        dir.mkdir(QString("servers/server_%1/logs/mysql").arg(serverId));
        dir.mkdir(QString("servers/server_%1/logs/samp").arg(serverId));

        QDir serverData("serverData");
        QFileInfoList fileList=serverData.entryInfoList();
        for(int i=0;i<fileList.count();i++){
            if(fileList.at(i).isDir()) continue;
            QFile::copy(QString("serverData/%1").arg(fileList.at(i).fileName()),QString("servers/server_%1/%2").arg(serverId).arg(fileList.at(i).fileName()));
        }

        fileList.clear();
        QDir plugins("serverData/plugins");
        fileList=plugins.entryInfoList();
        for(int i=0;i<fileList.count();i++){
            QFile::copy(QString("serverData/plugins/%1").arg(fileList.at(i).fileName()),QString("servers/server_%1/plugins/%2").arg(serverId).arg(fileList.at(i).fileName()));
        }

#ifdef Q_WS_X11
        QFile::copy(QString("servers/server_%1/samp03svr").arg(serverId),QString("servers/server_%1/samp03svr_%1").arg(serverId));
#endif
        xml->setAttribute(QString("data/server/server_%1/systemVersion").arg(serverId),"value",1);
        xml->saveToFile();
        return true;
    }
}

void SAMPServer::updateServers(){
    QStringList list=xml->getElements("data/server");
    for(int i=0;i<list.count();i++){
        int serverId=list.at(i).split("_").at(1).toInt();
        int v=xml->getAttribute(QString("data/server/server_%1/systemVersion").arg(serverId),"value",0);
        if(v!=1){
            bool restart=false;
            if(getServerInfo(serverId).online){
                stopServer(serverId);
                restart=true;
            }
            if(v==0){
#ifdef Q_WS_X11
                QFile::copy(QString("servers/server_%1/samp03svr").arg(serverId),QString("servers/server_%1/samp03svr_%1").arg(serverId));
                QFile::remove(QString("servers/server_%1/samp03svr").arg(serverId));
#endif
                v++;
            }
            xml->setAttribute(QString("data/server/server_%1/systemVersion").arg(serverId),"value",v);
            xml->saveToFile();
            if(restart){
                startServer(serverId);
            }
        }
    }
}

bool SAMPServer::startServer(int serverId){
    writeServerCfg(serverId);
    if(QFile::exists(QString("servers/server_%1/server.pid").arg(serverId))){
        if(getServerInfo(serverId).online){
            return false;
        }else{
            xml->setAttribute(QString("data/server/server_%1/restartCount").arg(serverId),"value",0);
            xml->saveToFile();
            QFile::remove(QString("servers/server_%1/server.pid").arg(serverId));
            QFile::copy(QString("servers/server_%1/server_log.txt").arg(serverId),QString("servers/server_%1/logs/samp/%2.txt").arg(serverId).arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")));
            QFile::remove(QString("servers/server_%1/server_log.txt").arg(serverId));
            if(QFile::exists(QString("servers/server_%1/mysql_log.txt").arg(serverId))){
                QFile::copy(QString("servers/server_%1/mysql_log.txt").arg(serverId),QString("servers/server_%1/logs/mysql/%2.txt").arg(serverId).arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")));
                QFile::remove(QString("servers/server_%1/mysql_log.txt").arg(serverId));
            }
            QProcess process;
#ifdef Q_WS_WIN
            process.setWorkingDirectory(QString("servers/server_%1").arg(serverId));
            process.start(QString("servers/server_%1/start.exe").arg(serverId));
#endif
#ifdef Q_WS_X11
            QFile file(QString("server_%1_start.sh").arg(serverId));
            file.open(QIODevice::WriteOnly);
            file.write("#!/bin/sh\n");
            file.write(QString("cd servers/server_%1/\n").arg(serverId).toAscii());
            file.write(QString("./samp03svr_%1\n").arg(serverId).toAscii());
            file.flush();
            file.close();
            process.start(QString("sh server_%1_start.sh").arg(serverId));
#endif
            process.closeWriteChannel();
            process.waitForFinished();
            QFile log(QString("servers/server_%1/system_log.txt").arg(serverId));
            log.open(QIODevice::Append);
            log.write(QString("[%1] Server started!\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).toAscii());
            log.flush();
            log.close();
            return true;
        }
    }else{
        xml->setAttribute(QString("data/server/server_%1/restartCount").arg(serverId),"value",0);
        xml->saveToFile();
        QProcess process;
#ifdef Q_WS_WIN
        process.setWorkingDirectory(QString("servers/server_%1").arg(serverId));
        process.start(QString("servers/server_%1/start.exe").arg(serverId));
#endif
#ifdef Q_WS_X11
        QFile file(QString("server_%1_start.sh").arg(serverId));
        file.open(QIODevice::WriteOnly);
        file.write("#!/bin/sh\n");
        file.write(QString("cd servers/server_%1/\n").arg(serverId).toAscii());
        file.write(QString("./samp03svr_%1\n").arg(serverId).toAscii());
        file.flush();
        file.close();
        process.start(QString("sh server_%1_start.sh").arg(serverId));
#endif
        process.closeWriteChannel();
        process.waitForFinished();
        QFile log(QString("servers/server_%1/system_log.txt").arg(serverId));
        log.open(QIODevice::Append);
        log.write(QString("[%1] Server started!\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).toAscii());
        log.flush();
        log.close();
        return true;
    }
}
bool SAMPServer::stopServer(int serverId){
    if(getServerInfo(serverId).online){
        QProcess process;
#ifdef Q_WS_WIN
        process.setWorkingDirectory(QString("servers/server_%1").arg(serverId));
        process.start(QString("servers/server_%1/stop.exe").arg(serverId));
#endif
#ifdef Q_WS_X11
        QFile file(QString("server_%1_stop.sh").arg(serverId));
        file.open(QIODevice::WriteOnly);
        file.write("#!/bin/sh\n");
        file.write(QString("killall samp03svr_%1\n").arg(serverId).toAscii());
        file.flush();
        file.close();
        process.start(QString("sh server_%1_stop.sh").arg(serverId));
#endif
        process.closeWriteChannel();
        process.waitForFinished();
        QFile::copy(QString("servers/server_%1/server_log.txt").arg(serverId),QString("servers/server_%1/logs/samp/%2.txt").arg(serverId).arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")));
        QFile::remove(QString("servers/server_%1/server_log.txt").arg(serverId));
        if(QFile::exists(QString("servers/server_%1/mysql_log.txt").arg(serverId))){
            QFile::copy(QString("servers/server_%1/mysql_log.txt").arg(serverId),QString("servers/server_%1/logs/mysql/%2.txt").arg(serverId).arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")));
            QFile::remove(QString("servers/server_%1/mysql_log.txt").arg(serverId));
        }
        QFile log(QString("servers/server_%1/system_log.txt").arg(serverId));
        log.open(QIODevice::Append);
        log.write(QString("[%1] Server stoped!\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).toAscii());
        log.flush();
        log.close();
        return true;
    }else{
        return false;
    }
}
bool SAMPServer::restartServer(int serverId){
    writeServerCfg(serverId);
    if(getServerInfo(serverId).online){
        QProcess process;
#ifdef Q_WS_WIN
        process.setWorkingDirectory(QString("servers/server_%1").arg(serverId));
        process.start(QString("servers/server_%1/stop.exe").arg(serverId));
#endif
#ifdef Q_WS_X11
        QFile file(QString("server_%1_stop.sh").arg(serverId));
        file.open(QIODevice::WriteOnly);
        file.write("#!/bin/sh\n");
        file.write(QString("killall samp03svr_%1\n").arg(serverId).toAscii());
        file.flush();
        file.close();
        process.start(QString("sh server_%1_stop.sh").arg(serverId));
#endif
        process.closeWriteChannel();
        process.waitForFinished();
        QFile::copy(QString("servers/server_%1/server_log.txt").arg(serverId),QString("servers/server_%1/logs/samp/%2.txt").arg(serverId).arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")));
        QFile::remove(QString("servers/server_%1/server_log.txt").arg(serverId));
        if(QFile::exists(QString("servers/server_%1/mysql_log.txt").arg(serverId))){
            QFile::copy(QString("servers/server_%1/mysql_log.txt").arg(serverId),QString("servers/server_%1/logs/mysql/%2.txt").arg(serverId).arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")));
            QFile::remove(QString("servers/server_%1/mysql_log.txt").arg(serverId));
        }
        QProcess process2;
#ifdef Q_WS_WIN
        process2.setWorkingDirectory(QString("servers/server_%1").arg(serverId));
        process.start(QString("servers/server_%1/start.exe").arg(serverId));
#endif
#ifdef Q_WS_X11
        QFile file2(QString("server_%1_start.sh").arg(serverId));
        file2.open(QIODevice::WriteOnly);
        file2.write("#!/bin/sh\n");
        file2.write(QString("cd servers/server_%1/\n").arg(serverId).toAscii());
        file2.write(QString("./samp03svr_%1\n").arg(serverId).toAscii());
        file2.flush();
        file2.close();
        process.start(QString("sh server_%1_start.sh").arg(serverId));
#endif
        process2.closeWriteChannel();
        process2.waitForFinished();
        QFile log(QString("servers/server_%1/system_log.txt").arg(serverId));
        log.open(QIODevice::Append);
        log.write(QString("[%1] Server restarted!\n").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss")).toAscii());
        log.flush();
        log.close();
        return true;
    }else{
        return false;
    }
}

SAMPServerInfo SAMPServer::getServerInfo(int serverId){
    SAMPServerInfo serverInfo;
    int port=xml->getAttribute(QString("data/server/server_%1/port").arg(serverId),"value",0);

    QByteArray data;
    data.append("SAMP");
    data.append((char)127);
    data.append((char)0);
    data.append((char)0);
    data.append((char)1);
    data.append((char)(port & 0xFF));
    data.append((char)(port >> 8 & 0xFF));
    data.append('i');

    QUdpSocket udpSocket;
    udpSocket.connectToHost(QHostAddress::LocalHost,port);
    udpSocket.waitForConnected();

    udpSocket.write(data);
    udpSocket.waitForReadyRead();

    QByteArray ret=udpSocket.readAll();

    if(ret.count()==0){
        serverInfo.online=false;
        return serverInfo;
    }else{
        serverInfo.online=true;
    }

    int pos=12;

    int player=0;
    player+=ret.at(pos++);
    player+=(ret.at(pos++)<<8);

    int maxPlayer=0;
    maxPlayer+=ret.at(pos++);
    maxPlayer+=(ret.at(pos++)<<8);

    int hostLen=0;
    hostLen+=ret.at(pos++);
    hostLen+=(ret.at(pos++)<<8);
    hostLen+=(ret.at(pos++)<<16);
    hostLen+=(ret.at(pos++)<<24);

    QString hostName;
    for(int i=0;i<hostLen;i++){
        hostName.append(ret.at(pos++));
    }

    int gameLen=0;
    gameLen+=ret.at(pos++);
    gameLen+=(ret.at(pos++)<<8);
    gameLen+=(ret.at(pos++)<<16);
    gameLen+=(ret.at(pos++)<<24);

    QString gameName;
    for(int i=0;i<gameLen;i++){
        gameName.append(ret.at(pos++));
    }

    int mapLen=0;
    mapLen+=ret.at(pos++);
    mapLen+=(ret.at(pos++)<<8);
    mapLen+=(ret.at(pos++)<<16);
    mapLen+=(ret.at(pos++)<<24);

    QString mapName;
    for(int i=0;i<mapLen;i++){
        mapName.append(ret.at(pos++));
    }

    serverInfo.playerOnline=player;
    serverInfo.maxPlayer=maxPlayer;
    serverInfo.serverName=hostName;
    serverInfo.gamemodeName=gameName;
    serverInfo.mapName=mapName;
    return serverInfo;
}
QStringList SAMPServer::rconCmd(int serverId, QString cmd){
    if(getServerInfo(serverId).online){
        int port=xml->getAttribute(QString("data/server/server_%1/port").arg(serverId),"value",0);
        QString rconPassword=xml->getAttribute(QString("data/server/server_%1/config/rconPassword").arg(serverId),"value","");

        QByteArray data;
        data.append("SAMP");
        data.append((char)127);
        data.append((char)0);
        data.append((char)0);
        data.append((char)1);
        data.append((char)(port & 0xFF));
        data.append((char)(port >> 8 & 0xFF));
        data.append('x');

        int pw=rconPassword.size();
        data.append((char)(pw & 0xFF));
        data.append((char)(pw >> 8 & 0xFF));
        data.append(rconPassword);

        int c=cmd.size();
        data.append((char)(c & 0xFF));
        data.append((char)(c >> 8 & 0xFF));
        data.append(cmd);

        QUdpSocket udpSocket;
        udpSocket.connectToHost(QHostAddress::LocalHost,port);
        udpSocket.waitForConnected();

        udpSocket.write(data);
        Sleep::sleep(1);
        udpSocket.waitForReadyRead();

        QByteArray returnData=udpSocket.readAll();

        if(!returnData.isEmpty()){
            QByteArray sep=returnData.left(11);
            returnData.remove(0,11);
            returnData.replace(sep,"\n");
            QList<QByteArray> byteLines=returnData.split('\n');
            QStringList ret;
            for(int i=0;i<byteLines.count();i++){
                QByteArray line=byteLines.at(i);
                line.remove(0,2);
                if(line.size()>1){
                    ret.append(QString(line));
                }
            }
            ret.append("\n");
            return ret;
        }else{
            return QStringList();
        }
    }else{
        return QStringList();
    }
}

QStringList SAMPServer::getGamemodes(int serverId){
    QDir dir(QString("servers/server_%1/gamemodes").arg(serverId));
    QFileInfoList fileList=dir.entryInfoList();
    QStringList ret;
    for(int i=0;i<fileList.count();i++){
        if(fileList.at(i).completeSuffix()=="amx"){
            ret.append(fileList.at(i).baseName());
        }
    }
    return ret;
}
QStringList SAMPServer::getFilterscripts(int serverId){
    QDir dir(QString("servers/server_%1/filterscripts").arg(serverId));
    QFileInfoList fileList=dir.entryInfoList();
    QStringList ret;
    for(int i=0;i<fileList.count();i++){
        if(fileList.at(i).completeSuffix()=="amx"){
            ret.append(fileList.at(i).baseName());
        }
    }
    return ret;
}
QStringList SAMPServer::getPlugins(int serverId){
    QDir dir(QString("servers/server_%1/plugins").arg(serverId));
    QFileInfoList fileList=dir.entryInfoList();
    QStringList ret;
    for(int i=0;i<fileList.count();i++){
#ifdef Q_WS_WIN
        if(fileList.at(i).completeSuffix()=="dll"){
#endif
#ifdef Q_WS_X11
        if(fileList.at(i).completeSuffix()=="so"){
#endif
            ret.append(fileList.at(i).baseName());
        }
    }
    return ret;
}

QString SAMPServer::getServerLog(int serverId){
    QFile file(QString("servers/server_%1/server_log.txt").arg(serverId));
    if(!file.open(QIODevice::ReadOnly)){
        return QString();
    }
    QByteArray data=file.readAll();
    file.close();
    return QString(data);
}
QString SAMPServer::getMysqlLog(int serverId){
    QFile file(QString("servers/server_%1/mysql_log.txt").arg(serverId));
    if(!file.open(QIODevice::ReadOnly)){
        return QString();
    }
    QByteArray data=file.readAll();
    file.close();
    return QString(data);
}
QString SAMPServer::getSystemLog(int serverId){
    QFile file(QString("servers/server_%1/system_log.txt").arg(serverId));
    if(!file.open(QIODevice::ReadOnly)){
        return QString();
    }
    QByteArray data=file.readAll();
    file.close();
    return QString(data);
}

void SAMPServer::writeServerCfg(int serverId){
    QFile file(QString("servers/server_%1/server.cfg").arg(serverId));
    file.open(QIODevice::WriteOnly);
    file.write("echo Executing Server Config...\n");
    file.write("lanmode 0\n");
    file.write(QString("maxplayers %1\n").arg(xml->getAttribute(QString("data/server/server_%1/maxPlayers").arg(serverId),"value",0)).toAscii());
    file.write(QString("port %1\n").arg(xml->getAttribute(QString("data/server/server_%1/port").arg(serverId),"value",0)).toAscii());
    file.write(QString("rcon_password %1\n").arg(xml->getAttribute(QString("data/server/server_%1/config/rconPassword").arg(serverId),"value","")).toAscii());
    file.write(QString("hostname %1\n").arg(xml->getAttribute(QString("data/server/server_%1/config/hostname").arg(serverId),"value","Server")).toAscii());
    file.write(QString("announce %1\n").arg(xml->getAttribute(QString("data/server/server_%1/config/announce").arg(serverId),"value",0)).toAscii());
    file.write(QString("weburl %1\n").arg(xml->getAttribute(QString("data/server/server_%1/config/weburl").arg(serverId),"value","www")).toAscii());
    file.write(QString("maxnpc %1\n").arg(xml->getAttribute(QString("data/server/server_%1/maxNPCs").arg(serverId),"value",0)).toAscii());
    file.write(QString("password %1\n").arg(xml->getAttribute(QString("data/server/server_%1/serverPassword").arg(serverId),"value","")).toAscii());
    file.write(QString("mapname %1\n").arg(xml->getAttribute(QString("data/server/server_%1/mapname").arg(serverId),"value","Map")).toAscii());

    QStringList filterScriptFiles=getFilterscripts(serverId);
    QStringList list=xml->getElements(QString("data/server/server_%1/filterscripts").arg(serverId));
    QString filterscripts("");
    for(int i=0;i<list.count();i++){
        if(xml->getAttribute(QString("data/server/server_%1/filterscripts/%2").arg(serverId).arg(list.at(i)),"value",0)==1 && filterScriptFiles.contains(list.at(i))){
            filterscripts.append(list.at(i));
            filterscripts.append(" ");
        }
    }
    file.write(QString("filterscripts %1\n").arg(filterscripts).toAscii());

    QStringList filterPluginFiles=getPlugins(serverId);
    list=xml->getElements(QString("data/server/server_%1/plugins").arg(serverId));
    QString plugins("");
    for(int i=0;i<list.count();i++){
        if(xml->getAttribute(QString("data/server/server_%1/plugins/%2").arg(serverId).arg(list.at(i)),"value",0)==1 && filterPluginFiles.contains(list.at(i))){
            plugins.append(list.at(i));
#ifdef Q_WS_X11
            plugins.append(".so");
#endif
            plugins.append(" ");
        }
    }
    file.write(QString("plugins %1\n").arg(plugins).toAscii());

    for(int i=0;i<8;i++){
        if(xml->getAttribute(QString("data/server/server_%1/gamemodes/gm_%2").arg(serverId).arg(i),"name","")=="") break;
        file.write(QString("gamemode%1 %2 %3\n").arg(i).arg(xml->getAttribute(QString("data/server/server_%1/gamemodes/gm_%2").arg(serverId).arg(i),"name","")).arg(xml->getAttribute(QString("data/server/server_%1/gamemodes/gm_%2").arg(serverId).arg(i),"num",0)).toAscii());
    }

    file.write("onfoot_rate 40\n");
    file.write("incar_rate 40\n");
    file.write("weapon_rate 40\n");
    file.write("stream_distance 400\n");
    file.write("stream_rate 500\n");
    file.flush();
    file.close();
}

void SAMPServer::checkServer(int serverId){
    if(xml->getAttribute(QString("data/server/server_%1/online").arg(serverId),"value",0)==1 && !getServerInfo(serverId).online){
        int num=xml->getAttribute(QString("data/server/server_%1/restartCount").arg(serverId),"value",0);
        if(num<=3){
            xml->setAttribute(QString("data/server/server_%1/restartCount").arg(serverId),"value",num+1);
            xml->saveToFile();
            startServer(serverId);
        }
    }else if(getServerInfo(serverId).online && xml->getAttribute(QString("data/server/server_%1/restartCount").arg(serverId),"value",0)!=0){
        xml->setAttribute(QString("data/server/server_%1/restartCount").arg(serverId),"value",0);
    }
}
void SAMPServer::checkServers(){
    QStringList list=xml->getElements("data/server");
    for(int i=0;i<list.count();i++){
        checkServer(list.at(i).split("_").at(1).toInt());
    }
}
