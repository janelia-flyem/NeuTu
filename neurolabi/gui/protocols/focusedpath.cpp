#include "focusedpath.h"

#include "focusedpathprotocol.h"
#include "focusededge.h"
#include "zintpoint.h"
#include "zjsonparser.h"

#include "dvid/zdvidannotation.h"
#include "dvid/zdvidreader.h"

/*
 * this class is a lightweight container for a list of edges = a path
 * between bodies; it does minimal loading and no saving itself
 */
FocusedPath::FocusedPath() {
    // I hate C++
}

FocusedPath::FocusedPath(ZDvidAnnotation annotation)
{
    m_firstpoint = annotation.getPosition();

    // last point; assume the relation only has one point
    m_lastpoint = ZJsonParser::toIntPoint(((ZJsonObject) annotation.getRelationJson().value(0))["To"]);

    // probability
    m_probability = annotation.getProperty(FocusedPathProtocol::PROPERTY_PROBABILITY.c_str()).toReal();

    // edge points
    ZJsonArray edgeList = ((ZJsonArray) annotation.getProperty(FocusedPathProtocol::PROPERTY_PATH.c_str()));
    for (size_t i=0; i<edgeList.size(); i++) {
        m_edgePoints.append(ZJsonParser::toIntPoint(edgeList.at(i)));
    }

    // we do not load the actual edges right away

}

bool FocusedPath::operator ==(const FocusedPath& other) const {
    // needed to get QList to behave
    return m_firstpoint == other.getFirstPoint() && m_lastpoint == other.getLastPoint();
}

ZIntPoint FocusedPath::getFirstPoint() const {
    return m_firstpoint;
}

ZIntPoint FocusedPath::getLastPoint() const {
    return m_lastpoint;
}

void FocusedPath::setProbability(double probability) {
    m_probability = probability;
}

double FocusedPath::getProbability() {
    return m_probability;
}

FocusedEdge FocusedPath::getEdge(ZIntPoint point) {
    return m_edgeMap[point];
}

FocusedEdge FocusedPath::getEdge(int i) {
    return getEdge(m_edgePoints[i]);
}

int FocusedPath::getNumEdges() {
    return m_edgePoints.size();
}

void FocusedPath::loadEdges(ZDvidReader& reader, std::string instance) {
    // unfortunately, no way to bulk load annotations by point right now

    m_edgeMap.clear();

    std::vector<ZIntPoint> points;

    foreach(ZIntPoint point, m_edgePoints) {

        ZJsonObject jsonEdge = reader.readAnnotationJson(instance, point);
        FocusedEdge edge(jsonEdge);

        m_edgeMap[edge.getFirstPoint()] = edge;
        m_edgeMap[edge.getLastPoint()] = edge;

        points.push_back(edge.getFirstPoint());
        points.push_back(edge.getLastPoint());
    }

    // load all the body IDs at the points
    m_bodyIDs.clear();
    std::vector<uint64_t> bodyIDs = reader.readBodyIdAt(points);
    for (size_t i=0; i<points.size(); i++) {
        m_bodyIDs[points[i]] = bodyIDs[i];
    }

}

bool FocusedPath::hasEdges() {
    return getNumEdges() > 0;
}

bool FocusedPath::isConnected() {

    ZIntPoint prevPoint = m_edgePoints.first();
    foreach(ZIntPoint point, m_edgePoints) {
        // connected to previous edge (could have been split)?
        if (m_bodyIDs[point] != m_bodyIDs[prevPoint]) {
            return false;
        }

        // connected across edge (could have been done some other time)?
        FocusedEdge edge = getEdge(point);
        if (m_bodyIDs[point] != m_bodyIDs[edge.getOtherPoint(point)]) {
            return false;
        }

        prevPoint = edge.getOtherPoint(point);
    }
    return true;
}

bool FocusedPath::isExamined() {
    // look in the json and see if it's been examined; should we check both ends
    //  and verify they match?

    return false;

}

// methods:

// --> keep the original annotation to make it easier to re-save?
// save (for changing probability)
// load body IDs?

// below: really, its state is broken, connected, or unknown
// isvalid (has prob > 0) (?)
// isbroken (at least one edge has been split and/or invalidated by user)







