#include "login.h"
#include "ui_login.h"

Login::Login(QWidget *parent) : QMainWindow(parent), ui(new Ui::Login){
    ui->setupUi(this);
}

Login::~Login(){
    delete ui;
}

void Login::on_loginButton_clicked(){
    if(!ui->name->text().isEmpty() && !ui->password->text().isEmpty()){
        QByteArray data;
        QDataStream stream(&data,QIODevice::WriteOnly);
        stream<<PACKET_CS_LOGIN;
        stream<<ui->name->text();
        stream<<ui->password->text();
        emit(send(data));
    }else{
        ui->statusBar->showMessage(tr("All fields are required!"));
    }
}

void Login::loginResponse(QByteArray data){
    QDataStream stream(&data,QIODevice::ReadOnly);
    int id;
    stream>>id;
    if(id==PACKET_SC_LOGIN_SUCCESS){
        int admin;
        stream>>admin;
        emit(loginSuccess(admin));
    }else if(id==PACKET_SC_LOGIN_FAIL){
        ui->statusBar->showMessage(tr("Wrong name or password!"));
    }else if(id==PACKET_SC_LOGIN_BANNED){
        ui->statusBar->showMessage(tr("You're banned!"));
    }
}

bool Login::event(QEvent *e){
    if(e->type()==QEvent::Close){
        ui->statusBar->clearMessage();
        ui->name->setText("");
        ui->password->setText("");
    }
    return QMainWindow::event(e);
}
