#include <iostream>
#include <stdlib.h>

#include <QtGui>
#include <QMessageBox>
#include <QColor>

#if QT_VERSION >= 0x050000
#include <QtConcurrent>
#include <QFileDialog>
#include <QColorDialog>
#else
#include <QtCore>
#endif

#include "zjsonarray.h"
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zstring.h"

#include "dvid/zdvidtarget.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidsynapse.h"

#include "flyembodyinfodialog.h"
#include "ui_flyembodyinfodialog.h"
#include "zdialogfactory.h"
#include "zstring.h"

/*
 * this dialog displays a list of bodies and their properties; data is
 * loaded from DVID
 *
 * to add/remove/alter columns in body table:
 * -- when creating model, change # columns
 * -- in setBodyHeaders(), adjust headers
 * -- in updateModel(), adjust data load and initial sort order
 * -- in enum in .h file, add new column constant
 *
 * notes:
 * -- I really should be creating model and headers from a constant headers list
 *
 * -- at some point, it would be good to separate the load logic into its own class
 *
 * -- in fact, a lot of things need to be separated out; I wrote this when I
 *      was just learning Qt, and using the designer on top of that, so
 *      I didn't create new widgets to compose due to lack of
 *      experience and time; that's why everthing is in this one class
 *
 *
 * djo, 7/15
 *
 */
FlyEmBodyInfoDialog::FlyEmBodyInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FlyEmBodyInfoDialog)
{
    ui->setupUi(this);

    // office phone number = random seed
    qsrand(2094656);
    m_quitting = false;
    m_connectionsLoading = false;


    // top body list stuff

    // top body list stuff

    // first table manages list of bodies
    m_bodyModel = new QStandardItemModel(0, 5, ui->bodyTableView);
    setBodyHeaders(m_bodyModel);

    m_bodyProxy = new QSortFilterProxyModel(this);
    m_bodyProxy->setSourceModel(m_bodyModel);
    m_bodyProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_bodyProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    // -1 = filter on all columns!  ALL!  no column left behind!
    m_bodyProxy->setFilterKeyColumn(-1);
    ui->bodyTableView->setModel(m_bodyProxy);

    // store body names for later use
    m_bodyNames = QMap<uint64_t, QString>();

    // color filter stuff

    // second table manages list of color filters, which 
    //  as a whole constitute the color map
    m_filterModel = new QStandardItemModel(0, 2, ui->filterTableView);
    setFilterHeaders(m_filterModel);

    m_filterProxy = new QSortFilterProxyModel(this);
    m_filterProxy->setSourceModel(m_filterModel);
    ui->filterTableView->setModel(m_filterProxy);
    ui->filterTableView->setColumnWidth(FILTER_NAME_COLUMN, 450);

    // this proxy is used to build color schemes; it's not hooked
    //  to a table view; match the filter settings of the body filter!
    m_schemeBuilderProxy = new QSortFilterProxyModel(this);
    m_schemeBuilderProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    // -1 = filter on all columns!  ALL!  no column left behind!
    m_schemeBuilderProxy->setFilterKeyColumn(-1);
    m_schemeBuilderProxy->setSourceModel(m_bodyModel);


    // connections tables stuff; table of bodies first
    m_connectionsTableState = CT_NONE;
    // table holding bodies that are input to the chosen body
    m_ioBodyModel = new QStandardItemModel(0, 3, ui->ioBodyTableView);
    setIOBodyHeaders(m_ioBodyModel);
    m_ioBodyProxy = new QSortFilterProxyModel(this);
    m_ioBodyProxy->setSourceModel(m_ioBodyModel);
    ui->ioBodyTableView->setModel(m_ioBodyProxy);
    // set needed col width here

    // table of connection sites (ie, synapses)
    m_connectionsModel = new QStandardItemModel(0, 3, ui->connectionsTableView);
    setConnectionsHeaders(m_connectionsModel);
    m_connectionsProxy = new QSortFilterProxyModel(this);
    m_connectionsProxy->setSourceModel(m_connectionsModel);
    ui->connectionsTableView->setModel(m_connectionsProxy);
    // set col width here?



    // UI connects
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(onCloseButton()));
    connect(ui->refreshButton, SIGNAL(clicked()), this, SLOT(onRefreshButton()));
    connect(ui->saveColorFilterButton, SIGNAL(clicked()), this, SLOT(onSaveColorFilter()));
    connect(ui->exportBodiesButton, SIGNAL(clicked(bool)), this, SLOT(onExportBodies()));
    connect(ui->saveButton, SIGNAL(clicked(bool)), this, SLOT(onSaveColorMap()));
    connect(ui->loadButton, SIGNAL(clicked(bool)), this, SLOT(onLoadColorMap()));
    connect(ui->bodyTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickBodyTable(QModelIndex)));
    connect(ui->gotoBodiesButton, SIGNAL(clicked(bool)), this, SLOT(onGotoBodies()));
    connect(ui->bodyFilterField, SIGNAL(textChanged(QString)), this, SLOT(bodyFilterUpdated(QString)));
    connect(ui->clearFilterButton, SIGNAL(clicked(bool)), ui->bodyFilterField, SLOT(clear()));
    connect(ui->toBodyListButton, SIGNAL(clicked(bool)), this, SLOT(moveToBodyList()));
    connect(ui->deleteButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteButton()));
    connect(ui->filterTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickFilterTable(QModelIndex)));
    connect(ui->ioBodyTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickIOBodyTable(QModelIndex)));
    connect(ui->connectionsTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickIOConnectionsTable(QModelIndex)));
    connect(QApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(applicationQuitting()));

    // data update connects
    // register our type so we can signal/slot it across threads:
    qRegisterMetaType<ZJsonValue>("ZJsonValue");
    connect(this, SIGNAL(dataChanged(ZJsonValue)), this, SLOT(updateModel(ZJsonValue)));
    connect(this, SIGNAL(loadCompleted()), this, SLOT(updateStatusAfterLoading()));
    connect(this, SIGNAL(loadCompleted()), this, SLOT(updateBodyFilterAfterLoading()));
    connect(this, SIGNAL(loadCompleted()), this, SLOT(updateColorScheme()));
    connect(this, SIGNAL(jsonLoadBookmarksError(QString)), this, SLOT(onjsonLoadBookmarksError(QString)));
    connect(this, SIGNAL(jsonLoadColorMapError(QString)), this, SLOT(onjsonLoadColorMapError(QString)));
    connect(this, SIGNAL(colorMapLoaded(ZJsonValue)), this, SLOT(onColorMapLoaded(ZJsonValue)));
    connect(this, SIGNAL(ioBodiesLoaded()), this, SLOT(onIOBodiesLoaded()));

}

