#include "adminadduser.h"
#include "ui_adminadduser.h"

AdminAddUser::AdminAddUser(QWidget *parent) : QDialog(parent),ui(new Ui::AdminAddUser){
    ui->setupUi(this);
    addUse=false;
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);
}

AdminAddUser::~AdminAddUser(){
    delete ui;
}

void AdminAddUser::errorMsg(QString data){
    addUse=false;
    QMessageBox::critical(this,tr("Error"),data);
}

void AdminAddUser::on_addUserButton_clicked(){
    if(ui->password1->text()!=ui->password2->text()){
        errorMsg(tr("Passwords are different!"));
    }else if(ui->name->text().length()<3){
        errorMsg(tr("Name to short!"));
    }else{
        addUse=true;
        emit(addUser(ui->name->text(),ui->password1->text()));
    }
}

void AdminAddUser::on_cancelButton_clicked(){
    close();
}
bool AdminAddUser::event(QEvent *e){
    if(e->type()==QEvent::Close){
        addUse=false;
        emit(guiClosed());
    }
    return QDialog::event(e);
}
