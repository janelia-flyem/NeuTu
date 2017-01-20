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
#include "dvid/zdvidwriter.h"
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
    ui->edgesTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->edgesTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    // set headers?
    ui->edgesTableView->setModel(m_edgeModel);


    // data load connections
    connect(this, SIGNAL(bodyListLoaded()), this, SLOT(onBodyListsLoaded()));
    connect(this, SIGNAL(currentBodyPathsLoaded()), this, SLOT(onCurrentBodyPathsLoaded()));


    // UI connections
    // buttons
    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));

    // tables
    connect(ui->edgesTableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
        this, SLOT(onEdgeSelectionChanged(QItemSelection,QItemSelection)));


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
const std::string FocusedPathProtocol::KEY_PATH_INSTANCE = "path-instance";

// keys used when reading stuff from DVID
const std::string FocusedPathProtocol::KEY_ASSIGNMENT_BODIES = "bodies";
const std::string FocusedPathProtocol::KEY_ASSIGNMENT_EDGE_INSTANCE = "edgedata";
const std::string FocusedPathProtocol::KEY_ASSIGNMENT_PATH_INSTANCE = "pathdata";
const std::string FocusedPathProtocol::TAG_PATH = "path";
const std::string FocusedPathProtocol::TAG_EDGE = "edge";
const std::string FocusedPathProtocol::PROPERTY_PROBABILITY = "probability";
const std::string FocusedPathProtocol::PROPERTY_PATH = "path";

// colors
const QColor FocusedPathProtocol::COLOR_BODY1 = QColor(0, 0, 255);
const QColor FocusedPathProtocol::COLOR_BODY2 = QColor(0, 128, 64);
const QColor FocusedPathProtocol::COLOR_EDGE1 = QColor(128, 0, 255);
const QColor FocusedPathProtocol::COLOR_EDGE2 = QColor(0, 255, 0);
const QColor FocusedPathProtocol::COLOR_PATH = QColor(100, 200, 255);
const QColor FocusedPathProtocol::COLOR_OTHER = QColor(128, 0, 64);
const QColor FocusedPathProtocol::COLOR_DEFAULT = QColor(0, 0, 0);

