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

public slots:
    void openProtocolRequested();
    void dvidTargetChanged(ZDvidTarget target);

private slots:
    void startProtocolRequested(QString protocolName);
    void loadProtocolRequested();
    void exitProtocolRequested();

private:
    enum Status {
        PROTOCOL_ACTIVE,
        PROTOCOL_INACTIVE,
        PROTOCOL_INITIALIZING,
        PROTOCOL_LOADING
    };

    QWidget * m_parent;
    ZDvidTarget m_currentDvidTarget;
    ProtocolChooser * m_chooser;
    Status m_protocolStatus;
    ProtocolDialog * m_activeProtocol;

    ProtocolMetadata readMetadata();
    void connectProtocolSignals();
    void disconnectProtocolSignals();
};

#endif // PROTOCOLSWITCHER_H
