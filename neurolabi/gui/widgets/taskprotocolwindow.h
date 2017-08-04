#ifndef TASKPROTOCOLWINDOW_H
#define TASKPROTOCOLWINDOW_H

#include <QWidget>

#include "protocols/taskprotocoltask.h"

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
    static const QString KEY_DESCRIPTION;
    static const QString VALUE_DESCRIPTION;
    static const QString KEY_VERSION;
    static const QString KEY_TASKLIST;
    static const QString KEY_TASKTYPE;
    static const int currentVersion;

    enum WindowConfigurations {
        LOAD_BUTTON,
        TASK_UI
        };

    Ui::TaskProtocolWindow *ui;
    QList<QSharedPointer<TaskProtocolTask>> m_taskList;

    void setWindowConfiguration(WindowConfigurations config);
    QJsonObject loadJsonFromFile(QString filepath);
    void showError(QString title, QString message);
    bool isValidJson(QJsonObject json);
    void loadTasks(QJsonObject json);
};

#endif // TASKPROTOCOLWINDOW_H
