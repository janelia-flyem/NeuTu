#ifndef ZDVIDTILEENSEMBLE_H
#define ZDVIDTILEENSEMBLE_H

#include <QMutex>
#include <vector>

#include "libdvidheader.h"
#include "zstackobject.h"
#include "zdvidtileinfo.h"
#include "zdvidtarget.h"
#include "zdvidtile.h"
#include "zdvidreader.h"
#include "zsharedpointer.h"

class ZStackView;
class ZDvidPatchDataFetcher;

class ZDvidTileEnsemble : public ZStackObject
{
public:
  ZDvidTileEnsemble();
  ~ZDvidTileEnsemble();

  static ZStackObject::EType GetType() {
    return ZStackObject::TYPE_DVID_TILE_ENSEMBLE;
  }

  void clear();
  bool isEmpty() const;

  void display(ZPainter &painter, int slice, EDisplayStyle option,
               NeuTube::EAxis sliceAxis) const;

  ZDvidTile* getTile(int resLevel, const ZDvidTileInfo::TIndex &index);

  void setDvidTarget(const ZDvidTarget &dvidTarget);
  void attachView(ZStackView *view);

  virtual const std::string& className() const;

  int getCurrentZ() const;

  ZStackView* getView() const;

  const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  void enhanceContrast(bool high);
  void setContrastProtocal(const ZJsonObject &obj);

  void setDataFetcher(ZDvidPatchDataFetcher *fetcher);

public:
  bool update(
      const std::vector<ZDvidTileInfo::TIndex>& tileIndices, int resLevel, int z);
  void updateContrast();
  void updatePatch(const ZImage *patch, const ZIntCuboid &region);
//#if defined(_ENABLE_LIBDVIDCPP_)
//  void updateTile(libdvid::Slice2D slice,
//                  int resLevel, const std::vector<int> &loc,
//                  int z, ZDvidTile *tile, libdvid::DVIDNodeService *service);
//#endif

private:
  std::vector<std::map<ZDvidTileInfo::TIndex, ZDvidTile*> > m_tileGroup;
  ZDvidTileInfo m_tilingInfo;
  ZDvidTarget m_dvidTarget;
  ZDvidReader m_reader;
  ZStackView *m_view;
  mutable ZImage *m_patch;
  mutable ZIntCuboid m_patchRange;
  bool m_highContrast;
  ZJsonObject m_contrastProtocal;

  ZDvidPatchDataFetcher *m_dataFetcher;

  mutable QMutex m_updateMutex;

#if defined(_ENABLE_LIBDVIDCPP_)
  std::vector<ZSharedPointer<libdvid::DVIDNodeService> > m_serviceArray;
#endif
};

#endif // ZDVIDTILEENSEMBLE_H
