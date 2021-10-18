#ifndef ZDVIDSTACKSOURCE_H
#define ZDVIDSTACKSOURCE_H

#include "imgproc/zstacksource.h"
#include "zdvidreader.h"
#include "zdvidinfo.h"

class ZDvidStackSource : public ZStackSource
{
public:
  ZDvidStackSource();

  void setDvidTarget(const ZDvidTarget &target);

  int getMaxZoom() const override;
  std::shared_ptr<ZStack> getStack(
      const ZIntCuboid &box, int zoom) const override;

private:
  ZDvidReader m_reader;
  ZDvidInfo m_dvidInfo;
};

#endif // ZDVIDSTACKSOURCE_H
