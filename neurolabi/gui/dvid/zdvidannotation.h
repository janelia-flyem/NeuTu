#ifndef ZDVIDANNOTATION_H
#define ZDVIDANNOTATION_H

#include <string>
#include <set>

#include <QStaticText>

#include "zstackobject.h"
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zdviddef.h"

class ZJsonObject;
class ZCuboid;
class ZResolution;

/*
 * Annotation json example:
{
"Pos":[33,30,31],
"Kind":"PostSyn",
"Rels":[ {"Rel":"PostSynTo", "To":[15,27,35]} ],
"Tags":["Synapse1"],
"Prop": { "SomeVar": "SomeValue", "Another Var": "A More Complex Value" }
}
*/

class ZDvidAnnotation : public ZStackObject
{
public:
  ZDvidAnnotation();
  ZDvidAnnotation(const ZDvidAnnotation &annotation);

  enum class EKind { KIND_POST_SYN, KIND_PRE_SYN, KIND_NOTE, KIND_UNKNOWN,
               KIND_INVALID };

  enum class EStatus { NORMAL, DUPLICATED };

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::DVID_ANNOTATION;
  }

  ZDvidAnnotation* clone() const override;

  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis sliceAxis) const override;

  void setPosition(int x, int y, int z);
  void setPosition(const ZIntPoint &pos);

  const ZIntPoint& getPosition() const { return m_position; }

  void setDefaultRadius();
  void setDefaultRadius(const ZResolution &resolution);
  void setRadius(double r);
  double getRadius() const { return m_radius; }

  void setKind(EKind kind) { m_kind = kind; }
  EKind getKind() const { return m_kind; }
  static std::string GetKindName(EKind kind);
  static EKind GetKind(const std::string &name);

  void setKind(const std::string &kind);

  uint64_t getBodyId() const {
    return m_bodyId;
  }

  void setBodyId(uint64_t bodyId) {
    m_bodyId = bodyId;
  }

  void setStatus(EStatus status) {
    m_status = status;
  }

  EStatus getStatus() const {
    return m_status;
  }

  void setDefaultColor();

  int getX() const;
  int getY() const;
  int getZ() const;

  using ZStackObject::hit; // suppress warning: hides overloaded virtual function [-Woverloaded-virtual]
  bool hit(double x, double y, neutu::EAxis axis);
  bool hit(double x, double y, double z);

  void loadJsonObject(
      const ZJsonObject &obj,
      dvid::EAnnotationLoadMode mode);
  ZJsonObject toJsonObject() const;

  void clearPartner();
  void addPartner(int x, int y, int z);
  bool hasPartner(const ZIntPoint &pos);

  void addTag(const std::string &tag);

  /*!
   * \brief Add a body ID tag.
   *
   * Mainly used as a temporary solution for unsyncable labelmap.
   */
  void addBodyIdTag();

//  static std::string GetBodyIdTag(uint64_t bodyId);

  void removeTag(const std::string &tag);
  bool hasTag(const std::string &tag) const;
  std::set<std::string> getTagSet() const;

  void clear();

  bool isValid() const;

  static QColor GetDefaultColor(EKind kind);
  static double GetDefaultRadius(EKind kind);
  static double GetDefaultRadius(EKind kind, const ZResolution &resolution);

  class Relation {
  public:
    enum class ERelation {
      UNKNOWN, POSTSYN_TO, PRESYN_TO,
      CONVERGENT_TO, GROUPED_WITH
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

  const std::vector<ZIntPoint>& getPartners() const {
    return m_partnerHint;
  }

  ZCuboid getBoundBox() const override;

//  void setProperty(ZJsonObject propJson);

  void updatePartner();
  void updatePartner(const ZJsonArray &jsonArray);

public: //Additional properties
  void setUserName(const std::string &name);
  std::string getUserName() const;

  ZJsonArray getRelationJson() const {
    return m_relJson;
  }

  void addProperty(const std::string &key, const std::string &value);
  void removeProperty(const std::string &key);

  bool hasProperty(const std::string &key) const;
  template <typename T>
  T getProperty(const std::string &key) const;

  /*!
   * \brief Set the comment property of the annotation.
   *
   * It removes the comment property if \a comment is empty.
   */
  void setComment(const std::string &comment);
  std::string getComment() const;

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
  static ZJsonArray GetRelationJson(ZJsonObject &json);

  static void SetProperty(ZJsonObject &json, ZJsonObject propJson);

  static bool RemoveRelation(ZJsonArray &json, const ZIntPoint &pt);
  static bool RemoveRelation(ZJsonObject &json, const ZIntPoint &pt);
  static bool RemoveRelation(ZJsonArray &json, const std::string &rel);
  static bool RemoveRelation(ZJsonObject &json, const std::string &rel);

  static void AddProperty(ZJsonObject &json, const std::string &key,
                          const std::string &value);
  static void AddProperty(ZJsonObject &json, const std::string &key,
                          const char* value);
  static void AddProperty(ZJsonObject &json, const std::string &key,
                          bool value);
  static void AddProperty(ZJsonObject &json, const std::string &key,
                          int value);

  static void Annotate(ZJsonObject &json, const std::string &annot);

  static std::vector<ZIntPoint> GetPartners(const ZJsonObject &json);
  static std::vector<ZIntPoint> GetPartners(
      const ZJsonObject &json, const std::string &relation);
  static ZIntPoint GetPosition(const ZJsonObject &json);
  static ZIntPoint GetRelPosition(const ZJsonObject &json);
  static std::string GetRelationType(const ZJsonObject &relJson);
  static int MatchRelation(
      const ZJsonArray &relArray, const ZIntPoint &pos, const ZJsonObject &rel);
  static int MatchRelation(
      const ZJsonArray &relArray, const ZIntPoint &pos,
      const std::string &relType);

  static std::string GetMatchingRelation(const std::string &relType);

  static EKind GetKind(const ZJsonObject &json);

protected:
  bool isSliceVisible(int z, neutu::EAxis sliceAxis) const;
  double getRadius(int z, neutu::EAxis sliceAxis) const;

private:
  void init();

public:
  static const char* KEY_COMMENT;
  static double DEFAULT_POST_SYN_RADIUS;
  static double DEFAULT_PRE_SYN_RADIUS;

protected:
  ZIntPoint m_position;
  EKind m_kind;
  double m_radius;
  uint64_t m_bodyId;
  EStatus m_status;

  std::vector<ZIntPoint> m_partnerHint;
  std::set<std::string> m_tagSet;
  ZJsonObject m_propertyJson;
  ZJsonArray m_relJson;

//  mutable QStaticText m_textDecoration;
};

#include "dvid/zdvidannotation.hpp"


#endif // ZDVIDANNOTATION_H
