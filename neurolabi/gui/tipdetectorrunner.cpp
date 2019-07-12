#include "tipdetectorrunner.h"

#include <QCoreApplication>
#include <QJsonDocument>

#include "widgets/zpythonprocess.h"

TipDetectorRunner::TipDetectorRunner()
{

}

void TipDetectorRunner::setBodyId(uint64_t bodyId) {
    m_bodyId = bodyId;
}

void TipDetectorRunner::setPoint(ZIntPoint point) {
    m_point = point;
}

void TipDetectorRunner::setDvidTarget(ZDvidTarget target) {
    m_target = target;
}

QJsonObject TipDetectorRunner::getOutput() {
    return m_output;
}

void TipDetectorRunner::run() {

    // for now, use ZPythonProcess even though not ideal; we should just
    //  use QProcess directly, and start it in a thread as well

    // we're running a standalone Python script whereas ZPythonProcess
    //  expects to call the script through Python; so instead, pretend
    //  the script is Python, the first arg is the script name, and add
    //  the other args as usual:

    ZPythonProcess process;
    process.setPythonPath("marktips");
    process.setScript(QString::fromStdString(m_target.getAddressWithPort()));
    process.addArg(QString::fromStdString(m_target.getUuid()));
    process.addArg(QString::number(m_bodyId));
    process.addArg(QString::fromStdString(m_target.getTodoListName()));

    bool status = process.run(false);
    if (status) {
        // the script return should be in json; if not, it's a serious failure,
        //  and we'll echo the output we did get; otherwise, just pass
        //  the script output to the calling routine
        QString output = process.getRawOutput();
        QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
        if (doc.isNull()) {
            m_output["status"] = false;
            m_output["message"] = "Tip detection script returned unparseable output: " + output;
        } else {
            m_output = doc.object();
        }

    } else {
        // this is the system-level failure branch
        m_output["status"] = false;
        m_output["message"] = "Tip detection script failed to run properly!";
    }
}


