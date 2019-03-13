#ifndef ZCOLOR_H
#define ZCOLOR_H

#include <cstdint>

class ZColor
{
public:
  ZColor();
  ZColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

  inline uint8_t red() const { return m_red; }
  inline uint8_t green() const { return m_green; }
  inline uint8_t blue() const { return m_blue; }
  inline uint8_t alpha() const { return m_alpha; }

  inline void setRed(uint8_t r) { m_red = r; }
  inline void setGreen(uint8_t g) { m_green = g; }
  inline void setBlue(uint8_t b) { m_blue = b; }
  inline void setAlpha(uint8_t a) { m_alpha = a; }


  double redF() const;
  double greenF() const;
  double blueF() const;
  double alphaF() const;

  void setRedF(double v);
  void setGreenF(double v);
  void setBlueF(double v);
  void setAlphaF(double v);

private:
  uint8_t m_red = 0;
  uint8_t m_green = 0;
  uint8_t m_blue = 0;
  uint8_t m_alpha = 255;
};

#endif // ZCOLOR_H
