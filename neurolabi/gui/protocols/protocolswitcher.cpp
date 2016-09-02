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
#include "synapsepredictionprotocol.h"

#include "doNthingsprotocol.h"

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
 * see comments in protocoldialog.cpp for how to add new protocol;
 * in this file, you'll add the name to protocolNames, and
 * add the class to if-else chain in instantiateProtocol()
 */
ProtocolSwitcher::ProtocolSwitcher(QWidget *parent) : QObject(parent), m_activeMetadata("", ZDvidTarget())
{
    m_parent = parent;
    m_chooser = new ProtocolChooser(m_parent);

    m_protocolStatus = PROTOCOL_INACTIVE;
    m_activeProtocol = NULL;


    // set up connections to ProtocolChooser
    connect(m_chooser, SIGNAL(requestStartProtocol(QString)),
            this, SLOT(startProtocolRequested(QString)));
    connect(m_chooser, SIGNAL(requestLoadProtocolKey(QString)),
            this, SLOT(loadProtocolKeyRequested(QString)));
    connect(this, SIGNAL(requestDisplaySavedProtocols(QStringList)),
            m_chooser, SLOT(displaySavedProtocolKeys(QStringList)));


}

// name of data instance in DVID; we do not abstract this out the way we
//  do some data names because we really don't think it'll ever change;
//  there is no reason to need parallel instances as you can use
//  different identifiers in the stored keys instead
const std::string ProtocolSwitcher::PROTOCOL_DATA_NAME = "NeuTu-protocols";

// suffix on keys for completed protocols
const std::string ProtocolSwitcher::PROTOCOL_COMPLETE_SUFFIX= "-complete";


// names of available protocols: the protocol switcher is responsible for
//  assigning names to the various protocols and relating the names to
//  the protocol classes; the names are listed here, and the mappings
//  are listed in instantiateProtocol()

// thank you, C++, for making constants so hard to define
QStringList ProtocolSwitcher::protocolNames = QStringList()
        // "doNthings" is a test protocol
        // << "doNthings"
        << "synapse_prediction_body"
        << "synapse_prediction_region";


void ProtocolSwitcher::openProtocolDialogRequested() {
    if (!m_currentDvidTarget.isValid()) {
        return;
    }

    if (m_protocolStatus == PROTOCOL_ACTIVE) {
        // show protocol dialog for the protocol we're loading
        m_activeProtocol->show();
        m_activeProtocol->raise();
    } else if (m_protocolStatus == PROTOCOL_INITIALIZING) {
        // show a message
        QMessageBox msgBox;
        msgBox.setText("A protocol is initializing...");
        msgBox.exec();
    } else {
        // PROTOCOL_INACTIVE: show the protocol chooser; tell it
        //  to display saved keys for this user
        emit requestDisplaySavedProtocols(getUserProtocolKeys(QString::fromStdString(NeuTube::GetCurrentUserName()), false));

        m_chooser->show();
        m_chooser->raise();
    }

}

void ProtocolSwitcher::exitProtocolRequested() {
    m_protocolStatus = PROTOCOL_INACTIVE;

    m_activeMetadata.clearActive();
    m_activeMetadata.write();

    // disconnect dialog signals
    disconnectProtocolSignals();

    // hide now, delete later
    m_activeProtocol->hide();
}

void ProtocolSwitcher::completeProtocolRequested() {
    // when we complete a protocol, we resave the data under a new key;
    //  it's the old one plus the completion suffix

    ZDvidReader reader;
    ZJsonObject data;
    if (reader.open(m_currentDvidTarget)) {
        const QByteArray &rawData = reader.readKeyValue(QString::fromStdString(PROTOCOL_DATA_NAME),
            QString::fromStdString(m_activeMetadata.getActiveProtocolKey()));
        data.decodeString(rawData.data());
    } else {
        warningDialog("Complete failed", "Couldn't read saved data from DVID; completion failed!");
        return;
    }

    ZDvidWriter writer;
    if (writer.open(m_currentDvidTarget)) {
        std::string incompleteKey = m_activeMetadata.getActiveProtocolKey();
        std::string completeKey = m_activeMetadata.getActiveProtocolKey() + PROTOCOL_COMPLETE_SUFFIX;
        writer.writeJson(PROTOCOL_DATA_NAME, completeKey, data);
        writer.deleteKey(QString::fromStdString(PROTOCOL_DATA_NAME), QString::fromStdString(incompleteKey));
    } else {
        warningDialog("Complete failed", "Couldn't write complete data to DVID; completion failed!");
        return;
    }

    // after the resave, it's just like exit:
    exitProtocolRequested();
}

