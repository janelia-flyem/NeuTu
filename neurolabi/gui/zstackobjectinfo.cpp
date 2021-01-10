#include "zstackobjectinfo.h"
#include <sstream>

uint qHash(const ZStackObjectInfo &info)
{
  return qHash(neutu::EnumValue(info.getTarget())) ^
      qHash(neutu::EnumValue(info.getType())) ^
      qHash(info.getRole().getRole());
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

void ZStackObjectInfo::print() const
{
  std::cout << toString()  << std::endl;
}

std::string ZStackObjectInfo::toString() const
{
  std::ostringstream stream;
  stream << "Object info: " << ZStackObject::GetTypeName(m_type) << "; Target "
         << neutu::EnumValue(m_target) << "; Role " << m_role.getRole();
  return stream.str();
}


///////////////////////////////////

ZStackObjectInfoSet::ZStackObjectInfoSet()
{

}

bool ZStackObjectInfoSet::contains(ZStackObject::ETarget target) const
{
  QHashIterator<ZStackObjectInfo, ZStackObjectInfo::TState> iter(*this);
  while (iter.hasNext()) {
    iter.next();
    const ZStackObjectInfo &info = iter.key();
    if (info.getTarget() == target) {
      return true;
    }
  }

  return false;
}

bool ZStackObjectInfoSet::contains(ZStackObject::EType type) const
{
  foreach (const ZStackObjectInfo &info, keys()) {
    if (info.getType() == type) {
      return true;
    }
  }

  return false;
}

bool ZStackObjectInfoSet::hasObjectModified(ZStackObject::EType type) const
{
  return contains(type);
}

namespace {

bool has_overlapping_flag(ZStackObjectInfo::TState state, ZStackObjectInfo::TState flag)
{
  return state & flag;
}

static const ZStackObjectInfo::TState DATA_MODIFIED_FLAG =
    ZStackObjectInfo::STATE_REMOVED | ZStackObjectInfo::STATE_ADDED |
    ZStackObjectInfo::STATE_MODIFIED;
}

bool ZStackObjectInfoSet::hasObjectModified(
    ZStackObject::EType type, ZStackObjectInfo::TState flag) const
{
  if (flag == ZStackObjectInfo::STATE_UNKNOWN) {
    return contains(type);
  } else {
    for (ZStackObjectInfoSet::const_iterator iter = begin(); iter != end();
         ++iter) {
      const ZStackObjectInfo &info = iter.key();
      if (info.getType() == type) {
        ZStackObjectInfo::TState state = iter.value();
        if (state == ZStackObjectInfo::STATE_UNKNOWN) {
          return true;
        } else {
          if ((state & flag) != 0) {
            return true;
          }
        }
      }
    }
  }

  return false;
}

bool ZStackObjectInfoSet::hasDataModified(ZStackObject::EType type) const
{
  return hasObjectModified(
        type, DATA_MODIFIED_FLAG);
}

bool ZStackObjectInfoSet::onlyVisibilityChanged(ZStackObject::EType type) const
{
  bool changed = false;

  for (ZStackObjectInfoSet::const_iterator iter = begin(); iter != end();
       ++iter) {
    const ZStackObjectInfo &info = iter.key();
    if (info.getType() == type) {
      ZStackObjectInfo::TState state = iter.value();
      if (state == ZStackObjectInfo::STATE_VISIBITLITY_CHANGED) {
        changed = true;
      } else {
        changed = false;
        break;
      }
    }
  }

  return changed;
}

bool ZStackObjectInfoSet::contains(ZStackObjectRole::TRole role) const
{
  foreach (const ZStackObjectInfo &info, keys()) {
    if (info.getRole().hasRole(role)) {
      return true;
    }
  }

  return false;
}

bool ZStackObjectInfoSet::hasDataModified(ZStackObjectRole::TRole role) const
{
  foreach (const ZStackObjectInfo &info, keys()) {
    if (info.getRole().hasRole(role) &&
        has_overlapping_flag((*this)[info], DATA_MODIFIED_FLAG)) {
      return true;
    }
  }

  return false;
}

bool ZStackObjectInfoSet::contains(const ZStackObjectInfo &info) const
{
  return QHash<ZStackObjectInfo, ZStackObjectInfo::TState>::contains(info);
}

void ZStackObjectInfoSet::add(
    const ZStackObjectInfo &info, ZStackObjectInfo::TState state)
{
  if (contains(info)) {
    if (state == ZStackObjectInfo::STATE_UNKNOWN) {
      (*this)[info] = ZStackObjectInfo::STATE_UNKNOWN;
    } else {
      (*this)[info] |= state;
    }
  } else {
    (*this)[info] = state;
  }
}

void ZStackObjectInfoSet::add(const ZStackObject &obj)
{
  ZStackObjectInfo info;
  info.set(obj);
  add(info);
}

void ZStackObjectInfoSet::add(ZStackObject::ETarget target)
{
  ZStackObjectInfo info;
  info.setTarget(target);
  add(info);
}

void ZStackObjectInfoSet::add(const QSet<ZStackObject::ETarget> &targetSet)
{
  foreach (ZStackObject::ETarget target, targetSet) {
    add(target);
  }
}

void ZStackObjectInfoSet::add(
    ZStackObject::EType type, ZStackObjectInfo::TState state)
{
  ZStackObjectInfo info;
  info.setType(type);
  add(info, state);
}

void ZStackObjectInfoSet::add(ZStackObjectRole::TRole role)
{
  ZStackObjectInfo info;  info.setRole(ZStackObjectRole(role));

  add(info);
}

void ZStackObjectInfoSet::add(const ZStackObjectInfo &info)
{
  if (!contains(info)) {
    (*this)[info] = ZStackObjectInfo::STATE_UNKNOWN;
  }
}

std::set<ZStackObject::EType> ZStackObjectInfoSet::getType() const
{
  std::set<ZStackObject::EType> typeSet;
  foreach (const ZStackObjectInfo &info, keys()) {
    typeSet.insert(info.getType());
  }

  return typeSet;
}

std::set<ZStackObject::ETarget> ZStackObjectInfoSet::getTarget() const
{
  std::set<ZStackObject::ETarget> targetSet;
  foreach (const ZStackObjectInfo &info, keys()) {
    if (info.getTarget() != ZStackObject::ETarget::NONE) {
      targetSet.insert(info.getTarget());
    }
  }

  return targetSet;
}


void ZStackObjectInfoSet::print() const
{
  QHashIterator<ZStackObjectInfo, ZStackObjectInfo::TState> iter(*this);
  while (iter.hasNext()) {
    iter.next();
    const ZStackObjectInfo &info = iter.key();
    std::cout << info.toString() << " => " << iter.value() << std::endl;
  }
}
