#include "synapsereviewprotocol.h"
#include "ui_synapsereviewprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QMessageBox>

#include "zjsonobject.h"

SynapseReviewProtocol::SynapseReviewProtocol(QWidget *parent) :
    ProtocolDialog(parent),
    ui(new Ui::SynapseReviewProtocol)
{
    ui->setupUi(this);
}

bool SynapseReviewProtocol::initialize() {

    // do initial load

    // get started

    // save (?)

    return true;
}

void SynapseReviewProtocol::loadDataRequested(ZJsonObject data) {

    // check version

    // read in data

    // do stuff

}

SynapseReviewProtocol::~SynapseReviewProtocol()
{
    delete ui;
}

