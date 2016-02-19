#ifndef ZFLYEMNEURONIMAGEFACTORY_H
#define ZFLYEMNEURONIMAGEFACTORY_H

#include "c_stack.h"
#include "flyem/zflyemneuron.h"
#include "neutube.h"

/*!
 * \brief The class of creating an image of a neuron
 */
class ZFlyEmNeuronImageFactory
{
public:
  ZFlyEmNeuronImageFactory();

  enum ESizePolicy {
    SIZE_BOUND_BOX, SIZE_SOURCE
  };

  void setSizePolicy(
      ESizePolicy xPolicy, ESizePolicy yPolicy, ESizePolicy zPolicy);
  void setSizePolicy(NeuTube::EAxis axis, ESizePolicy policy);

  void setDownsampleInterval(int dx, int dy, int dz);

  void setSourceDimension(int width, int height, int depth);

  Stack* createImage(const ZFlyEmNeuron &neuron) const;
  Stack* createImage(const ZObject3dScan &obj) const;

  Stack* createSurfaceImage(const ZObject3dScan &obj) const;

  int getSourceDimension(NeuTube::EAxis axis) const;

private:
  ESizePolicy m_sizePolicy[3];
  int m_sourceDimension[3];
  int m_downsampleInterval[3];
};

#endif // ZFLYEMNEURONIMAGEFACTORY_H
