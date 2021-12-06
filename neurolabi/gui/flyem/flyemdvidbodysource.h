#ifndef FLYEMDVIDBODYSOURCE_H
#define FLYEMDVIDBODYSOURCE_H

#include <memory>
#include <mutex>

#include "flyembodysource.h"
#include "dvid/zdvidinfo.h"

class ZDvidWriter;
class ZDvidTarget;
class ZDvidReader;

/*!
 * \brief The class of retrieving bodies from DVID
 */
class FlyEmDvidBodySource : public FlyEmBodySource
{
public:
  FlyEmDvidBodySource();

  void setDvidTarget(const ZDvidTarget &target);

  ZObject3dScan* getSparsevol(uint64_t bodyId, int dsLevel) const override;
  ZObject3dScan* getSparsevol(
        uint64_t bodyId, int dsLevel, const ZIntCuboid &range) const override;
  ZObject3dScan* getCoarseSparsevol(uint64_t bodyId) const override;
  ZObject3dScan* getCoarseSparsevol(
        uint64_t bodyId, const ZIntCuboid &range) const override;
  int getCoarseSparsevolScale() const override;

  ZIntPoint getBlockSize() const override;

  // Exposes these two pointers for resource sharing
  ZDvidReader* getReader() const;
  std::mutex* getIoMutex() const;

  ZIntCuboid getBoundBox(uint64_t bodyId) const override;
  ZIntCuboid getBoundBox(uint64_t bodyId, int dsLevel) const override;

private:
  mutable std::mutex m_ioMutex; // for making data io thread safe
  std::shared_ptr<ZDvidWriter> m_writer;
  ZDvidInfo m_dataInfo;
};

#endif // FLYEMDVIDBODYSOURCE_H
