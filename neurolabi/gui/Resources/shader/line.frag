uniform float alpha;
uniform bool no_alpha = false;

#if GLSL_VERSION >= 130
in vec4 color;
#else
varying vec4 color;
#endif

void main(void)
{
  FragData0 = no_alpha ? color : vec4(color.rgb * color.a * alpha, color.a * alpha);
}

