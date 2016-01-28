#include "zpaintbundle.h"

ZPaintBundle::ZPaintBundle(NeuTube::EAxis sliceAxis)
{
  m_swcNodes = &m_emptyNodeSet;
  clearAllDrawableLists();
  m_sliceAxis = sliceAxis;
}
