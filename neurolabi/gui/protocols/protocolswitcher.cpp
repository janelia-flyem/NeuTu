#include "protocolswitcher.h"

#include <iostream>
#include <stdlib.h>

#include "dvid/zdvidtarget.h"


ProtocolSwitcher::ProtocolSwitcher(QObject *parent) : QObject(parent)
{

}

void ProtocolSwitcher::openProtocolRequested() {
    std::cout << "prsw: open requested" << std::endl;
}

void ProtocolSwitcher::dvidTargetChanged(ZDvidTarget target) {
    std::cout << "prsw: dvid target changed" << std::endl;
}

