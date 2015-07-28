#include <iostream>

#include <QtGui>

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

    m_flyEmBodyInfoDialog = new FlyEmBodyInfoDialog(this);



    connect(ui->browseButton, SIGNAL(clicked()), this, SLOT(onBrowseButton()));

    connect(ui->bodyInfoButton, SIGNAL(clicked()), this, SLOT(onBodyInfoButton()));

}

ZTestDialog2::~ZTestDialog2()
{
    delete ui;
}

void ZTestDialog2::onBrowseButton() {

    QString filename = QFileDialog::getOpenFileName(this, "Open bookmarks");
    if (!filename.isEmpty()) {
        std::cout << "path chosen: " + filename.toStdString() << std::endl;
    }

}

void ZTestDialog2::onBodyInfoButton() {

    m_flyEmBodyInfoDialog->show();

}