void FlyEmBodyInfoDialog::onDoubleClickBodyTable(QModelIndex modelIndex)
{
    if (modelIndex.column() == BODY_ID_COLUMN) {
        activateBody(modelIndex);
    } else if (modelIndex.column() == BODY_NPRE_COLUMN || modelIndex.column() == BODY_NPOST_COLUMN) {
        gotoPrePost(modelIndex);
    }
}

void FlyEmBodyInfoDialog::activateBody(QModelIndex modelIndex)
{
    QStandardItem *item = m_bodyModel->itemFromIndex(m_bodyProxy->mapToSource(modelIndex));
    uint64_t bodyId = item->data(Qt::DisplayRole).toULongLong();

#ifdef _DEBUG_
    std::cout << bodyId << " activated." << std::endl;
#endif

    // double-click = select and goto
    // shift-double-click = select and goto, but don't clear previous bodies from 3d views
    Qt::KeyboardModifiers modifiers  = QApplication::queryKeyboardModifiers();
    if (modifiers.testFlag(Qt::ShiftModifier)) {
        emit addBodyActivated(bodyId);
    } else {
        // technically also catches alt, ctrl double-click
        emit bodyActivated(bodyId);
    }

}

void FlyEmBodyInfoDialog::onGotoBodies() {
    if (ui->bodyTableView->selectionModel()->hasSelection()) {
        QList<uint64_t> bodyIDList;
        QModelIndexList indices = ui->bodyTableView->selectionModel()->selectedIndexes();
        foreach(QModelIndex modelIndex, indices) {
            // if the item is in the first column (body ID), extract body ID, put in list
            if (modelIndex.column() == BODY_ID_COLUMN) {
                QStandardItem *item = m_bodyModel->itemFromIndex(m_bodyProxy->mapToSource(modelIndex));
                bodyIDList.append(item->data(Qt::DisplayRole).toULongLong());
            }
        }

        emit bodiesActivated(bodyIDList);
    }
}

void FlyEmBodyInfoDialog::dvidTargetChanged(ZDvidTarget target) {
#ifdef _DEBUG_
    std::cout << "dvid target changed to " << target.getUuid() << std::endl;
#endif

    // store dvid target (may not be necessary, now that I removed the
    //  option to turn off autoload?)
    m_currentDvidTarget = target;

    // if target isn't null, trigger load in thread
    if (target.isValid()) {
        // clear the model regardless at this point
        m_bodyModel->clear();
        setStatusLabel("Loading...");

        // we can load this info from different sources, depending on
        //  what's available in DVID

        // is the synapse file present?
        if (dvidBookmarksPresent(target)) {
            m_futureMap["importBookmarksDvid"] =
                QtConcurrent::run(this, &FlyEmBodyInfoDialog::importBookmarksDvid, target);
        } else if (bodies3Present(target)) {
            // this is the current expected fallback method...
            m_futureMap["importBodiesDvid"] =
                QtConcurrent::run(this, &FlyEmBodyInfoDialog::importBodiesDvid, target);
        } else {
            // ...but sometimes, we've got nothing
            emit loadCompleted();
        }
    }
}

void FlyEmBodyInfoDialog::applicationQuitting() {
    m_quitting = true;
}

void FlyEmBodyInfoDialog::setBodyHeaders(QStandardItemModel * model) {
    model->setHorizontalHeaderItem(BODY_ID_COLUMN, new QStandardItem(QString("Body ID")));
    model->setHorizontalHeaderItem(BODY_NAME_COLUMN, new QStandardItem(QString("name")));
    model->setHorizontalHeaderItem(BODY_NPRE_COLUMN, new QStandardItem(QString("# pre")));
    model->setHorizontalHeaderItem(BODY_NPOST_COLUMN, new QStandardItem(QString("# post")));
    model->setHorizontalHeaderItem(BODY_STATUS_COLUMN, new QStandardItem(QString("status")));
}

void FlyEmBodyInfoDialog::setFilterHeaders(QStandardItemModel * model) {
    model->setHorizontalHeaderItem(FILTER_NAME_COLUMN, new QStandardItem(QString("Filter")));
    model->setHorizontalHeaderItem(FILTER_COLOR_COLUMN, new QStandardItem(QString("Color")));
}

void FlyEmBodyInfoDialog::setIOBodyHeaders(QStandardItemModel * model) {
    model->setHorizontalHeaderItem(IOBODY_ID_COLUMN, new QStandardItem(QString("Body ID")));
    model->setHorizontalHeaderItem(IOBODY_NAME_COLUMN, new QStandardItem(QString("name")));
    model->setHorizontalHeaderItem(IOBODY_NUMBER_COLUMN, new QStandardItem(QString("#")));
}

void FlyEmBodyInfoDialog::setConnectionsHeaders(QStandardItemModel * model) {
    model->setHorizontalHeaderItem(CONNECTIONS_X_COLUMN, new QStandardItem(QString("x")));
    model->setHorizontalHeaderItem(CONNECTIONS_Y_COLUMN, new QStandardItem(QString("y")));
    model->setHorizontalHeaderItem(CONNECTIONS_Z_COLUMN, new QStandardItem(QString("z")));
}

void FlyEmBodyInfoDialog::setStatusLabel(QString label) {
    ui->statusLabel->setText(label);
}

void FlyEmBodyInfoDialog::clearStatusLabel() {
    ui->statusLabel->setText("");
}

void FlyEmBodyInfoDialog::updateStatusLabel() {
    qlonglong nPre = 0;
    qlonglong nPost = 0;
    for (qlonglong i=0; i<m_bodyProxy->rowCount(); i++) {
        nPre += m_bodyProxy->data(m_bodyProxy->index(i, BODY_NPRE_COLUMN)).toLongLong();
        nPost += m_bodyProxy->data(m_bodyProxy->index(i, BODY_NPOST_COLUMN)).toLongLong();
    }

    // have I mentioned how much I despise C++ strings?
    std::ostringstream outputStream;
    outputStream << "Showing " << m_bodyProxy->rowCount() << "/" << m_bodyModel->rowCount() << " bodies, ";
    outputStream << nPre << "/" <<  m_totalPre << " pre-syn, ";
    outputStream << nPost << "/" <<  m_totalPost << " post-syn";
    setStatusLabel(QString::fromStdString(outputStream.str()));
}

