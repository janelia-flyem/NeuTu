#include "todoreviewinputdialog.h"
#include "ui_todoreviewinputdialog.h"

ToDoReviewInputDialog::ToDoReviewInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ToDoReviewInputDialog)
{
    ui->setupUi(this);
}

int ToDoReviewInputDialog::getResult() {
    return 1;
}

ToDoReviewInputDialog::~ToDoReviewInputDialog()
{
    delete ui;
}
