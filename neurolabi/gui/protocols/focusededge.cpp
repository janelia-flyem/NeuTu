#include "focusededge.h"

#include <stdexcept>

# include "zintpoint.h"

#include "zjsonobject.h"
#include "zjsonparser.h"

#include "dvid/zdvidannotation.h"

/*
 * this class holds the info for an edge in the focused protocol
 *
 * you only need one of the two point annotations to define the edge,
 * because the points have the same weight/examiner, and they have
 * a relation to each other
 */
FocusedEdge::FocusedEdge() {

}

FocusedEdge::FocusedEdge(ZJsonObject edge)
{
    ZDvidAnnotation ann;
    ann.loadJsonObject(edge, FlyEM::LOAD_PARTNER_LOCATION);

    m_firstPoint = ZJsonParser::toIntPoint(edge["Pos"]);
    m_lastPoint = ann.getPartners()[0];

    ZJsonObject properties = edge.value("Prop");
    m_weight = ZJsonParser::numberValue(properties["weight"]);
    m_examiner = ZJsonParser::stringValue(properties["examiner"]);

    // the body IDs are not available yet, so fill in with invalid value
    m_firstBodyID = -1;
    m_lastBodyID = -1;
}

// ---------- constants ----------
// text glyphs for use in table
const std::string FocusedEdge::GLYPH_CONNECTED = "-----";
const std::string FocusedEdge::GLYPH_UNCONNECTED = "--X--";
const std::string FocusedEdge::GLYPH_UNKNOWN = "--?--";


ZIntPoint FocusedEdge::getFirstPoint() const
{
    return m_firstPoint;
}

ZIntPoint FocusedEdge::getLastPoint() const
{
    return m_lastPoint;
}

ZIntPoint FocusedEdge::getOtherPoint(ZIntPoint point) {
    if (point == m_firstPoint) {
        return m_lastPoint;
    } else if (point == m_lastPoint) {
        return m_firstPoint;
    } else {
        // should never happen
        throw std::invalid_argument("input point is not in edge");
    }
}

double FocusedEdge::getWeight() const
{
    return m_weight;
}

std::string FocusedEdge::getExaminer() const
{
    return m_examiner;
}

void FocusedEdge::setExaminer(const std::string &examiner)
{
    m_examiner = examiner;
}

std::string FocusedEdge::getTimeExamined() const
{
    return m_timeExamined;
}

uint64_t FocusedEdge::getFirstBodyID() const
{
    return m_firstBodyID;
}

void FocusedEdge::setFirstBodyID(const uint64_t &firstBodyID)
{
    m_firstBodyID = firstBodyID;
}

uint64_t FocusedEdge::getLastBodyID() const
{
    return m_lastBodyID;
}

void FocusedEdge::setLastBodyID(const uint64_t &lastBodyID)
{
    m_lastBodyID = lastBodyID;
}

bool FocusedEdge::isConnected() {
    return m_firstBodyID == m_lastBodyID && m_firstBodyID != -1UL;
}

bool FocusedEdge::isExamined() {
    return m_examiner != "";
}

std::string FocusedEdge::getConnectionTextIcon() {
    if (isConnected()) {
        return GLYPH_CONNECTED;
    } else if (isExamined()) {
        // not connected but examined = someone said it's unconnected
        return GLYPH_UNCONNECTED;
    } else {
        // not connected, not examined = unknown
        return GLYPH_UNKNOWN;
    }
}


