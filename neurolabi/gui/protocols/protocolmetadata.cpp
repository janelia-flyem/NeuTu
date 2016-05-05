#include "protocolmetadata.h"

/*
 * this class stores data about the protocols stored for a user;
 * it's intended to be a dumb container with no real functionality
 */
ProtocolMetadata::ProtocolMetadata()
{
    // default constructor should return a class that says nothing
    //  is saved and nothing is active

    // will store:
    //  - username
    //  - active protocol, if any
    //  - for active protocol: name, dataset, key
    //  - list of saved protocols?  with statuses?

}

bool ProtocolMetadata::isActive() {
    // is there an active protocol?

    // for now, no; haven't implemented storage yet
    return false;

}
