#ifndef ZDVIDSYNAPSE_H
#define ZDVIDSYNAPSE_H


#include <iostream>
#include <QColor>

#include "zstackobject.h"
#include "zintpoint.h"
#include "zjsonobject.h"
#include "zdvidannotation.h"

class ZJsonArray;
class ZDvidTarget;
class ZVaa3dMarker;
class ZDvidReader;

class ZDvidSynapse : public ZDvidAnnotation
{
public:
  ZDvidSynapse();

  static ZStackObject::EType GetType() {
    return ZStackObject::TYPE_DVID_SYNAPSE;
  }

//  enum EKind { KIND_POST_SYN, KIND_PRE_SYN, KIND_UNKNOWN, KIND_INVALID };

  const std::string& className() const;
  void display(ZPainter &painter, int slice, EDisplayStyle option,
               NeuTube::EAxis sliceAxis) const;

  double getConfidence() const;
  void setConfidence(double c);

  std::string getAnnotation() const;

  bool isVerified() const;
  bool isProtocolVerified(const ZDvidTarget &target) const;

  void setVerified(const std::string &userName);

  ZVaa3dMarker toVaa3dMarker(double radius) const;

  void updatePartnerProperty(ZDvidReader &reader);

  EKind getParterKind(size_t i) const;


  friend std::ostream& operator<< (
      std::ostream &stream, const ZDvidSynapse &synapse);

  /*
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
  */


  static void SetConfidenceProp(ZJsonObject &propJson, double conf);
  static void SetConfidence(ZJsonObject &json, double conf);

#if 0
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
#endif

//  std::vector<ZIntPoint> getPartners();
/*
public: //Additional properties
  void setUserName(const std::string &name);
  std::string getUserName() const;
*/
//private:
//  static ZJsonArray GetRelationJson(ZJsonObject &json);

private:
  void init();
//  bool isVisible(int z, NeuTube::EAxis sliceAxis) const;
//  double getRadius(int z, NeuTube::EAxis sliceAxis) const;
  ZJsonObject makeRelJson(const ZIntPoint &pt) const;

  static QColor GetArrowColor(bool verified);

private:
  std::vector<bool> m_isPartnerVerified;
  std::vector<EKind> m_partnerKind;
};

#endif // ZDVIDSYNAPSE_H
