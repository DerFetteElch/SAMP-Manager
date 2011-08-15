#ifndef ADMINADDUSER_H
#define ADMINADDUSER_H

#include <QDialog>
#include <QMessageBox>

namespace Ui {
    class AdminAddUser;
}

class AdminAddUser : public QDialog{
    Q_OBJECT

    public:
        explicit AdminAddUser(QWidget *parent = 0);
        ~AdminAddUser();

        void errorMsg(QString data);
        bool isAddUser(){ return addUse;}

    signals:
        void addUser(QString,QString);
        void guiClosed();

    private slots:
        void on_addUserButton_clicked();
        void on_cancelButton_clicked();

    private:
        Ui::AdminAddUser *ui;

        bool addUse;

    protected:
        bool event(QEvent* e);
};

#endif // ADMINADDUSER_H
