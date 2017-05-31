#ifndef SYNAPSEREVIEWPROTOCOL_H
#define SYNAPSEREVIEWPROTOCOL_H

#include <QDialog>

#include "zjsonobject.h"

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

private:
    Ui::SynapseReviewProtocol *ui;
};

#endif // SYNAPSEREVIEWPROTOCOL_H
