#ifndef ZSTACKOBJECTINFO_H
#define ZSTACKOBJECTINFO_H

#include <QHash>
#include <set>
#include "zstackobject.h"

class ZStackObject;

class ZStackObjectInfo
{
public:
  ZStackObjectInfo();


  typedef uint64_t TState;

  static const TState STATE_UNKNOWN = 0;
  static const TState STATE_MODIFIED = BIT_FLAG(1);
  static const TState STATE_ADDED = BIT_FLAG(2);
  static const TState STATE_REMOVED = BIT_FLAG(3);
  static const TState STATE_VISIBITLITY_CHANGED = BIT_FLAG(4);
  static const TState STATE_COLOR_CHANGED = BIT_FLAG(5);
  static const TState STATE_ROLE_CHANGED = BIT_FLAG(6);
  static const TState STATE_SOURCE_CHANGED = BIT_FLAG(7);
  static const TState STATE_VE_CHANGED = BIT_FLAG(8);

  void setType(ZStackObject::EType type) {
    m_type = type;
  }

  void setTarget(neutu::data3d::ETarget target) {
    m_target = target;
  }

  void setRole(const ZStackObjectRole &role) {
    m_role = role;
  }

  void set(const ZStackObject &obj);

  ZStackObject::EType getType() const {
    return m_type;
  }

  neutu::data3d::ETarget getTarget() const {
    return m_target;
  }

  const ZStackObjectRole& getRole() const{
    return m_role;
  }

  bool operator == (const ZStackObjectInfo &info) const;

  void print() const;
  std::string toString() const;

private:
  ZStackObject::EType m_type = ZStackObject::EType::UNIDENTIFIED;
  neutu::data3d::ETarget m_target = neutu::data3d::ETarget::TARGET_NONE;
  ZStackObjectRole m_role;
};

uint qHash(const ZStackObjectInfo &info);

class ZStackObjectInfoSet : public QHash<ZStackObjectInfo, ZStackObjectInfo::TState>
{
public:
  ZStackObjectInfoSet();

  bool contains(ZStackObject::EType type) const;
  bool contains(neutu::data3d::ETarget target) const;
  bool contains(ZStackObjectRole::TRole role) const;
  bool contains(const ZStackObjectInfo &info) const;

  std::set<ZStackObject::EType> getType() const;
  std::set<neutu::data3d::ETarget> getTarget() const;

  /*!
   * \brief Check if a certain type of object has been modified in a certain way.
   *
   * It returns true if an object with \a type has modification in \a flag.
   * Note that STATE_UNKNOWN will be treated as any kind of modification.
   */
  bool hasObjectModified(
      ZStackObject::EType type, ZStackObjectInfo::TState flag) const;
  bool hasObjectModified(
      ZStackObject::EType type) const;

  /*!
   * \brief Check if data is changed for a certain type of object
   *
   * \return true iff the changing flag contains STATE_ADDED, STATE_REMOVED, or
   * STATE_MODIFIED or the flag is STATE_UNKNOWN.
   */
  bool hasObjectDataModified(ZStackObject::EType type) const;

  bool onlyVisibilityChanged(ZStackObject::EType type) const;


  void add(const ZStackObject &obj);

  void add(neutu::data3d::ETarget target);
  void add(ZStackObject::EType type,
           ZStackObjectInfo::TState state = ZStackObjectInfo::STATE_UNKNOWN);
  void add(ZStackObjectRole::TRole role);
  void add(const ZStackObjectInfo &info);
  void add(const QSet<neutu::data3d::ETarget> &targetSet);
  void add(const ZStackObjectInfo &info, ZStackObjectInfo::TState state);

  void print() const;
};


#endif // ZSTACKOBJECTINFO_H
