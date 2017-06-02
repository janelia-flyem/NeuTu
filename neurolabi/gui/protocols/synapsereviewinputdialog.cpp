#include "synapsereviewinputdialog.h"
#include "ui_synapsereviewinputdialog.h"

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
        return SynapseReviewInputOptions::BY_BODYID;
    } else if (ui->roiRadioButton->isChecked()) {
        return SynapseReviewInputOptions::BY_ROI;
    } else if (ui->volumeRadioButton->isChecked()) {
        return SynapseReviewInputOptions::BY_VOLUME;
    }
}

QString SynapseReviewInputDialog::getRoI() {
    // didn't name the input boxes yet!
}

SynapseReviewInputDialog::~SynapseReviewInputDialog()
{
    delete ui;
}
