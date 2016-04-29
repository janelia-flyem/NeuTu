#ifndef PROTOCOLSWITCHER_H
#define PROTOCOLSWITCHER_H

#include <QObject>

#include "dvid/zdvidtarget.h"


class ProtocolSwitcher : public QObject
{
    Q_OBJECT
public:
    explicit ProtocolSwitcher(QObject *parent = 0);

signals:

public slots:
    void openProtocolRequested();
    void dvidTargetChanged(ZDvidTarget target);

private slots:


private:
    ZDvidTarget m_currentDvidTarget;

};

#endif // PROTOCOLSWITCHER_H
