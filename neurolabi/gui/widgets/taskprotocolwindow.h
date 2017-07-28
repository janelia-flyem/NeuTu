#ifndef TASKPROTOCOLWINDOW_H
#define TASKPROTOCOLWINDOW_H

#include <QWidget>

namespace Ui {
class TaskProtocolWindow;
}

class TaskProtocolWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TaskProtocolWindow(QWidget *parent = 0);
    ~TaskProtocolWindow();

private slots:
    void onDoneButton();
    void onLoadTasksButton();

private:
    enum WindowConfigurations {
        LOAD_BUTTON,
        TASK_UI
        };

    Ui::TaskProtocolWindow *ui;
    void setWindowConfiguration(WindowConfigurations config);
};

#endif // TASKPROTOCOLWINDOW_H
