#include "focusedpath.h"

#include "focusedpathprotocol.h"
#include "focusededge.h"
#include "zintpoint.h"
#include "zjsonparser.h"

#include "flyem/zflyemproofdoc.h"
#include "dvid/zdvidannotation.h"
#include "dvid/zdvidreader.h"

/*
 * this class is a lightweight container for a list of edges = a path
 * between bodies; it does minimal loading and no saving itself
 *
 * see DVID data format here: https://wiki.janelia.org/wiki/display/flyem/NeuTu+focused+protocol+DVID+data+formats
 *
 */
FocusedPath::FocusedPath() {
    // I hate C++
}

FocusedPath::FocusedPath(std::string pathID, ZJsonObject path)
{
    m_pathID = pathID;

    m_firstPoint = ZJsonParser::toIntPoint(path["point1"]);
    m_lastPoint = ZJsonParser::toIntPoint(path["point2"]);
    m_probability = ZJsonParser::numberValue(path["probability"]);

    // NOTE: there was an ambiguity in our format that led to this conditional parsing
    // I intended the path and edge IDs to be arbitrary but implied they could/should be
    //  integers in the format; as a result, in this case, the edge list comes back
    //  as a list of ints instead of strings, as intended; so account for that
    //  difference (but hopefully in the future, it'll be strings)

    ZJsonArray edgeList(path.value("edges"));
    for (size_t i=0; i<edgeList.size(); i++) {
        if (!ZJsonParser::isInteger(edgeList.at(i))) {
            // string
            m_edgeIDs.append(QString::fromUtf8(ZJsonParser::stringValue(edgeList.at(i))));
        } else {
            // int
            m_edgeIDs.append(QString::number(ZJsonParser::integerValue(edgeList.at(i))));
        }
    }

    // we do not load the actual edges right away
}

bool FocusedPath::operator ==(const FocusedPath& other) const {
    // needed to get QList to behave
    return m_firstPoint == other.getFirstPoint() && m_lastPoint == other.getLastPoint();
}

std::string FocusedPath::getPathID() const {
    return m_pathID;
}

ZIntPoint FocusedPath::getFirstPoint() const {
    return m_firstPoint;
}

ZIntPoint FocusedPath::getLastPoint() const {
    return m_lastPoint;
}

uint64_t FocusedPath::getFirstBodyID() const {
    if (hasEdges()) {
        return m_bodyIDs[getFirstPoint()];
    } else {
        return -1;
    }
}

uint64_t FocusedPath::getLastBodyID() const {
    if (hasEdges()) {
        return m_bodyIDs[getLastPoint()];
    } else {
        return -1;
    }
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

int FocusedPath::getNumEdges() const {
    return m_edgePoints.size();
}

void FocusedPath::loadEdges(ZDvidReader& reader, ZFlyEmProofDoc * doc, std::string edgeInstance) {
    // unfortunately, no way to bulk load key-value right now

    m_edgeMap.clear();
    m_edgePoints.clear();

    foreach(QString edgeID, m_edgeIDs) {
        const QByteArray &temp = reader.readKeyValue(QString::fromStdString(edgeInstance), edgeID);
        ZJsonObject edgeData;
        edgeData.decodeString(temp.data());

        FocusedEdge edge(edgeID.toStdString(), edgeData);
        m_edgePoints.append(edge.getFirstPoint());
        m_edgeMap[edge.getFirstPoint()] = edge;
        m_edgeMap[edge.getLastPoint()] = edge;
    }

    updateBodyIDs(doc);
}

void FocusedPath::updateBodyIDs(ZFlyEmProofDoc * doc) {
    std::vector<ZIntPoint> points;
    foreach(ZIntPoint point, m_edgePoints) {
        points.push_back(point);
        points.push_back(m_edgeMap[point].getOtherPoint(point));
    }

    // load all the body IDs at the points
    m_bodyIDs.clear();
    std::vector<uint64_t> bodyIDs = doc->getBodyId(points);
    for (size_t i=0; i<points.size(); i++) {
        m_bodyIDs[points[i]] = bodyIDs[i];
    }

    // fill in body IDs in edges
    foreach(ZIntPoint point, m_edgePoints) {
        m_edgeMap[point].setFirstBodyID(m_bodyIDs[point]);
        ZIntPoint otherPoint = m_edgeMap[point].getOtherPoint(point);
        m_edgeMap[point].setLastBodyID(m_bodyIDs[otherPoint]);
    }

}

bool FocusedPath::hasEdges() const {
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

bool FocusedPath::anyBrokenEdges() {
    // an edge is broken if it's unconnected and examined (meaning
    //  someone explicitly said it's not connected); do we have any?
    foreach(ZIntPoint point, m_edgePoints) {
        FocusedEdge edge = getEdge(point);
        if (!edge.isConnected() && edge.isExamined()) {
            return true;
        }
    }
    return false;
}

std::string FocusedPath::getConnectionTextIcon() {
    if (isConnected()) {
        return FocusedEdge::GLYPH_CONNECTED;
    } else if (getNumUnexaminedEdges() > 0) {
        return FocusedEdge::GLYPH_UNKNOWN;
    } else {
        return FocusedEdge::GLYPH_UNCONNECTED;
    }
}

int FocusedPath::getNumUnexaminedEdges() {
    int total = 0;
    foreach (ZIntPoint point, m_edgePoints) {
        if (!m_edgeMap[point].isExamined()) {
            total += 1;
        }
    }
    return total;
}

int FocusedPath::getFirstUnexaminedEdgeIndex() {
    for (int i=0; i<m_edgePoints.size(); i++) {
        if (!getEdge(i).isExamined()) {
            return i;
        }
    }
    return -1;
}

void FocusedPath::printInfo() {
    std::cout << "FocusedPath from " << getFirstPoint().toString() << " to " << getLastPoint().toString() << std::endl;
    std::cout << "  body IDs: " << getFirstBodyID() << " to " << getLastBodyID() << std::endl;
    std::cout << "  #edges: " << getNumEdges() << std::endl;
    std::cout << "  connected: " << isConnected() << std::endl;
}