void FlyEmBodyInfoDialog::updateStatusAfterLoading() {
    if (m_bodyModel->rowCount() > 0) {
        updateStatusLabel();
    } else {
        clearStatusLabel();
    }
}

void FlyEmBodyInfoDialog::updateBodyFilterAfterLoading() {
    // if the user typed something into the filter box while
    //  the body data was loading, we need to kick it to update:
    if (ui->bodyFilterField->text().size() > 0) {
        bodyFilterUpdated(ui->bodyFilterField->text());
    }
}

void FlyEmBodyInfoDialog::bodyFilterUpdated(QString filterText) {
    m_bodyProxy->setFilterFixedString(filterText);

    // turns out you need to explicitly tell it to resort after the filter
    //  changes; if you don't, and new filter shows more items, those items
    //  will appear somewhere lower than the existing items
    m_bodyProxy->sort(m_bodyProxy->sortColumn(), m_bodyProxy->sortOrder());
    updateStatusLabel();
}

bool FlyEmBodyInfoDialog::dvidBookmarksPresent(ZDvidTarget target) {
    ZDvidReader reader;
    reader.setVerbose(false);
    if (reader.open(target)) {
        // check for data name and key
        if (!reader.hasData(ZDvidData::GetName(ZDvidData::ROLE_BODY_ANNOTATION))) {
            #ifdef _DEBUG_
                std::cout << "UUID doesn't have body annotations" << std::endl;
            #endif
            return false;
        }

        // I don't like this hack, but we seem not to have "hasKey()", or any way to detect
        //  a failure to find a key (the reader doesn't report, eg, 404s after a call)
        if (reader.readKeys(ZDvidData::GetName(ZDvidData::ROLE_BODY_ANNOTATION),
            ZDvidData::GetName(ZDvidData::ROLE_BODY_SYNAPSES), 
            ZDvidData::GetName(ZDvidData::ROLE_BODY_SYNAPSES)).size() == 0) {
            #ifdef _DEBUG_
                std::cout << "UUID doesn't have body_synapses key" << std::endl;
            #endif
            return false;
        }
        return true;
    } else {
        return false;
    }
}

bool FlyEmBodyInfoDialog::bodies3Present(ZDvidTarget target) {
    ZDvidReader reader;
    reader.setVerbose(false);
    if (reader.open(target)) {
        // check for data name and key
        if (!reader.hasData(ZDvidData::GetName(ZDvidData::ROLE_BODY_ANNOTATION,
            ZDvidData::ROLE_BODY_LABEL, target.getBodyLabelName()))) {
            #ifdef _DEBUG_
                std::cout << "UUID doesn't have bodies3 annotations" << std::endl;
            #endif
            return false;
        }
        return true;
    } else {
        return false;
    }
}

void FlyEmBodyInfoDialog::importBookmarksDvid(ZDvidTarget target) {
#ifdef _DEBUG_
    std::cout << "loading bookmarks from " << target.getUuid() << std::endl;
#endif

    // this method assumes DVID target is valid, and necessary DVID keys
    //  are present

    ZDvidReader reader;
    reader.setVerbose(false);
    if (reader.open(target)) {
        const QByteArray &bookmarkData = reader.readKeyValue(
            ZDvidData::GetName(ZDvidData::ROLE_BODY_ANNOTATION),
            ZDvidData::GetName(ZDvidData::ROLE_BODY_SYNAPSES));
        ZJsonObject jsonDataObject;
        jsonDataObject.decodeString(bookmarkData.data());

        // validate; this method does its own error notifications
        if (!isValidBookmarkFile(jsonDataObject)) {
            emit loadCompleted();
            return;
        }

        // now we try to fill in name and status data from more dvid calls
        // note: not sure how well this will scale, as we're querying
        //  every body's data individually
        ZJsonArray bookmarks(jsonDataObject.value("data"));
        #ifdef _DEBUG_
            std::cout << "number of bookmarks to process = " << bookmarks.size() << std::endl;
        #endif

        QString bodyAnnotationName = ZDvidData::GetName(
              ZDvidData::ROLE_BODY_ANNOTATION,
              ZDvidData::ROLE_BODY_LABEL,
              target.getBodyLabelName()).c_str();

        // get all the keys rather than testing whether each body ID
        //  has a name individually
        QStringList keyList = reader.readKeys(bodyAnnotationName);
        QSet<uint64_t> bodySet;
        foreach (const QString &str, keyList) {
          std::vector<uint64_t> bodyIdArray =
              ZString(str.toStdString()).toUint64Array();
          if (bodyIdArray.size() == 1) {
            uint64_t bodyId = bodyIdArray.front();
            if (bodyId > 0) {
              bodySet.insert(bodyId);
            }
          }
        }

        m_bodyNames.clear();
        for (size_t i = 0; i < bookmarks.size(); ++i) {
            // if application is quitting, return = exit thread
            if (m_quitting) {
                return;
            }

            #ifdef _DEBUG_
                if (i % 100 == 0) {
                   std::cout << "processing bookmark " << i << std::endl;
                }
            #endif

            ZJsonObject bkmk(bookmarks.at(i), false);

            uint64_t bodyId = bkmk.value("body ID").toInteger();
            if (bodySet.contains(bodyId)) {
                const QByteArray &temp = reader.readKeyValue(bodyAnnotationName,
                      QString::number(bkmk.value("body ID").toInteger()));
                ZJsonObject tempJson;
                tempJson.decodeString(temp.data());

                // this is way too wordy to leave on all the time, even in debug
                // #ifdef _DEBUG_
                //     std::cout << "parsing info for body ID = " << bkmk.value("body ID").toInteger() << std::endl;
                //     std::cout << "name = " << ZJsonParser::stringValue(tempJson["name"]) << std::endl;
                //     std::cout << "status = " << ZJsonParser::stringValue(tempJson["status"]) << std::endl;
                // #endif

                // now push the value back in; don't put empty strings in (messes with sorting)
                // updateModel expects "body status", not "status" (matches original file version)
                if (tempJson.hasKey("status") && strlen(ZJsonParser::stringValue(tempJson["status"])) > 0) {
                    bkmk.setEntry("body status", tempJson["status"]);
                }
                if (tempJson.hasKey("name") && strlen(ZJsonParser::stringValue(tempJson["name"])) > 0) {
                    bkmk.setEntry("name", tempJson["name"]);

                    // store name for later use
                    m_bodyNames[bodyId] = QString(ZJsonParser::stringValue(tempJson["name"]));
                    }
                }
            }

        // no "loadCompleted()" here; it's emitted in updateModel(), when it's done
        emit dataChanged(jsonDataObject.value("data"));
    } else {
        // but we need to clear the loading message if we can't read from DVID
        emit loadCompleted();
    }
}

