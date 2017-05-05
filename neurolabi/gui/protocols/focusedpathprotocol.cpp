#include "focusedpathprotocol.h"
#include "ui_focusedpathprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QtGui>
#include <QInputDialog>
#include <QMessageBox>

#include "neutube.h"

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


    // UI connections
    // buttons
    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));
    connect(ui->skipPathButton, SIGNAL(clicked(bool)), this, SLOT(onSkipPathButton()));
    connect(ui->dontMergeButton, SIGNAL(clicked(bool)), this, SLOT(onDontMergeButton()));
    connect(ui->finishButton, SIGNAL(clicked(bool)), this, SLOT(onFinishPathButton()));

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
const std::string FocusedPathProtocol::KEY_POINT_INSTANCE = "point-instance";

// keys used when reading stuff from DVID
const std::string FocusedPathProtocol::KEY_ASSIGNMENT_BODIES = "bodies";
const std::string FocusedPathProtocol::KEY_ASSIGNMENT_EDGE_INSTANCE = "edgedata";
const std::string FocusedPathProtocol::KEY_ASSIGNMENT_PATH_INSTANCE = "pathdata";
const std::string FocusedPathProtocol::KEY_ASSIGNMENT_POINT_INSTANCE = "pointdata";
const std::string FocusedPathProtocol::TAG_PATH = "path";
const std::string FocusedPathProtocol::TAG_EDGE = "edge";
const std::string FocusedPathProtocol::PROPERTY_EDGES = "edges";
const std::string FocusedPathProtocol::PROPERTY_PATHS = "paths";

// colors
const QColor FocusedPathProtocol::COLOR_BODY1 = QColor(0, 0, 255);      // blue
const QColor FocusedPathProtocol::COLOR_BODY2 = QColor(0, 128, 64);     // moss
const QColor FocusedPathProtocol::COLOR_EDGE1 = QColor(128, 0, 255);    // grape
const QColor FocusedPathProtocol::COLOR_EDGE2 = QColor(0, 255, 0);      // green
const QColor FocusedPathProtocol::COLOR_PATH = QColor(100, 200, 255);   // sky
const QColor FocusedPathProtocol::COLOR_OTHER = QColor(128, 0, 64);     // brick
const QColor FocusedPathProtocol::COLOR_DEFAULT = QColor(0, 0, 0);      // none