bool FocusedPathProtocol::initialize() {

    if (m_variation == VARIATION_BODY) {
        // get input from user via dialog
        FocusedPathBodyInputDialog inputDialog;
        int ans = inputDialog.exec();
        if (ans == QDialog::Rejected) {
            return false;
        }
        // instances are validated below
        m_edgeDataInstance = inputDialog.getEdgeInstance();
        m_pathDataInstance = inputDialog.getPathInstance();

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


    // validate instances exist
    if (!m_reader.hasData(m_edgeDataInstance)) {
        QMessageBox::warning(m_parent, "Bad edge data instance name!",
            "Edge data instance" + QString::fromStdString(m_edgeDataInstance) + " does not seem to exist in DVID.",
            QMessageBox::Ok);
        return false;
    }
    if (!m_reader.hasData(m_pathDataInstance)) {
        QMessageBox::warning(m_parent, "Bad path data instance name!",
            "Path data instance" + QString::fromStdString(m_pathDataInstance) + " does not seem to exist in DVID.",
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

    // I think this is the only UI cleanup we need
    emit requestDeactivateColorMap();

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
    m_pathDataInstance = ZJsonParser::stringValue(data[KEY_PATH_INSTANCE.c_str()]);

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
         if (bookmark.getPropertyJson().hasKey(KEY_ASSIGNMENT_EDGE_INSTANCE.c_str())) {
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
    m_edgeDataInstance = ZJsonParser::stringValue(bookmark.getPropertyJson()[(KEY_ASSIGNMENT_EDGE_INSTANCE.c_str())]);
    m_pathDataInstance = ZJsonParser::stringValue(bookmark.getPropertyJson()[(KEY_ASSIGNMENT_PATH_INSTANCE.c_str())]);
    ZJsonArray bodies = ((ZJsonArray) bookmark.getPropertyJson().value(KEY_ASSIGNMENT_BODIES.c_str()));
    for (size_t i=0; i<bodies.size(); i++) {
        m_bodies.append(ZJsonParser::integerValue(bodies.at(i)));
    }
    */


    // debug
    std::cout << "edge data instance = " << m_edgeDataInstance << std::endl;
    std::cout << "path data instance = " << m_pathDataInstance << std::endl;
    std::cout << "# bodies loaded = " << m_bodies.size() << std::endl;



    // dummy values for testing:
    m_bodies.clear();
    m_bodies.append(2345);
    m_bodies.append(3456);


    // this isn't an edge data instance, but it does exist,
    //  thus keeping everything happy
    m_edgeDataInstance = "focused_edges";
    m_pathDataInstance = "focused_paths";


    emit bodyListLoaded();
}

void FocusedPathProtocol::loadCurrentBodyPaths(uint64_t bodyID) {

    m_currentBodyPaths.clear();

    ZJsonArray annotations = m_reader.readAnnotation(m_pathDataInstance, bodyID,
        FlyEM::LOAD_PARTNER_LOCATION);

    // debug
    // std::cout << "loadCurrentBodyPaths(): annotations json = " << annotations.dumpString() << std::endl;

    for (size_t i=0; i<annotations.size(); i++) {
        ZDvidAnnotation ann;
        ann.loadJsonObject(annotations.value(i), FlyEM::LOAD_PARTNER_RELJSON);
        m_currentBodyPaths << FocusedPath(ann);
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

    std::cout << "onCurrentBodyPathsLoaded" << std::endl;

    m_currentPath = findNextPath();
    m_currentPath.loadEdges(m_reader, m_edgeDataInstance);
    while (m_currentPath.isConnected()) {
        // if a path is connected, it shouldn't be in our
        //  list, nor should it be in DVID
        m_currentBodyPaths.removeOne(m_currentPath);
        deletePath(m_currentPath);
        m_currentPath = findNextPath();
        m_currentPath.loadEdges(m_reader, m_edgeDataInstance);
    }

    displayCurrentPath();
}

FocusedPath FocusedPathProtocol::findNextPath() {

    // load the body IDs at all endpoints
    std::vector<ZIntPoint> points;
    foreach(FocusedPath path, m_currentBodyPaths) {
        points.push_back(path.getFirstPoint());
        points.push_back(path.getLastPoint());
    }
    std::vector<uint64_t> bodyIDs = m_reader.readBodyIdAt(points);

    m_currentPathBodyIDs.clear();
    for (size_t i=0; i<points.size(); i++) {
        m_currentPathBodyIDs[points[i]] = bodyIDs[i];
    }

    // candidate path; get its endpoint bodyID and
    //  see if there are any other paths to that ID
    //  that have higher probability (yes, we check
    //  against itself first time through loop; it's
    //  just easier that way
    FocusedPath path = m_currentBodyPaths.first();
    foreach(FocusedPath path2, m_currentBodyPaths) {
        if (m_currentPathBodyIDs[path.getLastPoint()] == m_currentPathBodyIDs[path2.getLastPoint()] &&
            path2.getProbability() > path.getProbability()) {
            path = path2;
        }
    }

    return path;
}

void FocusedPathProtocol::deletePath(FocusedPath path) {
    ZDvidWriter writer;
    if (writer.open(m_dvidTarget)) {
        writer.deletePointAnnotation(m_pathDataInstance, path.getFirstPoint());
        writer.deletePointAnnotation(m_pathDataInstance, path.getLastPoint());
    }
}

void FocusedPathProtocol::displayCurrentPath() {
    // edges already loaded in path, and  path known to be unconnected;
    // load edges into UI and update labels

    // load data into model
    m_edgeModel->clear();
    // reset headers?

    m_edgeModel->setRowCount(m_currentPath.getNumEdges());
    for (int i=0; i<m_currentPath.getNumEdges(); i++) {
        FocusedEdge edge = m_currentPath.getEdge(i);

        QStandardItem * bodyID1Item = new QStandardItem();
        bodyID1Item->setData(QVariant(edge.getFirstBodyID()), Qt::DisplayRole);
        m_edgeModel->setItem(i, BODYID1_COLUMN, bodyID1Item);

        QStandardItem * bodyID2Item = new QStandardItem();
        bodyID2Item->setData(QVariant(edge.getLastBodyID()), Qt::DisplayRole);
        m_edgeModel->setItem(i, BODYID2_COLUMN, bodyID2Item);

        // connection status in text form, eg, --X-- or --?--
        QStandardItem * connectionItem = new QStandardItem();
        connectionItem->setData(QVariant(QString::fromStdString(edge.getConnectionTextIcon())), Qt::DisplayRole);
        m_edgeModel->setItem(i, CONNECTION_COLUMN, connectionItem);
    }

    // top label: overall connection, body IDs
    updateConnectionLabel();

    // bottom label: edges, paths, bodies
    updateProgressLabel();


    // set/go to first unknown edge
    int index = m_currentPath.getFirstUnexaminedEdgeIndex();
    if (index >= 0) {


        // goto edge (index)
        // which will call go to edge point


        updateColorMap();
        emit requestActivateColorMap();

    }



}

void FocusedPathProtocol::updateConnectionLabel() {
    std::ostringstream outputStream;
    outputStream << m_currentPathBodyIDs[m_currentPath.getFirstPoint()];
    outputStream << " " << m_currentPath.getConnectionTextIcon() << " ";
    outputStream << m_currentPathBodyIDs[m_currentPath.getLastPoint()];
    ui->connectionLabel->setText(QString::fromStdString(outputStream.str()));
}

void FocusedPathProtocol::updateProgressLabel() {

    std::ostringstream outputStream;

    // probably something like this:
    // Progress: 1/3 edges; 1/17 (6%) paths; 1/3 (%) bodies

    // that's going to be a pain to track...defer for now

    // edges:
    // (m_currentPath.getNumEdges() - m_currentPath.getNumUnexaminedEdges()) / m_currentPath.getNumEdges()

    // paths:


    // bodies:


    outputStream << "Progress: (coming soon)";


    ui->progressLabel->setText(QString::fromStdString(outputStream.str()));

}

void FocusedPathProtocol::updateColorMap() {

    std::cout << "updateColorMap()" << std::endl;

    // rebuild from scratch
    m_colorScheme.clear();
    m_colorScheme.setDefaultColor(COLOR_DEFAULT);

    // bodies along the path; second body in each edge, except for the last edge;
    //  do this first, so the end body colors applied later will win out if need be
    for (int i=0; i<m_currentPath.getNumEdges()-1; i++) {
        m_colorScheme.setBodyColor(m_currentPath.getEdge(i).getLastBodyID(), COLOR_PATH);
    }

    // color endpoint bodies
    m_colorScheme.setBodyColor(m_currentPathBodyIDs[m_currentPath.getFirstPoint()], COLOR_BODY1);
    m_colorScheme.setBodyColor(m_currentPathBodyIDs[m_currentPath.getLastPoint()], COLOR_BODY2);

    // bodies for this particular edge
    // these bodies could be the first or last body, remember, and that's OK
    if (ui->edgesTableView->selectionModel()->hasSelection()) {
        FocusedEdge edge = getSelectedEdge();
        m_colorScheme.setBodyColor(edge.getFirstBodyID(), COLOR_EDGE1);
        m_colorScheme.setBodyColor(edge.getLastBodyID(), COLOR_EDGE2);
        }

    // Steve wants other "interesting" bodies to be colored, not
    //  yet defined (probably other anchor bodies, or named bodies?)
    // m_colorScheme.setBodyColor(   body ID for some other body    , COLOR_OTHER);


    m_colorScheme.buildColorTable();
    emit requestColorMapChange(m_colorScheme);
}

void FocusedPathProtocol::onEdgeSelectionChanged(QItemSelection oldItem, QItemSelection newItem) {

    std::cout << "onEdgeSelectionChanged()" << std::endl;

    // go to new edge point
    gotoEdgePoint(m_currentPath.getEdge(newItem.indexes().first().row()));

    // update color map (it depends on edge, not just path)
    updateColorMap();

}

FocusedEdge FocusedPathProtocol::getSelectedEdge() {
    if (ui->edgesTableView->selectionModel()->hasSelection()) {
        QItemSelection selection = ui->edgesTableView->selectionModel()->selection();
        int index = selection.indexes().first().row();
        return m_currentPath.getEdge(index);
    } else {
        return FocusedEdge();
    }
}

void FocusedPathProtocol::gotoEdgePoint(FocusedEdge edge) {
    // consider the ege point to be halway between the two points
    //  that define the edge
    ZIntPoint point = (edge.getFirstPoint() + edge.getLastPoint()) / 2.0;
    emit requestDisplayPoint(point.getX(), point.getY(), point.getZ());
}

void FocusedPathProtocol::saveState() {

    ZJsonObject data;

    // always version your output files!
    data.setEntry(KEY_VERSION.c_str(), m_fileVersion);

    data.setEntry(KEY_EDGE_INSTANCE.c_str(), m_edgeDataInstance);
    data.setEntry(KEY_PATH_INSTANCE.c_str(), m_pathDataInstance);

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
