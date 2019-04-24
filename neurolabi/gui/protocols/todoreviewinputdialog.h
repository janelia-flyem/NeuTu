#ifndef TODOREVIEWINPUTDIALOG_H
#define TODOREVIEWINPUTDIALOG_H

#include <QDialog>

namespace Ui {
class ToDoReviewInputDialog;
}

class ToDoReviewInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ToDoReviewInputDialog(QWidget *parent = 0);
    ~ToDoReviewInputDialog();

    int getResult();

private:
    Ui::ToDoReviewInputDialog *ui;
};

#endif // TODOREVIEWINPUTDIALOG_H
