#include "xmlparser.h"

XMLParser::XMLParser(QObject *parent) : QObject(parent){

}
XMLParser::XMLParser(QDomDocument doc, QObject *parent) : QObject(parent),document(doc){

}

QString XMLParser::getText(QString path, QString defaultValue, bool create){
    if(path.isEmpty()) return defaultValue;
    QStringList pathList=path.split('/');
    QDomNodeList nodeList=document.elementsByTagName(pathList.at(0));
    if(nodeList.count()>0){
        QDomElement element=nodeList.at(0).toElement();
        if(pathList.count()==1){
            return element.text();
        }else{
            for(int i=1;i<pathList.count();i++){
                nodeList=element.elementsByTagName(pathList.at(i));
                if(nodeList.count()>0){
                    element=nodeList.at(0).toElement();
                }else{
                    if(create) setText(path,defaultValue);
                    return defaultValue;
                }
            }
            return element.text();
        }
    }else{
        if(create) setText(path,defaultValue);
        return defaultValue;
    }
}
void XMLParser::setText(QString path, QString value){
    if(path.isEmpty()) return;
    QDomText text=document.createTextNode(value);
    QStringList pathList=path.split('/');
    QDomNodeList nodeList=document.elementsByTagName(pathList.at(0));
    QDomElement element;
    if(nodeList.count()>0){
        element=nodeList.at(0).toElement();
    }else{
        element=document.createElement(pathList.at(0));
        document.appendChild(element);
    }
    if(pathList.count()==1){
        element.appendChild(text);
    }else{
        for(int i=1;i<pathList.count();i++){
            nodeList=element.elementsByTagName(pathList.at(i));
            QDomElement newElement;
            if(nodeList.count()>0){
                newElement=nodeList.at(0).toElement();
            }else{
                newElement=document.createElement(pathList.at(i));
                element.appendChild(newElement);
            }
            element=newElement;
        }
        element.appendChild(text);
    }
}

QString XMLParser::getAttribute(QString path, QString attribute, QString defaultValue, bool create){
    if(path.isEmpty()) return defaultValue;
    QStringList pathList=path.split('/');
    QDomNodeList nodeList=document.elementsByTagName(pathList.at(0));
    if(nodeList.count()>0){
        QDomElement element=nodeList.at(0).toElement();
        if(pathList.count()==1){
            QString value=element.attribute(attribute);
            if(create && value!=defaultValue && !defaultValue.isEmpty()){
                setAttribute(path,attribute,defaultValue);
            }
            if(value.isEmpty()) return defaultValue;
            return value;
        }else{
            for(int i=1;i<pathList.count();i++){
                nodeList=element.elementsByTagName(pathList.at(i));
                if(nodeList.count()>0){
                    element=nodeList.at(0).toElement();
                }else{
                    if(create) setAttribute(path,attribute,defaultValue);
                    return defaultValue;
                }
            }
            QString value=element.attribute(attribute);;
            if(create && value!=defaultValue && !defaultValue.isEmpty()){
                setAttribute(path,attribute,defaultValue);
            }
            if(value.isEmpty()) return defaultValue;
            return value;
        }
    }else{
        if(create) setAttribute(path,attribute,defaultValue);
        return defaultValue;
    }
}
int XMLParser::getAttribute(QString path, QString attribute, int defaultValue, bool create){
    return getAttribute(path,attribute,QString("%1").arg(defaultValue),create).toInt();
}
float XMLParser::getAttribute(QString path, QString attribute, float defaultValue, bool create){
    return getAttribute(path,attribute,QString("%1").arg(defaultValue),create).toFloat();
}

void XMLParser::setAttribute(QString path, QString attribute, QString value){
    if(path.isEmpty()) return;
    QStringList pathList=path.split('/');
    QDomNodeList nodeList=document.elementsByTagName(pathList.at(0));
    QDomElement element;
    if(nodeList.count()>0){
        element=nodeList.at(0).toElement();
    }else{
        element=document.createElement(pathList.at(0));
        document.appendChild(element);
    }
    if(pathList.count()==1){
        element.setAttribute(attribute,value);
    }else{
        for(int i=1;i<pathList.count();i++){
            nodeList=element.elementsByTagName(pathList.at(i));
            QDomElement newElement;
            if(nodeList.count()>0){
                newElement=nodeList.at(0).toElement();
            }else{
                newElement=document.createElement(pathList.at(i));
                element.appendChild(newElement);
            }
            element=newElement;
        }
        element.setAttribute(attribute,value);
    }
}
void XMLParser::setAttribute(QString path, QString attribute, int value){
    setAttribute(path,attribute,QString("%1").arg(value));
}
void XMLParser::setAttribute(QString path, QString attribute, float value){
    setAttribute(path,attribute,QString("%1").arg(value));
}

