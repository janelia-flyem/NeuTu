#ifndef ZDVIDBODYHELPER_H
#define ZDVIDBODYHELPER_H

#include "neutube_def.h"
#include "zintcuboid.h"

class ZDvidReader;
class ZObject3dScan;
class ZObject3dScanArray;

class ZDvidBodyHelper
{
public:
  ZDvidBodyHelper(const ZDvidReader *reader);

  const ZDvidReader* getDvidReader() const;

  void setCanonizing(bool on);
  void setLabelType(flyem::EBodyLabelType type);
  void setRange(const ZIntCuboid &box);
  void setCoarse(bool on);
  void setZoom(int zoom);
  void setLowresZoom(int zoom);

  ZObject3dScan* readBody(uint64_t bodyId, ZObject3dScan *result = nullptr);
  std::vector<ZObject3dScan*> readHybridBody(uint64_t bodyId);

  ZDvidBodyHelper fork() const;

private:
  const ZDvidReader *m_reader = nullptr;
  bool m_canonizing = true;
  flyem::EBodyLabelType m_labelType = flyem::LABEL_BODY;
  ZIntCuboid m_range;
  bool m_coarseVol = false;
  int m_zoom = 0;
  int m_lowresZoom = 0;
};

#endif // ZDVIDBODYHELPER_H
