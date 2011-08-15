#ifndef ADMINADDNEWS_H
#define ADMINADDNEWS_H

#include <QDialog>
#include <QMessageBox>

namespace Ui {
    class AdminAddNews;
}

class AdminAddNews : public QDialog{
    Q_OBJECT

    public:
        explicit AdminAddNews(QWidget *parent = 0);
        ~AdminAddNews();

        void errorMsg(QString data);
        bool isAddNews(){ return addNew;}

    signals:
        void addNews(QString,QString);
        void guiClosed();

    private slots:
        void on_addNewsButton_clicked();
        void on_cancelButton_clicked();

    private:
        Ui::AdminAddNews *ui;
        bool addNew;

    protected:
        bool event(QEvent* e);
    };

#endif // ADMINADDNEWS_H
