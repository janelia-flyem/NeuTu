#if GLSL_VERSION >= 130
in vec3 texCoord0;
#else
varying vec3 texCoord0;
#endif

#if GLSL_VERSION >= 330
layout(location = 0) out vec4 FragData0;
#elif GLSL_VERSION >= 130
out vec4 FragData0;  // call glBindFragDataLocation before linking
#else
#define FragData0 gl_FragData[0]
#endif

void main()
{
    FragData0 = vec4(texCoord0, 1.0);
}
