#include "protocolchooser.h"
#include "ui_protocolchooser.h"

#include <iostream>
#include <stdlib.h>

#include <QStringListModel>

#include "protocolswitcher.h"

/*
 * this class presents a dialog to the user that lets them
 * select either a new protocol to start or an old, saved
 * protocol to load
 */
ProtocolChooser::ProtocolChooser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProtocolChooser)
{
    ui->setupUi(this);

    setupNewProtocolList();
    setupSavedProtocolList();

    // ui connects
    connect(ui->startProtocolButton, SIGNAL(clicked(bool)), this, SLOT(onStartButton()));
    connect(ui->newProtocolListView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickedStartProtocol(QModelIndex)));
    connect(ui->loadProtocolButton, SIGNAL(clicked(bool)), this, SLOT(onLoadButton()));
    connect(ui->loadProtocolListView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickedLoadProtocol(QModelIndex)));

    // misc ui settings
    ui->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

}

ProtocolChooser::~ProtocolChooser()
{
    delete ui;
}

void ProtocolChooser::onLoadButton() {
    if (ui->loadProtocolListView->selectionModel()->hasSelection()) {
        QModelIndexList indices = ui->loadProtocolListView->selectionModel()->selectedIndexes();
        onDoubleClickedLoadProtocol(indices.at(0));
    }
}

void ProtocolChooser::onDoubleClickedLoadProtocol(QModelIndex modelIndex) {
    emit requestLoadProtocolKey(modelIndex.data().toString());
    close();
}

void ProtocolChooser::onStartButton() {
    if (ui->newProtocolListView->selectionModel()->hasSelection()) {
        // remember, only single selection is allowed
        QModelIndexList indices = ui->newProtocolListView->selectionModel()->selectedIndexes();
        onDoubleClickedStartProtocol(indices.at(0));
    }
}

void ProtocolChooser::onDoubleClickedStartProtocol(QModelIndex modelIndex) {
    // note: no sorting, so model index = view index
    emit requestStartProtocol(modelIndex.data().toString());
    close();
}

void ProtocolChooser::displaySavedProtocolKeys(QStringList keyList) {
    m_savedProtocolListModel->setStringList(keyList);
}

void ProtocolChooser::setupNewProtocolList() {
    // populate list of protocols that can be started; note that
    //  for now, they are all available; I thought about making this
    //  list dynamic, in case some protocols couldn't be entered at
    //  this time, but it seemed too much work for a feature that
    //  wasn't obviously needed at first
    QStringListModel * model = new QStringListModel(this);
    model->setStringList(ProtocolSwitcher::protocolNames);
    ui->newProtocolListView->setModel(model);
}

void ProtocolChooser::setupSavedProtocolList() {
    // populate list of saved protocols
    m_savedProtocolListModel = new QStringListModel(this);
    ui->loadProtocolListView->setModel(m_savedProtocolListModel);
}
