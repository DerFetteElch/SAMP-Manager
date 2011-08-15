#include "adminaddnews.h"
#include "ui_adminaddnews.h"

AdminAddNews::AdminAddNews(QWidget *parent) : QDialog(parent),ui(new Ui::AdminAddNews){
    ui->setupUi(this);
    addNew=false;
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);
}

AdminAddNews::~AdminAddNews(){
    delete ui;
}

void AdminAddNews::errorMsg(QString data){
    addNew=false;
    QMessageBox::critical(this,tr("Error"),data);
}

void AdminAddNews::on_addNewsButton_clicked(){
    if(ui->newsHeadline->text().isEmpty() || ui->newsText->toPlainText().isEmpty()){
        errorMsg(tr(""));
    }else{
        addNew=true;
        emit(addNews(ui->newsHeadline->text(),ui->newsText->toPlainText()));
    }
}
void AdminAddNews::on_cancelButton_clicked(){
    close();
}
bool AdminAddNews::event(QEvent *e){
    if(e->type()==QEvent::Close){
        addNew=false;
        emit(guiClosed());
    }
    return QDialog::event(e);
}
