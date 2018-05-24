#ifndef TASKPROTOCOLWINDOW_H
#define TASKPROTOCOLWINDOW_H

#include <QWidget>
#include <QThread>

#include "dvid/zdvidwriter.h"
#include "flyem/zflyemproofdoc.h"
#include "flyem/zflyembody3ddoc.h"
#include "protocols/bodyprefetchqueue.h"
#include "protocols/taskprotocoltask.h"

class ZWidgetMessage;

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

public:
    void test();

signals:
    // I'm keeping the names Ting used in ZBodyListWidget (for now)
    void bodyAdded(uint64_t bodyId);
    void allBodiesRemoved();

    void bodySelectionChanged(QSet<uint64_t> selectedSet);
    void prefetchBody(QSet<uint64_t> bodyIDs);
    void prefetchBody(uint64_t bodyID);
    void unprefetchBody(QSet<uint64_t> bodyIDs);
    void clearBodyQueue();
    void messageGenerated(const ZWidgetMessage &msg);

private slots:
    void onNextButton();
    void onPrevButton();
    void onDoneButton();
    void onLoadTasksButton();    
    void onBodiesUpdated();
    void onCompletedStateChanged(int state);
    void onReviewStateChanged(int state);
    void onShowCompletedStateChanged(int state);
    void onBodyMeshesAdded(int numMeshes);
    void onBodyMeshLoaded();
    void onBodyRecycled();
    void applicationQuitting();

private:
    /*!
     * \brief Get the previous index.
     * \param currentIndex The currentIndex.
     * \return (\a currentIndex - 1) if both \a currentIndex and
     *         (\a currentIndex - 1) is valid. It returns the last
     *         index if \a currentIndex is 0 and there are more than one tasks.
     *         It returns -1 in other cases.
     */
    int getPrevIndex(int currentIndex) const;

    /*!
     * \brief Get the next index.
     * \param currentIndex The currentIndex.
     * \return (\a currentIndex + 1) if both \a currentIndex and
     *         (\a currentIndex + 1) is valid. It returns 0 if
     *         \a currentIndex is the last index and not 0. It returns -1 in
     *          other cases.
     */
    int getNextIndex(int currentIndex) const;

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
    int m_currentTaskIndex = -1;
    QWidget * m_currentTaskWidget = nullptr;
    QAction * m_currentTaskMenuAction = nullptr;
    bool m_nodeLocked;
    BodyPrefetchQueue * m_prefetchQueue;
    QThread * m_prefetchThread;
    int m_bodyRecycledExpected = 0;
    int m_bodyRecycledReceived = 0;
    int m_bodyMeshesAddedExpected = 0;
    int m_bodyMeshesAddedReceived = 0;
    int m_bodyMeshLoadedExpected = 0;
    int m_bodyMeshLoadedReceived = 0;
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
    void updateButtonsEnabled();
    void updateMenu(bool add);
    int getFirstUncompleted();
    void showInfo(QString title, QString message);
    void gotoCurrentTask();
    void updateBodyWindow();
    void disableButtonsWhileUpdating();
    void enableButtonsAfterUpdating();
    int getNext();
    int getNextUncompleted();
    int getPrev();
    int getPrevUncompleted();
    void prefetch(uint64_t bodyID);
    void prefetch(QSet<uint64_t> bodyIDs);
    void prefetchForTaskIndex(int index);
    bool checkDVIDTarget();
    void unprefetchForTaskIndex(int index);
};

#endif // TASKPROTOCOLWINDOW_H
