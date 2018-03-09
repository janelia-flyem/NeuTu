#ifndef ZARBSLICESCROLLSTRATEGY_H
#define ZARBSLICESCROLLSTRATEGY_H

#include "zscrollslicestrategy.h"

class ZArbSliceScrollStrategy : public ZScrollSliceStrategy
{
public:
  ZArbSliceScrollStrategy(ZStackView *view);

  ZStackViewParam scroll(const ZStackViewParam &param, int step) const;

//  void scroll(int step);
};

#endif // ZARBSLICESCROLLSTRATEGY_H
