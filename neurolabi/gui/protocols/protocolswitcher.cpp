#include "protocolswitcher.h"

#include <iostream>
#include <stdlib.h>

#include "protocolchooser.h"

#include "dvid/zdvidtarget.h"

/*
 * this class manages protocols at the highest level; it loads,
 * unloads, starts, ends, and generally manages the user's interaction
 * with protocols; it's not a UI component itself, but it knows
 * which dialogs and windows to open at any given time
 */
ProtocolSwitcher::ProtocolSwitcher(QWidget *parent) : QObject(parent)
{
    m_parent = parent;
}

void ProtocolSwitcher::openProtocolRequested() {

    std::cout << "entering prsw::openProtocolRequested()"<< std::endl;

    // main logic:
    if (m_currentDvidTarget.isValid()) {
        // testing
        m_chooser = new ProtocolChooser(m_parent);
        m_chooser->show();
        m_chooser->raise();

    }



    std::cout << "leaving prsw::openProtocolRequested()"<< std::endl;
}

void ProtocolSwitcher::dvidTargetChanged(ZDvidTarget target) {
    m_currentDvidTarget = target;


    // probably should check for active protocol here and start loading
    //  even before user decides to open the window?


}

