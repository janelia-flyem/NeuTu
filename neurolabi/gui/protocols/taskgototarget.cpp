#include "taskgototarget.h"

TaskGotoTarget::TaskGotoTarget(TargetType targetType)
{
    m_targetType = targetType;
}

TaskGotoTarget::TargetType TaskGotoTarget::targetType() const
{
    return m_targetType;
}

uint64_t TaskGotoTarget::bodyID() const
{
    return m_bodyID;
}

ZIntPoint TaskGotoTarget::point() const
{
    return m_point;
}

void TaskGotoTarget::setBodyID(const uint64_t &bodyID)
{
    m_bodyID = bodyID;
}

void TaskGotoTarget::setPoint(const ZIntPoint &point)
{
    m_point = point;
}