void ProtocolSwitcher::dvidTargetChanged(ZDvidTarget target) {
    m_currentDvidTarget = target;

    // check for active protocol here and start loading
    //  even before user decides to open the dialog
    // in separate thread?
    m_activeMetadata = ProtocolMetadata::ReadProtocolMetadata(PROTOCOL_DATA_NAME, target);
    if (!m_activeMetadata.ioSuccessful()) {
        // error reading metadata; in principle, we should probably do *something*
        //  here, but popping a dialog would be too intrusive
        return;
    }
    if (m_activeMetadata.isActive()) {
        loadProtocolRequested();
    } else {
        m_protocolStatus = PROTOCOL_INACTIVE;
        // nothing else to do here; we can't set up the protocol
        //  chooser because the information about which protocols
        //  can be initiated or loaded can change over time
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
        warningDialog("Save failed", "DVID data instance for protocols was not present and/or could not be created!");
        m_protocolStatus = PROTOCOL_INACTIVE;
        return;
    }

    // generate the save key and be sure it's not in use
    std::string key = generateKey(protocolName);
    if (key.empty()) {
        warningDialog("Save failed", "Key for saved data was not generated!");
        return;
    }
    if (!askProceedIfKeyExists(key)) {
        warningDialog("Save failed", "Key for saved data might already be used!");
        return;
    }
    m_activeMetadata.setActive(protocolName.toStdString(), key);

    // tell user the key, data instance we're using
    QMessageBox::information(m_parent, "Save location",
        QString("Your data will be saved in DVID in data instance %1, key %2")
            .arg(QString::fromStdString(PROTOCOL_DATA_NAME))
            .arg(QString::fromStdString(key)),
        QMessageBox::Ok);


    m_protocolStatus = PROTOCOL_INITIALIZING;

    instantiateProtocol(protocolName);
    if (m_activeProtocol == NULL) {
        // instantiation failed!
        QMessageBox::warning(m_parent, "Protocol not started!",
            "The protocol could not be started!  Please report this error.", QMessageBox::Ok);
        m_protocolStatus = PROTOCOL_INACTIVE;
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

        m_activeMetadata.write();

        m_protocolStatus = PROTOCOL_ACTIVE;
        m_activeProtocol->show();
        m_activeProtocol->raise();

    } else {
        // note that dialog explaining failure is expected to be
        //  raised by the protocol during "initialize()"

        disconnectProtocolSignals();

        m_activeMetadata.clearActive();
        m_protocolStatus = PROTOCOL_INACTIVE;
    }

}

// load a saved protocol from key
void ProtocolSwitcher::loadProtocolKeyRequested(QString protocolKey) {

    // parse protocol name from key; update metadata
    if (protocolKey.count("-") != 2) {
        // problem!
        warningDialog("Load failed", "Badly formed key: " + protocolKey);
        return;
    }

    // middle part is protocol name
    QStringList parts = protocolKey.split("-");
    m_activeMetadata.setActive(parts.at(1).toStdString(), protocolKey.toStdString());
    m_activeMetadata.write();

    // call load save active
    loadProtocolRequested();

}

