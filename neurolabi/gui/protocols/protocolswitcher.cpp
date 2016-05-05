#include "protocolswitcher.h"

#include <iostream>
#include <stdlib.h>

#include "protocolchooser.h"
#include "protocolmetadata.h"

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
    m_chooser = new ProtocolChooser(m_parent);
    m_active = false;

    // set up connections to ProtocolChooser



}

void ProtocolSwitcher::openProtocolRequested() {

    std::cout << "entering prsw::openProtocolRequested()"<< std::endl;


    if (m_active) {
        // show protocol dialog
        std::cout << "protocol dialogs not implemented yet" << std::endl;

    } else {
        // loading message in the protocol chooser
        // trigger population of protocol chooser

        //  populate chooser:
        //      "loading" message
        //      check for inactive, incomplete protocols we could load;
        //          add to loadable list
        //      add allowable new protocols to new protocol list
        //      clear "loading" message

        // show chooser


        // testing:
        if (m_currentDvidTarget.isValid()) {
            m_chooser->show();
            m_chooser->raise();
        }

    }



    std::cout << "leaving prsw::openProtocolRequested()"<< std::endl;
}

void ProtocolSwitcher::dvidTargetChanged(ZDvidTarget target) {
    m_currentDvidTarget = target;

    // check for active protocol here and start loading
    //  even before user decides to open the dialog
    ProtocolMetadata metadata = readMetadata();
    if (metadata.isActive()) {

        m_active = true;


        std::cout << "active protocol not implemented yet" << std::endl;

        // create dialog first, empty, with loading message;
        //  it may be shown quickly
        // load saved protocol data
        // populate dialog
        // drop loading message

    } else {
        m_active = false;
        // nothing else to do here; we can't set up the protocol
        //  chooser because the information about which protocols
        //  can be initiated or loaded can change over time
    }
}

ProtocolMetadata ProtocolSwitcher::readMetadata() {
    if (m_currentDvidTarget.isValid()) {
        // in progress; currently returns metadata that says nothing is saved
        ProtocolMetadata metadata;
        return metadata;
    } else {
        // default metadata = nothing active, nothing saved
        ProtocolMetadata metadata;
        return metadata;
    }
}
