#include "protocolmetadata.h"

#include <iostream>

#include "neutube.h"

#include "dvid/zdvidreader.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidwriter.h"
#include "zjsonarray.h"
#include "zjsonobject.h"

/*
 * this class stores data about the protocols stored for a user;
 * eg, the currently active protocol
 *
 * data is stored in a DVID key-value as a json object with:
 *
 * active-protocol-name: string with name or ""
 * active-protocol-key: string with key or ""
 *
 */
ProtocolMetadata::ProtocolMetadata(std::string dataName, ZDvidTarget target)
{
    // note that the constructor doesn't read the metadata; you call
    //  read, or fill in values yourself
    m_dvidTarget = target;
    m_dvidDataName = dataName;

    m_metadataKey = NeuTube::GetCurrentUserName() + PROTOCOL_METADATA_SUFFIX;

    clearActive();

    // holds the success status of most recent read/write
    m_ioSuccessful = true;

}

const std::string ProtocolMetadata::PROTOCOL_METADATA_SUFFIX = "-metadata";

/*
 * factory function to read metadata from DVID and create object;
 * check ioSuccessful() for whether there's an error or not;
 * if error, isActive() will be false
 */
ProtocolMetadata ProtocolMetadata::ReadProtocolMetadata(std::string dataName, ZDvidTarget target)
{
    ProtocolMetadata metadata(dataName, target);
    metadata.read();
    return metadata;
}

void ProtocolMetadata::read() {
    ZDvidReader reader;
    if (reader.open(m_dvidTarget)) {

        // read, parse, etc.
        std::cout << "prmeta: read metadata not implemented yet" << std::endl;

    } else {
        // error, return empty
        m_isActive = false;
        m_ioSuccessful = false;
    }
}

void ProtocolMetadata::write()
{
    ZDvidWriter writer;
    if (writer.open(m_dvidTarget)) {
        ZJsonObject data;
        // probably should make keys into constants?  in enum?
        data.setEntry("active-protocol-name", getActiveProtocolName());
        data.setEntry("active-protocol-key", getActiveProtocolKey());
        writer.writeJson(m_dvidDataName, m_metadataKey, data);
    } else {
        m_ioSuccessful = false;
    }
}

void ProtocolMetadata::setActive(std::string name, std::string key) {
    m_activeProtocolName = name;
    m_activeProtocolKey = key;

    m_isActive = true;
}

void ProtocolMetadata::clearActive() {
    m_activeProtocolName = "";
    m_activeProtocolKey = "";

    m_isActive = false;
}

bool ProtocolMetadata::isActive() {
    return m_isActive;
}

std::string ProtocolMetadata::getActiveProtocolName()
{
    if (isActive()) {
        return m_activeProtocolName;
    } else {
        return "";
    }
}

std::string ProtocolMetadata::getActiveProtocolKey()
{
    if (isActive()) {
        return m_activeProtocolKey;
    } else {
        return "";
    }
}
