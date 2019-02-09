#ifndef SYNAPSEREVIEWPROTOCOL_H
#define SYNAPSEREVIEWPROTOCOL_H

#include <QDialog>
#include <QStandardItemModel>

#include "dvid/zdvidsynapse.h"
#include "zjsonobject.h"
#include "geometry/zintpoint.h"
#include "protocoldialog.h"


namespace Ui {
class SynapseReviewProtocol;
}

class SynapseReviewProtocol : public ProtocolDialog
{
    Q_OBJECT

public:
    explicit SynapseReviewProtocol(QWidget *parent = 0);
    ~SynapseReviewProtocol();
    bool initialize();

signals:
    void protocolCompleting();
    void protocolExiting();
    void requestSaveProtocol(ZJsonObject data);

public slots:
    void loadDataRequested(ZJsonObject data);

private slots:
    void onExitButton();
    void onCompleteButton();
    void onReviewFirstButton();
    void onReviewNextButton();
    void onReviewPreviousButton();
    void onFinishedLastButton();
    void onFinishedNextButton();
    void onFinishedPreviousButton();
    void onGotoCurrentButton();
    void onMarkReviewedButton();

    void onDoubleClickSitesTable(QModelIndex index);

private:
    static const std::string KEY_VERSION;
    static const std::string KEY_PENDING_LIST;
    static const std::string KEY_FINISHED_LIST;
    static const int fileVersion;

    enum SitesTableColumns {
        SITES_X_COLUMN,
        SITES_Y_COLUMN,
        SITES_Z_COLUMN
    };

    Ui::SynapseReviewProtocol *ui;
    QStandardItemModel * m_sitesModel;
    QList<ZIntPoint> m_pendingList;
    QList<ZIntPoint> m_finishedList;
    int m_currentPendingIndex;
    int m_currentFinishedIndex;

    void updateUI();
    void updatePSDTable();
    void updateLabels();
    void gotoCurrent();
    void gotoCurrentFinished();
    void saveState();    
    void inputErrorDialog(QString message);
    void setSitesHeaders(QStandardItemModel * model);
    void clearSitesTable();
    void populatePSDTable(std::vector<ZDvidSynapse>);
    std::vector<ZDvidSynapse> getWholeSynapse(ZIntPoint point);


};

#endif // SYNAPSEREVIEWPROTOCOL_H
