#ifndef TASKGOTOTARGET_H
#define TASKGOTOTARGET_H

#include "zintpoint.h"

class TaskGotoTarget
{
public:
    enum TargetType {
        BODY,
        POINT
    };

    TaskGotoTarget(TargetType targetType);
    TargetType targetType() const;
    uint64_t bodyID() const;
    void setBodyID(const uint64_t &bodyID);
    ZIntPoint point() const;
    void setPoint(const ZIntPoint &point);

private:
    TargetType m_targetType;
    uint64_t m_bodyID;
    ZIntPoint m_point;
};

#endif // TASKGOTOTARGET_H
