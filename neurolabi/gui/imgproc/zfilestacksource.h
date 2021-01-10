#ifndef ZFILESTACKSOURCE_H
#define ZFILESTACKSOURCE_H

#include <mutex>

#include "zstacksource.h"
#include "zstackfile.h"

class ZFileStackSource : public ZStackSource
{
public:
  ZFileStackSource();

  int getMaxZoom() const override;
  std::shared_ptr<ZStack> getStack(
      const ZIntCuboid &box, int zoom) const override;

  void setUrl(const std::string &path);

private:
  ZStackFile m_fileSource;
  mutable std::mutex m_stackCacheMutex;
  std::shared_ptr<ZStack> m_stackCache;
};

#endif // ZFILESTACKSOURCE_H
