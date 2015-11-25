#ifndef ZDVIDSYNAPSE_H
#define ZDVIDSYNAPSE_H

#include "zstackobject.h"
#include "zintpoint.h"

class ZJsonObject;

class ZDvidSynapse : public ZStackObject
{
public:
  ZDvidSynapse();

  enum EKind { KIND_POST_SYN, KIND_PRE_SYN, KIND_UNKNOWN };

  const std::string& className() const;
  void display(ZPainter &painter, int slice, EDisplayStyle option) const;

  void setPosition(int x, int y, int z);
  void setPosition(const ZIntPoint &pos);

  void setDefaultRadius();
  void setRadius(double r) { m_radius = r; }

  void setKind(EKind kind) { m_kind = kind; }
//  void setTag(const std::string &tag) { m_tag = tag; }

  void setKind(const std::string &kind);

  void setDefaultColor();


  bool hit(double x, double y);
  bool hit(double x, double y, double z);

  void loadJsonObject(const ZJsonObject &obj);

  void clear();

private:
  void init();
  bool isVisible(int z);


private:
  ZIntPoint m_position;
  double m_radius;
  EKind m_kind;
  std::vector<std::string> m_tagArray;
};

#endif // ZDVIDSYNAPSE_H
