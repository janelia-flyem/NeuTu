#if GLSL_VERSION >= 130
in vec3 attr_vertex;
#ifndef DISABLE_TEXTURE_COORD_OUTPUT
in vec3 attr_3dTexCoord0;
#endif
#else
attribute vec3 attr_vertex;
#ifndef DISABLE_TEXTURE_COORD_OUTPUT
attribute vec3 attr_3dTexCoord0;
#endif
#endif

uniform mat4 projection_view_matrix;

#if GLSL_VERSION >= 130
#ifndef DISABLE_TEXTURE_COORD_OUTPUT
out vec3 texCoord0;
#endif
#else
#ifndef DISABLE_TEXTURE_COORD_OUTPUT
varying vec3 texCoord0;
#endif
#endif

void main()
{
  gl_Position = projection_view_matrix * vec4(attr_vertex, 1.0);
#ifndef DISABLE_TEXTURE_COORD_OUTPUT
  texCoord0 = attr_3dTexCoord0;
#endif
}
