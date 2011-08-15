#ifndef ADMINADDSERVER_H
#define ADMINADDSERVER_H

#include <QDialog>
#include <QMessageBox>

namespace Ui {
    class AdminAddServer;
}

class AdminAddServer : public QDialog{
    Q_OBJECT

    public:
        explicit AdminAddServer(QWidget *parent = 0);
        ~AdminAddServer();

        void errorMsg(QString data);
        bool isAddServer(){ return addServ;}

    signals:
        void addServer(int,int,int);
        void guiClosed();

    private slots:
        void on_addButton_clicked();
        void on_cancelButton_clicked();

    private:
        Ui::AdminAddServer *ui;

        bool addServ;

    protected:
        bool event(QEvent* e);
};

#endif // ADMINADDSERVER_H
