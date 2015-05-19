#include "ztestdialog2.h"
#include "ui_ztestdialog2.h"

ZTestDialog2::ZTestDialog2(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ZTestDialog2)
{
    ui->setupUi(this);
}

ZTestDialog2::~ZTestDialog2()
{
    delete ui;
}
