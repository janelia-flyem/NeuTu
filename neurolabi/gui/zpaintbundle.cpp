#include "zpaintbundle.h"

ZPaintBundle::ZPaintBundle(neutube::EAxis sliceAxis)
{
//  m_swcNodes = &m_emptyNodeSet;
  clearAllDrawableLists();
  m_sliceAxis = sliceAxis;
}
