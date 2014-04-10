#ifndef ZFLYEMQUALITYANALYZER_H
#define ZFLYEMQUALITYANALYZER_H

#include "flyem/zintcuboidarray.h"
#include "zobject3dscan.h"
#include "flyem/zflyemneuronrange.h"
#include "flyem/zflyemneuron.h"
#include "flyem/zhotspot.h"
#include "flyem/zhotspotarray.h"
#include "zprogressable.h"

class ZPointArray;
class ZFlyEmDataBundle;

class ZFlyEmQualityAnalyzer : public ZProgressable
{
public:
  ZFlyEmQualityAnalyzer();

  bool isExitingOrphanBody(const ZObject3dScan &obj);

  //Boundary orphan
  bool isStitchedOrphanBody(const ZObject3dScan &obj);

  //Local orphan
  bool isOrphanBody(const ZObject3dScan &obj);

  class SubstackRegionCalbration {
  public:
    SubstackRegionCalbration();

    void setMargin(int x, int y, int z);
    void setBounding(bool x, bool y, bool z);
    void calibrate(FlyEm::ZIntCuboidArray &roi) const;

  private:
    int m_margin[3];
    bool m_bounding[3];
  };

  void setSubstackRegion(const FlyEm::ZIntCuboidArray &roi);
  void setSubstackRegion(const FlyEm::ZIntCuboidArray &roi,
                         const SubstackRegionCalbration &calbr);

  /*!
   * \brief Label SWC node that is out of range.
   *
   * Labels any node in the model of \a neuron with the value \a label if the
   * node is out of the range.
   */
  static void labelSwcNodeOutOfRange(const ZFlyEmNeuron &neuron,
                                     const ZFlyEmNeuronRange &range, int label);

  /*!
   * \brief Test if a body touching global boundary
   *
   * NOTE: This function assumes that at least a part \a obj is in the substack
   *       region.
   *
   * \return true iff \a obj touches the global boundary
   */
  bool touchingGlobalBoundary(const ZObject3dScan &obj);

  bool touchingSideBoundary(const ZObject3dScan &obj);

  FlyEm::ZHotSpotArray& computeHotSpot(const ZSwcTree *tree, ZObject3dScan *obj, double xRes, double yRes,
      double zRes, double lengthThre);

  FlyEm::ZHotSpotArray& computeHotSpot(const ZFlyEmNeuron &neuron);
  FlyEm::ZHotSpotArray& computeHotSpot(const ZFlyEmNeuron *neuron);

  FlyEm::ZHotSpotArray& computeHotSpot(const ZFlyEmNeuron &neuron,
                                       std::vector<ZFlyEmNeuron>& neuronArray);
  FlyEm::ZHotSpotArray& computeHotSpot(const ZFlyEmNeuron *neuron,
                                       std::vector<ZFlyEmNeuron>& neuronArray);

private:
  FlyEm::ZIntCuboidArray m_substackRegion;
  FlyEm::ZHotSpotArray m_hotSpotArray;

};

#endif // ZFLYEMQUALITYANALYZER_H
