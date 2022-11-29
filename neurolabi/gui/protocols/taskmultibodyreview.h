#ifndef TASKMULTIBODYREVIEW_H
#define TASKMULTIBODYREVIEW_H

#include <iostream>
#include <stdlib.h>

#include <QObject>
#include <QSet>

#include <QCheckBox>
#include <QModelIndex>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTableView>
#include <QWidget>

#include "common/neutudefs.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"
#include "protocols/taskprotocoltask.h"
#include "flyem/zflyembodyannotation.h"

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

    bool usePrefetching() override;

    virtual void beforeDone() override;

    QWidget * getTaskWidget();

private slots:
    void onClickedTable(QModelIndex index);
    void onRowPRTButton(int);
    void onRowRevertButton(int);
    void onAllPRTButton();

private:
    static const QString KEY_TASKTYPE;
    static const QString VALUE_TASKTYPE;
    static const QString KEY_BODYIDS;

    static const QString STATUS_PRT;

    bool loadSpecific(QJsonObject json) override;
    QJsonObject addToJson(QJsonObject json) override;
    void onLoaded() override;

    enum TableColumns {
        BODYID_COLUMN,
        CELL_TYPE_COLUMN,
        INSTANCE_COLUMN,
        STATUS_COLUMN,
        BUTTON_COLUMN
    };

    void loadBodyData();
    void setupUI();
    void setupDVID();
    void setColors();
    void setTableHeaders(QStandardItemModel * model);
    void updateTable();

    // data stuff
    ZFlyEmBody3dDoc * m_bodyDoc;
    // these lists are all in the same order
    QList<uint64_t> m_bodyIDs;
    QList<ZFlyEmBodyAnnotation> m_bodyAnnotations;
    QList<std::string> m_originalStatuses;
    bool m_originalStatusesLoaded = false;

    ZDvidReader m_reader;
    ZDvidWriter m_writer;

    void setStatus(uint64_t bodyId, ZFlyEmBodyAnnotation ann, std::string status);
    void setPRTStatusForRow(int row);
    void setOriginalStatusForRow(int row);

    // UI stuff
    QWidget *m_widget;

    QTableView *m_bodyTableView;
    QStandardItemModel *m_bodyModel;

    QPushButton *m_allPRTButton;



};

#endif // TASKBODYREVIEW_H
