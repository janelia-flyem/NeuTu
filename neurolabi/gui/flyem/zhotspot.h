#ifndef ZHOTSPOT_H
#define ZHOTSPOT_H

#include "zuncopyable.h"
#include "zpoint.h"
#include "ztextlinecompositer.h"
#include "zjsonobject.h"

namespace FlyEm {

class ZGeometry {
public:
  virtual ~ZGeometry() {}
  void print() const;
  virtual ZTextLineCompositer toLineCompositer() const = 0;
  virtual ZJsonObject toJsonObject() const = 0;
};

class ZPointGeometry : public ZGeometry {
public:
  ~ZPointGeometry() {}
  void setCenter(double x, double y, double z);
  ZTextLineCompositer toLineCompositer() const;
  ZJsonObject toJsonObject() const;

private:
  ZPoint m_center;
};

class ZStructureInfo {
public:
  void print() const;
  ZTextLineCompositer toLineCompositer() const;
  ZJsonObject toJsonObject() const;

  enum EType {
    TYPE_MERGE, TYPE_SPLIT, TYPE_UNKNOWN
  };

  inline void setType(EType type) { m_type = type; }
  inline void setSource(int id) { m_sourceBody = id; }
  inline void addTarget(int id) { m_targetBodyArray.push_back(id); }

  std::string getTypeString() const;

private:
  EType m_type;
  int m_sourceBody;
  std::vector<int> m_targetBodyArray;
};


class ZHotSpot : ZUncopyable
{
public:
  ZHotSpot();
  ~ZHotSpot();

  enum EType {
    TYPE_POINT, TYPE_REGION, TYPE_SURFACE
  };

  void print() const;
  inline void setType(EType type) {
    m_type = type;
  }
  inline void setConfidence(double confidence) {
    m_confidence = confidence;
  }

  void setGeometry(ZGeometry *geometry);
  void setStructure(ZStructureInfo *structure);

  ZTextLineCompositer toLineCompositer() const;

  ZJsonObject toJsonObject() const;

  std::string getTypeString() const;

private:
  ZGeometry *m_geometry;
  ZStructureInfo *m_structInfo;
  double m_confidence;
  EType m_type;
};

}

#endif // ZHOTSPOT_H
