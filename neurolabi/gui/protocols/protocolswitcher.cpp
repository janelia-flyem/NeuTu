#include "protocolswitcher.h"

#include <iostream>
#include <stdlib.h>

#include <QInputDialog>
#include <QLineEdit>
#include <QObject>
#include <QMessageBox>

#include "protocolchooser.h"
#include "protocoldialog.h"
#include "protocolmetadata.h"

#include "neutube.h"

#include "dvid/zdvidreader.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidwriter.h"
#include "zjsonobject.h"

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
 * -- add to if-else chain in startProtocolRequested()
 */
ProtocolSwitcher::ProtocolSwitcher(QWidget *parent) : QObject(parent)
{
    m_parent = parent;
    m_chooser = new ProtocolChooser(m_parent);

    m_protocolStatus = PROTOCOL_INACTIVE;



    // set up connections to ProtocolChooser
    connect(m_chooser, SIGNAL(requestStartProtocol(QString)), this, SLOT(startProtocolRequested(QString)));



}

// name of data instance in DVID; we do not abstract this out the way we
//  do some data names because we really don't think it'll ever change;
//  there is no reason to need parallel instances as you can use
//  different identifiers in the stored keys instead
const std::string ProtocolSwitcher::PROTOCOL_DATA_NAME = "NeuTu-protocols";

// names of available protocols; thank you, C++, for making
//  constants so hard to define
QStringList ProtocolSwitcher::protocolNames = QStringList()
        << "Do N things";


void ProtocolSwitcher::openProtocolDialogRequested() {
    if (!m_currentDvidTarget.isValid()) {
        return;
    }

    if (m_protocolStatus == PROTOCOL_ACTIVE) {
        // show protocol dialog for the protocol we're loading
        m_activeProtocol->raise();
        m_activeProtocol->show();
    } else if (m_protocolStatus == PROTOCOL_INITIALIZING) {
        // show a message
        QMessageBox msgBox;
        msgBox.setText("A protocol is initializing...");
        msgBox.exec();
    } else {
        // PROTOCOL_INACTIVE: show the protocol chooser

        // at some point, we will look for saved protocols to load:
        //      set "loading" message
        //      check for inactive, incomplete protocols we could load;
        //          add to loadable list
        //      clear "loading" message

        m_chooser->show();
        m_chooser->raise();
    }

}

void ProtocolSwitcher::exitProtocolRequested() {
    m_protocolStatus = PROTOCOL_INACTIVE;


    // write metadata: no active protocol


    // disconnect dialog signals
    disconnectProtocolSignals();

    // I think this is right...
    delete m_activeProtocol;
}

