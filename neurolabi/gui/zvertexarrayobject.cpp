#include "zvertexarrayobject.h"

#include "z3dgpuinfo.h"
#include "QsLog.h"

#if defined(__APPLE__) && !defined(_USE_CORE_PROFILE_)
#undef glGenVertexArrays
#undef glBindVertexArray
#undef glDeleteVertexArrays
#define glGenVertexArrays glGenVertexArraysAPPLE
#define glBindVertexArray glBindVertexArrayAPPLE
#define glDeleteVertexArrays glDeleteVertexArraysAPPLE
#endif

ZVertexArrayObject::ZVertexArrayObject(GLsizei n)
  : m_hardwareSupportVAO(Z3DGpuInfo::instance().isVAOSupported())
  , m_arrays(std::max(GLsizei(0), n), 0)
{
  if (m_hardwareSupportVAO)
    glGenVertexArrays(m_arrays.size(), m_arrays.data());
}

ZVertexArrayObject::~ZVertexArrayObject()
{
#ifdef CHECK_OPENGL_ERROR_FOR_ALL_GL_CALLS
  CHECK(m_context == Z3DContext());
#endif
  if (!m_hardwareSupportVAO)
    return;
  glDeleteVertexArrays(m_arrays.size(), m_arrays.data());
}

void ZVertexArrayObject::bind(size_t idx) const
{
#ifdef CHECK_OPENGL_ERROR_FOR_ALL_GL_CALLS
  CHECK(m_context == Z3DContext());
#endif
  if (!m_hardwareSupportVAO)
    return;
  glBindVertexArray(m_arrays[idx]);
}

void ZVertexArrayObject::release() const
{
#ifdef CHECK_OPENGL_ERROR_FOR_ALL_GL_CALLS
  CHECK(m_context == Z3DContext());
#endif
  if (!m_hardwareSupportVAO)
    return;
  glBindVertexArray(0);
}

void ZVertexArrayObject::resize(GLsizei n)
{
#ifdef CHECK_OPENGL_ERROR_FOR_ALL_GL_CALLS
  CHECK(m_context == Z3DContext());
#endif
  if (!m_hardwareSupportVAO || n == GLsizei(m_arrays.size()))
    return;
  glDeleteVertexArrays(m_arrays.size(), m_arrays.data());
  m_arrays.resize(std::max(GLsizei(0), n), 0);
  glGenVertexArrays(m_arrays.size(), m_arrays.data());
}
