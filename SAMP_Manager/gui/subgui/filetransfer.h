#ifndef FILETRANSFER_H
#define FILETRANSFER_H

#include <QDialog>

namespace Ui {
    class FileTransfer;
}

class FileTransfer : public QDialog{
    Q_OBJECT

    public:
        explicit FileTransfer(QWidget *parent = 0);
        ~FileTransfer();

        void setParts(int amount);
        void setPart(int num);

        bool isCanceled();

    private slots:
        void on_cancelButton_clicked();

    private:
        Ui::FileTransfer *ui;

        int parts;
        bool cancel;

    protected:
        bool event(QEvent* e);
};

#endif // FILETRANSFER_H
