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

    // other UI setup
    m_defaultPalette = QPalette(ui->x1spinBox->palette());
    m_redbgPalette.setColor(QPalette::Base, QColor(255, 128, 128));

    // UI connects
    connect(ui->x1spinBox, SIGNAL(valueChanged(int)), this, SLOT(xRangeChanged(int)));
    connect(ui->x2spinBox, SIGNAL(valueChanged(int)), this, SLOT(xRangeChanged(int)));
    connect(ui->y1spinBox, SIGNAL(valueChanged(int)), this, SLOT(yRangeChanged(int)));
    connect(ui->y2spinBox, SIGNAL(valueChanged(int)), this, SLOT(yRangeChanged(int)));
    connect(ui->z1spinBox, SIGNAL(valueChanged(int)), this, SLOT(zRangeChanged(int)));
    connect(ui->z2spinBox, SIGNAL(valueChanged(int)), this, SLOT(zRangeChanged(int)));

}

// override accept() so we can not accept if invalid
void SynapsePredictionInputDialog::accept() {
    if (isValid()) {
        QDialog::accept();
    }
}

void SynapsePredictionInputDialog::setRoI(QString roi){
    ui->roiInput->setText(roi);
}

QString SynapsePredictionInputDialog::getRoI() {
    return ui->roiInput->text();
}

bool SynapsePredictionInputDialog::xValid() {
    return (ui->x1spinBox->value() < ui->x2spinBox->value());
}

bool SynapsePredictionInputDialog::yValid() {
    return (ui->y1spinBox->value() < ui->y2spinBox->value());
}

bool SynapsePredictionInputDialog::zValid() {
    return (ui->z1spinBox->value() < ui->z2spinBox->value());
}

bool SynapsePredictionInputDialog::isValid() {
    return (xValid() && yValid() && zValid());
}

void SynapsePredictionInputDialog::xRangeChanged(int x) {
    // change background color if invalid entry
    if (!xValid()) {
        ui->x1spinBox->setPalette(m_redbgPalette);
        ui->x2spinBox->setPalette(m_redbgPalette);
    } else {
        ui->x1spinBox->setPalette(m_defaultPalette);
        ui->x2spinBox->setPalette(m_defaultPalette);
    }
}

void SynapsePredictionInputDialog::yRangeChanged(int y) {
    // change background color if invalid entry
    if (!yValid()) {
        ui->y1spinBox->setPalette(m_redbgPalette);
        ui->y2spinBox->setPalette(m_redbgPalette);
    } else {
        ui->y1spinBox->setPalette(m_defaultPalette);
        ui->y2spinBox->setPalette(m_defaultPalette);
    }
}

void SynapsePredictionInputDialog::zRangeChanged(int z) {
    // change background color if invalid entry
    if (!zValid()) {
        ui->z1spinBox->setPalette(m_redbgPalette);
        ui->z2spinBox->setPalette(m_redbgPalette);
    } else {
        ui->z1spinBox->setPalette(m_defaultPalette);
        ui->z2spinBox->setPalette(m_defaultPalette);
    }
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
