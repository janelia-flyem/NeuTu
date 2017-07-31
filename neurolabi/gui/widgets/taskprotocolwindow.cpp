#include <iostream>
#include <stdlib.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>

#include "taskprotocolwindow.h"
#include "ui_taskprotocolwindow.h"

TaskProtocolWindow::TaskProtocolWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskProtocolWindow)
{
    ui->setupUi(this);


    // check DVID; if user has a started task list, load it immediately

    // otherwise, show the load task file button
    // (for testing, start here)
    setWindowConfiguration(LOAD_BUTTON);



    // UI connections
    connect(ui->doneButton, SIGNAL(clicked(bool)), this, SLOT(onDoneButton()));
    connect(ui->loadTasksButton, SIGNAL(clicked(bool)), this, SLOT(onLoadTasksButton()));


}
// constants
const QString TaskProtocolWindow::KEY_DESCRIPTION = "file description";
const QString TaskProtocolWindow::VALUE_DESCRIPTION = "Neu3 task list";
const QString TaskProtocolWindow::KEY_VERSION= "file version";
const int TaskProtocolWindow::currentVersion = 1;

void TaskProtocolWindow::onDoneButton() {
    std::cout << "onDoneButton()" << std::endl;

    setWindowConfiguration(LOAD_BUTTON);

}

void TaskProtocolWindow::onLoadTasksButton() {
    std::cout << "onLoadTaskButton()" << std::endl;

    // prompt for file path; might need to adjust this after testing on
    //  Linux; not sure what default file type filter is?
    QString result = QFileDialog::getOpenFileName(this, "Open task json file");
    if (result.size() == 0) {
        // canceled
        return;
    }


    // load json from file (for now; eventually, allow user to browse from DVID,
    //  or maybe enter an assignment ID or something)
    QJsonObject json = loadJsonFromFile(result);

    // validate json; this call displays errors itself
    if (!isValidJson(json)) {
        return;
    }

    // load tasks from json into internal data structures
    std::cout << "onLoadTasksButton: json is valid" << std::endl;

    // load first task


    // enable UI and go
    setWindowConfiguration(TASK_UI);

}

QJsonObject TaskProtocolWindow::loadJsonFromFile(QString filepath) {
    QJsonObject emptyResult;

    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        showError("Error loading file", "Couldn't open file " + filepath + "!");
        return emptyResult;
        }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() or !doc.isObject()) {
        showError("Error parsing file", "Couldn't parse file " + filepath + "!");
        return emptyResult;
    }
    return doc.object();
}

bool TaskProtocolWindow::isValidJson(QJsonObject json) {

    // check file description and version
    if (!json.contains(KEY_DESCRIPTION) || json[KEY_DESCRIPTION].toString() != VALUE_DESCRIPTION) {
        showError("Json parsing error", "This file does not appear to be a Neu3 task list file!");
        return false;
    }

    if (!json.contains(KEY_VERSION)) {
        showError("Json parsing error", "No version info in file!");
        return false;
    }

    int fileVersion = json[VALUE_DESCRIPTION].toInt();
    if (fileVersion > currentVersion) {
        showError("Json parsing issue", "This file is from a newer version of this software!  Update and try again.");
        return false;
    }


    // at the point in time we have older versions hanging around, this is where you
    //  would convert them


    return true;
}

void TaskProtocolWindow::setWindowConfiguration(WindowConfigurations config) {
    // note that size constraint = fixed size, set in designer,
    //  ensures the widget resizes when child widgets are hidden/shown

    if (config == TASK_UI) {
        ui->loadTasksWidget->hide();
        ui->taskButtonsWidget->show();
        ui->taskDetailsWidget->show();
        ui->tasksProgressWidget->show();
    } else if (config == LOAD_BUTTON) {
        ui->loadTasksWidget->show();
        ui->taskButtonsWidget->hide();
        ui->taskDetailsWidget->hide();
        ui->tasksProgressWidget->hide();
    }
}

void TaskProtocolWindow::showError(QString title, QString message) {
    QMessageBox errorBox;
    errorBox.setText(title);
    errorBox.setInformativeText(message);
    errorBox.setStandardButtons(QMessageBox::Ok);
    errorBox.setIcon(QMessageBox::Warning);
    errorBox.exec();
}

TaskProtocolWindow::~TaskProtocolWindow()
{
    delete ui;
}
