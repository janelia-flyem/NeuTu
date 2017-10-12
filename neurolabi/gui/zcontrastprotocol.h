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
  enum ENonlinearMode {
    NONLINEAR_NONE, NONLINEAR_POWER, NONLINEAR_SIGMOID
  };

  ZContrastProtocol();
  ZContrastProtocol(double offset, double scale, ENonlinearMode nonlinear);

  uint8_t mapGrey(uint8_t v);
//  int mapInt(int v);
  double mapFloat(double v);

  void load(const ZJsonObject &obj);
  ZJsonObject toJsonObject() const;

  void setOffset(double offset);
  void setScale(double scale);
  void setNonlinear(ENonlinearMode mode);

  inline bool hasNoEffect() const {
    return (m_offset == 0.0) && (m_scale == 1.0) &&
        (m_nonlinearMode == NONLINEAR_NONE);
  }

  void setDefaultNonLinear();

private:
  double m_offset;
  double m_scale;
  ENonlinearMode m_nonlinearMode;
};


#endif // ZCONTRASTPROTOCOL_H
