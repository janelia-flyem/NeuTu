#ifndef ZDVIDSTACKBLOCKFACTORY_H
#define ZDVIDSTACKBLOCKFACTORY_H

#include "zstackblockfactory.h"
#include "dvid/zdvidreader.h"

class ZDvidStackBlockFactory : public ZStackBlockFactory
{
public:
  ZDvidStackBlockFactory();

  const ZDvidReader& getReader() const;

  void setDvidTarget(const ZDvidTarget &target);

  int getMaxZoom() const override;
  ZIntPoint getBlockSize() const override;
  ZIntPoint getGridSize() const override;

  const ZDvidInfo& getDvidInfo() const;

protected:
  std::vector<ZStack*> makeV(int i, int j, int k, int n, int zoom) const override;

private:
  ZDvidReader m_reader;
  ZDvidInfo m_dvidInfo;
};

#endif // ZDVIDSTACKBLOCKFACTORY_H
