#include "z3dcontext.h"

#include "QsLog.h"
#include <QOpenGLContext>

Z3DContext::Z3DContext()
  : m_context(QOpenGLContext::currentContext()->shareGroup())
{
  CHECK(m_context);
}

bool Z3DContext::operator<(const Z3DContext& rhs) const
{
  return m_context < rhs.m_context;
}

bool Z3DContext::operator==(const Z3DContext& rhs) const
{
  return m_context == rhs.m_context;
}

bool Z3DContext::operator!=(const Z3DContext& rhs) const
{
  return m_context != rhs.m_context;
}
