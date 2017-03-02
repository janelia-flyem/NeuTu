#ifndef FOCUSEDEDGE_H
#define FOCUSEDEDGE_H

#include "zintpoint.h"

#include "zjsonobject.h"


class FocusedEdge
{
public:
    FocusedEdge();
    FocusedEdge(std::string edgeID, ZJsonObject edge);

    static const std::string GLYPH_CONNECTED;
    static const std::string GLYPH_UNCONNECTED;
    static const std::string GLYPH_UNKNOWN;

    std::string getEdgeID() const;
    ZIntPoint getFirstPoint() const;
    ZIntPoint getLastPoint() const;
    ZIntPoint getOtherPoint(ZIntPoint point);
    double getWeight() const;
    std::string getExaminer() const;
    void setExaminer(const std::string &examiner);
    std::string getTimeExamined() const;
    uint64_t getFirstBodyID() const;
    void setFirstBodyID(const uint64_t &firstBodyID);
    uint64_t getLastBodyID() const;
    void setLastBodyID(const uint64_t &lastBodyID);
    bool isConnected();
    bool isExamined();
    std::string getConnectionTextIcon();

private:
    std::string m_edgeID;
    ZIntPoint m_firstPoint;
    ZIntPoint m_lastPoint;
    double m_weight;
    std::string m_examiner;
    std::string m_timeExamined;
    uint64_t m_firstBodyID;
    uint64_t m_lastBodyID;
};

#endif // FOCUSEDEDGE_H
