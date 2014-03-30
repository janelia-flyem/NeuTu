#if GLSL_VERSION >= 130
in vec3 attr_vertex;
in vec2 attr_2dTexCoord0;
#else
attribute vec3 attr_vertex;
attribute vec2 attr_2dTexCoord0;
#endif

#if GLSL_VERSION >= 130
out vec2 texCoord0;
#else
varying vec2 texCoord0;
#endif

void main()
{
    gl_Position = vec4(attr_vertex, 1.0);
    texCoord0 = attr_2dTexCoord0;
}
