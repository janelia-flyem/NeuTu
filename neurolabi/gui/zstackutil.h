#ifndef ZSTACKUTIL_H
#define ZSTACKUTIL_H

class ZStackPtr;
class ZStack;

namespace zstack {
struct DsIntvGreaterThan {
  bool operator() (const ZStackPtr &stack1, const ZStackPtr &stack2) const;
  bool operator() (const ZStack &stack1, const ZStack &stack2) const;
};


int GetDsVolume(const ZStackPtr &stack);
int GetDsVolume(const ZStack &stack);
int GetDsVolume(const ZStack *stack);

}

#endif // ZSTACKUTIL_H
