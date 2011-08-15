#include <QtCore/QCoreApplication>
#include <QTime>
#include <QSettings>

#include "server.h"

int main(int argc, char *argv[]){
    QCoreApplication a(argc, argv);
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    QSettings settings("config.ini",QSettings::IniFormat);

    Server server;
    server.listen(QHostAddress::Any,settings.value("config/port",9999).toInt());

    return a.exec();
}
