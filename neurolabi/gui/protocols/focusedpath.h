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
    FocusedPath(std::string pathID, ZJsonObject path);

    bool operator==(const FocusedPath&) const;

    std::string getPathID() const;
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
    void loadEdges(ZDvidReader& reader, std::string edgeInstance);
    bool isConnected();
    std::string getConnectionTextIcon();
    int getNumUnexaminedEdges();
    int getFirstUnexaminedEdgeIndex();    
    void updateBodyIDs(ZDvidReader &reader);

private:
    std::string m_pathID;
    ZIntPoint m_firstPoint;
    ZIntPoint m_lastPoint;

    QStringList m_edgeIDs;

    QList<ZIntPoint> m_edgePoints;
    QMap<ZIntPoint, FocusedEdge> m_edgeMap;
    QMap<ZIntPoint, uint64_t> m_bodyIDs;
    double m_probability;
};

#endif // FOCUSEDPATH_H
