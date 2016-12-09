#include "focusedpathprotocol.h"
#include "ui_focusedpathprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QtGui>
#include <QInputDialog>
#include <QMessageBox>

#include "focusedpathbodyinputdialog.h"
#include "flyem/zflyembookmark.h"

#include "neutube_def.h"
#include "dvid/zdvidreader.h"
#include "zjsonobject.h"
#include "zjsonparser.h"

FocusedPathProtocol::FocusedPathProtocol(QWidget *parent, std::string variation) :
    ProtocolDialog(parent),
    ui(new Ui::FocusedPathProtocol)
{
    m_variation = variation;
    m_parent = parent;

    ui->setupUi(this);

    // table setup
    m_edgeModel = new QStandardItemModel(0, 3, ui->edgesTableView);


    // data load connections
    connect(this, SIGNAL(bodyListLoaded()), this, SLOT(onBodyListsLoaded()));
    connect(this, SIGNAL(currentBodyPathsLoaded()), this, SLOT(onCurrentBodyPathsLoaded()));


    // UI connections
    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));


    // misc UI setup
    ui->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

}

// ---------- constants ----------
// protocol variations
const std::string FocusedPathProtocol::VARIATION_BODY = "body";
const std::string FocusedPathProtocol::VARIATION_BOOKMARK = "bookmark";

// keys (etc) used in loading/saving protocol data:
const int FocusedPathProtocol::m_fileVersion = 1;
const std::string FocusedPathProtocol::KEY_VERSION = "version";
const std::string FocusedPathProtocol::KEY_VARIATION = "variation";
const std::string FocusedPathProtocol::KEY_BODYID = "bodyID";
const std::string FocusedPathProtocol::KEY_EDGE_INSTANCE = "edge-instance";

// keys used when reading stuff from DVID
const std::string FocusedPathProtocol::KEY_ASSIGNMENT_BODIES= "bodies";
const std::string FocusedPathProtocol::KEY_ASSIGNMENT_INSTANCE= "edgedata";
const std::string FocusedPathProtocol::TAG_PATH= "path";
const std::string FocusedPathProtocol::TAG_EDGE= "edge";
const std::string FocusedPathProtocol::PROPERTY_PROBABILITY= "probability";
const std::string FocusedPathProtocol::PROPERTY_PATH= "path";


bool FocusedPathProtocol::initialize() {

    if (m_variation == VARIATION_BODY) {
        // get input from user via dialog
        FocusedPathBodyInputDialog inputDialog;
        int ans = inputDialog.exec();
        if (ans == QDialog::Rejected) {
            return false;
        }
        // edge instance is validated below
        m_edgeDataInstance = inputDialog.getEdgeInstance();

        // validate that the body ID exists:
        uint64_t bodyID = inputDialog.getBodyID();
        if (!m_reader.hasBody(bodyID)) {
            QMessageBox::warning(m_parent, "Body ID doesn't exist!",
                QString("The entered body ID %1 doesn't seem to exist!").arg(bodyID),
                QMessageBox::Ok);
            return false;
        }
        m_bodies.append(bodyID);


    } else if (m_variation == VARIATION_BOOKMARK) {

        // read appropriate bookmarks for this user; the bodies they
        //  are on are the bodies to proofread

        QMessageBox mb;
        mb.setText("Not implemented!");
        mb.setInformativeText("Not implemented--dummy bodies added to list!");
        mb.setStandardButtons(QMessageBox::Ok);
        mb.setDefaultButton(QMessageBox::Ok);
        mb.exec();


        loadBodiesFromBookmarks();


    } else {
        variationError(m_variation);
        return false;
    }


    // validate edge instance exists
    if (!m_reader.hasData(m_edgeDataInstance)) {
        QMessageBox::warning(m_parent, "Bad instance name!",
            "Edge data instance" + QString::fromStdString(m_edgeDataInstance) + " does not seem to exist in DVID.",
            QMessageBox::Ok);
        return false;
    }

    // everything OK; save and return
    saveState();
    return true;
}

void FocusedPathProtocol::setDvidTarget(ZDvidTarget target) {
    ProtocolDialog::setDvidTarget(target);

    m_reader.setVerbose(false);
    if (!m_reader.open(m_dvidTarget)) {
        QMessageBox::warning(m_parent, "Can't connect to DVID",
            "There was a problem connecting to the DVID server!", QMessageBox::Ok);
        return;
    }
}

void FocusedPathProtocol::onExitButton() {
    // exit protocol; can be reopened and worked on later
    emit protocolExiting();
}

void FocusedPathProtocol::onCompleteButton() {
    QMessageBox mb;
    mb.setText("Complete protocol");
    mb.setInformativeText("When you complete the protocol, it will save and exit immediately.  You will not be able to reopen it.\n\nComplete protocol now?");
    mb.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Cancel);
    int ans = mb.exec();
    if (ans == QMessageBox::Ok) {
        saveState();
        emit protocolCompleting();
    }
}

