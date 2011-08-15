#include "filecreatedir.h"
#include "ui_filecreatedir.h"

FileCreateDir::FileCreateDir(QWidget *parent) : QDialog(parent), ui(new Ui::FileCreateDir){
    ui->setupUi(this);
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);
}

FileCreateDir::~FileCreateDir(){
    delete ui;
}

void FileCreateDir::errorMsg(QString data){
    QMessageBox::critical(this,tr("Error"),data);
}

void FileCreateDir::on_createDirButton_clicked(){
    if(ui->name->text().isEmpty()){
        errorMsg(tr("Enter a Name!"));
    }else{
        emit(createDir(ui->name->text()));
        close();
    }
}
void FileCreateDir::on_cancelButton_clicked(){
    close();
}
bool FileCreateDir::event(QEvent *e){
    if(e->type()==QEvent::Close){
        emit(guiClosed());
    }
    return QDialog::event(e);
}
