#include <iostream>

#include <QtGui>
#include<QMessageBox>

#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zstring.h"

#include "flyembodyinfodialog.h"
#include "ui_flyembodyinfodialog.h"

/*
 * this dialog displays a list of bodies and their properties; data is
 * loaded from a static json bookmarks file
 *
 * djo, 7/15
 *
 */
FlyEmBodyInfoDialog::FlyEmBodyInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FlyEmBodyInfoDialog)
{
    ui->setupUi(this);
    connect(ui->openButton, SIGNAL(clicked()), this, SLOT(onOpenButton()));
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(onCloseButton()));

    m_model = createModel(ui->tableView);
    ui->tableView->setModel(m_model);

}

QStandardItemModel* FlyEmBodyInfoDialog::createModel(QObject* parent) {
    QStandardItemModel* model = new QStandardItemModel(0, 3, parent);
    setHeaders(model);
    return model;
}

void FlyEmBodyInfoDialog::setHeaders(QStandardItemModel * model) {
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Body ID")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("# synapses")));
    model->setHorizontalHeaderItem(2, new QStandardItem(QString("status")));
}

void FlyEmBodyInfoDialog::importBookmarksFile(QString filename) {
    ZJsonObject jsonObject;

    if (!jsonObject.load(filename.toStdString())) {
        QMessageBox errorBox;
        errorBox.setText("Problem parsing json file");
        errorBox.setInformativeText("Are you sure this is a Fly EM JSON file?");
        errorBox.setStandardButtons(QMessageBox::Ok);
        errorBox.setIcon(QMessageBox::Warning);
        errorBox.exec();
        return;
    }

    if (!isValidBookmarkFile(jsonObject)) {
        return;
    }

    // update model from the object, or the data piece of it
    ZJsonValue dataObject = jsonObject.value("data");
    updateModel(dataObject);

}

bool FlyEmBodyInfoDialog::isValidBookmarkFile(ZJsonObject jsonObject) {
    // validation is admittedly limited for now; ultimately, I
    //  expect we'll be getting the info from DVID rather than
    //  a file anyway, so I'm not going to spend much time on it

    // likewise, I'm copy/pasting the error dialog code rather than
    //  neatly factoring it out

    QMessageBox errorBox;
    if (!jsonObject.hasKey("data") || !jsonObject.hasKey("metadata")) {
        errorBox.setText("Problem with json file");
        errorBox.setInformativeText("This file is missing 'data' or 'metadata'. Are you sure this is a Fly EM JSON file?");
        errorBox.setStandardButtons(QMessageBox::Ok);
        errorBox.setIcon(QMessageBox::Warning);
        errorBox.exec();
        return false;
    }


    ZJsonObject metadata = (ZJsonObject) jsonObject.value("metadata");
    if (!metadata.hasKey("description")) {
        errorBox.setText("Problem with json file");
        errorBox.setInformativeText("This file is 'metadata/description'. Are you sure this is a Fly EM JSON file?");
        errorBox.setStandardButtons(QMessageBox::Ok);
        errorBox.setIcon(QMessageBox::Warning);
        errorBox.exec();
        return false;
    }

    ZString description = ZJsonParser::stringValue(metadata["description"]);
    if (description != "bookmarks") {
        errorBox.setText("Problem with json file");
        errorBox.setInformativeText("This json file does not have description 'bookmarks'!");
        errorBox.setStandardButtons(QMessageBox::Ok);
        errorBox.setIcon(QMessageBox::Warning);
        errorBox.exec();
        return false;
    }

    ZJsonValue data = jsonObject.value("data");
    if (!data.isArray()) {
        errorBox.setText("Problem with json file");
        errorBox.setInformativeText("The data section in this json file is not an array!");
        errorBox.setStandardButtons(QMessageBox::Ok);
        errorBox.setIcon(QMessageBox::Warning);
        errorBox.exec();
        return false;
    }

    // we could/should test all the elements of the array to see if they
    //  are bookmarks, but enough is enough...

    return true;
}

void FlyEmBodyInfoDialog::onCloseButton() {
    close();
}

void FlyEmBodyInfoDialog::onOpenButton() {
    QString filename = QFileDialog::getOpenFileName(this, "Open bookmarks file");
    if (!filename.isEmpty()) {
        importBookmarksFile(filename);
    }
}

/*
 * update the data model from json; input should be the
 * "data" part of our standard bookmarks json file
 */
void FlyEmBodyInfoDialog::updateModel(ZJsonValue data) {
    m_model->clear();
    setHeaders(m_model);

    ZJsonArray bookmarks(data);
    m_model->setRowCount(bookmarks.size());
    for (size_t i = 0; i < bookmarks.size(); ++i) {
        ZJsonObject bkmk(bookmarks.at(i), false);

        // carefully set data for items so they will sort numerically;
        //  contrast the "body status" entry, which we keep as a string
        int bodyID = ZJsonParser::integerValue(bkmk["body ID"]);
        QStandardItem * bodyIDItem = new QStandardItem();
        bodyIDItem->setData(QVariant(bodyID), Qt::DisplayRole);
        m_model->setItem(i, 0, bodyIDItem);

        int nSynapses = ZJsonParser::integerValue(bkmk["body synapses"]);
        QStandardItem * synapsesItem = new QStandardItem();
        synapsesItem->setData(QVariant(nSynapses), Qt::DisplayRole);
        m_model->setItem(i, 1, synapsesItem);

        const char* status = ZJsonParser::stringValue(bkmk["body status"]);
        m_model->setItem(i, 2, new QStandardItem(QString(status)));
    }
    ui->tableView->resizeColumnsToContents();

    // initial sort order is by synapses, descending
    ui->tableView->sortByColumn(1, Qt::DescendingOrder);

}

FlyEmBodyInfoDialog::~FlyEmBodyInfoDialog()
{
    delete ui;
}
