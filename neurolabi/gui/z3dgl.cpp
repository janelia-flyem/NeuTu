#include "z3dgl.h"

#include <glbinding/ContextInfo.h>
#include <glbinding/Version.h>
#include <glbinding/Meta.h>

#include "logging/zqslog.h"

bool GLVersionGE(int majorVersion, int minorVersion)
{
  return glbinding::ContextInfo::version() >= glbinding::Version(majorVersion, minorVersion);
}

void _CheckGLError(const char* file, int line)
{
  GLenum err = glGetError();

  if (err != GL_NO_ERROR) {
    LERRORF(file, line, "a") << "OpenGL error: " << glbinding::Meta::getString(err);
  }
}

bool checkGLState(GLenum pname, bool value)
{
  GLboolean b;
  glGetBooleanv(pname, &b);
  return (static_cast<bool>(b) == value);
}

bool checkGLState(GLenum pname, GLint value)
{
  GLint i;
  glGetIntegerv(pname, &i);
  return (i == value);
}

bool checkGLState(GLenum pname, GLenum value)
{
  GLint i;
  glGetIntegerv(pname, &i);
  return (GLenum(i) == value);
}

bool checkGLState(GLenum pname, GLfloat value)
{
  GLfloat f;
  glGetFloatv(pname, &f);
  return (f == value);
}

bool checkGLState(GLenum pname, const glm::vec4& value)
{
  glm::vec4 v;
  glGetFloatv(pname, &v[0]);
  return (v == value);
}