// other
// best I can do...probably safe...
const uint64_t FocusedPathProtocol::INVALID_BODY = UINT64_MAX;

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
        m_pointDataInstance = inputDialog.getPointInstance();

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
        // read assignment bookmark that contains the body list
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
    if (!m_reader.hasData(m_pointDataInstance)) {
        QMessageBox::warning(m_parent, "Bad point data instance name!",
            "Path data instance" + QString::fromStdString(m_pointDataInstance) + " does not seem to exist in DVID.",
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

void FocusedPathProtocol::onSkipPathButton() {
    m_currentBodyPaths.removeAll(m_currentPath);
    m_bodySkippedPathCount[m_currentBody] += 1;

    if (!loadNextPath()) {
        loadNextBodyAndPath();
    }
}

void FocusedPathProtocol::onDontMergeButton() {

    std::cout << "onDontMergeButton()" << std::endl;

    // user has decided that the selected edge is not connected;
    //  update edge and write it out
    FocusedEdge selectedEdge = getSelectedEdge();
    selectedEdge.setExaminer(NeuTube::GetCurrentUserName());
    selectedEdge.setTimeExaminedNow();
    ZDvidWriter writer;
    if (writer.open(m_dvidTarget)) {
        selectedEdge.writeEdge(writer, m_edgeDataInstance);
    }

    displayCurrentPath();
}

void FocusedPathProtocol::onFinishPathButton() {

    std::cout << "onFinishPathButton()" << std::endl;

    // user wants to finalize this path and move to the next;
    // refresh body IDs and check that end criteria is met
    //  (at least one broken or all connected (else dialog))
    m_currentPath.updateBodyIDs(getFlyEmProofDoc());
    if (!m_currentPath.isConnected() && !m_currentPath.anyBrokenEdges()) {
        QMessageBox::warning(m_parent, "Path isn't finished!",
            "The path must be connected, or one edge must be marked broken, before finishing path!",
            QMessageBox::Ok);
        return;
    }

    // can we disable finish button until criteria is met?  would need
    //  to run the above check after all merges/splits

    // don't need to write edge information here; should already be up to date

    // remove path from current path list; delete path
    //  other competing paths: leave as-is (they will be removed when they come up next)
    m_currentBodyPaths.removeOne(m_currentPath);
    deletePath(m_currentPath);


    // try to load next path for body
    bool status = loadNextPath();
    if (!status) {
        // no more paths on this body; if there aren't any skips,
        //  mark the body as done; either way, move to next body
        //  and path
        if (m_bodySkippedPathCount[m_currentBody] == 0) {
            m_bodyDone[m_currentBody] = true;
        }
        loadNextBodyAndPath();
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
    m_pointDataInstance= ZJsonParser::stringValue(data[KEY_POINT_INSTANCE.c_str()]);

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
    // list of bodies to review are encoded in a specific user bookmark;
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

    if (!found) {
        QMessageBox::warning(m_parent, "No bodies!",
            "Error retrieving bodies from assignment bookmark!", QMessageBox::Ok);
        return;
    }

    // get the values out of the bookmark
    m_edgeDataInstance = ZJsonParser::stringValue(bookmark.getPropertyJson()[(KEY_ASSIGNMENT_EDGE_INSTANCE.c_str())]);
    m_pathDataInstance = ZJsonParser::stringValue(bookmark.getPropertyJson()[(KEY_ASSIGNMENT_PATH_INSTANCE.c_str())]);
    m_pointDataInstance = ZJsonParser::stringValue(bookmark.getPropertyJson()[(KEY_ASSIGNMENT_POINT_INSTANCE.c_str())]);

    std::string bodyListString = ZJsonParser::stringValue(bookmark.getPropertyJson()[(KEY_ASSIGNMENT_BODIES.c_str())]);
    ZJsonArray bodies;
    bodies.decodeString(bodyListString.c_str());

    for (size_t i=0; i<bodies.size(); i++) {
        m_bodies.append(ZJsonParser::integerValue(bodies.at(i)));
    }

    emit bodyListLoaded();
}

void FocusedPathProtocol::loadCurrentBodyPaths() {

    m_currentBodyPaths.clear();

    // each point annotation has lists of path and edge IDs originating from its position
    ZJsonArray annotations = m_reader.readAnnotation(m_pointDataInstance, m_currentBody,
        FlyEM::LOAD_NO_PARTNER);

    for (size_t i=0; i<annotations.size(); i++) {

        // get path list at each point (if it exists) and parse it
        ZDvidAnnotation ann;
        ann.loadJsonObject(annotations.value(i), FlyEM::LOAD_NO_PARTNER);
        std::string pathListString = ann.getProperty<std::string>(FocusedPathProtocol::PROPERTY_PATHS);

        // it's easiest to parse that list in Qt; strip off the [], split on comma,
        //  then strip whitespace
        QString tempString = QString::fromStdString(pathListString);
        QStringList pathIDList = tempString.mid(1, tempString.size() - 2).split(",");
        foreach (QString pathID, pathIDList) {
            const QByteArray &temp = m_reader.readKeyValue(QString::fromStdString(m_pathDataInstance), pathID.trimmed());
            ZJsonObject pathData;
            pathData.decodeString(temp.data());

            m_currentBodyPaths << FocusedPath(pathID.trimmed().toStdString(), pathData);
        }
    }
}

std::string FocusedPathProtocol::getPropertyKey(std::string prefix, ZIntPoint point) {
    std::ostringstream outputStream;
    outputStream << prefix;
    outputStream << "_";
    outputStream << point.getX();
    outputStream << "_";
    outputStream << point.getY();
    outputStream << "_";
    outputStream << point.getZ();
    return outputStream.str();
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
    // initialize state before loading body and path
    m_currentBody = INVALID_BODY;
    m_bodyDone.clear();
    m_bodySkippedPathCount.clear();
    foreach (uint64_t bodyID, m_bodies) {
        m_bodyDone[bodyID] = false;
        m_bodySkippedPathCount[bodyID] = 0;
    }

    loadNextBodyAndPath();
}

void FocusedPathProtocol::loadNextBodyAndPath() {
    bool pathLoaded = false;
    while(!allBodiesDone() && !pathLoaded) {

        m_currentBody = getNextBody();
        if (m_bodyDone[m_currentBody]) {
            continue;
        }

        if (loadFirstPath()) {
            pathLoaded = true;
            break;
        } else {
            m_bodyDone[m_currentBody] = true;
            // (not needed but intended)
            // continue;
        }
    }

    if (!pathLoaded) {


        // if we arrive here, we're done


        std::cout << "all bodies done" << std::endl;

        // maybe split this off into a slot that we can signal?  but not
        //  sure you can get here any other way
        QMessageBox mb;
        mb.setText("Done!");
        mb.setInformativeText("All paths have been reviewed!  The protocol may now be completed.");
        mb.setStandardButtons(QMessageBox::Ok);
        mb.setDefaultButton(QMessageBox::Ok);
        mb.exec();


        // clear the edge display UI and update the progress UI



    }
}

bool FocusedPathProtocol::allBodiesDone() {
    foreach(bool status, m_bodyDone) {
        if (!status) {
            return false;
        }
    }
    return true;
}

uint64_t FocusedPathProtocol::getNextBody() {
    if (m_currentBody == INVALID_BODY || m_currentBody == m_bodies.last()) {
        return m_bodies.first();
    } else {
        return m_bodies[m_bodies.indexOf(m_currentBody) + 1];
    }
}

bool FocusedPathProtocol::loadFirstPath() {
    // this is the "start over" point for the current body;
    //  load all paths, load bodyIDs at path endpoints,
    //  reset skips, get next (first) path
    loadCurrentBodyPaths();
    if (m_currentBodyPaths.size() == 0) {
        m_bodyDone[m_currentBody] = true;
        return false;
    }

    // load the body IDs at all endpoints of all paths;
    //  save for later reuse
    std::vector<ZIntPoint> points;
    foreach(FocusedPath path, m_currentBodyPaths) {
        points.push_back(path.getFirstPoint());
        points.push_back(path.getLastPoint());
    }
    // check body IDs through the proof doc, which takes
    //  account of merges done but not uploaded (which are
    //  invisible if you go directly to DVID)
    std::vector<uint64_t> bodyIDs = getFlyEmProofDoc()->getBodyId(points);

    m_currentPathBodyIDs.clear();
    for (size_t i=0; i<points.size(); i++) {
        m_currentPathBodyIDs[points[i]] = bodyIDs[i];
    }

    m_bodySkippedPathCount[m_currentBody] = 0;
    return loadNextPath();
}

bool FocusedPathProtocol::loadNextPath() {
    while (m_currentBodyPaths.size() > 0) {
        m_currentPath = findNextPath();
        m_currentPath.loadEdges(m_reader, getFlyEmProofDoc(), m_edgeDataInstance);

        if (!m_currentPath.isConnected()) {
            displayCurrentPath();

            // set/go to first unknown edge
            int index = m_currentPath.getFirstUnexaminedEdgeIndex();
            if (index >= 0) {
                gotoEdgePoint(m_currentPath.getEdge(index));
                updateColorMap();
            }
            return true;
        } else {
            // if a path is connected, it shouldn't be in our
            //  list, nor should it be in DVID; basically, after
            //  you link a path, this is how other potential (and now
            //  invalid) paths between the same two bodies get cleaned up
            m_currentBodyPaths.removeOne(m_currentPath);
            deletePath(m_currentPath);
            // (not needed but intended)
            // continue;
        }
    }
    // can't find unconnected path
    return false;
}

FocusedPath FocusedPathProtocol::findNextPath() {
    // candidate path; get its endpoint bodyID and
    //  see if there are any other paths to that ID
    //  that have higher probability (yes, we check
    //  against itself first time through loop; it's
    //  just easier that way)
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

    // for testing: add a "really delete?" dialog here before you remove
    //  that unconditional return!
    // untested:
    QMessageBox confirmBox;
    confirmBox.setText("Are you sure?");
    confirmBox.setInformativeText("Really delete this path (yes) or skip (no)?");
    confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmBox.setDefaultButton(QMessageBox::No);
    confirmBox.setIcon(QMessageBox::Warning);
    if (confirmBox.exec() == QMessageBox::No) {
        return;
    }


    ZDvidWriter writer;
    if (writer.open(m_dvidTarget)) {

        // remove path ID from point annotation at both endpoints
        // ugh, need to read, parse, alter, then write back

        foreach (ZIntPoint point, QList<ZIntPoint>() << path.getFirstPoint() << path.getLastPoint()) {
            ZJsonObject annJson = m_reader.readAnnotationJson(m_pointDataInstance, point);

            // this is really roundabout...load the property, parse the path string list,
            //  break the list down to items, remove the one we don't want, then reassemble

            ZDvidAnnotation ann;
            ann.loadJsonObject(annJson, FlyEM::LOAD_NO_PARTNER);
            std::string pathListString = ann.getProperty<std::string>(FocusedPathProtocol::PROPERTY_PATHS);

            QString tempString = QString::fromStdString(pathListString).remove(' ');
            QStringList pathIDList = tempString.mid(1, tempString.size() - 2).split(",");

            pathIDList.removeAll(QString::fromStdString(path.getPathID()));

            ann.addProperty(FocusedPathProtocol::PROPERTY_PATHS, pathIDList.join(", ").toStdString());
            writer.writePointAnnotation(m_pointDataInstance, ann.toJsonObject());
        }

        // delete path from key-value
        writer.deleteKey(path.getPathID(), m_pathDataInstance);
    }
}

void FocusedPathProtocol::displayCurrentPath() {
    // edges already loaded in path, and path known to be unconnected;
    // load edges into UI and update labels

    // edge table (from scratch)
    loadEdgeTable();

    // top label: overall connection, body IDs
    updateConnectionLabel();

    // bottom label: edges, paths, bodies
    updateProgressLabel();
}

void FocusedPathProtocol::loadEdgeTable() {
    // load data into model
    m_edgeModel->clear();
    m_edgeModel->setRowCount(m_currentPath.getNumEdges());
    // reset headers here?

    // load edges so that the current body is on the left; thus,
    //  figure out the direction, and do all edges the same direction
    // (top to bottom, too)
    bool reverseDirection = false;
    if (m_currentPath.getLastBodyID() == m_currentBody) {
        reverseDirection = true;
    }

    for (int i=0; i<m_currentPath.getNumEdges(); i++) {
        int index = i;
        if (reverseDirection) {
            index = m_currentPath.getNumEdges() - i;
        }
        FocusedEdge edge = m_currentPath.getEdge(index);

        uint64_t bodyID1 = edge.getFirstBodyID();
        uint64_t bodyID2 = edge.getLastBodyID();
        if (reverseDirection) {
            uint64_t temp = bodyID1;
            bodyID1 = bodyID2;
            bodyID2 = temp;
        }

        QStandardItem * bodyID1Item = new QStandardItem();
        bodyID1Item->setData(QVariant(bodyID1), Qt::DisplayRole);
        m_edgeModel->setItem(i, BODYID1_COLUMN, bodyID1Item);

        QStandardItem * bodyID2Item = new QStandardItem();
        bodyID2Item->setData(QVariant(bodyID2), Qt::DisplayRole);
        m_edgeModel->setItem(i, BODYID2_COLUMN, bodyID2Item);

        // connection status in text form, eg, --X-- or --?--
        QStandardItem * connectionItem = new QStandardItem();
        connectionItem->setData(QVariant(QString::fromStdString(edge.getConnectionTextIcon())), Qt::DisplayRole);
        connectionItem->setTextAlignment(Qt::AlignCenter);
        m_edgeModel->setItem(i, CONNECTION_COLUMN, connectionItem);
    }

    // other table adjustments
#if QT_VERSION >= 0x050000
    ui->edgesTableView->horizontalHeader()->setSectionResizeMode(BODYID1_COLUMN, QHeaderView::Stretch);
    ui->edgesTableView->horizontalHeader()->setSectionResizeMode(CONNECTION_COLUMN, QHeaderView::ResizeToContents);
    ui->edgesTableView->horizontalHeader()->setSectionResizeMode(BODYID2_COLUMN, QHeaderView::Stretch);
#else
    ui->edgesTableView->horizontalHeader()->setResizeMode(BODYID1_COLUMN, QHeaderView::Stretch);
    ui->edgesTableView->horizontalHeader()->setResizeMode(CONNECTION_COLUMN, QHeaderView::ResizeToContents);
    ui->edgesTableView->horizontalHeader()->setResizeMode(BODYID2_COLUMN, QHeaderView::Stretch);
#endif

}

void FocusedPathProtocol::updateConnectionLabel() {
    // order the labels so current body is on the left:

    std::cout << "updateConnectionLabel(): " << std::endl;
    std::cout << "  curr body: " << m_currentBody << std::endl;
    m_currentPath.printInfo();

    uint64_t bodyID1, bodyID2;
    if (m_currentPath.getFirstBodyID() == m_currentBody) {
        bodyID1 = m_currentBody;
        bodyID2 = m_currentPath.getLastBodyID();
    } else {
        bodyID2 = m_currentBody;
        bodyID1 = m_currentPath.getLastBodyID();
    }

    std::ostringstream outputStream;
    outputStream << bodyID1;
    outputStream << " " << m_currentPath.getConnectionTextIcon() << " ";
    outputStream << bodyID2;
    ui->connectionLabel->setText(QString::fromStdString(outputStream.str()));
}

void FocusedPathProtocol::updateProgressLabel() {

    // not displaying anything involving edges at this time;
    //  it's evident by looking at the table


    // paths for current body:
    std::ostringstream outputStream1;

    // unhandled paths = current + skipped
    outputStream1 << m_currentBodyPaths.size() + m_bodySkippedPathCount[m_currentBody];
    outputStream1 << " paths for current body";
    ui->progressLabel1->setText(QString::fromStdString(outputStream1.str()));


    // bodies done:
    std::ostringstream outputStream2;
    int nBodiesDone = 0;
    foreach(bool status, m_bodyDone) {
        if (status) {
            nBodiesDone += 1;
        }
    }
    outputStream2 << nBodiesDone;
    outputStream2 << "/";
    outputStream2 << m_bodies.size();
    outputStream2 << " bodies done";

    ui->progressLabel2->setText(QString::fromStdString(outputStream2.str()));

}

void FocusedPathProtocol::updateColorMap() {

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
    emit requestActivateColorMap();
}

void FocusedPathProtocol::processBodyMerged() {
    
    // we're not getting the exact bodies merged; just refresh
    //  body IDs for the path and update the UI
    // note: that means we can't update edge examiner and time,
    //  unless we scan for un-updated edges?

    m_currentPath.updateBodyIDs(getFlyEmProofDoc());

    // we have the body IDs at paths endpoints saved locally, too,
    //  so update them:
    m_currentPathBodyIDs[m_currentPath.getFirstPoint()] = m_currentPath.getFirstBodyID();
    m_currentPathBodyIDs[m_currentPath.getLastPoint()] = m_currentPath.getLastBodyID();

    // note that this does not maintain edge selection, which
    //  we probably have to adjust
    displayCurrentPath();

}

void FocusedPathProtocol::onEdgeSelectionChanged(QItemSelection newItem, QItemSelection oldItem) {

    // go to new edge point
    gotoEdgePoint(m_currentPath.getEdge(newItem.indexes().first().row()));

    // debug:
    printEdge(m_currentPath.getEdge(newItem.indexes().first().row()));

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

void FocusedPathProtocol::printEdge(FocusedEdge edge) {
    // for debugging
    std::cout << "edge: " << edge.getFirstPoint().toString() << " - " << edge.getLastPoint().toString() << std::endl;
    std::cout << "body IDs: " << edge.getFirstBodyID() << " - " << edge.getLastBodyID() << std::endl;
    std::cout << "weight = " << edge.getWeight() << std::endl;
    std::cout << "examined by " << edge.getExaminer() << " at " << edge.getTimeExamined() << std::endl;
}

void FocusedPathProtocol::printPath(FocusedPath path) {
    // for debugging
    std::cout << "path: " << path.getFirstPoint().toString() << " - " << path.getLastPoint().toString() << std::endl;
    std::cout << "body IDs: " << path.getFirstBodyID() << " - " << path.getLastBodyID() << std::endl;
    std::cout << "unexamined edges: " << path.getNumUnexaminedEdges() << "/" << path.getNumEdges() << std::endl;
    std::cout << "probability = " << path.getProbability() << std::endl;
}

void FocusedPathProtocol::saveState() {

    ZJsonObject data;

    // always version your output files!
    data.setEntry(KEY_VERSION.c_str(), m_fileVersion);

    data.setEntry(KEY_EDGE_INSTANCE.c_str(), m_edgeDataInstance);
    data.setEntry(KEY_PATH_INSTANCE.c_str(), m_pathDataInstance);
    data.setEntry(KEY_POINT_INSTANCE.c_str(), m_pointDataInstance);

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
