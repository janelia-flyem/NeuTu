#ifndef ZFLYEMNEURONFEATURE_H
#define ZFLYEMNEURONFEATURE_H

#include <string>

/*!
 * \brief The class of managing flyem neuron features
 */
class ZFlyEmNeuronFeature
{
public:
  enum EFeatureId {
    LEAF_NUMBER, BRANCH_POINT_NUMBER, BOX_VOLUME, MAX_SEGMENT_LENGTH,
    MAX_BRANCH_PATH_LENGTH, RADIUS_MEAN, RADIUS_VARIANCE, LATERAL_VERTICAL_RATIO,
    AVERAGE_CURVATURE, MOST_SPREAD_LAYER, ARBOR_SPREAD, OVERALL_LENGTH,
    BRANCH_NUMBER, TBAR_NUMBER, PSD_NUMBER, CENTROID_X, CENTROID_Y,
    UNDEFINED
  };

  ZFlyEmNeuronFeature();
  ZFlyEmNeuronFeature(EFeatureId id, const std::string &name);

  inline EFeatureId getId() const {
    return m_id;
  }

  inline double getValue() const {
    return m_value;
  }

  inline void setValue(double value) {
    m_value = value;
  }

private:
  EFeatureId m_id;
  std::string m_name;
  double m_value;
};

#endif // ZFLYEMNEURONFEATURE_H
