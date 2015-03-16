#include "zdvidtileinfo.h"

#include <iostream>

#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"
#include "zstring.h"

const int ZDvidTileInfo::m_levelScale = 2;

ZDvidTileInfo::ZDvidTileInfo() : m_maxLevel(0)
{
}

void ZDvidTileInfo::clear()
{
  m_tileSize.set(0, 0, 0);
  m_maxLevel = 0;
}

void ZDvidTileInfo::load(const ZJsonObject &json)
{
  clear();
  if (json.hasKey("Extended")) {
    ZJsonObject jsonExtended(
          const_cast<json_t*>(json["Extended"]), ZJsonValue::SET_INCREASE_REF_COUNT);
    if (jsonExtended.hasKey("Levels")) {
      ZJsonObject allLevelJson(jsonExtended["Levels"],
          ZJsonValue::SET_INCREASE_REF_COUNT);
      const char *key;
      json_t *value;
      ZJsonObject_foreach(allLevelJson, key, value) {
        int level = ZString(key).firstInteger();
        if (m_maxLevel < level) {
          m_maxLevel = level;
        }
        if (level == 0) {
          ZJsonObject levelJson(value, ZJsonValue::SET_INCREASE_REF_COUNT);
          ZJsonArray tileSizeJson(levelJson["TileSize"],
              ZJsonValue::SET_INCREASE_REF_COUNT);
          m_tileSize.set(ZJsonParser::integerValue(tileSizeJson.at(0)),
                         ZJsonParser::integerValue(tileSizeJson.at(1)),
                         ZJsonParser::integerValue(tileSizeJson.at(2)));
        }
      }
    }
  }
}

void ZDvidTileInfo::print() const
{
  std::cout << "Max level: " << m_maxLevel << std::endl;
  std::cout << "Tile size: " << m_tileSize.getX() << " x " << m_tileSize.getY()
            << " x " << m_tileSize.getZ() << std::endl;
}

bool ZDvidTileInfo::isValid() const
{
  return getWidth() > 0 && getHeight() > 0;
}
