#include "zmockitemchunk.h"

ZMockItemChunk::ZMockItemChunk()
{
  m_invalidItem = 0;
}

int ZMockItemChunk::getKey(const int &item) const
{
  return item;
}
