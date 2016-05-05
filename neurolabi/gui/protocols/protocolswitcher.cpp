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
    m_chooser = new ProtocolChooser(m_parent);


    // set up connections to ProtocolChooser



}

void ProtocolSwitcher::openProtocolRequested() {

    std::cout << "entering prsw::openProtocolRequested()"<< std::endl;


    // testing
    if (m_currentDvidTarget.isValid()) {
        m_chooser->show();
        m_chooser->raise();
    }



    // actual logic
    // if protocol is active and loading:
    //  if protocol is done loading:
    //      show the protocol dialog
    //  else
    //      (how to wait for protocol dialog to be ready to show?)
    //      (does protocol dialog need to work like chooser, immediately show 
    //      while loading, then populate?)
    // else
    //  show the protocol chooser



    std::cout << "leaving prsw::openProtocolRequested()"<< std::endl;
}

void ProtocolSwitcher::dvidTargetChanged(ZDvidTarget target) {
    m_currentDvidTarget = target;


    // probably should check for active protocol here and start loading
    //  even before user decides to open the window?  

    // also check for paused protocols that could be loaded?
    // trigger population of ProtocolChooser here?


    // retrieve protocol metadata
    //  (need new ProtocolMetadata class)
    // if there's an active protocol:
    //  set "protocol loading" flag
    //  set "active protocol" flag
    //  load it: create, populate dialog; 
    // if no:
    //  unset "active protocol" flag
    //  populate chooser:
    //      "loading" message
    //      check for inactive, incomplete protocols we could load; 
    //          add to loadable list
    //      add allowable new protocols to new protocol list
    //      clear "loading" message

}

