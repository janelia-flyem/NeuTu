#ifndef ZSWCRADIUSESTIMATOR_H
#define ZSWCRADIUSESTIMATOR_H

#include "neutube_def.h"

class ZSwcRadiusEstimator
{
public:
  ZSwcRadiusEstimator();

  void setBackground(NeuTube::EImageBackground bg) {
    m_background = bg;
  }

private:
  void init();

private:
  NeuTube::EImageBackground m_background;

};

#endif // ZSWCRADIUSESTIMATOR_H
