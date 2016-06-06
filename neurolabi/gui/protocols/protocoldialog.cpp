#include "protocoldialog.h"
#include "ui_protocoldialog.h"

#include <iostream>
#include <stdlib.h>

#include <QInputDialog>
#include <QMessageBox>

#include "zjsonarray.h"
#include "zjsonobject.h"
#include "zjsonparser.h"

/*
 * this is the base class for all protocols
 *
 * to implement a protocol:
 * -- subclass ProtocolDialog
 * -- implement initialize(); this is called when the protocol is
 *      started for the first time
 * -- implement getName(); returns protocol name; must not have hyphens
 * -- signal protocolExiting(): emit when the protocol is being closed
 *      by the user but can be reopened later for more work; your
 *      responsibility to save first
 * -- signal protocolCompleting(): emit when protocol is finished
 *      and will not be reopened by the user; your responsibility
 *      to save first
 * -- requestSaveProtocol(): emit data to the switcher, who
 *      will then save it in DVID in the appropriate key
 * -- implement slot loadDataRequested(); the switcher will send data
 *      to this slot when a saved protocol is loaded
 */
ProtocolDialog::ProtocolDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProtocolDialog)
{

}

/*
 * start the protocol from scratch; returns success status;
 * initialize is also expected to do the first save
 * of the protocol's data
 */
bool ProtocolDialog::initialize() {

    std::cout << "in ProtocolDialog::initialize" << std::endl;
    return true;

}

/*
 * return the protocol's name; must not include hyphens; should probably
 * not contain space, though I don't *think* it'll break anything
 */
std::string ProtocolDialog::getName() {

    std::cout << "in ProtocolDialog::getName" << std::endl;
    return "protocol_name";
}

/*
 * this slot is hit by the protocol switcher when a protocol is loaded;
 * the data is the previously saved data
 */
void ProtocolDialog::loadDataRequested(ZJsonObject data) {

    std::cout << "in ProtocolDialog::loadDataRequested()" << std::endl;

}

ProtocolDialog::~ProtocolDialog()
{
    delete ui;
}
