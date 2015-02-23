#include "zscalablestack.h"
#include "zstack.hxx"

ZScalableStack::ZScalableStack() : m_stack(NULL)
{
}

ZScalableStack::ZScalableStack(ZStack *stack) : m_stack(stack)
{

}

ZScalableStack::ZScalableStack(ZStack *stack, double sx, double sy, double sz) :
  m_stack(stack), m_sx(sx), m_sy(sy), m_sz(sz)
{
}

ZScalableStack::~ZScalableStack()
{
  clear();
}

void ZScalableStack::clear()
{
  delete m_stack;
  m_stack = NULL;
}

ZStack* ZScalableStack::releaseStack()
{
  ZStack *stack = getStack();

  m_stack = NULL;

  return stack;
}

void ZScalableStack::setScale(double sx, double sy, double sz)
{
  m_sx = sx;
  m_sy = sy;
  m_sz = sz;
}

ZPoint ZScalableStack::getOffset() const
{
  ZPoint pt;
  if (m_stack != NULL) {
    pt = m_stack->getOffset().toPoint();
    pt *= ZPoint(m_sx, m_sy, m_sz);
  }

  return pt;
}

