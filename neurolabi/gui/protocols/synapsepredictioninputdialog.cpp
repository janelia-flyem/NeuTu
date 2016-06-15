#include "synapsepredictioninputdialog.h"
#include "ui_synapsepredictioninputdialog.h"

#include "zintcuboid.h"

SynapsePredictionInputDialog::SynapsePredictionInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SynapsePredictionInputDialog)
{
    ui->setupUi(this);

}

void SynapsePredictionInputDialog::setRoI(QString roi){
    ui->roiInput->setText(roi);
}

QString SynapsePredictionInputDialog::getRoI() {
    return ui->roiInput->text();
}

ZIntCuboid SynapsePredictionInputDialog::getVolume() {
    // no validation yet

    return ZIntCuboid(ui->x1spinBox->value(), ui->y1spinBox->value(), ui->z1spinBox->value(),
                      ui->x2spinBox->value(), ui->y2spinBox->value(), ui->z2spinBox->value());
}

void SynapsePredictionInputDialog::setVolume(ZIntCuboid volume) {
    ui->x1spinBox->setValue(volume.getFirstCorner().getX());
    ui->y1spinBox->setValue(volume.getFirstCorner().getY());
    ui->z1spinBox->setValue(volume.getFirstCorner().getZ());
    ui->x2spinBox->setValue(volume.getLastCorner().getX());
    ui->y2spinBox->setValue(volume.getLastCorner().getY());
    ui->z2spinBox->setValue(volume.getLastCorner().getZ());
}

SynapsePredictionInputDialog::~SynapsePredictionInputDialog()
{
    delete ui;
}
