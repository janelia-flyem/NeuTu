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

public slots:
    void openProtocolDialogRequested();
    void dvidTargetChanged(ZDvidTarget target);

private slots:
    void startProtocolRequested(QString protocolName);
    void saveProtocolRequested(ZJsonObject data);
    void loadProtocolRequested();
    void loadProtocolKeyRequested(QString protocolKey);
    void exitProtocolRequested();

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
    std::string generateKey();
    std::string generateIdentifier();
    bool askProceedIfKeyExists(std::string key);
    ProtocolDialog * instantiateProtocol(QString protocolName);
    QStringList getUserProtocolKeys(QString username, bool showComplete);
};

#endif // PROTOCOLSWITCHER_H
