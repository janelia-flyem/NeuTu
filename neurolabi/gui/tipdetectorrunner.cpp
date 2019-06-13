#include "tipdetectorrunner.h"

#include <QJsonDocument>
#include <QJsonObject>

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

void TipDetectorRunner::run() {

    // first we cheat and hardcode everything
    ZPythonProcess process;

    // probably ought to set up Python and script path in constructor
    // my local Fly EM conda env that has dvid_tools installed
    process.setPythonPath("/Users/olbrisd/projects/flyem/miniconda/envs/neutu-dvidtools/bin/python");

    // my local tip detection script, within NeuTu source:
    process.setScript("/Users/olbrisd/projects/flyem/NeuTu/neutube/neurolabi/python/detect_tips.py");

    process.addArg(QString::fromStdString(m_target.getAddressWithPort()));
    process.addArg(QString::fromStdString(m_target.getUuid()));
    process.addArg(QString::number(m_bodyId));
    process.addArg(QString::fromStdString(m_target.getTodoListName()));

    bool status = process.run(false);
    if (status) {
        // the script return should be in json; if not, it's a serious failure,
        //  and we'll echo the output we did get
        // otherwise we expect a json dict with a "status" and "message" at a
        //  minimum; I intend to add more details to a successful return that
        //  the UI can present as appropriate (eg, number of tips added)
        QString output = process.getRawOutput();
        QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
        if (doc.isNull()) {
            // undefined behavior
            std::cout << "script returned unexpected output; output:" << std::endl;
            std::cout << output.toStdString() << std::endl;
            return;
        }

        QJsonObject obj = doc.object();
        if (!obj["status"].toBool()) {
            // failure, but one with a message
            std::cout << "script returned failure: " << obj["message"].toString().toStdString() << std::endl;


        } else {
            // success
            std::cout << "script returned success: " << obj["message"].toString().toStdString() << std::endl;



        }

    } else {
        // this is the system-level failure branch
        std::cout << "script failed to run properly" << std::endl;
    }

}


