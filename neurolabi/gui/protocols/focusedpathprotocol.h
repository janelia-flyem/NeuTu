#ifndef FOCUSEDPATHPROTOCOL_H
#define FOCUSEDPATHPROTOCOL_H

#include "zjsonobject.h"

#include "protocoldialog.h"

namespace Ui {
class FocusedPathProtocol;
}

class FocusedPathProtocol : public ProtocolDialog
{
    Q_OBJECT

public:
    explicit FocusedPathProtocol(QWidget *parent = 0);
    ~FocusedPathProtocol();

signals:
    void protocolCompleting();
    void protocolExiting();
    void requestSaveProtocol(ZJsonObject data);

public slots:
    void loadDataRequested(ZJsonObject data);

private slots:
    void onExitButton();
    void onCompleteButton();

private:
    Ui::FocusedPathProtocol *ui;
    bool initialize();
};

#endif // FOCUSEDPATHPROTOCOL_H