bool FlyEmBodyInfoDialog::isValidBookmarkFile(ZJsonObject jsonObject) {
    // validation is admittedly limited for now; ultimately, I
    //  expect we'll be getting the info from DVID rather than
    //  a file anyway, so I'm not going to spend much time on it
    
    // in a perfect world, all these json constants would be collected
    //  somewhere central

    if (!jsonObject.hasKey("data") || !jsonObject.hasKey("metadata")) {
        emit jsonLoadBookmarksError("This file is missing 'data' or 'metadata'. Are you sure this is a Fly EM JSON file?");
        return false;
    }

    ZJsonObject metadata = (ZJsonObject) jsonObject.value("metadata");
    if (!metadata.hasKey("description")) {
        emit jsonLoadBookmarksError("This file is missing 'metadata/description'. Are you sure this is a Fly EM JSON file?");
        return false;
    }

    ZString description = ZJsonParser::stringValue(metadata["description"]);
    if (description != "bookmarks") {
        emit jsonLoadBookmarksError("This json file does not have description 'bookmarks'!");
        return false;
    }

    ZJsonValue data = jsonObject.value("data");
    if (!data.isArray()) {
        emit jsonLoadBookmarksError("The data section in this json file is not an array!");
        return false;
    }

    // we could/should test all the elements of the array to see if they
    //  are bookmarks, but enough is enough...

    return true;
}

void FlyEmBodyInfoDialog::importBodiesDvid(ZDvidTarget target) {
    // this method is a lot like importBookmarksDvid; where that method
    //  (1) reads a json file from dvid, then (2) adds name, status 
    //  from different DVID call, this one just goes straight to step 
    //  two; as a result, a lot of the code is copied from there 
    //  (and should probably be refactored to remove duplication)

    // DVID target assumed to be valid 

    ZDvidReader reader;
    reader.setVerbose(false);
    if (reader.open(target)) {
        // we need to construct a json structure from scratch to
        //  match what we'd get out of the bookmarks annotation file;
        //  probably this should be refactored 

        // get all the bodies that have annotations in this UUID        
        // note that this list contains body IDs in strings, *plus*
        //  some other nonnumeric strings (!!)

        QString bodyAnnotationName = ZDvidData::GetName(
              ZDvidData::ROLE_BODY_ANNOTATION,
              ZDvidData::ROLE_BODY_LABEL,
              target.getBodyLabelName()).c_str();
        QStringList keyList = reader.readKeys(bodyAnnotationName);

        ZJsonArray bodies;
        bool ok;
        qlonglong bodyID;
        foreach (const QString &bodyIDstr, keyList) {
            // skip the few non-numeric keys mixed in there
            bodyID = bodyIDstr.toLongLong(&ok);
            if (ok) {
                // get body annotations and transform to what we need
                const QByteArray &temp = reader.readKeyValue(bodyAnnotationName, bodyIDstr);
                ZJsonObject bodyData;
                bodyData.decodeString(temp.data());

                // remove name if empty
                if (bodyData.hasKey("name") && strlen(ZJsonParser::stringValue(bodyData["name"])) == 0) {
                    bodyData.removeKey("name");
                }

                // remove status if empty; change status > body status
                if (bodyData.hasKey("status")) {
                    if (strlen(ZJsonParser::stringValue(bodyData["status"])) > 0) {
                        bodyData.setEntry("body status", bodyData["status"]);
                    } 
                    // don't really need to remove this, but why not
                    bodyData.removeKey("status");
                }

                bodies.append(bodyData);
            } 
        }

        // no "loadCompleted()" here; it's emitted in updateModel(), when it's done
        emit dataChanged(bodies);
    } else {
        // but we need to clear the loading message if we can't read from DVID
        emit loadCompleted();
    }
}

void FlyEmBodyInfoDialog::onRefreshButton() {
    if (m_currentDvidTarget.isValid()) {
        ui->bodyFilterField->clear();
        dvidTargetChanged(m_currentDvidTarget);
    }
}

void FlyEmBodyInfoDialog::onCloseButton() {
    close();
}

/*
 * update the data model from json; input should be the
 * "data" part of our standard bookmarks json file
 *
 * input should be json array of dictionaries looking
 * like this (all values basically optional except 
 * body ID, extras ignored):
 * 
 *    {
 *      "name": "neuron name",
 *      "body status": "Not examined",
 *      "body PSDs": 0,
 *      "body T-bars": 1,
 *      "body ID": 15379594
 *    }
 */
