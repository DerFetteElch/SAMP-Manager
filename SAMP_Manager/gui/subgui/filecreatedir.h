#ifndef FILECREATEDIR_H
#define FILECREATEDIR_H

#include <QDialog>
#include <QMessageBox>

namespace Ui {
    class FileCreateDir;
}

class FileCreateDir : public QDialog{
    Q_OBJECT

    public:
        explicit FileCreateDir(QWidget *parent = 0);
        ~FileCreateDir();

        void errorMsg(QString data);

    signals:
        void createDir(QString);
        void guiClosed();

    private slots:
        void on_createDirButton_clicked();
        void on_cancelButton_clicked();

    private:
        Ui::FileCreateDir *ui;

    protected:
        bool event(QEvent* e);
};

#endif // FILECREATEDIR_H
