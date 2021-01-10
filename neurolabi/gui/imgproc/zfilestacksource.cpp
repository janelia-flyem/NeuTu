#include "zfilestacksource.h"

#include "zstack.hxx"
#include "geometry/zintcuboid.h"

ZFileStackSource::ZFileStackSource()
{

}

void ZFileStackSource::setUrl(const std::string &path)
{
  m_fileSource.clear();
  m_fileSource.appendUrl(path);
  m_fileSource.setType("single");

  if (m_stackCache) {
    std::lock_guard<std::mutex> guard(m_stackCacheMutex);
    m_stackCache.reset();
  }
}

int ZFileStackSource::getMaxZoom() const
{
  return 0;
}

std::shared_ptr<ZStack> ZFileStackSource::getStack(
    const ZIntCuboid &box, int zoom) const
{
  if (zoom == 0) {
    if (!m_stackCache) {
      std::lock_guard<std::mutex> guard(m_stackCacheMutex);
      const_cast<std::shared_ptr<ZStack>&>(m_stackCache) =
          std::shared_ptr<ZStack>(m_fileSource.readStack());
    }
    if (m_stackCache) {
      if (box.isEmpty() || box.contains(m_stackCache->getBoundBox())) {
        return m_stackCache;
      } else {
        return std::shared_ptr<ZStack>(m_stackCache->makeCrop(box));
      }
    }
  }

  return nullptr;
}