void FlyEmBodyInfoDialog::updateModel(ZJsonValue data) {
    m_bodyModel->clear();
    setBodyHeaders(m_bodyModel);

    m_totalPre = 0;
    m_totalPost = 0;

    ZJsonArray bookmarks(data);
    m_bodyModel->setRowCount(bookmarks.size());
    for (size_t i = 0; i < bookmarks.size(); ++i) {
        ZJsonObject bkmk(bookmarks.at(i), false);

        // carefully set data for column items so they will sort
        //  properly (eg, IDs numerically, not lexically)
        qulonglong bodyID = ZJsonParser::integerValue(bkmk["body ID"]);
        QStandardItem * bodyIDItem = new QStandardItem();
        bodyIDItem->setData(QVariant(bodyID), Qt::DisplayRole);
        m_bodyModel->setItem(i, BODY_ID_COLUMN, bodyIDItem);

        if (bkmk.hasKey("name")) {
            const char* name = ZJsonParser::stringValue(bkmk["name"]);
            m_bodyModel->setItem(i, BODY_NAME_COLUMN, new QStandardItem(QString(name)));
        }

        if (bkmk.hasKey("body T-bars")) {
            int nPre = ZJsonParser::integerValue(bkmk["body T-bars"]);
            m_totalPre += nPre;
            QStandardItem * preSynapseItem = new QStandardItem();
            preSynapseItem->setData(QVariant(nPre), Qt::DisplayRole);
            m_bodyModel->setItem(i, BODY_NPRE_COLUMN, preSynapseItem);
        }

        if (bkmk.hasKey("body PSDs")) {
            int nPost = ZJsonParser::integerValue(bkmk["body PSDs"]);
            m_totalPost += nPost;
            QStandardItem * postSynapseItem = new QStandardItem();
            postSynapseItem->setData(QVariant(nPost), Qt::DisplayRole);
            m_bodyModel->setItem(i, BODY_NPOST_COLUMN, postSynapseItem);
        }

        // note that this routine expects "body status", not "status";
        //  historical side-effect of the original file format we read from
        if (bkmk.hasKey("body status")) {
            const char* status = ZJsonParser::stringValue(bkmk["body status"]);
            m_bodyModel->setItem(i, BODY_STATUS_COLUMN, new QStandardItem(QString(status)));
        }
    }
    // the resize isn't reliable, so set the name column wider by hand
    ui->bodyTableView->resizeColumnsToContents();
    ui->bodyTableView->setColumnWidth(BODY_NAME_COLUMN, 150);

    // currently initially sorting on # pre-synaptic sites
    ui->bodyTableView->sortByColumn(BODY_NPRE_COLUMN, Qt::DescendingOrder);

    emit loadCompleted();
}

void FlyEmBodyInfoDialog::onjsonLoadBookmarksError(QString message) {
    QMessageBox errorBox;
    errorBox.setText("Error loading body information");
    errorBox.setInformativeText(message);
    errorBox.setStandardButtons(QMessageBox::Ok);
    errorBox.setIcon(QMessageBox::Warning);
    errorBox.exec();
}

void FlyEmBodyInfoDialog::onjsonLoadColorMapError(QString message) {
    QMessageBox errorBox;
    errorBox.setText("Error loading color map");
    errorBox.setInformativeText(message);
    errorBox.setStandardButtons(QMessageBox::Ok);
    errorBox.setIcon(QMessageBox::Warning);
    errorBox.exec();
}

void FlyEmBodyInfoDialog::onSaveColorFilter() {
    if (ui->bodyFilterField->text().size() > 0) {
        // no duplicates
        if (m_filterModel->findItems(ui->bodyFilterField->text(), Qt::MatchExactly, FILTER_NAME_COLUMN).size() == 0) {
            updateColorFilter(ui->bodyFilterField->text());
        }
    }
}

void FlyEmBodyInfoDialog::onExportBodies() {
    // note that you can't specify a default name for a new file
    QString filename = QFileDialog::getSaveFileName(this, "Export bodies");
    if (!filename.isNull()) {
        exportBodies(filename);
    }
}

void FlyEmBodyInfoDialog::onSaveColorMap() {
    QString filename = QFileDialog::getSaveFileName(this, "Save color map");
    if (!filename.isNull()) {
        saveColorMapDisk(filename);
    }
}

void FlyEmBodyInfoDialog::onLoadColorMap() {
    QString filename = QFileDialog::getOpenFileName(this, "Load color map");
    if (!filename.isNull()) {
        ZJsonValue colors;
        if (colors.load(filename.toStdString())) {
            // validation is done later
            emit colorMapLoaded(colors);
        } else {
            emit jsonLoadColorMapError("Error opening or parsing color map file " + filename);
        }
    }
}

bool FlyEmBodyInfoDialog::isValidColorMap(ZJsonValue colors) {
    // there's only so much we can do...but make an effort

    // NOTE: this format is a messy hack; should be revised!

    // it's an array
    if (!colors.isArray()) {
        emit jsonLoadColorMapError("Color map json file must contain an array");
        return false;
    }

    ZJsonArray colorArray(colors);
    if (colorArray.size() == 0) {
        return true;
    }

    // look at first element and check its structure; it's an object
    if (!ZJsonParser::isObject(colorArray.at(0))) {
        emit jsonLoadColorMapError("Color map json file must contain an array of objects");
        return false;
    }

    // has keys "color", "filter":
    ZJsonObject first(colorArray.at(0), false);
    if (!first.hasKey("color") || !first.hasKey("filter")) {
        emit jsonLoadColorMapError("Color map json entries must have 'color' and 'filter' keys");
        return false;
    }

    // we could keep going, but let's stop for now
    // could also check:
    // key "color": array of four ints 0-255
    // key "filter": string

    return true;
}

void FlyEmBodyInfoDialog::onColorMapLoaded(ZJsonValue colors) {
    if (!isValidColorMap(colors)) {
        // validator spits out its own errors
        return;
    }

    // clear existing color map and put in new values
    m_filterModel->clear();
    setFilterHeaders(m_filterModel);

    // we've passed validation at this point, so we know it's an array;
    //  iterate over each color, filter and insert into table;
    ZJsonArray colorArray(colors);
    for (size_t i=0; i<colorArray.size(); i++) {
        ZJsonObject entry(colorArray.at(i), false);

        QString filter(ZJsonParser::stringValue(entry["filter"]));
        QStandardItem * filterTextItem = new QStandardItem(filter);
        m_filterModel->appendRow(filterTextItem);

        std::vector<int> rgba = ZJsonParser::integerArray(entry["color"]);
        setFilterTableModelColor(QColor(rgba[0], rgba[1], rgba[2], rgba[3]), 
            m_filterModel->rowCount() - 1);
    }


    // update the table view
    ui->filterTableView->resizeColumnsToContents();
    ui->filterTableView->setColumnWidth(FILTER_NAME_COLUMN, 450);

    updateColorScheme();
}

void FlyEmBodyInfoDialog::updateColorFilter(QString filter, QString /*oldFilter*/) {
    // note: oldFilter currently unused; I was thinking about allowing an edit
    //  to a filter that would replace an older filter but didn't implement it

    QStandardItem * filterTextItem = new QStandardItem(filter);
    m_filterModel->appendRow(filterTextItem);

    // Raveler's palette: h, s, v = (random.uniform(0, 6.283), random.uniform(0.3, 1.0), 
    //  random.uniform(0.3, 0.8)); that approximately translates to:
    int randomH = qrand() % 360;
    int randomS = 76 + qrand() % 180;
    int randomV = 76 + qrand() % 128;
    setFilterTableModelColor(QColor::fromHsv(randomH, randomS, randomV), m_filterModel->rowCount() - 1);

    ui->filterTableView->resizeColumnsToContents();
    ui->filterTableView->setColumnWidth(FILTER_NAME_COLUMN, 450);

    // activate the tab, and dispatch the changed info
    ui->tabWidget->setCurrentIndex(COLORS_TAB);
    updateColorScheme();
}

