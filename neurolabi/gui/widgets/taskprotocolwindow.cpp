#include <iostream>
#include <stdlib.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QsLog.h>

#include "protocols/taskbodyreview.h"

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
const QString TaskProtocolWindow::KEY_DESCRIPTION = "file type";
const QString TaskProtocolWindow::VALUE_DESCRIPTION = "Neu3 task list";
const QString TaskProtocolWindow::KEY_VERSION = "file version";
const QString TaskProtocolWindow::KEY_TASKLIST = "task list";
const QString TaskProtocolWindow::KEY_TASKTYPE = "task type";
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


    // testing
    std::cout << "onLoadTasksButton: json is valid" << std::endl;




    // at the point in time we have older versions hanging around, this is where you
    //  would convert them


    // load tasks from json into internal data structures
    loadTasks(json);

    // save to DVID
    // convert data from internal structures to json
    // save json to dvid




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

    if (!json.contains(KEY_TASKLIST)) {
        showError("Json parsing issue", "Can't find list of tasks in json file!");
        return false;
    }
    // could validate that it's a list and each element is a map, but I'll
    //  draw the line here for now

    return true;
}

void TaskProtocolWindow::loadTasks(QJsonObject json) {

    m_taskList.clear();
    foreach(QJsonValue taskJson, json[KEY_TASKLIST].toArray()) {
        if (!taskJson.isObject()) {
            LWARN() << "found task json that is not an object; skipping";
            continue;
        }

        // this if-else tree will get more awkward with more types...

        // also, need to collect these keys and values better; hard-code this one
        //  for now
        QString taskType = taskJson.toObject()[KEY_TASKTYPE].toString();
        if (taskType == "body review") {
            QSharedPointer<TaskProtocolTask> task(new TaskBodyReview(taskJson.toObject()));
            m_taskList.append(task);
        } else {
            // unknown task type; log it and move on
            LWARN() << "found unknown task type " << taskType << " in task json; skipping";
        }
    }

    // testing
    std::cout << "loadTasks(): # tasks = " << m_taskList.size() << std::endl;

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
