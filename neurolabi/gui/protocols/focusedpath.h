#ifndef FOCUSEDPATH_H
#define FOCUSEDPATH_H

#include "zintpoint.h"

#include "dvid/zdvidannotation.h"



class FocusedPath
{
public:
    FocusedPath(ZDvidAnnotation annotation);

    ZIntPoint getFirstPoint();
    ZIntPoint getLastPoint();
    double getProbability();
    void setProbability(double probability);
    QList<ZIntPoint> getEdgePoints();

private:
    ZIntPoint m_firstpoint;
    ZIntPoint m_lastpoint;
    QList<ZIntPoint> m_edgePoints;
    // something to hold edges, probably a map between points and edge class instances
    double m_probability;
};

#endif // FOCUSEDPATH_H
