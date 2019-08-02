#include "tipdetectordialog.h"
#include "ui_tipdetectordialog.h"

#include <QApplication>
#include <QJsonObject>

#include "tipdetectorrunner.h"

TipDetectorDialog::TipDetectorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TipDetectorDialog)
{
    ui->setupUi(this);

    setStatus(NOT_RUNNING);

    connect(ui->runButton, SIGNAL(clicked(bool)), this, SLOT(onRunButton()));

}

void TipDetectorDialog::onRunButton() {

    TipDetectorRunner runner;
    runner.setBodyId(m_bodyID);
    QString currentRoi = ui->roiMenu->currentText();
    if (currentRoi != "(none)") {
        runner.setRoI(currentRoi);
    }
    runner.setDvidTarget(m_target);

    // adjust UI for run state
    ui->runButton->setEnabled(false);
    ui->roiMenu->setEnabled(false);
    setStatus(RUNNING);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // this doesn't force the widgets to update (label and button):
    //  (the cursor changes properly, though)
    // QApplication::processEvents();

    runner.run();

    // we're back
    QApplication::restoreOverrideCursor();

    // display some output
    QJsonObject output = runner.getOutput();

    if (output["status"].toBool()) {
        setStatus(FINISHED);
        ui->todoLabel->setText(QString::number(output["nplaced"].toInt()));
        ui->locationsLabel->setText(QString::number(output["nlocations"].toInt()));
        ui->locationsRoiLabel->setText(QString::number(output["nlocationsRoI"].toInt()));
        ui->timeLabel->setText(QString::number(output["ttotal"].toDouble()) + "s");
        setMessage("Script ran successfully!");
    } else {
        setStatus(ERROR);
        setMessage(output["message"].toString());
    }
}

void TipDetectorDialog::setMessage(QString message) {
    ui->messageText->setHtml(message);
}

void TipDetectorDialog::setStatus(ScriptStatus status) {
    QString statusString;
    QString color;
    switch (status) {
    case NOT_RUNNING:
        statusString = "Not running";
        color = "silver";
        break;
    case RUNNING:
        statusString = "Running";
        color = "teal";
        break;
    case FINISHED:
        statusString = "Finished";
        color = "green";
        break;
    case ERROR:
        statusString = "Error";
        color = "orange";
        break;
    default:
        statusString = "";
    }
    ui->statusLabel->setText(statusString);
    ui->statusLabel->setStyleSheet("QLabel { background-color : " + color + "; }");

    // none of these make the label get updated, ugh
    // ui->statusLabel->repaint();
    // ui->statusLabel->update();
    // QApplication::processEvents();
}

void TipDetectorDialog::setDvidTarget(ZDvidTarget target) {
    m_target = target;
}

void TipDetectorDialog::setBodyID(uint64_t bodyID) {
    m_bodyID = bodyID;
    ui->bodyIDLabel->setText(QString::number(bodyID));
}

void TipDetectorDialog::setRoiList(QStringList roiList) {
    roiList.insert(0, "(none)");
    ui->roiMenu->addItems(roiList);
}

TipDetectorDialog::~TipDetectorDialog()
{
    delete ui;
}
