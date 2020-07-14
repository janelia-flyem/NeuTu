#ifndef ZMOCKITEMCHUNK_H
#define ZMOCKITEMCHUNK_H

#include "zitemchunk.hpp"

class ZMockItemChunk : public ZItemChunk<int, int>
{
public:
  ZMockItemChunk();

protected:
  int getKey(const int &item) const override;
};

#endif // ZMOCKITEMCHUNK_H
