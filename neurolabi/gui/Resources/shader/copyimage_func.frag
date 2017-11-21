uniform vec2 screen_dim_RCP;

uniform sampler2D color_texture;
uniform sampler2D depth_texture;
uniform bool discard_transparent = false;

#if GLSL_VERSION < 130
#define texture texture2D
#endif

void fragment_func(out vec4 fragColor, out float fragDepth)
{
  vec2 texCoords = gl_FragCoord.xy * screen_dim_RCP;
  fragColor = texture(color_texture, texCoords);

  if (discard_transparent && fragColor.a == 0.0)
    discard;

#ifdef Multiply_Alpha
  fragColor.rgb *= fragColor.a;
#endif
#ifdef Divide_By_Alpha
  if (fragColor.a > 0.0)
    fragColor.rgb /= fragColor.a;
#endif

  fragDepth = texture(depth_texture, texCoords).r;
}
