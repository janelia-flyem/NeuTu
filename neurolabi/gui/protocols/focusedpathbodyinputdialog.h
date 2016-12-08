#ifndef FOCUSEDPATHBODYINPUTDIALOG_H
#define FOCUSEDPATHBODYINPUTDIALOG_H

#include <QDialog>

namespace Ui {
class FocusedPathBodyInputDialog;
}

class FocusedPathBodyInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FocusedPathBodyInputDialog(QWidget *parent = 0);
    ~FocusedPathBodyInputDialog();
    std::string getEdgeInstance();
    uint64_t getBodyID();

private:
    Ui::FocusedPathBodyInputDialog *ui;
};

#endif // FOCUSEDPATHBODYINPUTDIALOG_H
