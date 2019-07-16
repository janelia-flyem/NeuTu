#include "todoreviewinputdialog.h"
#include "ui_todoreviewinputdialog.h"

#include <QMessageBox>

ToDoReviewInputDialog::ToDoReviewInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ToDoReviewInputDialog)
{
    ui->setupUi(this);

    // restrict body ID to integers of indeterminate size; this isn't exact, but it
    //  will exclude most though not all bad input:
    QRegularExpression regex("[1-9][0-9]*");
    ui->bodyIDInput->setValidator(new QRegularExpressionValidator(regex, this));
}

QString ToDoReviewInputDialog::getBodyID() {
    return ui->bodyIDInput->text();
}

QString ToDoReviewInputDialog::getTextFilter() {
    return ui->textFilterInput->text();
}

ToDoReviewInputDialog::~ToDoReviewInputDialog()
{
    delete ui;
}
