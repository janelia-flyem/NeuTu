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

void TipDetectorRunner::run() {

    std::cout << "TipDetectorRunner::run()" << std::endl;


    // now we test stuff

    // can I run a simple python test script?

    // first we cheat and hardcode everything

    ZPythonProcess process;
    process.setPythonPath("/Users/olbrisd/anaconda/envs/py3/bin/python");
    process.setScript("/Users/olbrisd/projects/flyem/NeuTu/testing/tip-detection/simple.py");

    process.addArg(QString::number(m_bodyId));

    bool status = process.run();
    if (status) {

        // one or the other of the output; they each read the output

        // std::cout << "raw output:" << std::endl;
        // std::cout << process.getRawOutput().toStdString() << std::endl;

        std::cout << "output summary:" << std::endl;
        process.parsePythonOutput();
        process.printOutputSummary();

    } else {
        std::cout << "run failed" << std::endl;
    }



}
