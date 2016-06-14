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

signals:
    void protocolCompleting();
    void protocolExiting();
    void requestSaveProtocol(ZJsonObject data);

public slots:
    void loadDataRequested(ZJsonObject data);


private slots:
    void onFirstButton();
    void onMarkedButton();
    void onSkipButton();
    void onGotoButton();
    void onExitButton();
    void onCompleteButton();

private:
    static const std::string PROTOCOL_NAME;
    static const std::string KEY_PENDING;
    static const std::string KEY_FINISHED;

    Ui::SynapsePredictionProtocol *ui;
    QList<ZIntPoint> m_pendingList;
    QList<ZIntPoint> m_finishedList;
    ZIntPoint m_currentPoint;

    void saveState();
    void updateLabels();
    void gotoNextItem();
    ZIntCuboid parseVolumeString(QString input);
    void loadInitialSynapseList(ZIntCuboid volume, QString roi);

};

#endif // SYNAPSEPREDICTIONPROTOCOL_H
