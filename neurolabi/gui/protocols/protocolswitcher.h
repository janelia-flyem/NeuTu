#ifndef PROTOCOLSWITCHER_H
#define PROTOCOLSWITCHER_H

#include <QObject>

#include "protocolchooser.h"
#include "protocolmetadata.h"

#include "dvid/zdvidtarget.h"


class ProtocolSwitcher : public QObject
{
    Q_OBJECT
public:
    explicit ProtocolSwitcher(QWidget *parent = 0);

signals:

public slots:
    void openProtocolRequested();
    void dvidTargetChanged(ZDvidTarget target);

private slots:


private:
    QWidget * m_parent;
    ZDvidTarget m_currentDvidTarget;
    ProtocolChooser * m_chooser;
    bool m_active;

    ProtocolMetadata readMetadata();
};

#endif // PROTOCOLSWITCHER_H
