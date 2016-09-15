#ifndef PROTOCOLDIALOG_H
#define PROTOCOLDIALOG_H

#include <QDialog>

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
    void setDvidTarget(ZDvidTarget target);

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

public slots:
    virtual void loadDataRequested(ZJsonObject data) = 0;

protected:
    ZDvidTarget m_dvidTarget;

private:
    Ui::ProtocolDialog *ui;
};

#endif // PROTOCOLDIALOG_H
