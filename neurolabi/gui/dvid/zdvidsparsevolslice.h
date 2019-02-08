#ifndef ZDVIDSPARSEVOLSLICE_H
#define ZDVIDSPARSEVOLSLICE_H

#include "zobject3dscan.h"
#include "zdvidtarget.h"
#include "zdvidreader.h"
#include "zstackviewparam.h"

class ZDvidSparsevolSlice : public ZObject3dScan
{
public:
  ZDvidSparsevolSlice();
  ZDvidSparsevolSlice(const ZDvidSparsevolSlice& obj);
  ~ZDvidSparsevolSlice();

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::DVID_SPARSEVOL_SLICE;
  }

  void setDvidTarget(const ZDvidTarget &target);
  void setReader(ZDvidReader *reader);

  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis sliceAxis) const;

  void update();
  bool update(const ZStackViewParam &viewParam);

  bool isSliceVisible(int z, neutu::EAxis axis) const;

private:
  ZDvidSparsevolSlice& operator=(const ZDvidSparsevolSlice& obj);
  void forceUpdate(const ZStackViewParam &viewParam, bool ignoringHidden);
  bool updateRequired(const ZStackViewParam &viewParam) const;
//  bool updateRequired(int z) const;
  void forceUpdate(int z);

private:
//  int m_currentZ;
  ZStackViewParam m_currentViewParam;
  bool m_isFullView;
  ZDvidTarget m_dvidTarget;
  ZDvidReader m_reader;
  ZDvidReader *m_externalReader;
};

#endif // ZDVIDSPARSEVOLSLICE_H
