#include "zcolor.h"

#include <cmath>

ZColor::ZColor()
{

}

ZColor::ZColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) :
  m_red(r), m_green(g), m_blue(b), m_alpha(a)
{
}

namespace {
inline double gray_to_double(uint8_t g)
{
  return double(g) / 255.0;
}

inline uint8_t double_to_gray(double v)
{
  return (v >= 1.0) ? 255 : ((v <= 0.0) ? 0 : (std::lround(v * 255.0)));
}

}

double ZColor::redF() const
{
  return gray_to_double(red());
}

double ZColor::greenF() const
{
  return gray_to_double(green());
}
double ZColor::blueF() const
{
  return gray_to_double(blue());
}

double ZColor::alphaF() const
{
  return gray_to_double(alpha());
}

void ZColor::setRedF(double v)
{
  setRed(double_to_gray(v));
}

void ZColor::setGreenF(double v)
{
  setGreen(double_to_gray(v));
}

void ZColor::setBlueF(double v)
{
  setBlue(double_to_gray(v));
}

void ZColor::setAlphaF(double v)
{
  setAlpha(double_to_gray(v));
}
