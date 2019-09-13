#include "tipdetectordialog.h"
#include "ui_tipdetectordialog.h"

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

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

void TipDetectorDialog::disableRunUI() {
    ui->runButton->setEnabled(false);
    ui->roiMenu->setEnabled(false);
    ui->excludeRoiMenu->setEnabled(false);
}

void TipDetectorDialog::enableRunUI() {
    ui->runButton->setEnabled(true);
    ui->roiMenu->setEnabled(true);
    ui->excludeRoiMenu->setEnabled(true);
}

void TipDetectorDialog::clearOutput() {
    ui->timeLabel->clear();
    ui->todoLabel->clear();
    ui->locationsLabel->clear();
    ui->locationsRoiLabel->clear();
    ui->messageText->clear();
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

    QString excludeRoi = ui->excludeRoiMenu->currentText();
    if (excludeRoi != "(none)") {
        args << "--excluded-roi";
        args << excludeRoi;
    }

    clearOutput();
    disableRunUI();
    setStatus(RUNNING);

#ifdef _DEBUG_
    qDebug() << "marktips" << args;
#endif

    m_process.start("marktips", args);
}

void TipDetectorDialog::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    enableRunUI();
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
        QJsonObject data = doc.object();
        if (data["status"].toBool()) {
            setStatus(FINISHED);
            setMessage("Script ran successfully!");
            ui->todoLabel->setText(QString::number(data["nplaced"].toInt()));
            ui->locationsLabel->setText(QString::number(data["nlocations"].toInt()));
            ui->locationsRoiLabel->setText(QString::number(data["nlocationsRoI"].toInt()));
            ui->timeLabel->setText(QString::number(data["ttotal"].toDouble()) + "s");
        } else {
            setStatus(ERROR);
            QString message = data["message"].toString();
            setMessage(message);
        }
    }
}

void TipDetectorDialog::onProcessError(QProcess::ProcessError error) {
    enableRunUI();
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

TipDetectorDialog::ScriptStatus TipDetectorDialog::getStatus() {
    return m_status;
}

void TipDetectorDialog::setStatus(ScriptStatus status) {
    m_status = status;
    updateStatus();
}

void TipDetectorDialog::updateStatus() {
    QString statusString;
    QString fgColor;
    QString bgColor;
    switch (m_status) {
    case NOT_RUNNING:
        statusString = "Not running";
        fgColor = "black";
        bgColor = "silver";
        break;
    case RUNNING:
        statusString = "Running";
        fgColor = "white";
        bgColor = "teal";
        break;
    case FINISHED:
        statusString = "Finished";
        fgColor = "white";
        bgColor = "green";
        break;
    case ERROR:
        statusString = "Error";
        fgColor = "black";
        bgColor = "orange";
        break;
    default:
        statusString = "";
    }
    ui->statusLabel->setText(statusString);
    ui->statusLabel->setStyleSheet("QLabel { color : " + fgColor + "; background-color : " + bgColor + "; }");
}

void TipDetectorDialog::setDvidTarget(ZDvidTarget target) {
    m_target = target;
}

void TipDetectorDialog::setBodyID(uint64_t bodyID) {
    // this is called when the user selects a new body and chooses "Tip
    //  Detection Dialog" from the right-click menu

    // reset the UI to the new body, unless we're running!
    if (getStatus() != RUNNING) {
        m_bodyID = bodyID;
        ui->bodyIDLabel->setText(QString::number(bodyID));
        setStatus(NOT_RUNNING);
        clearOutput();
        resetRoiMenus();
    }
}

void TipDetectorDialog::setRoiList(QStringList roiList) {
    // don't change things while we're running!
    if (getStatus() != RUNNING) {
        roiList.insert(0, "(none)");
        ui->roiMenu->addItems(roiList);
        ui->excludeRoiMenu->addItems(roiList);
        resetRoiMenus();
    }
}

void TipDetectorDialog::resetRoiMenus() {
    ui->roiMenu->setCurrentIndex(0);
    ui->excludeRoiMenu->setCurrentIndex(0);
}

void TipDetectorDialog::applicationQuitting() {
    // this isn't 100% necessary...the Python process won't block the application
    //  from quitting
    // you could even argue that you *don't* want the Python process to die
    //  when NeuTu does...maybe you want tip detection to continue for next time
    if (m_process.state() != QProcess::NotRunning) {
        m_process.kill();
    }
}

TipDetectorDialog::~TipDetectorDialog()
{
    delete ui;
}
