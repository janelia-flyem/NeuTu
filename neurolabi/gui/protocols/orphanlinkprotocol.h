#ifndef ORPHANLINKPROTOCOL_H
#define ORPHANLINKPROTOCOL_H

#include <QDialog>

#include "protocoldialog.h"
#include "protocolassignmentclient.h"

#include "zjsonobject.h"

namespace Ui {
class OrphanLinkProtocol;
}

class OrphanLinkProtocol : public ProtocolDialog
{
    Q_OBJECT

public:
    explicit OrphanLinkProtocol(QWidget *parent = 0);
    ~OrphanLinkProtocol();
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
    static const std::string KEY_PARAMETERS;
    static const int fileVersion;

    void saveState();
    void showError(QString title, QString message);

    Ui::OrphanLinkProtocol *ui;
    ProtocolAssignmentClient m_assignments;

};

#endif // ORPHANLINKPROTOCOL_H