void FocusedPathProtocol::loadDataRequested(ZJsonObject data) {

    // check version of saved data here, once we have a second version
    if (!data.hasKey(KEY_VERSION.c_str())) {
        QMessageBox::warning(m_parent, "No version!",
            "No version info in saved data; data not loaded!",
            QMessageBox::Ok);
        return;
    }
    int version = ZJsonParser::integerValue(data[KEY_VERSION.c_str()]);
    if (version > m_fileVersion) {
        QMessageBox::warning(m_parent, "Old version!",
            "Saved data is from a newer version of NeuTu; update NeuTu and try again!",
            QMessageBox::Ok);
        return;
    }

    // convert to newer version here if needed

    // do actual load
    m_bodies.clear();
    m_edgeDataInstance = ZJsonParser::stringValue(data[KEY_EDGE_INSTANCE.c_str()]);

    if (m_variation == VARIATION_BODY) {
        m_bodies.append(ZJsonParser::integerValue(data[KEY_BODYID.c_str()]));
        emit bodyListLoaded();
    } else if (m_variation == VARIATION_BOOKMARK) {
        // we don't load anything out of the data, but we do
        //  load data from DVID at this point:
        loadBodiesFromBookmarks();
    } else {
        variationError(m_variation);
    }
}

void FocusedPathProtocol::loadBodiesFromBookmarks() {

    // look for bookmarks of the appropriate type for this
    //  user; the bodies under those bookmarks are the bodies we want

    // identify the focused bookmark; look for "edgedata" in props;
    // take the first one you find
    ZJsonArray bookmarks = m_reader.readTaggedBookmark("user:" + NeuTube::GetCurrentUserName());
    bool found = false;
    ZFlyEmBookmark bookmark;
    for (size_t i=0; i<bookmarks.size(); i++) {
         bookmark.loadDvidAnnotation(bookmarks.value(i));
         if (bookmark.getPropertyJson().hasKey(KEY_ASSIGNMENT_INSTANCE.c_str())) {
             found = true;
             break;
         }
    }

    // for testing: skip the parsing that can'at happen yet and
    //  put some dummy values in
    /*
    if (!found) {
        // we'll deal with the error in the calling routine
        return;
    }

    // get the values out of the bookmark
    m_edgeDataInstance = ZJsonParser::stringValue(bookmark.getPropertyJson()[(KEY_ASSIGNMENT_INSTANCE.c_str())]);
    ZJsonArray bodies = ((ZJsonArray) bookmark.getPropertyJson().value(KEY_ASSIGNMENT_BODIES.c_str()));
    for (size_t i=0; i<bodies.size(); i++) {
        m_bodies.append(ZJsonParser::integerValue(bodies.at(i)));
    }
    */


    // debug
    std::cout << "edge data instance = " << m_edgeDataInstance << std::endl;
    std::cout << "# bodies loaded = " << m_bodies.size() << std::endl;



    // dummy values for testing:
    m_bodies.clear();
    m_bodies.append(2345);
    m_bodies.append(3456);


    // this isn't an edge data instance, but it does exist,
    //  thus keeping everything happy
    m_edgeDataInstance = "labels";


    emit bodyListLoaded();
}

void FocusedPathProtocol::loadCurrentBodyPaths(uint64_t bodyID) {

    m_currentBodyPaths.clear();

    ZJsonArray annotations = m_reader.readAnnotation(m_edgeDataInstance, bodyID);
    for (size_t i=0; i<annotations.size(); i++) {
        ZDvidAnnotation ann;
        ann.loadJsonObject(annotations.value(i), FlyEM::LOAD_PARTNER_LOCATION);
        if (ann.hasTag(TAG_PATH)) {
            m_currentBodyPaths << FocusedPath(ann);
        }
    }

    emit currentBodyPathsLoaded();
}

/*
 * body list is available; set first body to current body, and
 * read paths for it
 */
void FocusedPathProtocol::onBodyListsLoaded() {
    if (m_bodies.size() == 0) {
        QMessageBox::warning(m_parent, "No bodies!",
            "No bodies were loaded!", QMessageBox::Ok);
        return;
    }
    m_currentBody = m_bodies.first();
    // this could be started in a thread if it turns out to be slow
    loadCurrentBodyPaths(m_currentBody);
}

void FocusedPathProtocol::onCurrentBodyPathsLoaded() {


    std::cout << "onCurrentBodyPathsLoaded()" << std::endl;


    // sort paths by other endpoint body ID
    // -- if any paths already linked: discard

    // for each other body ID:

    // sort paths to that body ID by probability

    // for each path:

    // check prob > 0

    // read all edges in path

    // determine if any edges already broken

    // if so, set path prob = 0 (save)

    // load edges into UI and update labels





}

void FocusedPathProtocol::saveState() {

    ZJsonObject data;

    // always version your output files!
    data.setEntry(KEY_VERSION.c_str(), m_fileVersion);

    data.setEntry(KEY_EDGE_INSTANCE.c_str(), m_edgeDataInstance);

    if (m_variation == VARIATION_BODY) {
        // in this variation, there's only one body ID; save it
        data.setEntry(KEY_BODYID.c_str(), m_bodies.first());
    } else if (m_variation == VARIATION_BOOKMARK) {
        // nothing saved right now; we examine DVID for the data
    } else {
        variationError(m_variation);
    }

    emit requestSaveProtocol(data);
}

void FocusedPathProtocol::variationError(std::string variation) {
    QMessageBox mb;
    mb.setText("Unknown protocol variation!");
    mb.setInformativeText("Unknown protocol variation " + QString::fromStdString(variation) + " was encountered!  Please report this error!");
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Ok);
    mb.exec();
}

FocusedPathProtocol::~FocusedPathProtocol()
{
    delete ui;
}
