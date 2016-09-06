#include "zstackobjectrole.h"
#include "neutube_def.h"

const ZStackObjectRole::TRole ZStackObjectRole::ROLE_NONE = 0;
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_DISPLAY = BIT_FLAG(1);
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_SEED = BIT_FLAG(2);
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_TMP_RESULT = BIT_FLAG(3);
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_3DPAINT = BIT_FLAG(4);
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_MANAGED_OBJECT = BIT_FLAG(5);
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_3DGRAPH_DECORATOR = BIT_FLAG(6);
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_TMP_BOOKMARK = BIT_FLAG(7);
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_ROI = BIT_FLAG(8);
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_MASK = BIT_FLAG(9);
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_SEGMENTATION = BIT_FLAG(10);
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_ACTIVE_VIEW = BIT_FLAG(11);
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_SKELETON_MASK = BIT_FLAG(12);
const ZStackObjectRole::TRole ZStackObjectRole::ROLE_ROI_MASK = BIT_FLAG(13);

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
