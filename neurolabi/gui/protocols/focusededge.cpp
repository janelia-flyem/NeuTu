#include "focusededge.h"

#include <iostream>
#include <stdexcept>

#include <QDateTime>

# include "zintpoint.h"

#include "zjsonobject.h"
#include "zjsonparser.h"

#include "dvid/zdvidannotation.h"
#include "dvid/zdvidwriter.h"

/*
 * this class holds the info for an edge in the focused protocol
 *
 * you only need one of the two point annotations to define the edge,
 * because the points have the same weight/examiner, and they have
 * a relation to each other
 *
 * see DVID json format here: https://wiki.janelia.org/wiki/display/flyem/NeuTu+focused+protocol+DVID+data+formats
 *
 */
FocusedEdge::FocusedEdge() {

}

FocusedEdge::FocusedEdge(std::string edgeID, ZJsonObject edge)
{
    m_edgeID = edgeID;

    m_firstPoint = ZJsonParser::toIntPoint(edge["point1"]);
    m_lastPoint = ZJsonParser::toIntPoint(edge["point2"]);

    m_weight = ZJsonParser::numberValue(edge["weight"]);
    m_examiner = ZJsonParser::stringValue(edge["examiner"]);

    // we don't need the time examined for this task

    // the body IDs are not available yet, so fill in with invalid value
    m_firstBodyID = -1;
    m_lastBodyID = -1;
}

// ---------- constants ----------
// text glyphs for use in table
const std::string FocusedEdge::GLYPH_CONNECTED = "-----";
const std::string FocusedEdge::GLYPH_UNCONNECTED = "--X--";
const std::string FocusedEdge::GLYPH_UNKNOWN = "--?--";

// format used in "time examined" field
const std::string FocusedEdge::DATETIME_FORMAT = "yyyyMMddhhmmss";

// keys for DVID object
const std::string KEY_POINT1 = "point1";
const std::string KEY_POINT2 = "point2";
const std::string KEY_WEIGHT = "weight";
const std::string KEY_EXAMINER = "examiner";
const std::string KEY_TIME_EXAMINED = "time-examined";


std::string FocusedEdge::getEdgeID() const {
    return m_edgeID;
}

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

void FocusedEdge::setTimeExaminedNow() {
    // sets the time examined to the current time
    m_timeExamined = QDateTime.currentDateTime().toString(DATETIME_FORMAT);
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

void FocusedEdge::writeEdge(ZDvidWriter& writer, std::string instance) {
    ZJsonObject ann;

    ZJsonArray point1;
    point1.append(getFirstPoint().getX());
    point1.append(getFirstPoint().getY());
    point1.append(getFirstPoint().getZ());
    ann.setEntry(KEY_POINT1, point1);

    ZJsonArray point2;
    point2.append(getLastPoint().getX());
    point2.append(getLastPoint().getY());
    point2.append(getLastPoint().getZ());
    ann.setEntry(KEY_POINT2, point2);

    ann.setEntry(KEY_WEIGHT, getWeight());
    ann.setEntry(KEY_EXAMINER, getExaminer());
    ann.setEntry(KEY_TIME_EXAMINED, getTimeExamined());

    writer.writePointAnnotation(instance, ann);
}
