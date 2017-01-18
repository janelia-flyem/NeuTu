#include "protocolmetadata.h"

#include <iostream>

#include "neutube.h"

#include "dvid/zdvidreader.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidwriter.h"
#include "zjsonarray.h"
#include "zjsonobject.h"
#include "zjsonparser.h"

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

    m_metadataKey = GetUserMetadataKey();

    clearActive();

    // holds the success status of most recent read/write
    m_ioSuccessful = true;

}

/*
 * constants; thank you, C++, for making this a pain
 */
const std::string ProtocolMetadata::PROTOCOL_METADATA_SUFFIX = "-metadata";
const std::string ProtocolMetadata::KEY_PROTOCOL_NAME = "active-protocol-name";
const std::string ProtocolMetadata::KEY_PROTOCOL_KEY = "active-protocol-key";
const std::string ProtocolMetadata::KEY_VERSION = "version";
const int ProtocolMetadata::fileVersion = 1;

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

std::string ProtocolMetadata::GetUserMetadataKey() {
    // return NeuTube::GetCurrentUserName() + PROTOCOL_METADATA_SUFFIX;
    return GetUserMetadataKey(NeuTube::GetCurrentUserName());
}

std::string ProtocolMetadata::GetUserMetadataKey(std::string username) {
    return username + PROTOCOL_METADATA_SUFFIX;
}

void ProtocolMetadata::read() {
    ZDvidReader reader;
    if (reader.open(m_dvidTarget)) {
        const QByteArray &rawData = reader.readKeyValue(QString::fromStdString(m_dvidDataName),
            QString::fromStdString(m_metadataKey));
        ZJsonObject data;
        data.decodeString(rawData.data());

        // check version here, once we have a version two

        // we write all the data, so a ton of checking shouldn't be needed;
        //  do one check to make us feel better
        if (!data.hasKey(KEY_PROTOCOL_KEY.c_str()) || !data.hasKey(KEY_PROTOCOL_NAME.c_str())) {
            m_ioSuccessful = false;
            return;
        }
        // OMG, C++, make up your mind on how you want to represent strings!
        m_activeProtocolName = ZJsonParser::stringValue(data[KEY_PROTOCOL_NAME.c_str()]);
        m_activeProtocolKey = ZJsonParser::stringValue(data[KEY_PROTOCOL_KEY.c_str()]);
        m_isActive = true;

        m_ioSuccessful = true;
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
        data.setEntry(KEY_PROTOCOL_NAME, getActiveProtocolName());
        data.setEntry(KEY_PROTOCOL_KEY, getActiveProtocolKey());

        // always version your output files!
        data.setEntry(KEY_VERSION.c_str(), fileVersion);

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
    return m_isActive && !m_activeProtocolName.empty();
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

bool ProtocolMetadata::ioSuccessful() {
    return m_ioSuccessful;
}
