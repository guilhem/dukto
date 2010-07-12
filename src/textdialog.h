#ifndef TEXTDIALOG_H
#define TEXTDIALOG_H

#include <QDialog>

namespace Ui {
    class TextDialog;
}

class TextDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextDialog(QWidget *parent = 0);
    ~TextDialog();
    void setMode(bool send);
    void setContents(QString c);
    QString contents();

private:
    Ui::TextDialog *ui;
};

#endif // TEXTDIALOG_H
