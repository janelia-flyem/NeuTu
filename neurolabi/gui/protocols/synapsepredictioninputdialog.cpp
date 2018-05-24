#include "synapsepredictioninputdialog.h"
#include "ui_synapsepredictioninputdialog.h"

#include <QPalette>

#include "zintcuboid.h"

/*
 * this class is the input dialog for starting the synapse_prediction protocol
 */
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
  ZIntCuboid box;
  box.setFirstCorner(
        ui->x1spinBox->value(), ui->y1spinBox->value(), ui->z1spinBox->value());
  box.setSize(ui->widthSpinBox->value(), ui->heightSpinBox->value(),
              ui->depthSpinBox->value());

  return box;
}

void SynapsePredictionInputDialog::setVolume(ZIntCuboid volume) {
  ui->x1spinBox->setValue(volume.getFirstCorner().getX());
  ui->y1spinBox->setValue(volume.getFirstCorner().getY());
  ui->z1spinBox->setValue(volume.getFirstCorner().getZ());
  ui->widthSpinBox->setValue(volume.getWidth());
  ui->heightSpinBox->setValue(volume.getHeight());
  ui->depthSpinBox->setValue(volume.getDepth());
}

SynapsePredictionInputDialog::~SynapsePredictionInputDialog()
{
    delete ui;
}
