#if GLSL_VERSION >= 130
in vec3 texCoord0;
#else
varying vec3 texCoord0;
#endif

void main()
{
    FragData0 = vec4(texCoord0, 1.0);
}
