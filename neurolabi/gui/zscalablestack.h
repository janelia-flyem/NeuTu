#ifndef ZSCALABLESTACK_H
#define ZSCALABLESTACK_H

#include "zuncopyable.h"

class ZStack;
class ZPoint;

class ZScalableStack : public ZUncopyable
{
public:
  ZScalableStack();
  ZScalableStack(ZStack *stack);
  ZScalableStack(ZStack *stack, double sx, double sy, double sz);
  virtual ~ZScalableStack();

  void clear();

  void setScale(double sx, double sy, double sz);

  inline ZStack* getStack() { return m_stack; }
  ZStack* releaseStack();

  inline double getXScale() const { return m_sx; }
  inline double getYScale() const { return m_sy; }
  inline double getZScale() const { return m_sz; }

  ZPoint getOffset() const;

private:
  ZStack *m_stack;
  double m_sx;
  double m_sy;
  double m_sz;
};

#endif // ZSCALABLESTACK_H
