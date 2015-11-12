uniform float alpha;
uniform bool no_alpha = false;

#if GLSL_VERSION >= 130
in vec4 color;
#else
varying vec4 color;
#endif

void fragment_func(out vec4 fragColor0, out vec4 fragColor1, out float fragDepth)
{
  fragDepth = gl_FragCoord.z;
  fragColor0 = no_alpha ? color : vec4(color.rgb * color.a * alpha, color.a * alpha);
  fragColor1 = vec4(0.0,0.0,0.0,0.0);
}

