#ifndef FOCUSEDEDGE_H
#define FOCUSEDEDGE_H

#include "zintpoint.h"

#include "zjsonobject.h"


class FocusedEdge
{
public:
    FocusedEdge();
    FocusedEdge(ZJsonObject edge);

    ZIntPoint getFirstPoint() const;
    ZIntPoint getLastPoint() const;
    ZIntPoint getOtherPoint(ZIntPoint point);
    double getWeight() const;
    std::string getExaminer() const;
    void setExaminer(const std::string &examiner);
    std::string getTimeExamined() const;

private:
    ZIntPoint m_firstPoint;
    ZIntPoint m_lastPoint;
    double m_weight;
    std::string m_examiner;
    std::string m_timeExamined;
};

#endif // FOCUSEDEDGE_H
