#include "todosearcher.h"

#include "dvid/zdvidreader.h"

#include "zjsonparser.h"

ToDoSearcher::ToDoSearcher(QObject *parent) : QObject(parent)
{


}

// constants
const std::string ToDoSearcher::KEY_BODYID = "bodyID";

void ToDoSearcher::setBodyID(uint64_t bodyID) {

    m_bodyID = bodyID;

}

void ToDoSearcher::search(ZDvidReader reader) {

    // placeholder; will do a search and return a list of items

    // not clear if this will take a passed in reader or create its own, since it
    //  could end up doing the searching in another thread



}

ZJsonObject ToDoSearcher::toJson() {
    ZJsonObject parameters;
    parameters.setEntry(KEY_BODYID.c_str(), m_bodyID);
    return parameters;
}

void ToDoSearcher::fromJson(ZJsonObject object) {
    m_bodyID = ZJsonParser::integerValue(object[KEY_BODYID.c_str()]);
}
