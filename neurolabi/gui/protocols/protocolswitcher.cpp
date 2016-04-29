#include "protocolswitcher.h"

#include <iostream>
#include <stdlib.h>

#include "dvid/zdvidtarget.h"

/*
 * this class manages protocols at the highest level; it loads,
 * unloads, starts, ends, and generally manages the user's interaction
 * with protocols; it's not a UI component itself, but it knows
 * which dialogs and windows to open at any given time
 */
ProtocolSwitcher::ProtocolSwitcher(QObject *parent) : QObject(parent)
{

}

void ProtocolSwitcher::openProtocolRequested() {

    // main logic

}

void ProtocolSwitcher::dvidTargetChanged(ZDvidTarget target) {
    m_currentDvidTarget = target;


    // probably should check for active protocol here and start loading
    //  even before user decides to open the window?


}

