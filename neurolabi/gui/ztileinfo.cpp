#include "ztileinfo.h"
#include "zjsonparser.h"
#include <QFileInfo>
#include <QDir>

ZTileInfo::ZTileInfo()
{
  for (int i = 0; i < 3; ++i) {
    m_dim[i] = 0;
  }
}


bool ZTileInfo::loadJsonObject(const ZJsonObject &obj, QString tileFilePath)
{
  if (obj.hasKey("source")) {
    m_source = ZJsonParser::stringValue(obj["source"]);
    QFileInfo fileInfo(m_source.c_str());
    if (!fileInfo.isAbsolute()) {
      QString fn = tileFilePath+QDir::separator()+
          QFileInfo(m_source.c_str()).fileName();
      m_source = fn.toStdString();
    }
  }

  if (obj.hasKey("offset")) {
    const json_t *value = obj["offset"];
    if (ZJsonParser::isArray(value)) {
      if (ZJsonParser::arraySize(value)  == 3) {
        m_offset.set(ZJsonParser::numberValue(value, 0),
                     ZJsonParser::numberValue(value, 1),
                     ZJsonParser::numberValue(value, 2));
      } else {
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }


  if (obj.hasKey("size")) {
    const json_t *value = obj["size"];
    if (ZJsonParser::isArray(value)) {
      if (ZJsonParser::arraySize(value)  == 3) {
        for (int i = 0; i < 3; ++i) {
          m_dim[i] = ZJsonParser::integerValue(value, i);
        }
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  if (obj.hasKey("image")) {
      m_imageSourse = ZJsonParser::stringValue(obj["image"]);
      QFileInfo fileInfo(m_imageSourse.c_str());
      if (!fileInfo.isAbsolute()) {
        QString fn = tileFilePath+QDir::separator()+QFileInfo(m_imageSourse.c_str()).fileName();
        m_imageSourse = fn.toStdString();
      }
  }

  return true;
}
