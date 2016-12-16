#ifndef FOCUSEDPATH_H
#define FOCUSEDPATH_H

#include "zintpoint.h"

#include "dvid/zdvidannotation.h"
#include "dvid/zdvidreader.h"



class FocusedPath
{
public:
    FocusedPath();
    FocusedPath(ZDvidAnnotation annotation);

    bool operator==(const FocusedPath&) const;

    ZIntPoint getFirstPoint() const;
    ZIntPoint getLastPoint() const;
    double getProbability();
    void setProbability(double probability);
    QList<ZIntPoint> getEdgePoints();
    bool hasEdges();
    void loadEdges(ZDvidReader& reader, std::string instance);
    bool isConnected();
    bool isExamined();

private:
    ZIntPoint m_firstpoint;
    ZIntPoint m_lastpoint;
    QList<ZIntPoint> m_edgePoints;
    QMap<ZIntPoint, ZIntPoint> m_edgePairs;
    QMap<ZIntPoint, ZJsonObject> m_edgeMap;
    QMap<ZIntPoint, uint64_t> m_bodyIDs;
    double m_probability;
};

#endif // FOCUSEDPATH_H
