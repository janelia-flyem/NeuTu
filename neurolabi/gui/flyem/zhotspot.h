#ifndef ZHOTSPOT_H
#define ZHOTSPOT_H

#include "zuncopyable.h"
#include "zpoint.h"
#include "ztextlinecompositer.h"

namespace FlyEm {

class ZGeometry {
public:
  virtual ~ZGeometry() {}
  void print() const;
  virtual ZTextLineCompositer toLineCompositer() const = 0;
};

class ZPointGeometry : public ZGeometry {
public:
  ~ZPointGeometry() {}
  void setCenter(double x, double y, double z);
  ZTextLineCompositer toLineCompositer() const;

private:
  ZPoint m_center;
};

class ZStructureInfo {
public:
  void print() const;
  ZTextLineCompositer toLineCompositer() const;

  enum EType {
    TYPE_MERGE, TYPE_SPLIT, TYPE_UNKNOWN
  };

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
  void setGeometry(ZGeometry *geometry);

  ZTextLineCompositer toLineCompositer() const;

private:
  ZGeometry *m_geometry;
  ZStructureInfo *m_structInfo;
  double m_confidence;
  EType m_type;
};

}

#endif // ZHOTSPOT_H
