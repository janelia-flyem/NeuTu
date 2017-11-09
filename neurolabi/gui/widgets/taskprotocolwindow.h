#ifndef TASKPROTOCOLWINDOW_H
#define TASKPROTOCOLWINDOW_H

#include <QWidget>
#include <QThread>

#include "dvid/zdvidwriter.h"
#include "flyem/zflyemproofdoc.h"
#include "flyem/zflyembody3ddoc.h"
#include "protocols/bodyprefetchqueue.h"
#include "protocols/taskprotocoltask.h"


namespace Ui {
class TaskProtocolWindow;
}

class TaskProtocolWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TaskProtocolWindow(ZFlyEmProofDoc *doc, ZFlyEmBody3dDoc *bodyDoc, QWidget *parent = 0);
    void init();
    ~TaskProtocolWindow();

    BodyPrefetchQueue *getPrefetchQueue() const;

signals:
    // I'm keeping the names Ting used in ZBodyListWidget (for now)
    void bodyAdded(uint64_t bodyId);
    void bodyRemoved(uint64_t bodyId);
    void bodySelectionChanged(QSet<uint64_t> selectedSet);
    void prefetchBody(QSet<uint64_t> bodyIDs);
    void prefetchBody(uint64_t bodyID);

private slots:
    void onNextButton();
    void onPrevButton();
    void onDoneButton();
    void onLoadTasksButton();    
    void onCompletedStateChanged(int state);
    void onReviewStateChanged(int state);
    void onShowCompletedStateChanged(int state);
    void applicationQuitting();

private:
    static const QString KEY_DESCRIPTION;
    static const QString VALUE_DESCRIPTION;
    static const QString KEY_VERSION;
    static const int currentVersion;
    static const QString KEY_ID;
    static const QString KEY_DVID_SERVER;
    static const QString KEY_UUID;
    static const QString KEY_TASKLIST;
    static const QString KEY_TASKTYPE;
    static const QString PROTOCOL_INSTANCE;
    static const QString TASK_PROTOCOL_KEY;
    static const QString TAG_NEEDS_REVIEW;

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
    QString m_ID;
    QString m_DVIDServer;
    QString m_UUID;
    QList<QSharedPointer<TaskProtocolTask>> m_taskList;
    ZFlyEmProofDoc * m_proofDoc;
    ZFlyEmBody3dDoc * m_body3dDoc;
    ZDvidWriter m_writer;
    ProtocolInstanceStatus m_protocolInstanceStatus;
    int m_currentTaskIndex;
    QWidget * m_currentTaskWidget = NULL;
    bool m_nodeLocked;
    BodyPrefetchQueue * m_prefetchQueue;
    QThread * m_prefetchThread;

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
    void updateCurrentTaskLabel();
    int getFirstUncompleted();
    void showInfo(QString title, QString message);
    void gotoCurrentTask();
    void updateBodyWindow();
    int getNext();
    int getNextUncompleted();
    int getPrev();
    int getPrevUncompleted();
    void prefetch(uint64_t bodyID);
    void prefetch(QSet<uint64_t> bodyIDs);
    void prefetchForTaskIndex(int index);
    bool checkDVIDTarget();
};

#endif // TASKPROTOCOLWINDOW_H
