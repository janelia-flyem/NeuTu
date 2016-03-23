#ifndef ZDVIDSYNAPSE_H
#define ZDVIDSYNAPSE_H


#include <iostream>
#include <QColor>

#include "zstackobject.h"
#include "zintpoint.h"
#include "zjsonobject.h"

class ZJsonArray;

class ZDvidSynapse : public ZStackObject
{
public:
  ZDvidSynapse();

  enum EKind { KIND_POST_SYN, KIND_PRE_SYN, KIND_UNKNOWN, KIND_INVALID };

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

//  void setTag(const std::string &tag) { m_tag = tag; }

  void setKind(const std::string &kind);

  void setDefaultColor();

  int getX() const;
  int getY() const;
  int getZ() const;

  using ZStackObject::hit; // suppress warning: hides overloaded virtual function [-Woverloaded-virtual]
  bool hit(double x, double y);
  bool hit(double x, double y, double z);

  void loadJsonObject(
      const ZJsonObject &obj,
      NeuTube::FlyEM::ESynapseLoadMode mode = NeuTube::FlyEM::LOAD_NO_PARTNER);
  ZJsonObject toJsonObject() const;

  void clear();

  friend std::ostream& operator<< (
      std::ostream &stream, const ZDvidSynapse &synapse);

  void clearPartner();
  void addPartner(int x, int y, int z);
  void addTag(const std::string &tag);

  bool isValid() const;

  static QColor GetDefaultColor(EKind kind);
  static double GetDefaultRadius(EKind kind);

  class Relation {
  public:
    enum ERelation {
      RELATION_UNKNOWN, RELATION_POSTSYN_TO, RELATION_PRESYN_TO,
      RELATION_CONVERGENT_TO, RELATION_GROUPED_WITH
    };

    Relation(const ZIntPoint &to, ERelation relation) :
      m_to(to), m_relation(relation) {
    }

    static std::string GetName(ERelation rel);

    ZJsonObject toJsonObject() const;



  private:
    ZIntPoint m_to;
    ERelation m_relation;
  };

public: //Json APIs
  static ZJsonObject MakeRelJson(const ZIntPoint &pt, const std::string &rel);
  static bool AddRelation(
      ZJsonObject &json, const ZIntPoint &to, const std::string &rel);
  static bool AddRelation(
      ZJsonArray &json, const ZIntPoint &to, const std::string &rel);
  static bool AddRelation(ZJsonObject &json, const ZJsonObject &relJson);
  static bool AddRelation(ZJsonArray &json, const ZJsonObject &relJson);
  static int AddRelation(ZJsonObject &json, const ZJsonArray &relJson);
  static int AddRelation(ZJsonArray &json, const ZJsonArray &relJson);
  template <typename InputIterator>
  static int AddRelation(
      ZJsonObject &json, const InputIterator &first,
      const InputIterator &last, const std::string &rel);

  static bool RemoveRelation(ZJsonArray &json, const ZIntPoint &pt);
  static bool RemoveRelation(ZJsonObject &json, const ZIntPoint &pt);

  static void AddProperty(ZJsonObject &json, const std::string &key,
                          const std::string &value);

  std::vector<ZIntPoint> getPartners();

public: //Additional properties
  void setUserName(const std::string &name);
  std::string getUserName() const;

private:
  static ZJsonArray GetRelationJson(ZJsonObject &json);

private:
  void init();
  bool isVisible(int z, NeuTube::EAxis sliceAxis) const;
  double getRadius(int z, NeuTube::EAxis sliceAxis) const;
  ZJsonObject makeRelJson(const ZIntPoint &pt) const;

private:
  ZIntPoint m_position;
  double m_radius;
  EKind m_kind;
  std::vector<std::string> m_tagArray;
  std::vector<ZIntPoint> m_partnerHint;
  ZJsonObject m_propertyJson;
};

template <typename InputIterator>
int ZDvidSynapse::AddRelation(
    ZJsonObject &json, const InputIterator &first,
    const InputIterator &last, const std::string &rel)
{
  int count = 0;
  for (InputIterator iter = first; iter != last; ++iter) {
    if (AddRelation(json, *iter, rel)) {
      ++count;
    }
  }

  return count;
}


#endif // ZDVIDSYNAPSE_H
