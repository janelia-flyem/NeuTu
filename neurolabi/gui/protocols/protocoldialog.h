#ifndef PROTOCOLDIALOG_H
#define PROTOCOLDIALOG_H

#include <QDialog>

#include "flyem/zflyemsequencercolorscheme.h"

#include "zjsonobject.h"
#include "dvid/zdvidreader.h"

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
    void setDvidTarget(ZDvidTarget target);
    const ZDvidTarget& getDvidTarget() const;

public:
    //Special behaviors
    virtual void processSynapseVerification(int x, int y, int z, bool verified);
    virtual void processSynapseMoving(
        const ZIntPoint &from, const ZIntPoint &to);

signals:
    void protocolCompleting();
    void protocolExiting();
    void requestSaveProtocol(ZJsonObject data);
    void requestDisplayPoint(int x, int y, int z);
    void requestDisplayBody(uint64_t bodyID);
    void requestColorMapChange(ZFlyEmSequencerColorScheme scheme);
    void requestActivateColorMap();
    void requestDeactivateColorMap();
    void rangeChanged(ZIntPoint firstCorner, ZIntPoint lastCorner);

public slots:
    virtual void loadDataRequested(ZJsonObject data) = 0;

protected:
    ZDvidReader m_dvidReader;

private:
    Ui::ProtocolDialog *ui;
};

#endif // PROTOCOLDIALOG_H
