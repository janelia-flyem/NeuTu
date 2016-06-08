#include "synapsepredictionprotocol.h"
#include "ui_synapsepredictionprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QInputDialog>
#include <QMessageBox>

#include "zjsonarray.h"
#include "zjsonobject.h"
#include "zjsonparser.h"

SynapsePredictionProtocol::SynapsePredictionProtocol(QWidget *parent) :
    ui(new Ui::SynapsePredictionProtocol),
    ProtocolDialog(parent)
{
    ui->setupUi(this);
}

// protocol name should not contain hyphens
const std::string SynapsePredictionProtocol::PROTOCOL_NAME = "synapse_prediction";

/*
 * start the protocol anew; returns success status;
 * initialize is also expected to do the first save
 * of the protocol's data
 */
bool SynapsePredictionProtocol::initialize() {


    return true;
}

std::string SynapsePredictionProtocol::getName() {
    return PROTOCOL_NAME;
}

void SynapsePredictionProtocol::loadDataRequested(ZJsonObject data) {

}

SynapsePredictionProtocol::~SynapsePredictionProtocol()
{
    delete ui;
}
