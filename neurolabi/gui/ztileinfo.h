#ifndef ZTILEINFO_H
#define ZTILEINFO_H

#include <string>
#include "zpoint.h"
#include "zjsonobject.h"

class ZTileInfo
{
public:
  ZTileInfo();

  inline const std::string& getSource() const {
    return m_source;
  }

  inline const ZPoint& getOffset() const {
    return m_offset;
  }

  inline int getWidth() const {
    return m_dim[0];
  }

  inline int getHeight() const {
    return m_dim[1];
  }

  inline int getDepth() const {
    return m_dim[2];
  }

  inline const std::string& getImageSource() const {
      return m_imageSourse;
  }

  inline const std::string& getSWCSource() const {
      return m_swcSource;
  }

  bool loadJsonObject(const ZJsonObject &obj);

private:
  std::string m_source;
  std::string m_swcSource;
  std::string m_imageSourse;
  ZPoint m_offset;
  int m_dim[3];
};

#endif // ZTILEINFO_H
