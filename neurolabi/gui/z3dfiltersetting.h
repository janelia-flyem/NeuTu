#ifndef Z3DFILTERSETTING_H
#define Z3DFILTERSETTING_H

#include <QString>

class ZJsonObject;

class Z3DFilterSetting
{
public:
  Z3DFilterSetting();

  void setVisible(bool visible);
  void setFront(bool front);
  void setSizeScale(double scale);

  bool isVisible() const;
  bool isFront() const;
  double getSizeScale() const;
  QString getColorMode() const;
  QString getShapeMode() const;


  void load(const ZJsonObject &obj);

public:
  static const char* VISIBLE_KEY;
  static const char* SIZE_SCALE_KEY;
  static const char* FRONT_KEY;
  static const char* COLOR_MODE_KEY;
  static const char* SHAPE_MODE_KEY;
  static const char* OPACITY_KEY;

private:
  void init();

private:
  bool m_visible;
  double m_sizeScale;
  bool m_front;
  QString m_colorMode;
  QString m_shapeMode;


};

#endif // Z3DFILTERSETTING_H
