#ifndef ZDVIDTILEENSEMBLE_H
#define ZDVIDTILEENSEMBLE_H

#include <vector>

#include "libdvidheader.h"
#include "zstackobject.h"
#include "zdvidtileinfo.h"
#include "zdvidtarget.h"
#include "zdvidtile.h"
#include "zdvidreader.h"

class ZStackView;

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

public:
  bool update(
      const std::vector<ZDvidTileInfo::TIndex>& tileIndices, int resLevel, int z);
  void updateContrast();
#if defined(_ENABLE_LIBDVIDCPP_)
  void updateTile(libdvid::Slice2D slice,
                  int resLevel, const std::vector<int> &loc,
                  int z, ZDvidTile *tile, libdvid::DVIDNodeService *service);
#endif

private:
  std::vector<std::map<ZDvidTileInfo::TIndex, ZDvidTile*> > m_tileGroup;
  ZDvidTileInfo m_tilingInfo;
  ZDvidTarget m_dvidTarget;
  ZDvidReader m_reader;
  ZStackView *m_view;
  bool m_highContrast;
#if defined(_ENABLE_LIBDVIDCPP_)
  std::vector<libdvid::DVIDNodeService*> m_serviceArray;
#endif
};

#endif // ZDVIDTILEENSEMBLE_H
