#include "showfile.h"
#include "ui_showfile.h"

ShowFile::ShowFile(QWidget *parent) : QDialog(parent),ui(new Ui::ShowFile){
    ui->setupUi(this);
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);
}

ShowFile::~ShowFile(){
    delete ui;
}

void ShowFile::setText(QString text){
    ui->file->setPlainText(text);
}

bool ShowFile::event(QEvent *e){
    if(e->type()==QEvent::Close){
        emit(guiClosed());
    }
    return QDialog::event(e);
}