void FlyEmBodyInfoDialog::onDoubleClickFilterTable(const QModelIndex &proxyIndex) {
    QModelIndex modelIndex = m_filterProxy->mapToSource(proxyIndex);
    if (proxyIndex.column() == FILTER_NAME_COLUMN) {
        // double-click on filter text; move to body list
        QString filterString = m_filterProxy->data(m_filterProxy->index(proxyIndex.row(),
            FILTER_NAME_COLUMN)).toString();
        ui->bodyFilterField->setText(filterString);
    } else if (proxyIndex.column() == FILTER_COLOR_COLUMN) {
        // double-click on color; change it
        QColor currentColor = m_filterProxy->data(m_filterProxy->index(proxyIndex.row(),
            FILTER_COLOR_COLUMN), Qt::BackgroundRole).value<QColor>();
        QColor newColor = QColorDialog::getColor(currentColor, this, "Choose color");
        if (newColor.isValid()) {
            setFilterTableModelColor(newColor, modelIndex.row());
            updateColorScheme();
        }
    }
}

void FlyEmBodyInfoDialog::setFilterTableModelColor(QColor color, int modelRow) {
    QStandardItem * filterColorItem = new QStandardItem();
    filterColorItem->setData(color, Qt::BackgroundRole);
    // table rows are selectable, but selecting changes how color is rendered;
    //  so disallow selection of color items
    Qt::ItemFlags flags = filterColorItem->flags();
    flags &= ~Qt::ItemIsSelectable;
    filterColorItem->setFlags(flags);
    m_filterModel->setItem(modelRow, FILTER_COLOR_COLUMN, filterColorItem);
}

void FlyEmBodyInfoDialog::onDeleteButton() {
    if (ui->filterTableView->selectionModel()->hasSelection()) {
        // we only allow single row selections; get the filter string
        //  from the selected row; only one, so take the first index thereof:
        QModelIndex viewIndex = ui->filterTableView->selectionModel()->selectedRows(0).at(0);
        QModelIndex modelIndex = m_filterProxy->mapToSource(viewIndex);
        m_filterModel->removeRow(modelIndex.row());
        updateColorScheme();
    }
}

void FlyEmBodyInfoDialog::moveToBodyList() {
    // moves selected color filter to body list
    if (ui->filterTableView->selectionModel()->hasSelection()) {
        // we only allow single row selections; get the filter string
        //  from the selected row; only one, so take the first index thereof:
        QModelIndex viewIndex = ui->filterTableView->selectionModel()->selectedRows(0).at(0);
        QString filterString = m_filterModel->data(m_filterProxy->mapToSource(viewIndex)).toString();
        ui->bodyFilterField->setText(filterString);
        }
    }

void FlyEmBodyInfoDialog::updateColorScheme() {
    // loop over filters in filter table; attach filter to our scheme builder
    //  proxy; loop over the filtered body IDs and throw them and the color into
    //  the color scheme

    m_colorScheme.clear();
    for (int i=0; i<m_filterProxy->rowCount(); i++) {
        QString filterString = m_filterProxy->data(m_filterProxy->index(i, FILTER_NAME_COLUMN)).toString();
        m_schemeBuilderProxy->setFilterFixedString(filterString);

        QColor color = m_filterProxy->data(m_filterProxy->index(i, FILTER_COLOR_COLUMN), Qt::BackgroundRole).value<QColor>();
        for (int j=0; j<m_schemeBuilderProxy->rowCount(); j++) {
            qulonglong bodyId = m_schemeBuilderProxy->data(m_schemeBuilderProxy->index(j, BODY_ID_COLUMN)).toLongLong();
            m_colorScheme.setBodyColor(bodyId, color);
        }
    }

    emit colorMapChanged(m_colorScheme);

    // test: print it out
    // m_colorScheme.print();

}

void FlyEmBodyInfoDialog::exportBodies(QString filename) {
    QFile outputFile(filename);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox errorBox;
        errorBox.setText("Error");
        errorBox.setInformativeText("Error opening file for export");
        errorBox.setStandardButtons(QMessageBox::Ok);
        errorBox.setIcon(QMessageBox::Warning);
        errorBox.exec();
        return;
    }

    // tab-separated text file, with header
    QTextStream outputStream(&outputFile);
    for (int i=0; i<m_bodyModel->columnCount(); i++) {
        if (i != 0) {
            outputStream << "\t";
        }
        outputStream << m_bodyModel->horizontalHeaderItem(i)->text();
    }
    outputStream << "\n";

    for (int j=0; j<m_bodyProxy->rowCount(); j++) {
        for (int i=0; i<m_bodyProxy->columnCount(); i++) {
            if (i != 0) {
                outputStream << "\t";
            }
            outputStream << m_bodyProxy->data(m_bodyProxy->index(j, i)).toString();
        }
        outputStream << "\n";
    }

    outputFile.close();
}

ZJsonArray FlyEmBodyInfoDialog::getColorMapAsJson(ZJsonArray colors) {
    for (int i=0; i<m_filterModel->rowCount(); i++) {
        QString filterString = m_filterModel->data(m_filterModel->index(i, FILTER_NAME_COLUMN)).toString();
        QColor color = m_filterModel->data(m_filterModel->index(i, FILTER_COLOR_COLUMN), Qt::BackgroundRole).value<QColor>();
        ZJsonArray rgba;
        rgba.append(color.red());
        rgba.append(color.green());
        rgba.append(color.blue());
        rgba.append(color.alpha());

        ZJsonObject entry;
        entry.setEntry("filter", filterString.toStdString());
        entry.setEntry("color", rgba);
        colors.append(entry);
    }
    return colors;
}

void FlyEmBodyInfoDialog::saveColorMapDisk(QString filename) {
    ZJsonArray colors;
    getColorMapAsJson(colors).dump(filename.toStdString());
}

