uniform float alpha;
uniform bool lighting_enabled = true;

#if GLSL_VERSION >= 130
in vec4 color;
#else
varying vec4 color;
#endif

void fragment_func(out vec4 fragColor, out float fragDepth)
{
  fragDepth = gl_FragCoord.z;
  fragColor = !lighting_enabled ? color : vec4(color.rgb * color.a * alpha, color.a * alpha);
}

