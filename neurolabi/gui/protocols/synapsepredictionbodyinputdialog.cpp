#include "synapsepredictionbodyinputdialog.h"
#include "ui_synapsepredictionbodyinputdialog.h"

#include "synapsepredictionprotocol.h"

SynapsePredictionBodyInputDialog::SynapsePredictionBodyInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SynapsePredictionBodyInputDialog)
{
    ui->setupUi(this);

    // body ID must be int >= 1; could be 64 bits; input mask enforces this,
    //  although technically I'm allowing some numbers outside the range on the high side
    ui->bodyIDInput->setInputMask("D0000000000000000000");

}

bool SynapsePredictionBodyInputDialog::hasBodyID() {
    return ui->bodyIDInput->text().size() > 0;
}

uint64_t SynapsePredictionBodyInputDialog::getBodyID() {
    // because of the input mask above, this parsing should always work
    uint64_t bodyID;
    if (hasBodyID()) {
        bool ok;
        bodyID = ui->bodyIDInput->text().toLong(&ok);
        if (ok) {
            return bodyID;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

std::string SynapsePredictionBodyInputDialog::getMode() {
    if (ui->tbarButton->isChecked()) {
        return SynapsePredictionProtocol::SUBVARIATION_BODY_TBAR;
    } else if (ui->psdButton->isChecked()) {
        return SynapsePredictionProtocol::SUBVARIATION_BODY_PSD;
    } else if (ui->bothButton->isChecked()) {
        return SynapsePredictionProtocol::SUBVARIATION_BODY_BOTH;
    } else {
        // should never happen
        return "unknown";
    }
}

SynapsePredictionBodyInputDialog::~SynapsePredictionBodyInputDialog()
{
    delete ui;
}
