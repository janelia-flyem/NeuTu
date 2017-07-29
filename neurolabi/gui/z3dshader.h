#ifndef Z3DSHADER_H
#define Z3DSHADER_H

#include "z3dcontext.h"
#include <QString>

// throw ZGLException if error
class Z3DShader
{
public:
  enum class Type
  {
    Vertex, Fragment, Geometry, TessellationControl, TessellationEvaluation, Compute
  };

  explicit Z3DShader(Z3DShader::Type type);

  ~Z3DShader();

  Z3DShader(const Z3DShader&) = delete;

  Z3DShader& operator=(const Z3DShader&) = delete;

  Z3DShader::Type shaderType() const
  { return m_type; }

  void compileSourceCode(const char* source);

  void compileSourceCode(const QString& source);

  QByteArray sourceCode() const;

  bool isCompiled() const
  { return m_compiled; }

  unsigned int shaderId() const
  { return m_id; }

  Z3DContext context() const
  { return m_context; }

private:
  Type m_type;
  bool m_compiled;
  unsigned int m_id;
  Z3DContext m_context;
};

#endif // Z3DSHADER_H
