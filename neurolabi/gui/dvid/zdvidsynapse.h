#ifndef ZDVIDSYNAPSE_H
#define ZDVIDSYNAPSE_H


#include <iostream>
#include <QColor>
#include <unordered_map>

#include "zstackobject.h"
#include "geometry/zintpoint.h"
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
    return ZStackObject::EType::DVID_SYNAPSE;
  }

//  enum EKind { KIND_POST_SYN, KIND_PRE_SYN, KIND_UNKNOWN, KIND_INVALID };

//  const std::string& className() const;
  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis sliceAxis) const;

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


  static void SetConfidenceProp(ZJsonObject &propJson, double conf);
  static void SetConfidence(ZJsonObject &json, double conf);

  std::string getConnString(
      const std::unordered_map<ZIntPoint, uint64_t> &labelMap) const;

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

private:
  void init();
  ZJsonObject makeRelJson(const ZIntPoint &pt) const;

  static QColor GetArrowColor(bool verified);

private:
  std::vector<bool> m_isPartnerVerified;
  std::vector<EKind> m_partnerKind;
  std::vector<EStatus> m_partnerStatus;
};

#endif // ZDVIDSYNAPSE_H
