#ifndef SHOWFILE_H
#define SHOWFILE_H

#include <QDialog>

namespace Ui {
    class ShowFile;
}

class ShowFile : public QDialog{
    Q_OBJECT

    public:
        explicit ShowFile(QWidget *parent = 0);
        ~ShowFile();

        void setText(QString text);

    signals:
        void guiClosed();

    private:
        Ui::ShowFile *ui;

    protected:
        bool event(QEvent* e);
};

#endif // SHOWFILE_H
