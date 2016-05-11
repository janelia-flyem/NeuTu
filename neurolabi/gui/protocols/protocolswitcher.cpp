#include "protocolswitcher.h"

#include <iostream>
#include <stdlib.h>

#include "protocolchooser.h"
#include "protocoldialog.h"
#include "protocolmetadata.h"

#include "dvid/zdvidtarget.h"

/*
 * this class manages protocols at the highest level; it loads,
 * unloads, starts, ends, and generally manages the user's interaction
 * with protocols; it's not a UI component itself, but it knows
 * which dialogs and windows to open at any given time
 *
 * to add new protocol:
 * -- (procedure is tentative, in development!)
 * -- subclass ProtocolDialog and implement
 * -- add name to protocolNames array
 * -- add to if-else chaing in startProtocolRequested()
 */
ProtocolSwitcher::ProtocolSwitcher(QWidget *parent) : QObject(parent)
{
    m_parent = parent;
    m_chooser = new ProtocolChooser(m_parent);

    // flag is true if a protocol is active, including if it's
    //  being started or loaded
    m_active = false;



    // set up connections to ProtocolChooser
    connect(m_chooser, SIGNAL(requestStartProtocol(QString)), this, SLOT(startProtocolRequested(QString)));



}

// names of available protocols; thank you, C++, for making
//  constants so hard to define
QStringList ProtocolSwitcher::protocolNames = QStringList()
        << "Do N things";


void ProtocolSwitcher::openProtocolRequested() {
    if (!m_currentDvidTarget.isValid()) {
        return;
    }

    if (m_active) {
        // show protocol dialog for the protocol we're loading
        std::cout << "protocol dialogs not implemented yet" << std::endl;

    } else {
        // show the protocol chooser

        // at some point, we will look for saved protocols to load:
        //      set "loading" message
        //      check for inactive, incomplete protocols we could load;
        //          add to loadable list
        //      clear "loading" message

        m_chooser->show();
        m_chooser->raise();
    }

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

// start a new protocol
void ProtocolSwitcher::startProtocolRequested(QString protocolName) {

    // if-else chain not ideal, but C++ is too stupid to switch 
    //  on strings; however, the chain won't get *too* long,
    //  so it's not that bad
    if (protocolName == "Do N things") {
        m_activeProtocol = new ProtocolDialog(m_parent);
    } else {
        // should never happen
        return;
    }

    // connect protocol connections:



    // trigger protocol initialization and go to town
    // what init is there?  could be interactive or not, so
    //  has to be in this thread; if the protocol needs to do
    //  something long-running, it has to manage it

    m_activeProtocol->initialize();
    m_activeProtocol->raise();
    m_activeProtocol->show();

}

// load a saved protocol
void ProtocolSwitcher::loadProtocolRequested() {

    // this also needs to have the saved info as input

    // do stuff
}





















