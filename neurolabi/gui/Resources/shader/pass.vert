#if GLSL_VERSION >= 130
in vec3 attr_vertex;
#else
attribute vec3 attr_vertex;
#endif

void main()
{
    gl_Position = vec4(attr_vertex, 1.0);
}
