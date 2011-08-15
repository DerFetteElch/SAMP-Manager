#include "changepassword.h"
#include "ui_changepassword.h"

ChangePassword::ChangePassword(QWidget *parent) : QDialog(parent), ui(new Ui::ChangePassword){
    ui->setupUi(this);
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);
}

ChangePassword::~ChangePassword(){
    delete ui;
}

void ChangePassword::errorMsg(QString data){
    QMessageBox::critical(this,tr("Error"),data);
}

void ChangePassword::on_changePasswordButton_clicked(){
    if(ui->oldPassword->text().isEmpty() || ui->newPassword1->text().isEmpty() || ui->newPassword2->text().isEmpty()){
        errorMsg(tr("All fields are required!"));
    }else if(ui->newPassword1->text()!=ui->newPassword2->text()){
        errorMsg(tr("Passwords are different!"));
    }else{
        emit(changePassword(ui->oldPassword->text(),ui->newPassword1->text()));
    }
}

void ChangePassword::on_cancelButton_clicked(){
    close();
}
bool ChangePassword::event(QEvent *e){
    if(e->type()==QEvent::Close){
        emit(guiClosed());
    }
    return QDialog::event(e);
}
