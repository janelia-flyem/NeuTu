#ifndef ZSTACKPATCH_H
#define ZSTACKPATCH_H

#include "zstackobject.h"
#include "zimage.h"
#include "zpoint.h"

class ZStack;

class ZStackPatch : public ZStackObject
{
public:
  ZStackPatch(ZStack *stack = NULL);
  ~ZStackPatch();

public:
  virtual void display(ZPainter &painter, int slice, EDisplayStyle option,
                       NeuTube::EAxis sliceAxis) const;

  virtual const std::string& className() const;

  ZImage getImage(int z) const;
  ZPoint getFinalOffset() const;
  void setFinalOffset(double dx, double dy);
  int getZOffset() const;
  void setScale(double sx, double sy);

  ZStack* getStack() { return m_stack; }

  inline void setXScale(double sx) { m_sx = sx; }
  inline void setYScale(double sy) { m_sy = sy; }

private:
  ZStack *m_stack;
  double m_sx;
  double m_sy;
  ZPoint m_offset; //additional offset (after scaling)
};

#endif // ZSTACKPATCH_H
