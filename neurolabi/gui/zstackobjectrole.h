#ifndef ZSTACKOBJECTROLE_H
#define ZSTACKOBJECTROLE_H

#include "tz_stdint.h"

class ZStackObjectRole
{
public:
  typedef uint32_t TRole;
  const static TRole ROLE_NONE;
  const static TRole ROLE_SEED;
  const static TRole ROLE_DISPLAY;
  const static TRole ROLE_TMP_RESULT; //Temporary result
  const static TRole ROLE_3DPAINT;
  const static TRole ROLE_MANAGED_OBJECT;
  const static TRole ROLE_3DSWC_DECORATOR;
  const static TRole ROLE_3DGRAPH_DECORATOR;
  const static TRole ROLE_TMP_BOOKMARK;
  const static TRole ROLE_ROI;
  const static TRole ROLE_MASK;
  const static TRole ROLE_SEGMENTATION;
  const static TRole ROLE_ACTIVE_VIEW;
  const static TRole ROLE_SKELETON_MASK;

  ZStackObjectRole();
  ZStackObjectRole(TRole role);



  inline const TRole& getRole() const {
    return m_role;
  }

  inline void setRole(TRole role) {
    m_role = role;
  }

  inline void addRole(TRole role) {
    m_role |= role;
  }

  inline void removeRole(TRole role) {
    m_role &= ~role;
  }


  /*!
   * \brief Check if the object has specific roles.
   *
   * It returns true iff:
   *   1) The \a role is non-empty and contained in the object's role
   *   2) Both the bojects's role and \a role are ROLE_NONE
   */
  inline bool hasRole(TRole role) const {
    if (role == ROLE_NONE) {
      return (m_role == ROLE_NONE);
    }

    return role == (m_role & role);
  }

  inline bool isNone() const {
    return m_role == ROLE_NONE;
  }

  void clear();


public:
  TRole m_role;
};

#endif // ZSTACKOBJECTROLE_H
