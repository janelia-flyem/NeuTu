#ifndef ZCONTRASTPROTOCOL_H
#define ZCONTRASTPROTOCOL_H

#include "tz_stdint.h"

class ZJsonObject;

/*!
 * \brief The class for adjusting image contrast
 */
class ZContrastProtocol
{
public:
  enum class ENonlinearMode {
    NONE, POWER, SIGMOID
  };

  ZContrastProtocol();
  ZContrastProtocol(double offset, double scale, ENonlinearMode nonlinear);

  void setOffset(double offset);
  void setScale(double scale);
  void setNonlinear(ENonlinearMode mode);

  double getOffset() const;
  double getScale() const;
  bool isNonlinear() const;
  ENonlinearMode getNonlinearMode() const;

  uint8_t mapGrey(uint8_t v);
//  int mapInt(int v);
  double mapFloat(double v);

  /*!
   * \brief Set protocal from a json object
   *
   * Example: {"offset": -0.2, "scale": 1.8, "nonlinear": true}
   *
   * Non-exist fields will be ignored and the corresponding values in the object
   * will be kept unchanged.
   */
  void load(const ZJsonObject &obj);
  ZJsonObject toJsonObject() const;

  inline bool hasNoEffect() const {
    return (m_offset == 0.0) && (m_scale == 1.0) &&
        (m_nonlinearMode == ENonlinearMode::NONE);
  }

  void setDefaultNonLinear();


private:
  double m_offset;
  double m_scale;
  ENonlinearMode m_nonlinearMode;
};


#endif // ZCONTRASTPROTOCOL_H
