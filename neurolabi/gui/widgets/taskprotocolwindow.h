#ifndef TASKPROTOCOLWINDOW_H
#define TASKPROTOCOLWINDOW_H

#include <QWidget>

#include "dvid/zdvidwriter.h"
#include "flyem/zflyemproofdoc.h"
#include "protocols/taskprotocoltask.h"


namespace Ui {
class TaskProtocolWindow;
}

class TaskProtocolWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TaskProtocolWindow(ZFlyEmProofDoc *doc, QWidget *parent = 0);
    ~TaskProtocolWindow();

private slots:
    void onDoneButton();
    void onLoadTasksButton();

private:
    static const QString KEY_DESCRIPTION;
    static const QString VALUE_DESCRIPTION;
    static const QString KEY_VERSION;
    static const int currentVersion;
    static const QString KEY_TASKLIST;
    static const QString KEY_TASKTYPE;
    static const QString PROTOCOL_INSTANCE;
    static const QString TASK_PROTOCOL_KEY;

    enum WindowConfigurations {
        LOAD_BUTTON,
        TASK_UI
        };

    enum ProtocolInstanceStatus {
        UNCHECKED,
        CHECKED_PRESENT,
        CHECKED_ABSENT
    };

    Ui::TaskProtocolWindow *ui;
    QList<QSharedPointer<TaskProtocolTask>> m_taskList;
    ZFlyEmProofDoc *m_proofDoc;
    ZDvidWriter m_writer;
    ProtocolInstanceStatus m_protocolInstanceStatus;


    void setWindowConfiguration(WindowConfigurations config);
    QJsonObject loadJsonFromFile(QString filepath);
    void showError(QString title, QString message);
    bool isValidJson(QJsonObject json);
    void loadTasks(QJsonObject json);
    QJsonObject storeTasks();
    void saveJsonToDvid(QJsonObject json);
    void saveState();
    bool checkCreateDataInstance();
    QString generateDataKey();
    QJsonObject loadJsonFromDVID(QString instance, QString key);
    void startProtocol(QJsonObject json, bool save);
    void updateLabel();
};

#endif // TASKPROTOCOLWINDOW_H
