#ifndef ZDVIDSPARSEVOLSLICE_H
#define ZDVIDSPARSEVOLSLICE_H

#include "zobject3dscan.h"
#include "zdvidtarget.h"
#include "zdvidreader.h"

class ZDvidSparsevolSlice : public ZObject3dScan
{
public:
  ZDvidSparsevolSlice();
  ZDvidSparsevolSlice(const ZDvidSparsevolSlice& obj);

  void setDvidTarget(const ZDvidTarget &target);

  void display(ZPainter &painter, int slice, EDisplayStyle option) const;

  void update(int z);

private:
  ZDvidSparsevolSlice& operator=(const ZDvidSparsevolSlice& obj);


private:
  int m_currentZ;
  ZDvidTarget m_dvidTarget;
  ZDvidReader m_reader;
};

#endif // ZDVIDSPARSEVOLSLICE_H
