#include "tipdetectordialog.h"
#include "ui_tipdetectordialog.h"

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>

TipDetectorDialog::TipDetectorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TipDetectorDialog)
{
    ui->setupUi(this);

    setStatus(NOT_RUNNING);

    // UI connections
    connect(ui->runButton, SIGNAL(clicked(bool)), this, SLOT(onRunButton()));

    // process connections
    connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
        this, SLOT(onProcessFinished(int, QProcess::ExitStatus)));
    connect(&m_process, SIGNAL(errorOccurred(QProcess::ProcessError)),
        this, SLOT(onProcessError(QProcess::ProcessError)));
}

void TipDetectorDialog::onRunButton() {
    QStringList args;
    args << QString::fromStdString(m_target.getAddressWithPort());
    args << QString::fromStdString(m_target.getUuid());
    args << QString::number(m_bodyID);
    args << QString::fromStdString(m_target.getTodoListName());

    QString currentRoi = ui->roiMenu->currentText();
    if (currentRoi != "(none)") {
        args << "--roi";
        args << currentRoi;
    }

    // adjust UI for run state and go!
    ui->runButton->setEnabled(false);
    ui->roiMenu->setEnabled(false);
    setStatus(RUNNING);

    m_process.start("marktips", args);
}

void TipDetectorDialog::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    setStatus(FINISHED);
    QString output = m_process.readAllStandardOutput();
    QString errorOutput = m_process.readAllStandardError();
    QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
    if (doc.isNull()) {
        if (output.isEmpty()) {
            output = "(no output)";
        }
        setMessage("Tip detection script returned unparseable output!  std out: " +
                   output + "; std err: " + errorOutput);
    } else {
        setMessage("Script ran successfully!");
        QJsonObject data = doc.object();
        ui->todoLabel->setText(QString::number(data["nplaced"].toInt()));
        ui->locationsLabel->setText(QString::number(data["nlocations"].toInt()));
        ui->locationsRoiLabel->setText(QString::number(data["nlocationsRoI"].toInt()));
        ui->timeLabel->setText(QString::number(data["ttotal"].toDouble()) + "s");
    }
}

void TipDetectorDialog::onProcessError(QProcess::ProcessError error) {
    setStatus(ERROR);
    QString message = "Tip detection script failed to run properly!";
    QString errorOutput = m_process.readAllStandardError();
    if (!errorOutput.isEmpty()) {
        message += ("  Error: " + errorOutput);
    }
    setMessage(message);
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
