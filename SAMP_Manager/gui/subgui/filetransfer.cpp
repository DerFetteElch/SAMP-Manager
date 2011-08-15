#include "filetransfer.h"
#include "ui_filetransfer.h"

FileTransfer::FileTransfer(QWidget *parent) : QDialog(parent), ui(new Ui::FileTransfer){
    ui->setupUi(this);
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);
}

FileTransfer::~FileTransfer(){
    delete ui;
}

void FileTransfer::on_cancelButton_clicked(){
    cancel=true;
    ui->cancelButton->setEnabled(false);
}

void FileTransfer::setParts(int amount){
    ui->cancelButton->setEnabled(true);
    cancel=false;
    parts=amount;
    ui->progressBar->setValue(0);
}
void FileTransfer::setPart(int num){
    ui->progressBar->setValue((num*1000)/parts);
}
bool FileTransfer::isCanceled(){
    return cancel;
}
bool FileTransfer::event(QEvent *e){
    if(e->type()==QEvent::Close){
        cancel=true;
        ui->cancelButton->setEnabled(false);
        return false;
    }
    return QDialog::event(e);
}
