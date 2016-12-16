#include "focusedpath.h"

#include "focusedpathprotocol.h"
#include "zintpoint.h"
#include "zjsonparser.h"

#include "dvid/zdvidannotation.h"
#include "dvid/zdvidreader.h"

/*
 * this class is a lightweight container for a list of edges = a path
 * between bodies; it doesn't do saves or loads itself
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

QList<ZIntPoint> FocusedPath::getEdgePoints() {
    return m_edgePoints;
}

void FocusedPath::loadEdges(ZDvidReader& reader, std::string instance) {
    // unfortunately, no way to bulk load annotations by point right now

    m_edgeMap.clear();
    m_edgePairs.clear();

    std::vector<ZIntPoint> points;

    foreach(ZIntPoint point, m_edgePoints) {

        // store ZJsonObject, because that's what we need if we want to save back;
        // ZDvidAnn, though, will get us the relation faster
        ZJsonObject edge1 = reader.readAnnotationJson(instance, point);

        ZDvidAnnotation ann;
        ann.loadJsonObject(edge1, FlyEM::LOAD_PARTNER_LOCATION);
        ZIntPoint point2 = ann.getPartners()[0];
        ZJsonObject edge2 = reader.readAnnotationJson(instance, point2);

        m_edgeMap[point] = edge1;
        m_edgeMap[point2] = edge2;
        m_edgePairs[point] = point2;

        points.push_back(point);
        points.push_back(point2);
    }

    // load all the body IDs at the points
    m_bodyIDs.clear();
    std::vector<uint64_t> bodyIDs = reader.readBodyIdAt(points);
    for (size_t i=0; i<points.size(); i++) {
        m_bodyIDs[points[i]] = bodyIDs[i];
    }

}

bool FocusedPath::hasEdges() {
    return m_edgeMap.size() > 0;
}

bool FocusedPath::isConnected() {

    ZIntPoint prevPoint = m_edgePoints.first();
    foreach(ZIntPoint point1, m_edgePoints) {
        // connected to previous edge (could have been split)?
        if (m_bodyIDs[point1] != m_bodyIDs[prevPoint]) {
            return false;
        }

        // connected across edge (could have been done some other time)?
        if (m_bodyIDs[point1] != m_bodyIDs[m_edgePairs[point1]]) {
            return false;
        }

        prevPoint = m_edgePairs[point1];
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







