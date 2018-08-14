#include <iostream>

#include "ztestdialog2.h"
#include "flyembodyinfodialog.h"
#include "ui_ztestdialog2.h"

/*
 * dialog for testing by Don; copy of Ting's test dialog
 *
 * djo, 7/15
 *
 */
ZTestDialog2::ZTestDialog2(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ZTestDialog2)
{
    ui->setupUi(this);

    m_dialog = new DvidBranchDialog(this);

    connect(ui->openDvidButton, SIGNAL(clicked(bool)), this, SLOT(onOpenDvidButton()));

}

void ZTestDialog2::onOpenDvidButton() {
    m_dialog->show();


}


ZTestDialog2::~ZTestDialog2()
{
    delete ui;
}

