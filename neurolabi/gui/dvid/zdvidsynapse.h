#ifndef ZDVIDSYNAPSE_H
#define ZDVIDSYNAPSE_H

#include <iostream>

#include "zstackobject.h"
#include "zintpoint.h"

class ZJsonObject;

class ZDvidSynapse : public ZStackObject
{
public:
  ZDvidSynapse();

  enum EKind { KIND_POST_SYN, KIND_PRE_SYN, KIND_UNKNOWN, KIND_INVALID };

  const std::string& className() const;
  void display(ZPainter &painter, int slice, EDisplayStyle option) const;

  void setPosition(int x, int y, int z);
  void setPosition(const ZIntPoint &pos);

  const ZIntPoint& getPosition() const { return m_position; }

  void setDefaultRadius();
  void setRadius(double r) { m_radius = r; }

  void setKind(EKind kind) { m_kind = kind; }
  EKind getKind() const { return m_kind; }
  static std::string GetKindName(EKind kind);
  static EKind GetKind(const std::string &name);

//  void setTag(const std::string &tag) { m_tag = tag; }

  void setKind(const std::string &kind);

  void setDefaultColor();


  bool hit(double x, double y);
  bool hit(double x, double y, double z);

  void loadJsonObject(const ZJsonObject &obj);
  ZJsonObject toJsonObject() const;

  void clear();

  friend std::ostream& operator<< (
      std::ostream &stream, const ZDvidSynapse &synapse);

  void clearPartner();
  void addPartner(int x, int y, int z);
  void addTag(const std::string &tag);

  bool isValid() const;

private:
  void init();
  bool isVisible(int z);
  ZJsonObject makeRelJson(const ZIntPoint &pt) const;


private:
  ZIntPoint m_position;
  double m_radius;
  EKind m_kind;
  std::vector<std::string> m_tagArray;
  std::vector<ZIntPoint> m_partnerHint;
};

#endif // ZDVIDSYNAPSE_H