bool XMLParser::existPath(QString path){
    if(path.isEmpty()) return false;
    QStringList pathList=path.split('/');
    QDomNodeList nodeList=document.elementsByTagName(pathList.at(0));
    if(nodeList.count()>0){
        QDomElement element=nodeList.at(0).toElement();
        if(pathList.count()==1){
            return true;
        }else{
            for(int i=1;i<pathList.count();i++){
                nodeList=element.elementsByTagName(pathList.at(i));
                if(nodeList.count()>0){
                    element=nodeList.at(0).toElement();
                }else{
                    return false;
                }
            }
            return true;
        }
    }else{
        return false;
    }
}
bool XMLParser::deleteChild(QString path, QString name){
    if(path.isEmpty()) return false;
    QStringList pathList=path.split('/');
    QDomNodeList nodeList=document.elementsByTagName(pathList.at(0));
    if(nodeList.count()>0){
        QDomElement element=nodeList.at(0).toElement();
        if(pathList.count()==1){
            element.removeChild(element.elementsByTagName(name).at(0));
            return true;
        }else{
            for(int i=1;i<pathList.count();i++){
                nodeList=element.elementsByTagName(pathList.at(i));
                if(nodeList.count()>0){
                    element=nodeList.at(0).toElement();
                }else{
                    return false;
                }
            }
            element.removeChild(element.elementsByTagName(name).at(0));
            return true;
        }
    }else{
        return false;
    }
}

bool XMLParser::loadFromFile(QString name, bool create){
    if(QFile::exists(name)){
        QFile file(name);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        document.setContent(&file);
        file.close();
        openFileName=name;
        return true;
    }else if(create){
        QFile file(name);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.close();
        openFileName=name;
        return true;
    }else{
        return false;
    }
}
bool XMLParser::saveToFile(){
    QFile file(openFileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        return false;
    }
    file.write(document.toByteArray(4));
    file.close();
    return true;
}
bool XMLParser::saveToFileName(QString name){
    QFile file(name);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        return false;
    }
    file.write(document.toByteArray(4));
    file.close();
    return true;
}

QStringList XMLParser::getElements(QString path){
    if(path.isEmpty()) return QStringList();
    QStringList pathList=path.split('/');
    QDomNodeList nodeList=document.elementsByTagName(pathList.at(0));
    if(nodeList.count()>0){
        QDomElement element=nodeList.at(0).toElement();
        if(pathList.count()==1){
            nodeList=element.childNodes();
            QStringList list;
            for(int i=0;i<nodeList.count();i++){
                list.append(nodeList.at(i).nodeName());
            }
            return list;
        }else{
            for(int i=1;i<pathList.count();i++){
                nodeList=element.elementsByTagName(pathList.at(i));
                if(nodeList.count()>0){
                    element=nodeList.at(0).toElement();
                }else{
                    return QStringList();
                }
            }
            nodeList=element.childNodes();
            QStringList list;
            for(int i=0;i<nodeList.count();i++){
                list.append(nodeList.at(i).nodeName());
            }
            return list;
        }
    }else{
        return QStringList();
    }
}

QDomElement XMLParser::getElement(QString path){
    if(path.isEmpty()) return QDomElement();
    QStringList pathList=path.split('/');
    QDomNodeList nodeList=document.elementsByTagName(pathList.at(0));
    if(nodeList.count()>0){
        QDomElement element=nodeList.at(0).toElement();
        if(pathList.count()==1){
            return element;
        }else{
            for(int i=1;i<pathList.count();i++){
                nodeList=element.elementsByTagName(pathList.at(i));
                if(nodeList.count()>0){
                    element=nodeList.at(0).toElement();
                }else{
                    return QDomElement();
                }
            }
            return element;
        }
    }else{
        return QDomElement();
    }
}
