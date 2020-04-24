#include "synapsereviewinputdialog.h"
#include "ui_synapsereviewinputdialog.h"

#include "geometry/zintcuboid.h"

SynapseReviewInputDialog::SynapseReviewInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SynapseReviewInputDialog)
{
    ui->setupUi(this);
}

// note: not currently providing methods to set the initial values for
//  the dialog parameters; it would be easy to do so (see synapsepredictioninputdialog.cpp)

SynapseReviewInputDialog::SynapseReviewInputOptions SynapseReviewInputDialog::getInputOption() {
    if (ui->bodyIDRadioButton->isChecked()) {
        return BY_BODYID;
    } else if (ui->volumeRadioButton->isChecked()) {
        return BY_VOLUME;
    }

    return BY_BODYID;
}

QString SynapseReviewInputDialog::getBodyID() {
    return ui->bodyIDInput->text();
}

ZIntCuboid SynapseReviewInputDialog::getVolume() {
    ZIntCuboid box;
    box.setMinCorner(ui->xSpinBox->value(), ui->ySpinBox->value(),
        ui->zSpinBox->value());
    box.setSize(ui->widthSpinBox->value(), ui->heightSpinBox->value(),
        ui->depthSpinBox->value());

    return box;
}

SynapseReviewInputDialog::~SynapseReviewInputDialog()
{
    delete ui;
}
