#include "zcontrastprotocol.h"

#include <cmath>
#include "tz_math.h"
#include "zjsonparser.h"
#include "zjsonobject.h"

ZContrastProtocol::ZContrastProtocol()
{
  setDefaultNonLinear();
}

ZContrastProtocol::ZContrastProtocol(
    double offset, double scale, ENonlinearMode nonlinear) :
  m_offset(offset), m_scale(scale), m_nonlinearMode(nonlinear)
{
}

double ZContrastProtocol::getOffset() const
{
  return m_offset;
}

double ZContrastProtocol::getScale() const
{
  return m_scale;
}

bool ZContrastProtocol::isNonlinear() const
{
  return m_nonlinearMode != NONLINEAR_NONE;
}

ZJsonObject ZContrastProtocol::toJsonObject() const
{
  ZJsonObject obj;

  obj.setEntry("offset", m_offset);
  obj.setEntry("scale", m_scale);
  obj.setEntry("nonlinear", int(m_nonlinearMode));

  return obj;
}

void ZContrastProtocol::setNonlinear(ENonlinearMode mode)
{
  m_nonlinearMode = mode;
}

void ZContrastProtocol::setOffset(double offset)
{
  m_offset = offset;
}

void ZContrastProtocol::setScale(double scale)
{
  m_scale = scale;
}

void ZContrastProtocol::load(const ZJsonObject &obj)
{
  if (obj.hasKey("nonlinear")) {
    if (ZJsonParser::IsBoolean(obj["nonlinear"])) {
      bool nonlinear = ZJsonParser::booleanValue(obj["nonlinear"]);
      if (nonlinear) {
        m_nonlinearMode = NONLINEAR_POWER;
      } else {
        m_nonlinearMode = NONLINEAR_NONE;
      }
    } else {
      m_nonlinearMode = static_cast<ENonlinearMode>(
            ZJsonParser::integerValue(obj["nonlinear"]));
    }
  }

  if (obj.hasKey("offset")) {
    m_offset = ZJsonParser::numberValue(obj["offset"]);
  }

  if (obj.hasKey("scale")) {
    m_scale = ZJsonParser::numberValue(obj["scale"]);
  }
}

uint8_t ZContrastProtocol::mapGrey(uint8_t v)
{
  if (hasNoEffect()) {
    return v;
  }

  double nv = mapFloat(v / 255.0);

  return iround(nv * 255.0);
}

/*
int ZContrastProtocol::mapInt(int v)
{
  if (hasNoEffect()) {
    return v;
  }

  double s = m_scale;
  double nv = (v + m_offset) * s;
  if (m_nonlinear) {
    double y0 = -1.0 / (1 + std::exp(-m_offset * m_scale));

    nv = 1.0 / (1.0 + std::exp(-nv)) + y0;
  }

  return iround(nv);
}
*/

/*!
 * \brief Map a float to be within the range of [0, 1].
 *
 * Nonlinear mapping: 1/(1+exp(-(x+x0)*s)), 0->0, 1->1.
 */
double ZContrastProtocol::mapFloat(double v)
{
  if (hasNoEffect()) {
    return v;
  }

  double s = m_scale;
  double nv = (v + m_offset) * s;
  switch (m_nonlinearMode) {
  case NONLINEAR_POWER:
    nv = std::max(0.0, (v + m_offset / 255.0) * s);
    nv = sqrt(nv) * v;
    break;
  case NONLINEAR_SIGMOID:
  {
    double y0 = -1.0 / (1 + std::exp(-m_offset * m_scale));
    double ym = 1.0 / (1 + std::exp(-m_scale * (1.0 + m_offset))) + y0;

    nv = (1.0 / (1.0 + std::exp(-nv)) + y0) / ym;
  }
    break;
  case NONLINEAR_NONE:
    break;
  }

  if (nv < 0.0) {
    nv = 0.0;
  } else if (nv > 1.0) {
    nv = 1.0;
  }

  return nv;
}

void ZContrastProtocol::setDefaultNonLinear()
{
//  setOffset(-1.0);
//  setScale(2.197);
  setOffset(-0.5);
  setScale(4.197);
  setNonlinear(NONLINEAR_SIGMOID);
}
