#ifndef Z3DSDFONT_H
#define Z3DSDFONT_H

#include "z3dtexture.h"
#include <QString>
#include <QImage>

class Z3DSDFont
{
public:
  struct CharInfo
  {
    explicit CharInfo(int id_ = 0, int x_ = 0, int y_ = 0, int width_ = 0, int height_ = 0,
                      float xoffset_ = 0.f, float yoffset_ = 0.f,
                      float xadvance_ = 0.f, int page_ = 0, int chnl_ = 0, int texWidth = 1, int texHeight = 1)
      : id(id_), x(x_), y(y_), width(width_), height(height_), xoffset(xoffset_), yoffset(yoffset_)
      , xadvance(xadvance_), page(page_), chnl(chnl_)
    {
      sMin = static_cast<float>(x) / static_cast<float>(texWidth);
      tMin = static_cast<float>(y + height) / static_cast<float>(texHeight);

      sMax = static_cast<float>(x + width) / static_cast<float>(texWidth);
      tMax = static_cast<float>(y) / static_cast<float>(texHeight);
    }

    int id;
    int x;
    int y;
    int width;
    int height;
    float xoffset;
    float yoffset;
    float xadvance;
    int page;
    int chnl;

    float sMin;
    float sMax;
    float tMin;
    float tMax;
  };

  Z3DSDFont(const QString& imageFileName, const QString& txtFileName);

  inline QString fontName() const
  { return m_fontName; }

  inline int maxFontHeight() const
  { return m_maxFontHeight; }

  bool isEmpty() const
  { return m_isEmpty; }

  CharInfo charInfo(int id) const;

  Z3DTexture* texture();

protected:
  void loadImage();

  void parseFontFile();

  void createTexture();

private:
  QString m_imageFileName;
  QImage m_GLFormattedImage;
  QString m_txtFileName;

  QString m_fontName;
  bool m_isEmpty;   //if load image or txt failed, the font is empty
  QList<CharInfo> m_charInfos;
  int m_maxFontHeight;

  std::unique_ptr<Z3DTexture> m_texture;
};

#endif // Z3DSDFONT_H
