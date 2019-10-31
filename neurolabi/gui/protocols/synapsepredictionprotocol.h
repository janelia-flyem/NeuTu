#ifndef SYNAPSEPREDICTIONPROTOCOL_H
#define SYNAPSEPREDICTIONPROTOCOL_H

#include <QDialog>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include "dvid/zdvidsynapse.h"
#include "zjsonarray.h"
#include "zjsonobject.h"
#include "geometry/zintcuboid.h"
#include "geometry/zintpoint.h"

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
    static const std::string SUBVARIATION_BODY_TBAR;
    static const std::string SUBVARIATION_BODY_PSD;
    static const std::string SUBVARIATION_BODY_BOTH;
    void processSynapseVerification(int x, int y, int z, bool verified);
    void processSynapseVerification(const ZIntPoint &pt, bool verified);
    void processSynapseMoving(const ZIntPoint &from, const ZIntPoint &to);

    ZIntCuboid getRange() const;
    void setRange(const ZIntCuboid &range);
    void setRange(const ZJsonArray &rangeJson);

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
    void onModeChanged(QString item);    
    void onGridToggled();

private:
    static const std::string KEY_VARIATION;
    static const std::string KEY_SUBVARIATION;
    static const std::string KEY_VERSION;
    static const std::string KEY_PROTOCOL_RANGE;
    static const std::string KEY_BODYID;
    static const std::string KEY_MODE;
    static const int fileVersion;
    static const QColor COLOR_DEFAULT;
    static const QColor COLOR_TARGET_BODY;
    static const QString MODE_SYNAPSE;
    static const QString MODE_TBAR;
    static const QString MODE_PSD;

    enum SitesTableColumns {
        SITES_STATUS_COLUMN,
        SITES_CONFIDENCE_COLUMN,
        SITES_X_COLUMN,
        SITES_Y_COLUMN,
        SITES_Z_COLUMN
    };

    Ui::SynapsePredictionProtocol *ui;
    QStandardItemModel * m_sitesModel;
    QSortFilterProxyModel * m_sitesProxy;
    std::string m_variation;
    std::string m_subvariation;
    QList<ZIntPoint> m_pendingList;
    QList<ZIntPoint> m_finishedList;
    int m_currentPendingIndex; //Index for locating in pending list
    int m_currentFinishedIndex;
    ZIntCuboid m_protocolRange;
    uint64_t  m_bodyID;
    QList<QColor> m_postColorList;
    ZFlyEmSequencerColorScheme m_colorScheme;
    QString m_currentMode;
    ZDvidReader m_reader;

    void saveState();
    void updateLabels();
    void gotoCurrent();
    void gotoCurrentFinished();
    void loadInitialSynapseList();
    void setSitesHeaders(QStandardItemModel * model);
    void clearSitesTable();
    void updateSitesTable(std::vector<ZDvidSynapse> synapses, std::vector<uint64_t> bodyList);
    void updateColorMap(std::vector<ZDvidSynapse> synapses, std::vector<uint64_t> bodyList);
    void updateSiteListLabel();
    std::vector<ZDvidSynapse> getWholeSynapse(
        ZIntPoint point, const ZDvidReader &reader);
    static bool sortXY(const ZIntPoint &p1, const ZIntPoint &p2);
    static bool compareSynapses(const ZDvidSynapse &synapse1, const ZDvidSynapse &synapse2);
    void variationError(std::string variation);    
    void setupColorList();
    void enableProtocolColorMap();
    void disableProtocolColorMap();
    QColor getColor(int index);
    bool isFinished(ZIntPoint point);
    bool isFinished(std::vector<ZDvidSynapse> synapseElements);
    void refreshData(bool unfinishCurrent);
    bool keepSynapse(ZDvidSynapse synapse);
    std::vector<uint64_t> getBodiesForSynapse(std::vector<ZDvidSynapse> synapse);
    QString targetBodyStylesheet(QColor color);
    void initPendingIndex();
};

#endif // SYNAPSEPREDICTIONPROTOCOL_H
