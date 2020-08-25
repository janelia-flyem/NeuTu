#include "zflyemcompositebodycolorscheme.h"

ZFlyEmCompositeBodyColorScheme::ZFlyEmCompositeBodyColorScheme()
{

}

void ZFlyEmCompositeBodyColorScheme::appendScheme(
    std::shared_ptr<ZFlyEmBodyColorScheme> scheme)
{
  m_schemeList.push_back(scheme);
}

uint32_t ZFlyEmCompositeBodyColorScheme::getBodyColorCode(uint64_t bodyId) const
{
  for (auto scheme : m_schemeList) {
    if (scheme->hasExplicitColor(bodyId)) {
      return scheme->getBodyColorCode(bodyId);
    }
  }

  return m_defaultColor;
}

bool ZFlyEmCompositeBodyColorScheme::hasExplicitColor(uint64_t bodyId) const
{
  for (auto scheme : m_schemeList) {
    if (scheme->hasExplicitColor(bodyId)) {
      return true;
    }
  }

  return false;
}

int ZFlyEmCompositeBodyColorScheme::getBodyColorIndex(uint64_t bodyId) const
{
  int index = 0;
  for (auto scheme : m_schemeList) {
    int subidx = scheme->getBodyColorIndex(bodyId);
    if (subidx >= 0) {
      index += subidx;
      return index;
    } else {
      index += scheme->getColorNumber();
    }
  }

  return -1;
}

QColor ZFlyEmCompositeBodyColorScheme::getBodyColorFromIndex(int index) const
{
  if (index >= 0) {
    for (auto scheme : m_schemeList) {
      if (index >= scheme->getColorNumber()) {
        index -= scheme->getColorNumber();
      } else {
        return scheme->getBodyColorFromIndex(index);
      }
    }
  }

  return m_defaultColor;
}

int ZFlyEmCompositeBodyColorScheme::getColorNumber() const
{
  int n = 0;
  for (auto scheme : m_schemeList) {
    n += scheme->getColorNumber();
  }

  return n;
}

void ZFlyEmCompositeBodyColorScheme::update()
{
  for (auto scheme : m_schemeList) {
    scheme->update();
  }
}
