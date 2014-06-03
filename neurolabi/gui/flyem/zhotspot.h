#ifndef ZHOTSPOT_H
#define ZHOTSPOT_H

#include "zuncopyable.h"
#include "zpoint.h"
#include "ztextlinecompositer.h"
#include "zjsonobject.h"
#include "zpointarray.h"
#include "zlinesegmentarray.h"

namespace FlyEm {

class ZGeometry {
public:
  virtual ~ZGeometry() {}
  void print() const;
  virtual ZTextLineCompositer toLineCompositer() const = 0;
  virtual ZJsonObject toJsonObject() const = 0;
  virtual ZPointArray toPointArray() const = 0;
  virtual ZLineSegmentArray toLineSegmentArray() const = 0;
};

class ZPointGeometry : public ZGeometry {
public:
  ~ZPointGeometry() {}
  void setCenter(double x, double y, double z);
  inline const ZPoint& getCenter() const { return m_center; }
  ZTextLineCompositer toLineCompositer() const;
  ZJsonObject toJsonObject() const;
  ZPointArray toPointArray() const;
  ZLineSegmentArray toLineSegmentArray() const;

private:
  ZPoint m_center;
};

class ZCurveGeometry : public ZGeometry {
public:
  ~ZCurveGeometry() {}
  ZTextLineCompositer toLineCompositer() const;
  ZJsonObject toJsonObject() const;

  void appendPoint(double x, double y, double z);
  void appendPoint(const ZPoint &pt);

  size_t getAnchorNumber() const;
  const ZPoint& getAnchor(size_t index) const;
  void setAnchor(const ZPointArray &curve);

  ZPointArray toPointArray() const;
  ZLineSegmentArray toLineSegmentArray() const;

private:
  ZPointArray m_curve;
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

  inline int getSource() { return m_sourceBody; }

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
    TYPE_POINT, TYPE_REGION, TYPE_SURFACE, TYPE_CURVE
  };

  void print() const;
  inline void setType(EType type) {
    m_type = type;
  }
  inline void setConfidence(double confidence) {
    m_confidence = confidence;
  }

  void setGeometry(ZGeometry *geometry);
  void setGuidance(ZGeometry *geometry);
  void setStructure(ZStructureInfo *structure);

  ZTextLineCompositer toLineCompositer() const;

  ZJsonObject toJsonObject() const;
  ZJsonObject toRavelerJsonObject(const double *resolution,
                                  const int *imageSize) const;

  std::string getTypeString() const;
  ZPointArray toPointArray() const;
  ZLineSegmentArray toLineSegmentArray() const;

  inline ZGeometry* getGeometry() const {
    return m_geometry;
  }

  inline double getConfidence() const {
    return m_confidence;
  }

  static bool compareConfidence(const ZHotSpot *spot1, const ZHotSpot *spot2);

private:
  ZGeometry *m_geometry;
  ZGeometry *m_guidance;
  ZStructureInfo *m_structInfo;
  double m_confidence;
  EType m_type;
};

}

#endif // ZHOTSPOT_H
