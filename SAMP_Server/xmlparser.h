#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <QObject>
#include <QtXml>
#include <QString>
#include <QStringList>
#include <QFile>

class XMLParser : public QObject{
    Q_OBJECT
    public:
        XMLParser(QObject *parent = 0);
        XMLParser(QDomDocument doc, QObject *parent = 0);

        QString getText(QString path, QString defaultValue=QString(), bool create=false);
        void setText(QString path, QString value);

        QString getAttribute(QString path, QString attribute, QString defaultValue=QString(), bool create=false);
        int getAttribute(QString path, QString attribute, int defaultValue=0, bool create=false);
        float getAttribute(QString path, QString attribute, float defaultValue=0.0f, bool create=false);

        void setAttribute(QString path, QString attribute, QString value);
        void setAttribute(QString path, QString attribute, int value);
        void setAttribute(QString path, QString attribute, float value);

        bool existPath(QString path);

        bool deleteChild(QString path, QString name);

        bool loadFromFile(QString name, bool create=true);
        bool saveToFileName(QString name);
        bool saveToFile();

        QStringList getElements(QString path);

        QDomElement getElement(QString path);
        QDomDocument getDocument(){return document;}

    private:
        QDomDocument document;
        QString openFileName;

};

#endif // XMLPARSER_H