// load a saved active protocol from metadata info
void ProtocolSwitcher::loadProtocolRequested() {
    // locked dvid node check
    if (!askProceedIfNodeLocked()) {
        return;
    }

    m_protocolStatus = PROTOCOL_LOADING;

    instantiateProtocol(QString::fromStdString(m_activeMetadata.getActiveProtocolName()));
    if (m_activeProtocol == NULL) {
        // instantiation failed!
        QMessageBox::warning(m_parent, "Protocol not started!",
            "The protocol could not be started!  Please report this error.", QMessageBox::Ok);
        m_protocolStatus = PROTOCOL_INACTIVE;
        return;
    }

    connectProtocolSignals();

    // load data from dvid and send to protocol
    ZDvidReader reader;
    ZJsonObject data;
    if (reader.open(m_currentDvidTarget)) {
        const QByteArray &rawData = reader.readKeyValue(QString::fromStdString(PROTOCOL_DATA_NAME),
            QString::fromStdString(m_activeMetadata.getActiveProtocolKey()));
        data.decodeString(rawData.data());
    } else {
        warningDialog("Load failed", "Could not open DVID to read data!");
        return;
    }
#ifdef _DEBUG_
    std::cout << data.dumpString(2) << std::endl;
#endif

    requestLoadProtocol(data);

    // ready to show dialog
    m_protocolStatus = PROTOCOL_ACTIVE;
    m_activeProtocol->show();
    m_activeProtocol->raise();

}

void ProtocolSwitcher::saveProtocolRequested(ZJsonObject data) {

    // check if node still unlocked?

    ZDvidWriter writer;
    if (writer.open(m_currentDvidTarget)) {
        writer.writeJson(PROTOCOL_DATA_NAME, m_activeMetadata.getActiveProtocolKey(), data);
    } else {
        warningDialog("Save failed", "Failed to open DVID for writing");
    }
}

void ProtocolSwitcher::instantiateProtocol(QString protocolName) {
    // note: this is where we map protocol names to classes;
    //  C++ makes this difficult because classes (types) aren't
    //  first-class languages like in higher-level languages, like Python

    // note that if you decide to rename a protocol, you should keep
    //  the old name's mapping present, to handle old saves; if not,
    //  you'd better handle it in some other way (eg, rename, refuse to
    //  open + dialog, whatever)

    // if-else chain not ideal, but C++ is too stupid to switch
    //  on strings; however, the chain won't get *too* long,
    //  so it's not that bad
    if (m_activeProtocol) {
        delete m_activeProtocol;
    }
    if (protocolName == "doNthings") {
        m_activeProtocol = new DoNThingsProtocol(m_parent);
    } else if (protocolName == "synapse_prediction_region") {
        m_activeProtocol =new SynapsePredictionProtocol(m_parent, SynapsePredictionProtocol::VARIATION_REGION);
    } else if (protocolName == "synapse_prediction_body") {
        m_activeProtocol =new SynapsePredictionProtocol(m_parent, SynapsePredictionProtocol::VARIATION_BODY);
    }
    // below here: old protocols (renamed, deleted, etc.)
    // old synapse_prediction is always region:
    else if (protocolName == "synapse_prediction") {
        m_activeProtocol =new SynapsePredictionProtocol(m_parent, SynapsePredictionProtocol::VARIATION_REGION);
    } else {
        // should never happen; the null will cause errors later
        m_activeProtocol = NULL;
    }

    if (m_activeProtocol != NULL) {
      m_activeProtocol->setDvidTarget(m_currentDvidTarget);
    }
}

void ProtocolSwitcher::displayPointRequested(int x, int y, int z) {
    emit requestDisplayPoint(x, y, z);
}

/*
 * read keys from dvid and return keys for a user; flag = whether to include
 * completed protocols or not
 */
