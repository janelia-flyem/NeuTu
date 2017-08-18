#ifndef ZVERTEXARRAYOBJECT_H
#define ZVERTEXARRAYOBJECT_H


#include "z3dgl.h"
#include "z3dcontext.h"
#include <vector>

class ZVertexArrayObject
{
public:
  explicit ZVertexArrayObject(GLsizei n = 1);

  ~ZVertexArrayObject();

  ZVertexArrayObject(ZVertexArrayObject&&) = default;

  ZVertexArrayObject& operator=(ZVertexArrayObject&&) = default;

  ZVertexArrayObject(const ZVertexArrayObject&) = default;

  ZVertexArrayObject& operator=(const ZVertexArrayObject&) = default;

  void bind(size_t idx = 0) const;

  void release() const;

  void resize(GLsizei n);

  inline void clear()
  { resize(0); }

private:
  bool m_hardwareSupportVAO;
  std::vector<GLuint> m_arrays;

#ifdef CHECK_OPENGL_ERROR_FOR_ALL_GL_CALLS
  Z3DContext m_context;
#endif
};

#endif // ZVERTEXARRAYOBJECT_H
