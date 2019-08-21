#ifndef CONNECTIONVALIDATIONPROTOCOL_H
#define CONNECTIONVALIDATIONPROTOCOL_H

#include <QDialog>
#include <QDialog>

#include "protocoldialog.h"

#include "zjsonobject.h"

namespace Ui {
class ConnectionValidationProtocol;
}

class ConnectionValidationProtocol : public ProtocolDialog
{
    Q_OBJECT

public:
    explicit ConnectionValidationProtocol(QWidget *parent = 0);
    ~ConnectionValidationProtocol();

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

private:
    static const std::string KEY_VERSION;
    static const int fileVersion;

    void saveState();

    Ui::ConnectionValidationProtocol *ui;

};

#endif // CONNECTIONVALIDATIONPROTOCOL_H
