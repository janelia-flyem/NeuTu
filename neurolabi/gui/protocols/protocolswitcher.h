#ifndef PROTOCOLSWITCHER_H
#define PROTOCOLSWITCHER_H

#include <QObject>

#include "protocolchooser.h"
#include "protocoldialog.h"
#include "protocolmetadata.h"

#include "dvid/zdvidtarget.h"


class ProtocolSwitcher : public QObject
{
    Q_OBJECT
public:
    explicit ProtocolSwitcher(QWidget *parent = 0);

    static QStringList protocolNames;

signals:
    void requestLoadProtocol(ZJsonObject data);
    void requestDisplaySavedProtocols(QStringList keyList);
    void requestDisplayPoint(int x, int y, int z);

public slots:
    void openProtocolDialogRequested();
    void dvidTargetChanged(ZDvidTarget target);

    void processSynapseVerification(int x, int y, int z, bool verified);
    void processSynapseMoving(const ZIntPoint &from, const ZIntPoint &to);

private slots:
    void startProtocolRequested(QString protocolName);
    void saveProtocolRequested(ZJsonObject data);
    void loadProtocolRequested();
    void loadProtocolKeyRequested(QString protocolKey);
    void completeProtocolRequested();
    void exitProtocolRequested();

    // these are slots that are used to pass signals from
    //  protocols to the main application
    void displayPointRequested(int x, int y, int z);

private:
    enum Status {
        PROTOCOL_ACTIVE,
        PROTOCOL_INACTIVE,
        PROTOCOL_INITIALIZING,
        PROTOCOL_LOADING
    };

    static const std::string PROTOCOL_DATA_NAME;
    static const std::string PROTOCOL_COMPLETE_SUFFIX;

    QWidget * m_parent;
    ZDvidTarget m_currentDvidTarget;
    ProtocolChooser * m_chooser;
    Status m_protocolStatus;
    ProtocolDialog * m_activeProtocol;
    ProtocolMetadata m_activeMetadata;

    void connectProtocolSignals();
    void disconnectProtocolSignals();
    bool askProceedIfNodeLocked();
    bool checkCreateDataInstance();
    void warningDialog(QString title, QString message);
    std::string generateKey(QString protocolName);
    std::string generateIdentifier();
    bool askProceedIfKeyExists(std::string key);
    void instantiateProtocol(QString protocolName);
    QStringList getUserProtocolKeys(QString username, bool showComplete);
};

#endif // PROTOCOLSWITCHER_H
