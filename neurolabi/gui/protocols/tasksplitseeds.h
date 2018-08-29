#ifndef TaskSplitSeeds_H
#define TaskSplitSeeds_H

#include <iostream>
#include <stdlib.h>

#include <QObject>
#include <QSet>
#include <QCheckBox>
#include <QWidget>

#include "flyem/zflyembody3ddoc.h"
#include "protocols/taskprotocoltask.h"

class TaskSplitSeeds : public TaskProtocolTask
{
    Q_OBJECT

public:
    TaskSplitSeeds(QJsonObject json, ZFlyEmBody3dDoc * bodyDoc);
    QString tasktype() const;
    QString actionString();
    QString targetString();
    QWidget * getTaskWidget();

signals:
    void saveSplitTask();

private slots:
    void onNoSplitStateChanged(int state);

private:
    static const QString KEY_TASKTYPE;
    static const QString VALUE_TASKTYPE;
    static const QString KEY_BODYID;
    static const QString TAG_SEEDS_ADDED;
    ZFlyEmBody3dDoc * m_bodyDoc;
    uint64_t m_bodyID;
    QWidget * m_widget;
    QCheckBox * m_noSplitCheck;

    bool loadSpecific(QJsonObject json);
    QJsonObject addToJson(QJsonObject json);
    void onCompleted();
    void beforeNext();
    void beforePrev();
    void updateSeedsTag();
};

#endif // TaskSplitSeeds_H
