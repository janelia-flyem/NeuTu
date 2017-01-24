#ifndef FOCUSEDPATH_H
#define FOCUSEDPATH_H

#include "focusededge.h"
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
    uint64_t getFirstBodyID() const;
    uint64_t getLastBodyID() const;
    double getProbability();
    void setProbability(double probability);
    int getNumEdges() const;
    FocusedEdge getEdge(ZIntPoint point);
    FocusedEdge getEdge(int i);
    bool hasEdges() const;
    void loadEdges(ZDvidReader& reader, std::string instance);
    bool isConnected();
    std::string getConnectionTextIcon();
    int getNumUnexaminedEdges();
    int getFirstUnexaminedEdgeIndex();

private:
    ZIntPoint m_firstPoint;
    ZIntPoint m_lastPoint;
    QList<ZIntPoint> m_edgePoints;
    QMap<ZIntPoint, FocusedEdge> m_edgeMap;
    QMap<ZIntPoint, uint64_t> m_bodyIDs;
    double m_probability;
};

#endif // FOCUSEDPATH_H
