#include "zstackobjectinfo.h"

uint qHash(const ZStackObjectInfo &info)
{
  return qHash(info.getTarget()) ^ qHash(info.getType()) ^ qHash(info.getRole().getRole());
}


ZStackObjectInfo::ZStackObjectInfo()
{

}

bool ZStackObjectInfo::operator ==(const ZStackObjectInfo &info) const
{
  return (m_type == info.m_type) && (m_target == info.m_target) &&
      (m_role == info.m_role);
}

void ZStackObjectInfo::set(const ZStackObject &obj)
{
  setTarget(obj.getTarget());
  setType(obj.getType());
  setRole(obj.getRole());
}

///////////////////////////////////

ZStackObjectInfoSet::ZStackObjectInfoSet()
{

}

bool ZStackObjectInfoSet::contains(ZStackObject::ETarget target) const
{
  foreach (const ZStackObjectInfo &info, *this) {
    if (info.getTarget() == target) {
      return true;
    }
  }

  return false;
}

bool ZStackObjectInfoSet::contains(ZStackObject::EType type) const
{
  foreach (const ZStackObjectInfo &info, *this) {
    if (info.getType() == type) {
      return true;
    }
  }

  return false;
}

bool ZStackObjectInfoSet::contains(ZStackObjectRole::TRole role) const
{
  foreach (const ZStackObjectInfo &info, *this) {
    if (info.getRole() == role) {
      return true;
    }
  }

  return false;
}

void ZStackObjectInfoSet::add(const ZStackObject &obj)
{
  ZStackObjectInfo info;
  info.set(obj);
  insert(info);
}

void ZStackObjectInfoSet::add(ZStackObject::ETarget target)
{
  ZStackObjectInfo info;
  info.setTarget(target);
  insert(info);
}

void ZStackObjectInfoSet::add(const QSet<ZStackObject::ETarget> &targetSet)
{
  foreach (ZStackObject::ETarget target, targetSet) {
    add(target);
  }
}

void ZStackObjectInfoSet::add(ZStackObject::EType type)
{
  ZStackObjectInfo info;
  info.setType(type);
  insert(info);
}

void ZStackObjectInfoSet::add(ZStackObjectRole::TRole role)
{
  ZStackObjectInfo info;
  info.setRole(ZStackObjectRole(role));
  insert(info);
}

void ZStackObjectInfoSet::add(const ZStackObjectInfo &info)
{
  insert(info);
}

QSet<ZStackObject::EType> ZStackObjectInfoSet::getType() const
{
  QSet<ZStackObject::EType> typeSet;
  foreach (const ZStackObjectInfo &info, *this) {
    typeSet.insert(info.getType());
  }

  return typeSet;
}
