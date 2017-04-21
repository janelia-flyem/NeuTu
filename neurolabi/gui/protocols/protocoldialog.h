#ifndef PROTOCOLDIALOG_H
#define PROTOCOLDIALOG_H

#include <QDialog>

#include "flyem/zflyemsequencercolorscheme.h"
#include "flyem/zflyemproofmvc.h"
#include "flyem/zflyemproofdoc.h"

#include "zjsonobject.h"
#include "dvid/zdvidtarget.h"

class ZIntPoint;

namespace Ui {
class ProtocolDialog;
}

class ProtocolDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProtocolDialog(QWidget *parent = 0);
    virtual ~ProtocolDialog();
    virtual bool initialize() = 0;
    virtual void setDvidTarget(ZDvidTarget target);

public:
    // optional: implement these calls to connect to signals sent by ZFlyEmProofDoc
    virtual void processSynapseVerification(int x, int y, int z, bool verified);
    virtual void processSynapseMoving(
        const ZIntPoint &from, const ZIntPoint &to);
    virtual void processBodyMerged();

signals:
    void protocolCompleting();
    void protocolExiting();
    void requestSaveProtocol(ZJsonObject data);
    void requestDisplayPoint(int x, int y, int z);
    void requestColorMapChange(ZFlyEmSequencerColorScheme scheme);
    void requestActivateColorMap();
    void requestDeactivateColorMap();

public slots:
    virtual void loadDataRequested(ZJsonObject data) = 0;

protected:
    ZDvidTarget m_dvidTarget;    
    ZFlyEmProofMvc *getParentFrame() const;
    ZFlyEmProofDoc *getDocument() const;

private:
    Ui::ProtocolDialog *ui;
};

#endif // PROTOCOLDIALOG_H
