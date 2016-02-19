#include "zstackobjectrole.h"

const ZStackObjectRole::TRole ZStackObjectRole::ROLE_NONE = 0;
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_DISPLAY = 1;
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_SEED = 2;
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_TMP_RESULT = 4;
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_3DPAINT = 8;
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_MANAGED_OBJECT = 16;
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_3DGRAPH_DECORATOR = 32;
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_TMP_BOOKMARK = 64;
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_ROI = 128;
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_MASK = 256;
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_SEGMENTATION = 512;
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_ACTIVE_VIEW = 1024;
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_SKELETON_MASK = 2048;

ZStackObjectRole::ZStackObjectRole() : m_role(ZStackObjectRole::ROLE_NONE)
{
}

ZStackObjectRole::ZStackObjectRole(TRole role) : m_role(role)
{

}

void ZStackObjectRole::clear()
{
  m_role = ZStackObjectRole::ROLE_NONE;
}