QStringList ProtocolSwitcher::getUserProtocolKeys(QString username, bool showComplete) {

    QStringList keyList;

    ZDvidReader reader;
    if (reader.open(m_currentDvidTarget)) {
        // read all keys; filter to current user
        // (not really tested in presence of other users' data yet)
        QStringList keyList = reader.readKeys(QString::fromStdString(PROTOCOL_DATA_NAME));
        keyList = keyList.filter(QRegExp(QString::fromStdString("^" + username.toStdString())));

        // remove metadata key
        keyList.removeAll(QString::fromStdString(ProtocolMetadata::GetUserMetadataKey(username.toStdString())));

        // remove "complete" keys
        if (!showComplete) {
            QRegExp reComplete(QString::fromStdString("*" + PROTOCOL_COMPLETE_SUFFIX));
            reComplete.setPatternSyntax(QRegExp::Wildcard);
            QStringList completeList = keyList.filter(reComplete);
            for (int i=0; i<completeList.size(); i++) {
                keyList.removeAll(completeList.at(i));
            }
        }

        return keyList;
    } else {
        // empty results on error
        return keyList;
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
            ZDvidWriter writer;
            if (writer.open(m_currentDvidTarget)) {
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
std::string ProtocolSwitcher::generateKey(QString protocolName) {
    // key = (username)-(protocolname)-(identifier)
    std::string identifier = generateIdentifier();
    if (identifier.empty()) {
        return "";
    } else {
        return NeuTube::GetCurrentUserName() + "-" + protocolName.toStdString() + "-" + identifier;
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
        QLineEdit::Normal, "", &status);
    if (status && !ans.isEmpty()) {
        if (ans.contains('-') || ans.contains(' ')) {
            QMessageBox::warning(m_parent, "Invalid identifier",
                "The identifier may not contain hyphens or spaces.", QMessageBox::Ok);
            return "";
        } else {
            return ans.toStdString();
        }
    } else {
        return "";
    }
}

void ProtocolSwitcher::warningDialog(QString title, QString message) {
    QMessageBox::warning(m_parent, title, message, QMessageBox::Ok);
}

// when a protocol is entered/exited, we need to handle
//  its signals; be sure the next two methods are updated
//  together!  disconnect everything you connect!
void ProtocolSwitcher::connectProtocolSignals() {
    // lifecycle connects
    connect(m_activeProtocol, SIGNAL(protocolExiting()), this, SLOT(exitProtocolRequested()));
    connect(m_activeProtocol, SIGNAL(protocolCompleting()), this, SLOT(completeProtocolRequested()));
    connect(m_activeProtocol, SIGNAL(requestSaveProtocol(ZJsonObject)), this, SLOT(saveProtocolRequested(ZJsonObject)));
    connect(this, SIGNAL(requestLoadProtocol(ZJsonObject)), m_activeProtocol, SLOT(loadDataRequested(ZJsonObject)));

    // interaction connects
    connect(m_activeProtocol, SIGNAL(requestDisplayPoint(int,int,int)), this, SLOT(displayPointRequested(int,int,int)));
}

void ProtocolSwitcher::disconnectProtocolSignals() {
    // lifecycle connects
    disconnect(m_activeProtocol, SIGNAL(protocolExiting()), this, SLOT(exitProtocolRequested()));
    disconnect(m_activeProtocol, SIGNAL(protocolCompleting()), this, SLOT(completeProtocolRequested()));
    disconnect(m_activeProtocol, SIGNAL(requestSaveProtocol(ZJsonObject)), this, SLOT(saveProtocolRequested(ZJsonObject)));
    disconnect(this, SIGNAL(requestLoadProtocol(ZJsonObject)), m_activeProtocol, SLOT(loadDataRequested(ZJsonObject)));

    // interaction connects
    disconnect(m_activeProtocol, SIGNAL(requestDisplayPoint(int,int,int)), this, SLOT(displayPointRequested(int,int,int)));
}

void ProtocolSwitcher::processSynapseVerification(int x, int y, int z, bool verified)
{
  if (m_activeProtocol != NULL) {
    m_activeProtocol->processSynapseVerification(x, y, z, verified);
  }
}

void ProtocolSwitcher::processSynapseMoving(
    const ZIntPoint &from, const ZIntPoint &to)
{
  if (m_activeProtocol != NULL) {
    m_activeProtocol->processSynapseMoving(from, to);
  }
}

















