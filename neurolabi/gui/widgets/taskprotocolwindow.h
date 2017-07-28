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
    Ui::TaskProtocolWindow *ui;
};

#endif // TASKPROTOCOLWINDOW_H
