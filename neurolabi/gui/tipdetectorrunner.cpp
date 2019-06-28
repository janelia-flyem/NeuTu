#include "tipdetectorrunner.h"

#include <QCoreApplication>
#include <QJsonDocument>

#include "widgets/zpythonprocess.h"

TipDetectorRunner::TipDetectorRunner()
{
    // my local Fly EM conda env that has dvid_tools installed
    m_pythonPath = "/Users/olbrisd/projects/flyem/miniconda/envs/neutu-dvidtools/bin/python";

    // this doesn't work on Mac
    // m_scriptPath = QCoreApplication::applicationDirPath()+"/../python/detect_tips.py";

    // my local tip detection script, within NeuTu source:
    m_scriptPath = "/Users/olbrisd/projects/flyem/NeuTu/neutube/neurolabi/python/detect_tips.py";

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
    ZPythonProcess process;
    process.setPythonPath(m_pythonPath);
    process.setScript(m_scriptPath);

    process.addArg(QString::fromStdString(m_target.getAddressWithPort()));
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