void FlyEmBodyInfoDialog::gotoPrePost(QModelIndex modelIndex) {

    // this guard is not vital, but it's convenient; I suspect almost
    //  everything will work, but I'm not 100% sure about some of the
    //  instance variables
    if (m_connectionsLoading) {
        QMessageBox errorBox;
        errorBox.setText("Still loading");
        errorBox.setInformativeText("The previous body connections are still loading; please retry when they are finished.");
        errorBox.setStandardButtons(QMessageBox::Ok);
        errorBox.setIcon(QMessageBox::Warning);
        errorBox.exec();
        return;
    }
    m_connectionsLoading = true;

    // grab the body ID; same row, first column
    QModelIndex index = m_bodyProxy->mapToSource(modelIndex);
    QStandardItem *item = m_bodyModel->item(index.row(), BODY_ID_COLUMN);
    m_connectionsBody = item->data(Qt::DisplayRole).toULongLong();

    // get name, too, if it's there, then update label
    QStandardItem *item2 = m_bodyModel->item(index.row(), BODY_NAME_COLUMN);
    if (item2) {
        updateBodyConnectionLabel(m_connectionsBody, item2->data(Qt::DisplayRole).toString());
    } else {
        updateBodyConnectionLabel(m_connectionsBody, "");
    }


    // table shows inputs or outputs, based on whether the pre or post
    //  cell was double-clicked:
    if (index.column() == BODY_NPRE_COLUMN) {
        m_connectionsTableState = CT_OUTPUT;
    } else {
        m_connectionsTableState = CT_INPUT;
    }

    // set labels above tables
    ui->ioBodyTableLabel->setText("Loading");
    ui->connectionsTableLabel->setText("Connections");

    // activate tab & clear model
    ui->tabWidget->setCurrentIndex(CONNECTIONS_TAB);
    m_ioBodyModel->clear();
    m_connectionsModel->clear();

    // trigger retrieval of synapse partners
    if (m_currentDvidTarget.isValid()) {
        m_futureMap["retrieveIOBodiesDvid"] =
            QtConcurrent::run(this, &FlyEmBodyInfoDialog::retrieveIOBodiesDvid, m_currentDvidTarget, m_connectionsBody);
    }
}

void FlyEmBodyInfoDialog::updateBodyConnectionLabel(uint64_t bodyID, QString bodyName) {

    std::ostringstream outputStream;
    outputStream << "Connections for ";
    if (bodyName.size() > 0) {
        outputStream << bodyName.toStdString() << " (ID " << bodyID << ")";
    } else {
        outputStream << "body ID " << bodyID;
    }
    ui->connectionBodyLabel->setText(QString::fromStdString(outputStream.str()));

}

void FlyEmBodyInfoDialog::retrieveIOBodiesDvid(ZDvidTarget target, uint64_t bodyID) {


    // std::cout << "retrieving input/output bodies from DVID for body "<< bodyID << std::endl;

    // I'm leaving all the timing prints commented out for future use
    // QElapsedTimer timer;
    // timer.start();

    ZDvidReader reader;
    reader.setVerbose(false);

    // std::cout << "opening DVID reader: " << timer.elapsed() / 1000.0 << "s" << std::endl;

    if (reader.open(target)) {
        // std::cout << "reading synapses: " << timer.elapsed() / 1000.0 << "s" << std::endl;
        std::vector<ZDvidSynapse> synapses = reader.readSynapse(bodyID, NeuTube::FlyEM::LOAD_PARTNER_LOCATION);

        // std::cout << "got " << synapses.size() << " synapses" << std::endl;
        // std::cout << "getting synapse info: " << timer.elapsed() / 1000.0 << "s" << std::endl;

        // how many pre/post?
        int npre = 0;
        int npost = 0;
        for (size_t i=0; i<synapses.size(); i++) {
            if (synapses[i].getKind() == ZDvidSynapse::KIND_PRE_SYN) {
                npre++;
            } else {
                npost++;
            }
        }
        // std::cout << "found " << npre << " pre and " << npost << " post sites" << std::endl;

        // at this point, we'll need to iterate over the list and find either
        //  the pre or post sites depending on whether we are looking for inputs
        //  or outputs
        // then for each site, find the partner site, then find the body at the 
        //  partner site; in a QMap, keep a list of synapses for each partner body
        //  (the qmap is an instance variable)
        // we only want to call dvid once, so we need to loop over synapses once
        //  to find at which x, y, z locations the partners are, then call dvid
        //  to get the bodies at those locations, then we can build the 
        //  final connections map; it's about a 3x performance improvement over
        //  calling dvid once per site

        m_connectionsSites.clear();

        // std::cout << "building site list: " << timer.elapsed() / 1000.0 << "s" << std::endl;
        std::vector<ZIntPoint> siteList;
        for (size_t i=0; i<synapses.size(); i++) {

            // if we are looking for input bodies, pick out the sites
            //  that are post-synaptic, and vice versa:
            if ( (m_connectionsTableState == CT_INPUT && synapses[i].getKind() == ZDvidSynapse::KIND_POST_SYN) ||
                 (m_connectionsTableState == CT_OUTPUT && synapses[i].getKind() == ZDvidSynapse::KIND_PRE_SYN) )  {

                // find the partner site and the body at that location
                // note that there could be multiple partners for outputs

                // currently Ting doesn't expose a method for retrieving relations
                //  directly; I've patched in a getter to use, temporarily
                std::vector<ZIntPoint> sites = synapses[i].getPartners();
                for (size_t j=0; j<sites.size(); j++) {
                    siteList.push_back(sites[j]);
                }

            } else {
                // there are other connection types we aren't handling right now; eg, convergent
            }
        }

        // get the body list from DVID
        // std::cout << "retrieving body list from DVID: " << timer.elapsed() / 1000.0 << "s" << std::endl;
        std::vector<uint64_t> bodyList = reader.readBodyIdAt(siteList);

        // copy into the map
        // std::cout << "building qmap: " << timer.elapsed() / 1000.0 << "s" << std::endl;
        for (size_t i=0; i<siteList.size(); i++) {
            if (!m_connectionsSites.contains(bodyList[i])) {
                m_connectionsSites[bodyList[i]] = QList<ZIntPoint>();
            } 
            m_connectionsSites[bodyList[i]].append(siteList[i]);
        }
    }

    emit ioBodiesLoaded();

    // std::cout << "exiting retrieveIOBodiesDvid(): " << timer.elapsed() / 1000.0 << "s" << std::endl;

}

