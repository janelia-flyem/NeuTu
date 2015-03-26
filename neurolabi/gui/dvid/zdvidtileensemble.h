#ifndef ZDVIDTILEENSEMBLE_H
#define ZDVIDTILEENSEMBLE_H

#include <vector>

#include "zstackobject.h"
#include "zdvidtileinfo.h"
#include "zdvidtarget.h"
#include "zdvidtile.h"

class ZStackView;

class ZDvidTileEnsemble : public ZStackObject
{
public:
  ZDvidTileEnsemble();
  ~ZDvidTileEnsemble();

  void clear();

  void display(ZPainter &painter, int slice, EDisplayStyle option) const;

  ZDvidTile* getTile(int resLevel, const ZDvidTileInfo::TIndex &index);

  void setDvidTarget(const ZDvidTarget &dvidTarget);
  void attachView(ZStackView *view);

  virtual const std::string& className() const;

private:
  std::vector<std::map<ZDvidTileInfo::TIndex, ZDvidTile*> > m_tileGroup;
  ZDvidTileInfo m_tilingInfo;
  ZDvidTarget m_dvidTarget;
  ZStackView *m_view;
};

#endif // ZDVIDTILEENSEMBLE_H
