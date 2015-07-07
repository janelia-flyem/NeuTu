#include <QtGui>

#include "flyembodyinfodialog.h"
#include "ui_flyembodyinfodialog.h"

FlyEmBodyInfoDialog::FlyEmBodyInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FlyEmBodyInfoDialog)
{
    ui->setupUi(this);




    ui->tableView->setModel(createModel(ui->tableView));



}

QStandardItemModel* FlyEmBodyInfoDialog::createModel(QObject* parent) {
    const int numRows = 3;
    const int numColumns = 2;

    QStandardItemModel* model = new QStandardItemModel(numRows, numColumns, parent);

    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Body ID")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("# synapses")));

    for (int row = 0; row < numRows; ++row)
        {
            for (int column = 0; column < numColumns; ++column)
            {
                QString text = QString('A' + row) + QString::number(column + 1);
                QStandardItem* item = new QStandardItem(text);
                model->setItem(row, column, item);
            }
         }

        return model;
}

FlyEmBodyInfoDialog::~FlyEmBodyInfoDialog()
{
    delete ui;
}
