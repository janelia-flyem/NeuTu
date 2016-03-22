#ifndef ZDVIDANNOTATION_H
#define ZDVIDANNOTATION_H

#include <string>
#include "zstackobject.h"
#include "zjsonobject.h"
#include "zjsonarray.h"

class ZJsonObject;

class ZDvidAnnotation : public ZStackObject
{
public:
  ZDvidAnnotation();

  enum EKind { KIND_POST_SYN, KIND_PRE_SYN, KIND_NOTE, KIND_UNKNOWN,
               KIND_INVALID };

  static ZStackObject::EType GetType() {
    return ZStackObject::TYPE_DVID_ANNOTATION;
  }

  const std::string& className() const;
  void display(ZPainter &painter, int slice, EDisplayStyle option,
               NeuTube::EAxis sliceAxis) const;

  void setPosition(int x, int y, int z);
  void setPosition(const ZIntPoint &pos);

  const ZIntPoint& getPosition() const { return m_position; }

  void setDefaultRadius();
  void setRadius(double r) { m_radius = r; }

  double getRadius() const { return m_radius; }

  void setKind(EKind kind) { m_kind = kind; }
  EKind getKind() const { return m_kind; }
  static std::string GetKindName(EKind kind);
  static EKind GetKind(const std::string &name);

  void setKind(const std::string &kind);

  void setDefaultColor();

  int getX() const;
  int getY() const;
  int getZ() const;

  using ZStackObject::hit; // suppress warning: hides overloaded virtual function [-Woverloaded-virtual]
  bool hit(double x, double y);
  bool hit(double x, double y, double z);

  void loadJsonObject(const ZJsonObject &obj);
  ZJsonObject toJsonObject() const;

  void clear();

  bool isValid() const;

  static QColor GetDefaultColor(EKind kind);
  static double GetDefaultRadius(EKind kind);

public: //Additional properties
  void setUserName(const std::string &name);
  std::string getUserName() const;

public:
  static void AddProperty(ZJsonObject &json, const std::string &key,
                          const std::string &value);
  static void AddProperty(ZJsonObject &json, const std::string &key,
                          bool value);

private:
  void init();
  bool isVisible(int z, NeuTube::EAxis sliceAxis) const;
  double getRadius(int z, NeuTube::EAxis sliceAxis) const;

private:
  ZIntPoint m_position;
  EKind m_kind;
  std::vector<std::string> m_tagArray;
  ZJsonObject m_propertyJson;
  ZJsonArray m_relJson;

  double m_radius;
};

#endif // ZDVIDANNOTATION_H
