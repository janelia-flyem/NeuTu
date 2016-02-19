#if GLSL_VERSION >= 130
in vec3 attr_vertex;
in vec2 attr_2dTexCoord0;
in vec4 attr_color;
#else
attribute vec3 attr_vertex;
attribute vec2 attr_2dTexCoord0;
attribute vec4 attr_color;
#endif

uniform mat4 projection_view_matrix;

#if GLSL_VERSION >= 130
out vec2 texCoord0;
out vec4 color;
#else
varying vec2 texCoord0;
varying vec4 color;
#endif

void main()
{
    gl_Position = projection_view_matrix * vec4(attr_vertex, 1.0);
    texCoord0 = attr_2dTexCoord0;
    color = attr_color;
}