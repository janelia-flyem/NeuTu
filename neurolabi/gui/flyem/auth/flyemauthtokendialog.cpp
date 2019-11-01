#include "flyemauthtokendialog.h"
#include "ui_flyemauthtokendialog.h"

FlyEmAuthTokenDialog::FlyEmAuthTokenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FlyEmAuthTokenDialog)
{
    ui->setupUi(this);

    updateServerLabel(m_handler.getServer());

}

void FlyEmAuthTokenDialog::updateServerLabel(QString server) {
    ui->serverLabel->setText("Current authentication server: " + server);
}

FlyEmAuthTokenDialog::~FlyEmAuthTokenDialog()
{
    delete ui;
}
