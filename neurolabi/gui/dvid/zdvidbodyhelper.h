#ifndef ZDVIDBODYHELPER_H
#define ZDVIDBODYHELPER_H

#include <memory>

#include "common/neutudefs.h"
#include "geometry/zintcuboid.h"

class ZDvidReader;
class ZObject3dScan;
class ZObject3dScanArray;
class FlyEmBodySource;

class ZDvidBodyHelper
{
public:
//  ZDvidBodyHelper(const ZDvidReader *reader);
  ZDvidBodyHelper(std::shared_ptr<FlyEmBodySource> source);

//  const ZDvidReader* getDvidReader() const;

  void setCanonizing(bool on);
//  void setLabelType(neutu::EBodyLabelType type);
  void setRange(const ZIntCuboid &box);
  void setCoarse(bool on);
  void setZoom(int zoom);
  void setLowresZoom(int zoom);

  ZObject3dScan* readBody(uint64_t bodyId);
  std::vector<std::shared_ptr<ZObject3dScan>> readHybridBody(uint64_t bodyId);

  ZDvidBodyHelper fork() const;

private:
  ZIntCuboid getAdjustedRange() const;

private:
//  const ZDvidReader *m_reader = nullptr;
  std::shared_ptr<FlyEmBodySource> m_bodySource;
  bool m_canonizing = true;
//  neutu::EBodyLabelType m_labelType = neutu::EBodyLabelType::BODY;
  ZIntCuboid m_range;
  bool m_coarseVol = false;
  int m_zoom = 0;
  int m_lowresZoom = 0;
};

#endif // ZDVIDBODYHELPER_H
