#ifndef SYNAPSEPREDICTIONPROTOCOL_H
#define SYNAPSEPREDICTIONPROTOCOL_H

#include <QDialog>
#include <QtGui>

#include "dvid/zdvidsynapse.h"
#include "zjsonarray.h"
#include "zjsonobject.h"
#include "zintcuboid.h"
#include "zintpoint.h"

#include "protocoldialog.h"

namespace Ui {
class SynapsePredictionProtocol;
}

class SynapsePredictionProtocol : public ProtocolDialog
{
    Q_OBJECT

public:
    explicit SynapsePredictionProtocol(QWidget *parent = 0, std::string variation = VARIATION_REGION);
    ~SynapsePredictionProtocol();
    bool initialize();
    static const std::string VARIATION_REGION;
    static const std::string VARIATION_BODY;
    void processSynapseVerification(int x, int y, int z, bool verified);
    void processSynapseVerification(const ZIntPoint &pt, bool verified);
    void processSynapseMoving(const ZIntPoint &from, const ZIntPoint &to);

signals:
    void protocolCompleting();
    void protocolExiting();
    void requestSaveProtocol(ZJsonObject data);

public slots:
    void loadDataRequested(ZJsonObject data);

    void verifySynapse(const ZIntPoint &pt);
    void moveSynapse(const ZIntPoint &src, const ZIntPoint &dst);
    void unverifySynapse(const ZIntPoint &pt);

private slots:
    void onFirstButton();
    void onPrevButton();
    void onNextButton();

    void onReviewFirstButton();
    void onReviewPrevButton();
    void onReviewNextButton();
    void onLastVerifiedButton();

    void onGotoButton();
    void onFinishCurrentButton();
    void onExitButton();
    void onCompleteButton();
    void onRefreshButton();

    void onDoubleClickSitesTable(QModelIndex index);

private:
    static const std::string KEY_VARIATION;
    static const std::string KEY_VERSION;
    static const std::string KEY_PROTOCOL_RANGE;
    static const std::string KEY_BODYID;
    static const int fileVersion;

    enum SitesTableColumns {
        SITES_STATUS_COLUMN,
        SITES_CONFIDENCE_COLUMN,
        SITES_X_COLUMN,
        SITES_Y_COLUMN,
        SITES_Z_COLUMN
    };

    Ui::SynapsePredictionProtocol *ui;
    QStandardItemModel * m_sitesModel;
    std::string m_variation;
    QList<ZIntPoint> m_pendingList;
    QList<ZIntPoint> m_finishedList;
    int m_currentPendingIndex; //Index for locating in pending list
    int m_currentFinishedIndex;
    ZIntCuboid m_protocolRange;
    uint64_t  m_bodyID;

    void saveState();
    void updateLabels();
    void gotoCurrent();
    void gotoCurrentFinished();
    void loadInitialSynapseList();
    void setSitesHeaders(QStandardItemModel * model);
    void clearSitesTable();
    void updateSitesTable(std::vector<ZDvidSynapse>);
    std::vector<ZDvidSynapse> getWholeSynapse(ZIntPoint point);
    static bool sortXY(const ZIntPoint &p1, const ZIntPoint &p2);
    static bool compareSynapses(const ZDvidSynapse &synapse1, const ZDvidSynapse &synapse2);
    void variationError(std::string variation);

};

#endif // SYNAPSEPREDICTIONPROTOCOL_H
