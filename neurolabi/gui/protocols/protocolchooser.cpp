#include "protocolchooser.h"
#include "ui_protocolchooser.h"

#include <iostream>
#include <stdlib.h>

#include "protocolswitcher.h"

#include <QStringListModel>

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



    // ui connects
    connect(ui->startProtocolButton, SIGNAL(clicked(bool)), this, SLOT(onStartButton()));
    connect(ui->newProtocolListView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickedStartProtocol(QModelIndex)));
    connect(ui->loadProtocolButton, SIGNAL(clicked(bool)), this, SLOT(onLoadButton()));

    // misc ui settings
    ui->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

}

ProtocolChooser::~ProtocolChooser()
{
    delete ui;
}

void ProtocolChooser::onLoadButton() {

    std::cout << "load button clicked" << std::endl;

}

void ProtocolChooser::onStartButton() {
    if(ui->newProtocolListView->selectionModel()->hasSelection()) {
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
