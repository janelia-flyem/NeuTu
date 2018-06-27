#ifndef ZSTACKBLENDER_H
#define ZSTACKBLENDER_H

class ZStack;

class ZStackBlender
{
public:
  ZStackBlender();

  enum EMode {
    BLEND_NORMAL, BLEND_VALUE_WEIGHT, BLEND_NO_BLACK
  };

  void setBlendingMode(EMode mode) {
    m_blendingMode = mode;
  }

  ZStack *blend(const ZStack &stack1,
                const ZStack &stack2, double alpha);

private:
  EMode m_blendingMode = BLEND_VALUE_WEIGHT;
};

#endif // ZSTACKBLENDER_H