void ProtocolSwitcher::dvidTargetChanged(ZDvidTarget target) {
    m_currentDvidTarget = target;

    // check for active protocol here and start loading
    //  even before user decides to open the dialog
    ProtocolMetadata metadata = readMetadata();
    if (metadata.isActive()) {

        std::cout << "active protocol not implemented yet" << std::endl;

        // create dialog first, empty, with loading message;
        //  it may be shown quickly
        // load saved protocol data
        m_protocolStatus = PROTOCOL_LOADING;
        // populate dialog
        // drop loading message
        m_protocolStatus = PROTOCOL_ACTIVE;

    } else {
        m_protocolStatus = PROTOCOL_INACTIVE;
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

    // locked dvid node check
    if (!askProceedIfNodeLocked()) {
        return;
    }

    // don't start protocol if we can't save
    if (!checkCreateDataInstance()) {
        saveFailedDialog("DVID data instance for protocols was not present and/or could not be created!");
        m_protocolStatus = PROTOCOL_INACTIVE;
        return;
    }

    // generate the save key and be sure it's not in use
    if (m_activeProtocolKey.empty()) {
        std::string key = generateKey();
        if (key.empty()) {
            saveFailedDialog("Key for saved data couldn't be generated; did the user cancel?");
            return;
        }
        if (!askProceedIfKeyExists(key)) {
            saveFailedDialog("Key for saved data might already be used!");
            return;
        }
        m_activeProtocolKey = key;

        // tell user the key, data instance we're using
        QMessageBox::information(m_parent, "Save location",
            QString("Your data will be saved in DVID in data instance %1, key %2")
                .arg(QString::fromStdString(PROTOCOL_DATA_NAME))
                .arg(QString::fromStdString(m_activeProtocolKey)),
            QMessageBox::Ok);
    }


    m_protocolStatus = PROTOCOL_INITIALIZING;

    // if-else chain not ideal, but C++ is too stupid to switch 
    //  on strings; however, the chain won't get *too* long,
    //  so it's not that bad
    if (protocolName == "Do N things") {
        m_activeProtocol = new ProtocolDialog(m_parent);
    } else {
        // should never happen
        return;
    }

    // connect happens here, because if init succeeds, it'll
    //  want to request a save
    connectProtocolSignals();

    // trigger protocol initialization and go to town
    // what init is there?  could be interactive or not, so
    //  has to be in this thread; if the protocol needs to do
    //  something long-running, it has to manage it

    bool success = m_activeProtocol->initialize();
    if (success) {

        m_activeProtocol->raise();
        m_activeProtocol->show();
        m_protocolStatus = PROTOCOL_ACTIVE;

        // write protocol metadata (who's active)

    } else {
        // note that dialog explaining failure is expected to be
        //  raised by the protocol during "initialize()"

        disconnectProtocolSignals();

        m_protocolStatus = PROTOCOL_INACTIVE;
    }

}

// load a saved protocol
void ProtocolSwitcher::loadProtocolRequested() {

    // locked dvid node check
    if (!askProceedIfNodeLocked()) {
        return;
    }
    
    // handle m_protocolStatus state

    // this also needs to have the saved info as input

    // do stuff
}

void ProtocolSwitcher::saveProtocolRequested(ZJsonObject data) {
    std::cout << "prswi: save using key " << m_activeProtocolKey << std::endl;


    // check if node still unlocked?


    ZDvidWriter writer;
    if (writer.open(m_currentDvidTarget)) {
        // writer.writeJson(PROTOCOL_DATA_NAME, m_activeProtocolKey, data);
        std::cout << "prswi: pretending to write json: " << std::endl;
        std::cout << data.dumpString() << std::endl;

        //  - update metadata:
        //      - active protocol
        //      - list of incomplete protocols, if we decide to do that

    } else {
        saveFailedDialog("Failed to open DVID for writing");
    }

}

/*
 * check if dvid node is locked; if so, ask user if they want to
 * proceed even if they can't save; return true = node unlocked or
 * user says go ahead; false = user says no
 */
bool ProtocolSwitcher::askProceedIfNodeLocked() {

    // not implemented yet; not sure how to check for locked node

    // check if node locked; if not, return true
    // if locked, dialog: ask user if they want to proceed
    //  even though they can't save (this is useful for, eg,
    //  inspecting old uncompleted protocols in locked nodes)

    return true;
}

/*
 * check if key is used; if so, ask user if they want to overwrite;
 * return true if it's ok
 */
bool ProtocolSwitcher::askProceedIfKeyExists(std::string key) {
    ZDvidReader reader;
    if (reader.open(m_currentDvidTarget)) {
        if (reader.hasKey(QString::fromStdString(PROTOCOL_DATA_NAME), QString::fromStdString(key))) {
            int ans = QMessageBox::question(m_parent, "Key exists",
                QString("There is data already stored at key %1; overwrite?").arg(QString::fromStdString(key)),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            if (ans == QMessageBox::Yes) {
                // user says overwrite
                return true;
            } else {
                return false;
            }
        } else {
            // key doesn't exist
            return true;
        }
    } else {
        // can't open dvid
        return false;
    }
}

/*
 * check that our data instance is present in DVID; if not,
 * create it; as of now, any user is able to create data
 * instances; returns success status
 */
bool ProtocolSwitcher::checkCreateDataInstance() {
    // do in different thread, since we read/write DVID?  or assume it's
    //  (a) quick and (b) necessary or we can't proceed?  let's go with that

    // does it exist?  if not, create
    ZDvidReader reader;
    reader.setVerbose(false);
    if (reader.open(m_currentDvidTarget)) {
        if (!reader.hasData(PROTOCOL_DATA_NAME)) {

            std::cout << "prswi: data name being created" << std::endl;

            ZDvidWriter writer;
            if (writer.open(m_currentDvidTarget)) {
                std::cout << "creating keyvalue " << PROTOCOL_DATA_NAME << std::endl;
                writer.createKeyvalue(PROTOCOL_DATA_NAME);
                // did it actually create?  I'm only going to try once
                if (reader.hasData(PROTOCOL_DATA_NAME)) {
                    return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else {
            return true;
        }
    } else {
        return false;
    }
}

/*
 * returns the key we will use to store the protocol data;
 * returns empty string if user cancels identifier input
 */
std::string ProtocolSwitcher::generateKey() {
    // key = (username)-(protocolname)-(identifier)
    std::string identifier = generateIdentifier();
    if (identifier.empty()) {
        return "";
    } else {
        return NeuTube::GetCurrentUserName() + "-" + m_activeProtocol->getName() + "-" + identifier;
    }
}

/*
 * this method returns an identifier; it's either an
 * assignment ID or something provided by the user;'
 * returns empty string if user cancels
 */
std::string ProtocolSwitcher::generateIdentifier() {

    // if assignment in progress: return assignment ID string
    // not implemented yes

    // else: prompt user for identifier;  no hyphens!  I want to split on them later
    //  also, no spaces, because web reasons)
    bool status;
    QString ans = QInputDialog::getText(m_parent, "Input identifier",
        "Input an identifier to be used as part of the key for saved data (no spaces or hyphens!): ",
        QLineEdit::Normal, "myData", &status);
    if (status && !ans.isEmpty()) {
        if (ans.contains('-') || ans.contains(' ')) {
            QMessageBox::warning(m_parent, "Invalid identifier",
                "The identifier may not contain hyphens or spaces", QMessageBox::Ok);
            return "";
        } else {
            return ans.toStdString();
        }
    } else {
        return "";
    }
}

void ProtocolSwitcher::saveFailedDialog(QString message) {
    QMessageBox::warning(m_parent, "Save failed", message, QMessageBox::Ok);
}

// when a protocol is entered/exited, we need to handle
//  its signals; be sure the next two methods are updated
//  together!  disconnect everything you connect!
void ProtocolSwitcher::connectProtocolSignals() {
    connect(m_activeProtocol, SIGNAL(protocolExiting()), this, SLOT(exitProtocolRequested()));
    connect(m_activeProtocol, SIGNAL(requestSaveProtocol(ZJsonObject)), this, SLOT(saveProtocolRequested(ZJsonObject)));
}

void ProtocolSwitcher::disconnectProtocolSignals() {
    disconnect(m_activeProtocol, SIGNAL(protocolExiting()), this, SLOT(exitProtocolRequested()));
    disconnect(m_activeProtocol, SIGNAL(requestSaveProtocol(ZJsonObject)), this, SLOT(saveProtocolRequested(ZJsonObject)));
}



















