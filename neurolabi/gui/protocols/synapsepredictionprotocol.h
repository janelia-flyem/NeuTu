#ifndef SYNAPSEPREDICTIONPROTOCOL_H
#define SYNAPSEPREDICTIONPROTOCOL_H

#include <QDialog>

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
    explicit SynapsePredictionProtocol(QWidget *parent = 0);
    ~SynapsePredictionProtocol();
    bool initialize();
    std::string getName();

public:
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

#ifdef _DON_
    void onMarkedButton();
    void onSkipButton();
#endif
    void onGotoButton();
    void onExitButton();
    void onCompleteButton();
    void onRefreshButton();

private:
    static const std::string PROTOCOL_NAME;
    static const std::string KEY_PENDING;
    static const std::string KEY_FINISHED;
    static const std::string KEY_VERSION;
    static const std::string KEY_PROTOCOL_RANGE;
    static const int fileVversion;

    Ui::SynapsePredictionProtocol *ui;
    QList<ZIntPoint> m_pendingList;
    QList<ZIntPoint> m_finishedList;
#ifdef _DON_
    ZIntPoint m_currentPoint;
#else
    int m_currentPendingIndex; //Index for locating in pending list
    int m_currentFinishedIndex;
#endif
    ZIntCuboid m_protocolRange;

    void saveState();
    void updateLabels();
    void gotoCurrent();
    void gotoCurrentFinished();
#ifdef _DON_
    ZIntPoint getNextPoint(ZIntPoint point);
    ZIntPoint getPrevPoint(ZIntPoint point);
#endif
    void loadInitialSynapseList(ZIntCuboid volume, QString roi);
    void loadInitialSynapseList();

};

#endif // SYNAPSEPREDICTIONPROTOCOL_H
