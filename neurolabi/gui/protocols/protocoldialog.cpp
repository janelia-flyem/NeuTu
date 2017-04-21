#include "protocoldialog.h"
#include "ui_protocoldialog.h"

#include <iostream>
#include <stdlib.h>

#include <QInputDialog>
#include <QMessageBox>

#include "flyem/zflyemproofmvc.h"
#include "flyem/zflyemproofdoc.h"
#include "zjsonarray.h"
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zintpoint.h"

/*
 * this is the base class for all protocols
 *
 * to implement a protocol:
 * -- subclass ProtocolDialog
 * -- implement initialize(); this is called when the protocol is
 *      started for the first time
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
 * -- ProtocolSwitcher::protocolNames: add to list
 * -- in ProtocolSwitcher::instantiateProtocol: add else-if
 * --> note that you can map more than one name to one protocol
 *      class and then pass different parameters into the constructor
 *      to change its behavior at initialization time (and later)
 *
 * other signals you can use:
 * -- emit requestDisplayPoint(x, y, z) to have the 2d view move to a point
 *
 * notes:
 * -- the protocol class does not know its name!  names are managed by
 *      the switcher
 *
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

void ProtocolDialog::setDvidTarget(ZDvidTarget target) {
    m_dvidTarget = target;
}

/*
 * this slot is hit by the protocol switcher when a protocol is loaded;
 * the data is the previously saved data
 */
void ProtocolDialog::loadDataRequested(ZJsonObject /*data*/) {

    std::cout << "in ProtocolDialog::loadDataRequested()" << std::endl;

}

ProtocolDialog::~ProtocolDialog()
{
    delete ui;
}

// these methods retrieve the ZFlyEmProofDoc object, which is very useful
// copied from ZFlyEmRoiToolDialog

ZFlyEmProofMvc* ProtocolDialog::getParentFrame() const
{
  return qobject_cast<ZFlyEmProofMvc*>(parentWidget());
}

ZFlyEmProofDoc* ProtocolDialog::getDocument() const
{
  ZFlyEmProofMvc *mvc = getParentFrame();
  if (mvc != NULL) {
    return mvc->getCompleteDocument();
  }

  return NULL;
}

// these methods are called when certain proofreading events occur

void ProtocolDialog::processSynapseMoving(
    const ZIntPoint &/*from*/, const ZIntPoint &/*to*/)
{

}

void ProtocolDialog::processSynapseVerification(
    int /*x*/, int /*y*/, int /*z*/, bool /*verified*/)
{

}

void ProtocolDialog::processBodyMerged() {
    
}
