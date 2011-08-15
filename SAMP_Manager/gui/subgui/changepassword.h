#ifndef CHANGEPASSWORD_H
#define CHANGEPASSWORD_H

#include <QDialog>
#include <QMessageBox>

namespace Ui {
    class ChangePassword;
}

class ChangePassword : public QDialog{
    Q_OBJECT

    public:
        explicit ChangePassword(QWidget *parent = 0);
        ~ChangePassword();

        void errorMsg(QString data);

    signals:
        void guiClosed();
        void changePassword(QString,QString);

    private slots:
        void on_changePasswordButton_clicked();
        void on_cancelButton_clicked();

    private:
            Ui::ChangePassword *ui;

    protected:
        bool event(QEvent* e);
};

#endif // CHANGEPASSWORD_H
