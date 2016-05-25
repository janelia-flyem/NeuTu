#ifndef PROTOCOLMETADATA_H
#define PROTOCOLMETADATA_H

#include <iostream>

#include "dvid/zdvidtarget.h"


class ProtocolMetadata
{
public:
    ProtocolMetadata(std::string dataName, ZDvidTarget target);
    static ProtocolMetadata ReadProtocolMetadata(std::string dataName, ZDvidTarget target);

    bool isActive();
    std::string getActiveProtocolName();
    std::string getActiveProtocolKey();
    void setActive(std::string name, std::string key);
    void clearActive();
    void write();

private:
    static const std::string PROTOCOL_METADATA_SUFFIX;

    ZDvidTarget m_dvidTarget;
    std::string m_dvidDataName;
    std::string m_metadataKey;
    bool m_isActive;
    bool m_ioSuccessful;
    std::string m_activeProtocolName;
    std::string m_activeProtocolKey;

    void read();
};

#endif // PROTOCOLMETADATA_H
