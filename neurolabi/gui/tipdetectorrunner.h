#ifndef TIPDETECTORRUNNER_H
#define TIPDETECTORRUNNER_H

#include "geometry/zintpoint.h"

class TipDetectorRunner
{
public:
    TipDetectorRunner();

    void setPoint(ZIntPoint point);
    void setBodyId(uint64_t bodyId);
    void run();

private:
    ZIntPoint m_point;
    uint64_t m_bodyId;
};

#endif // TIPDETECTORRUNNER_H
