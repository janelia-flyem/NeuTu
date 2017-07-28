#ifndef Z3DGL_H
#define Z3DGL_H

#include "zglmutils.h"
#include "zutils.h"
#include <glbinding/gl/gl.h>

using namespace gl;
#define ATLAS_CHECK_OPENGL_ERROR_FOR_ALL_GL_CALLS

enum class Z3DEye
{
  Left = 0, Mono, Right
};

enum class Z3DScreenShotType
{
  MonoView, HalfSideBySideStereoView, FullSideBySideStereoView
};

bool GLVersionGE(int majorVersion, int minorVersion);

void _CheckGLError(const char* file, int line);

#ifdef ATLAS_CHECK_OPENGL_ERROR_FOR_ALL_GL_CALLS
#define CHECK_GL_ERROR
#else
#define CHECK_GL_ERROR _CheckGLError(__FILE__, __LINE__);
#endif

bool checkGLState(GLenum pname, bool value);

bool checkGLState(GLenum pname, GLint value);

bool checkGLState(GLenum pname, GLenum value);

bool checkGLState(GLenum pname, GLfloat value);

bool checkGLState(GLenum pname, const glm::vec4& value);

#endif  //Z3DGL_H
