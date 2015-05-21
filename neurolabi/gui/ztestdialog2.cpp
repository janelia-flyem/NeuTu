#include <iostream>

#include <QtGui>

#include "ztestdialog2.h"
#include "ui_ztestdialog2.h"

ZTestDialog2::ZTestDialog2(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ZTestDialog2)
{
    ui->setupUi(this);

    connect(ui->browseButton, SIGNAL(clicked()), this, SLOT(onBrowseButton()));

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
