#include "orphanlinkinputdialog.h"
#include "ui_orphanlinkinputdialog.h"

OrphanLinkInputDialog::OrphanLinkInputDialog(QStringList projectList, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OrphanLinkInputDialog)
{
    ui->setupUi(this);

    ui->projectMenu->addItems(projectList);

    connect(ui->getButton, SIGNAL(clicked(bool)), this, SLOT(onGetButton()));

}

void OrphanLinkInputDialog::onGetButton() {

    m_project = ui->projectMenu->currentText();
    done(QDialog::Accepted);

    // or this is the same?
    // setResult(QDialog::Accepted);

}

QString OrphanLinkInputDialog::getProject() {
    return m_project;
}

OrphanLinkInputDialog::~OrphanLinkInputDialog()
{
    delete ui;
}
