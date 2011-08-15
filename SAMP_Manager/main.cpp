#include <QtGui/QApplication>
#include "client.h"

#include <QTranslator>
#include <QSettings>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);

    QTranslator myappTranslator;
    myappTranslator.load("sampManager_" + QLocale::system().name());
    a.installTranslator(&myappTranslator);

    QSettings settings("config.ini",QSettings::IniFormat);

    Client client;
    client.connectToHost(settings.value("host/address","127.0.0.1").toString(),settings.value("host/port",9999).toInt());

    return a.exec();
}
