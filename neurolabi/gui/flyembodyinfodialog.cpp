#include <iostream>

#include <QtGui>
#include<QMessageBox>

#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zstring.h"

#include "flyembodyinfodialog.h"
#include "ui_flyembodyinfodialog.h"

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

void FlyEmBodyInfoDialog::updateModel(ZJsonValue data) {
    if (!data.isArray()) {
        // move to validation step?  we don't check that
        //  elements are actually bookmarks, either
        std::cout << "data is not an array?" << std::endl;
        return;
    }

    // clear model, then load new data
    m_model->clear();
    setHeaders(m_model);


    // NOTE: don't currently really need to convert to ints if we're
    //  just stuffing into table, but we do eventually want to sort
    //  on the numeric value

    ZJsonArray bookmarks(data);
    m_model->setRowCount(bookmarks.size());
    for (size_t i = 0; i < bookmarks.size(); ++i) {
        ZJsonObject bkmk(bookmarks.at(i), false);

        int bodyID = ZJsonParser::integerValue(bkmk["body ID"]);
        m_model->setItem(i, 0, new QStandardItem(QString::number(bodyID)));

        int nSynapses = ZJsonParser::integerValue(bkmk["body synapses"]);
        m_model->setItem(i, 1, new QStandardItem(QString::number(nSynapses)));

        const char* status = ZJsonParser::stringValue(bkmk["body status"]);
        m_model->setItem(i, 2, new QStandardItem(QString(status)));
    }

}

FlyEmBodyInfoDialog::~FlyEmBodyInfoDialog()
{
    delete ui;
}
