#ifndef TIPDETECTORRUNNER_H
#define TIPDETECTORRUNNER_H

#include <QString>

#include "dvid/zdvidtarget.h"
#include "geometry/zintpoint.h"

class TipDetectorRunner
{
public:
    TipDetectorRunner();

    void setPoint(ZIntPoint point);
    void setBodyId(uint64_t bodyId);
    void setDvidTarget(ZDvidTarget target);
    void run();

private:
    ZIntPoint m_point;
    uint64_t m_bodyId;
    ZDvidTarget m_target;
    QString m_pythonPath;
    QString m_scriptPath;
};

#endif // TIPDETECTORRUNNER_H
