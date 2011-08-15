#include "adminaddserver.h"
#include "ui_adminaddserver.h"

AdminAddServer::AdminAddServer(QWidget *parent) : QDialog(parent),ui(new Ui::AdminAddServer){
    ui->setupUi(this);
    addServ=false;
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);
}

AdminAddServer::~AdminAddServer(){
    delete ui;
}

void AdminAddServer::errorMsg(QString data){
    addServ=false;
    QMessageBox::critical(this,tr("Error"),data);
}

void AdminAddServer::on_addButton_clicked(){
    addServ=true;
    emit(addServer(ui->port->value(),ui->maxPlayers->value(),ui->maxNPCs->value()));
}
void AdminAddServer::on_cancelButton_clicked(){
    close();
}
bool AdminAddServer::event(QEvent *e){
    if(e->type()==QEvent::Close){
        addServ=false;
        emit(guiClosed());
    }
    return QDialog::event(e);
}
