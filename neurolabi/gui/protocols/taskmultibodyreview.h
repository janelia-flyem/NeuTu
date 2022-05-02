#ifndef TASKMULTIBODYREVIEW_H
#define TASKMULTIBODYREVIEW_H

#include <iostream>
#include <stdlib.h>

#include <QObject>
#include <QSet>

#include <QStandardItemModel>
#include <QTableView>
#include <QWidget>

#include "common/neutudefs.h"
#include "protocols/taskprotocoltask.h"

class ZFlyEmBody3dDoc;

class TaskMultiBodyReview : public TaskProtocolTask
{
    Q_OBJECT

public:
    TaskMultiBodyReview(QJsonObject json, ZFlyEmBody3dDoc * bodyDoc);

    // For use with TaskProtocolTaskFactory.
    static QString taskTypeStatic();
    static TaskMultiBodyReview* createFromJson(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);

    QString taskType() const override;
    QString actionString() override;
    QString targetString() override;

    QWidget * getTaskWidget();

private slots:
    void onTestButton();

private:
    static const QString KEY_TASKTYPE;
    static const QString VALUE_TASKTYPE;
    static const QString KEY_BODYIDS;

    bool loadSpecific(QJsonObject json) override;
    QJsonObject addToJson(QJsonObject json) override;

    enum TableColumns {
        BODYID_COLUMN,
        CELL_TYPE_COLUMN,
        STATUS_COLUMN,
        BUTTON_COLUMN
    };

    void loadBodyData();
    void setupUI();
    void setTableHeaders(QStandardItemModel * model);
    void updateTable();

    // data stuff
    ZFlyEmBody3dDoc * m_bodyDoc;
    QList<uint64_t> m_bodyIDs;
    QStringList m_celltypes;
    QStringList m_statuses;

    // UI stuff
    QWidget *m_widget;

    QTableView *m_bodyTableView;
    QStandardItemModel *m_bodyModel;





};

#endif // TASKBODYREVIEW_H
