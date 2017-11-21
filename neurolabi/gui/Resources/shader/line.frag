uniform float alpha;
uniform bool lighting_enabled = true;

#if GLSL_VERSION >= 130
in vec4 color;
#else
varying vec4 color;
#endif

#if GLSL_VERSION >= 330
layout(location = 0) out vec4 FragData0;
#elif GLSL_VERSION >= 130
out vec4 FragData0;  // call glBindFragDataLocation before linking
#else
#define FragData0 gl_FragData[0]
#endif

void main(void)
{
  FragData0 = !lighting_enabled ? color : vec4(color.rgb * color.a * alpha, color.a * alpha);
}

