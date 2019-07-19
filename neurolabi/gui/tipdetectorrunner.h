#ifndef TIPDETECTORRUNNER_H
#define TIPDETECTORRUNNER_H

#include <QString>
#include <QJsonObject>

#include "dvid/zdvidtarget.h"
#include "geometry/zintpoint.h"

class TipDetectorRunner
{
public:
    TipDetectorRunner();

    void setPoint(ZIntPoint point);
    void setBodyId(uint64_t bodyId);
    void setRoI(QString roi);
    void setDvidTarget(ZDvidTarget target);
    QJsonObject getOutput();
    void run();

private:
    ZIntPoint m_point;
    uint64_t m_bodyId;
    QString m_roi;
    ZDvidTarget m_target;
    QJsonObject m_output;
};

#endif // TIPDETECTORRUNNER_H
