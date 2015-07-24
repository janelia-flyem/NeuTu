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
    QStandardItemModel* model = new QStandardItemModel(10, 2, parent);
    setHeaders(model);
    return model;
}

void FlyEmBodyInfoDialog::setHeaders(QStandardItemModel * model) {
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Body ID")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("# synapses")));
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

    std::cout << "json file valid" << std::endl;

    return true;
}

void FlyEmBodyInfoDialog::onCloseButton() {
    close();
}

void FlyEmBodyInfoDialog::onOpenButton() {
    QString filename = QFileDialog::getOpenFileName(this, "Open bookmarks file");
    if (!filename.isEmpty()) {
        // std::cout << "path chosen: " + filename.toStdString() << std::endl;
        importBookmarksFile(filename);
    }
}

void FlyEmBodyInfoDialog::updateModel(ZJsonValue data) {
    // clear model, then load new data
    m_model->clear();

    // this doesn't help:
    m_model->setRowCount(10);
    m_model->setColumnCount(2);

    setHeaders(m_model);

    if (!data.isArray()) {
        // problem, error?
        std::cout << "data is not an array?" << std::endl;
        return;
    }

    int row = 0;
    for (ZJsonValue bkmk: data.toArray()) {
        std::cout << "starting row " << row << std::endl;

        // parse out the stuff we want & put in model
        std::cout << "parsing body ID" << std::endl;
        int bodyID = ZJsonParser::integerValue(((ZJsonObject) bkmk)["body ID"]);
        std::cout << "setting body ID" << std::endl;
        m_model->setItem(row, 0, new QStandardItem(QString::number(bodyID)));

        std::cout << "parsing #synapses" << std::endl;
        int nSynapses = ZJsonParser::integerValue(((ZJsonObject) bkmk)["body synapses"]);
        std::cout << "setting #synapses" << std::endl;
        m_model->setItem(row, 1, new QStandardItem(QString::number(nSynapses)));

        std::cout << "appended body ID " << bodyID << std::endl;
        row ++;
    }



    /*
    // this has same errors as my version
    ZJsonArray bookmarks(data);
    std::cout << "size = " << bookmarks.size() << std::endl;

    for (size_t i = 0; i < bookmarks.size(); ++i) {
        std::cout << "starting row " << i << std::endl;

        ZJsonObject bkmk(bookmarks.at(i), false);
        ZString text = ZJsonParser::stringValue(bkmk["text"]);
        std::cout << i << ": text = " << text << std::endl;


        // parse out the stuff we want & put in model
        std::cout << "parsing body ID" << std::endl;
        int bodyID = ZJsonParser::integerValue(((ZJsonObject) bkmk)["body ID"]);

        std::cout << "setting body ID" << std::endl;
        m_model->setItem(i, 0, new QStandardItem(bodyID));


        std::cout << "parsing #synapses" << std::endl;
        int nSynapses = ZJsonParser::integerValue(((ZJsonObject) bkmk)["body synapses"]);

        std::cout << "appended body ID " << bodyID << std::endl;

    }
    */

}

FlyEmBodyInfoDialog::~FlyEmBodyInfoDialog()
{
    delete ui;
}
