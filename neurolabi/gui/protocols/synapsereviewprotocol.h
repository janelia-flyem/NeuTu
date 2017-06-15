#ifndef SYNAPSEREVIEWPROTOCOL_H
#define SYNAPSEREVIEWPROTOCOL_H

#include <QDialog>

#include "zjsonobject.h"
#include "zintpoint.h"
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
    void onGotoCurrentButton();
    void onMarkReviewedButton();

private:
    static const std::string KEY_VERSION;
    static const std::string KEY_PENDING_LIST;
    static const std::string KEY_FINISHED_LIST;
    static const int fileVersion;

    Ui::SynapseReviewProtocol *ui;
    QList<ZIntPoint> m_pendingList;
    QList<ZIntPoint> m_finishedList;
    ZIntPoint m_currentSite;

    void updateUI();
    void updatePSDTable();
    void updateLabels();
    void gotoCurrent();
    void saveState();    
    void inputErrorDialog(QString message);

};

#endif // SYNAPSEREVIEWPROTOCOL_H
