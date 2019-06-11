#include "tipdetectorrunner.h"

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

    // simple test script
    // process.setScript("/Users/olbrisd/projects/flyem/NeuTu/testing/tip-detection/simple.py");



    process.addArg(QString::fromStdString(m_target.getAddressWithPort()));
    process.addArg(QString::fromStdString(m_target.getUuid()));
    process.addArg(QString::number(m_bodyId));
    process.addArg(QString::fromStdString(m_target.getTodoListName()));



    bool status = process.run(false);
    if (status) {
        // at this point the script ran and gave output, but it may not
        //  have run successfully
        QStringList outputLines = process.getRawOutput().split("\n");

        if (outputLines.first() == "error") {
            std::cout << "script returned failure; output:" << std::endl;
            for (QString line: outputLines.mid(1)) {
                std::cout << line.toStdString() << std::endl;
            }



        } else if (outputLines.first() == "success") {
            std::cout << "script returned success; output:" << std::endl;
            for (QString line: outputLines.mid(1)) {
                std::cout << line.toStdString() << std::endl;
            }

            // probably successful returns will be json?



        } else {
            // undefined behavior
            std::cout << "script returned unexpected output; output:" << std::endl;
            for (QString line: outputLines) {
                std::cout << line.toStdString() << std::endl;
            }



        }


    } else {
        // this is the system-level failure branch
        std::cout << "script failed to run properly" << std::endl;
    }

}


