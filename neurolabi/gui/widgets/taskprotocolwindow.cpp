#include <iostream>
#include <stdlib.h>

#include "taskprotocolwindow.h"
#include "ui_taskprotocolwindow.h"

TaskProtocolWindow::TaskProtocolWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskProtocolWindow)
{
    ui->setupUi(this);

    // we only show some of the widgets depending on our state
    // note that size constraint = fixed size, set in designer,
    //  ensures the widget resizes when child widgets are hidden/shown

    // for testing, assume we start unloaded
    ui->taskButtonsWidget->hide();
    ui->taskDetailsWidget->hide();
    ui->tasksProgressWidget->hide();



    // connections
    connect(ui->doneButton, SIGNAL(clicked(bool)), this, SLOT(onDoneButton()));
    connect(ui->loadTasksButton, SIGNAL(clicked(bool)), this, SLOT(onLoadTasksButton()));


}

void TaskProtocolWindow::onDoneButton() {
    std::cout << "onDoneButton()" << std::endl;

    ui->loadTasksWidget->show();
    ui->taskButtonsWidget->hide();
    ui->taskDetailsWidget->hide();
    ui->tasksProgressWidget->hide();

}

void TaskProtocolWindow::onLoadTasksButton() {
    std::cout << "onLoadTaskButton()" << std::endl;

    ui->loadTasksWidget->hide();
    ui->taskButtonsWidget->show();
    ui->taskDetailsWidget->show();
    ui->tasksProgressWidget->show();

}

TaskProtocolWindow::~TaskProtocolWindow()
{
    delete ui;
}
