#include <iostream>
#include <stdlib.h>

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

void TaskProtocolWindow::onDoneButton() {
    std::cout << "onDoneButton()" << std::endl;

    setWindowConfiguration(LOAD_BUTTON);

}

void TaskProtocolWindow::onLoadTasksButton() {
    std::cout << "onLoadTaskButton()" << std::endl;

    setWindowConfiguration(TASK_UI);

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

TaskProtocolWindow::~TaskProtocolWindow()
{
    delete ui;
}
