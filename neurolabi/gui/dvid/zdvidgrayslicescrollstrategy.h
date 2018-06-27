#ifndef ZDVIDGRAYSLICESCROLLSTRATEGY_H
#define ZDVIDGRAYSLICESCROLLSTRATEGY_H

#include "zscrollslicestrategy.h"

class ZDvidGraySlice;

class ZDvidGraySliceScrollStrategy : public ZScrollSliceStrategy
{
public:
  ZDvidGraySliceScrollStrategy(ZStackView *view);

  int scroll(int slice, int step) const;

  void setGraySlice(ZDvidGraySlice *slice);

private:
  ZDvidGraySlice *m_graySlice;

};

#endif // ZDVIDGRAYSLICESCROLLSTRATEGY_H
