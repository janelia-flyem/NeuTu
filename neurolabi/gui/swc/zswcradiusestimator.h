#ifndef ZSWCRADIUSESTIMATOR_H
#define ZSWCRADIUSESTIMATOR_H

#include "common/neutudefs.h"

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
