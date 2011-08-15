#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
#include "../packets.h"

namespace Ui {
    class Login;
}

class Login : public QMainWindow{
    Q_OBJECT

    public:
        explicit Login(QWidget *parent = 0);
        ~Login();

    signals:
        void send(QByteArray);
        void loginSuccess(int);

    public slots:
        void loginResponse(QByteArray data);

    private slots:
        void on_loginButton_clicked();

    private:
        Ui::Login *ui;

    protected:
        bool event(QEvent* e);
};

#endif // LOGIN_H
