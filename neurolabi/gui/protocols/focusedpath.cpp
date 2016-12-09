#include "focusedpath.h"

#include "focusedpathprotocol.h"
#include "zintpoint.h"

#include "dvid/zdvidannotation.h"

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

ZIntPoint FocusedPath::getFirstPoint() {
    return m_firstpoint;
}

ZIntPoint FocusedPath::getLastPoint() {
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


// methods:

// --> keep the original annotation to make it easier to re-save?
// save (for changing probability)
// load edges

// below: really, its state is broken, connected, or unknown
// isvalid (has prob > 0)
// isbroken (at least one edge has been split and/or invalidated by user)
// isconnected (same body at body ends)


