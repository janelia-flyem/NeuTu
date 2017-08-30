#ifndef ZVERTEXBUFFEROBJECT_H
#define ZVERTEXBUFFEROBJECT_H


#include "z3dgl.h"
#include <vector>

class ZVertexBufferObject
{
public:
  explicit ZVertexBufferObject(GLsizei n = 1);

  ~ZVertexBufferObject();

  ZVertexBufferObject(ZVertexBufferObject&&) = default;

  ZVertexBufferObject& operator=(ZVertexBufferObject&&) = default;

  ZVertexBufferObject(const ZVertexBufferObject&) = default;

  ZVertexBufferObject& operator=(const ZVertexBufferObject&) = default;

  void bind(GLenum target, size_t idx = 0);

  void release(GLenum target);

  void resize(GLsizei n);

  inline void clear()
  { resize(0); }

private:
  std::vector<GLuint> m_arrays;
};

#endif // ZVERTEXBUFFEROBJECT_H