void FlyEmBodyInfoDialog::onIOBodiesLoaded() {
    m_ioBodyModel->clear();
    setIOBodyHeaders(m_ioBodyModel);

    QList<uint64_t> partnerBodyIDs = m_connectionsSites.keys();


    // table label: "Input (123)" or "Output (123)"
    // should factor this out
    std::ostringstream labelStream;
    if (m_connectionsTableState == CT_INPUT) {
        labelStream << "Inputs (" ;
    } else {
        labelStream << "Outputs (" ;
    }
    labelStream << partnerBodyIDs.size();
    labelStream << ")";
    ui->ioBodyTableLabel->setText(QString::fromStdString(labelStream.str()));


    m_ioBodyModel->setRowCount(partnerBodyIDs.size());
    for (int i=0; i<partnerBodyIDs.size(); i++) {
        // carefully set data for column items so they will sort
        //  properly (eg, IDs numerically, not lexically)
        QStandardItem * bodyIDItem = new QStandardItem();
        bodyIDItem->setData(QVariant(quint64(partnerBodyIDs[i])), Qt::DisplayRole);
        m_ioBodyModel->setItem(i, IOBODY_ID_COLUMN, bodyIDItem);

        if (m_bodyNames.contains(partnerBodyIDs[i])) {
            m_ioBodyModel->setItem(i, IOBODY_NAME_COLUMN, new QStandardItem(m_bodyNames[partnerBodyIDs[i]]));
        }

        QStandardItem * numberItem = new QStandardItem();
        numberItem->setData(
              QVariant(quint64(m_connectionsSites[partnerBodyIDs[i]].size())),
            Qt::DisplayRole);
        m_ioBodyModel->setItem(i, IOBODY_NUMBER_COLUMN, numberItem);
    }


    // for some reason, this table gave me more trouble than the
    //  others; set its column behaviors individually
#if QT_VERSION >= 0x050000
    ui->ioBodyTableView->horizontalHeader()->setSectionResizeMode(IOBODY_ID_COLUMN, QHeaderView::ResizeToContents);
    ui->ioBodyTableView->horizontalHeader()->setSectionResizeMode(IOBODY_NAME_COLUMN, QHeaderView::Stretch);
    ui->ioBodyTableView->horizontalHeader()->setSectionResizeMode(IOBODY_NUMBER_COLUMN, QHeaderView::ResizeToContents);
#else
    ui->ioBodyTableView->horizontalHeader()->setResizeMode(IOBODY_ID_COLUMN, QHeaderView::ResizeToContents);
    ui->ioBodyTableView->horizontalHeader()->setResizeMode(IOBODY_NAME_COLUMN, QHeaderView::Stretch);
    ui->ioBodyTableView->horizontalHeader()->setResizeMode(IOBODY_NUMBER_COLUMN, QHeaderView::ResizeToContents);
#endif
    ui->ioBodyTableView->sortByColumn(IOBODY_NUMBER_COLUMN, Qt::DescendingOrder);

    m_connectionsLoading = false;

}

void FlyEmBodyInfoDialog::onDoubleClickIOBodyTable(QModelIndex proxyIndex) {

    // grab the body ID; same row, first column
    QModelIndex modelIndex = m_ioBodyProxy->mapToSource(proxyIndex);
    QStandardItem *item = m_ioBodyModel->item(modelIndex.row(), IOBODY_ID_COLUMN);
    uint64_t bodyID = item->data(Qt::DisplayRole).toULongLong();

    if (proxyIndex.column() == IOBODY_ID_COLUMN || proxyIndex.column() == IOBODY_NAME_COLUMN) {
        // double-click on ID, name = select in body table
        QList<QStandardItem*> results = m_bodyModel->findItems(item->data(Qt::DisplayRole).toString(), Qt::MatchExactly, BODY_ID_COLUMN);
        if (results.size() > 0) {
            ui->bodyTableView->selectRow(m_bodyProxy->mapFromSource(results.at(0)->index()).row());
        }

    } else if (proxyIndex.column() == IOBODY_NUMBER_COLUMN) {
        // double-click on number connections = show connections list
        m_connectionsModel->clear();
        setConnectionsHeaders(m_connectionsModel);
        for (int i=0; i<m_connectionsSites[bodyID].size(); i++) {
            ZIntPoint point = m_connectionsSites[bodyID][i];

            // start with x, y, z; but should it be z, x, y?
            QStandardItem * xItem = new QStandardItem();
            xItem->setData(QVariant(point.getX()), Qt::DisplayRole);
            m_connectionsModel->setItem(i, CONNECTIONS_X_COLUMN, xItem);

            QStandardItem * yItem = new QStandardItem();
            yItem->setData(QVariant(point.getY()), Qt::DisplayRole);
            m_connectionsModel->setItem(i, CONNECTIONS_Y_COLUMN, yItem);

            QStandardItem * zItem = new QStandardItem();
            zItem->setData(QVariant(point.getZ()), Qt::DisplayRole);
            m_connectionsModel->setItem(i, CONNECTIONS_Z_COLUMN, zItem);
        }

        // for this table, we want all columns same width, filling full width
#if QT_VERSION >= 0x050000
        ui->connectionsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
        ui->connectionsTableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif
        ui->connectionsTableView->sortByColumn(CONNECTIONS_Z_COLUMN, Qt::AscendingOrder);
    }
}

void FlyEmBodyInfoDialog::onDoubleClickIOConnectionsTable(QModelIndex proxyIndex) {

    std::cout << "in onDoubleClickIOConnectionsTable at index " << proxyIndex.row() << ", " << proxyIndex.column() << std::endl;

    QModelIndex modelIndex = m_connectionsProxy->mapToSource(proxyIndex);

    QStandardItem *itemX = m_connectionsModel->item(modelIndex.row(), CONNECTIONS_X_COLUMN);
    int x = itemX->data(Qt::DisplayRole).toInt();

    QStandardItem *itemY = m_connectionsModel->item(modelIndex.row(), CONNECTIONS_Y_COLUMN);
    int y = itemY->data(Qt::DisplayRole).toInt();

    QStandardItem *itemZ = m_connectionsModel->item(modelIndex.row(), CONNECTIONS_Z_COLUMN);
    int z = itemZ->data(Qt::DisplayRole).toInt();

    emit pointDisplayRequested(x, y, z);
}

FlyEmBodyInfoDialog::~FlyEmBodyInfoDialog()
{
    m_futureMap.waitForFinished(); //to avoid crash while quitting too early
    delete ui;
}


