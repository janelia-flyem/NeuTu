#include "zpaintbundle.h"

ZPaintBundle::ZPaintBundle(neutu::EAxis sliceAxis)
{
//  m_swcNodes = &m_emptyNodeSet;
  clearAllDrawableLists();
  m_sliceAxis = sliceAxis;
}
