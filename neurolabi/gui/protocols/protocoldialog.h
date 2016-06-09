#ifndef PROTOCOLDIALOG_H
#define PROTOCOLDIALOG_H

#include <QDialog>

#include "zjsonobject.h"
#include "dvid/zdvidtarget.h"

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
    virtual std::string getName() = 0;
    void setDvidTarget(ZDvidTarget target);

signals:
    void protocolCompleting();
    void protocolExiting();
    void requestSaveProtocol(ZJsonObject data);

public slots:
    virtual void loadDataRequested(ZJsonObject data) = 0;

protected:
    ZDvidTarget m_dvidTarget;

private:
    Ui::ProtocolDialog *ui;
};

#endif // PROTOCOLDIALOG_H
