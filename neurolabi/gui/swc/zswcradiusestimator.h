#ifndef ZSWCRADIUSESTIMATOR_H
#define ZSWCRADIUSESTIMATOR_H

#include "common/neutube_def.h"

class ZSwcRadiusEstimator
{
public:
  ZSwcRadiusEstimator();

  void setBackground(neutu::EImageBackground bg) {
    m_background = bg;
  }

private:
  void init();

private:
  neutu::EImageBackground m_background;

};

#endif // ZSWCRADIUSESTIMATOR_H
