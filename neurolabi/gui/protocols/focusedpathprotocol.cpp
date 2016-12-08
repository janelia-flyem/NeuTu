#include "focusedpathprotocol.h"
#include "ui_focusedpathprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QtGui>
#include <QInputDialog>
#include <QMessageBox>

#include "focusedpathbodyinputdialog.h"

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



    // UI connections


    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));


    // misc UI setup
    ui->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

}

const std::string FocusedPathProtocol::KEY_VERSION = "version";
const std::string FocusedPathProtocol::KEY_VARIATION = "variation";
const std::string FocusedPathProtocol::KEY_BODYID = "bodyID";
const std::string FocusedPathProtocol::KEY_EDGE_INSTANCE = "edge-instance";
const std::string FocusedPathProtocol::VARIATION_BODY = "body";
const std::string FocusedPathProtocol::VARIATION_BOOKMARK = "bookmark";
const int FocusedPathProtocol::m_fileVersion = 1;

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
            QMessageBox mb;
            mb.setText("Body ID doesn't exist!");
            mb.setInformativeText("The entered body ID " +  QString::number(bodyID) + " doesn't seem to exist!");
            mb.setStandardButtons(QMessageBox::Ok);
            mb.setDefaultButton(QMessageBox::Ok);
            mb.exec();
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

        // display error in UI; for now, print it
        std::cout << "No version info in saved data; data not loaded!" << std::endl;
        return;
    }
    int version = ZJsonParser::integerValue(data[KEY_VERSION.c_str()]);
    if (version > m_fileVersion) {

        // likewise...
        std::cout << "Saved data is from a newer version of NeuTu; update NeuTu and try again!" << std::endl;



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

    // maybe do in a thread?



    // dummy values for testing:
    m_bodies.append(2345);
    m_bodies.append(3456);


    // edge data instance:
    m_edgeDataInstance = "fake edge data instance name";


    emit bodyListLoaded();
}

void FocusedPathProtocol::loadCurrentBodyPaths(uint64_t bodyID) {

    m_currentBodyPaths.clear();



    // -- no DVID API for get annotations on body with tag, so
    //      probably need to get from body and filter by hand by tag



}

void FocusedPathProtocol::onBodyListsLoaded() {

    // debug
    std::cout << m_bodies.size() << " bodies loaded" << std::endl;
    std::cout << "first body ID = " << m_bodies.first() << std::endl;


    // body list is available; load data into the UI

    // read all paths from current body
    m_currentBody = m_bodies.first();
    loadCurrentBodyPaths(m_currentBody);




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
