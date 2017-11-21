uniform vec2 screen_dim_RCP;

uniform sampler2D color_texture;
uniform sampler2D depth_texture;
uniform bool discard_transparent = false;

#if GLSL_VERSION >= 330
layout(location = 0) out vec4 FragData0;
#elif GLSL_VERSION >= 130
out vec4 FragData0;  // call glBindFragDataLocation before linking
#else
#define FragData0 gl_FragData[0]
#endif

#if GLSL_VERSION < 130
#define texture texture2D
#endif

void main()
{
  vec2 texCoords = gl_FragCoord.xy * screen_dim_RCP;
  vec4 fragColor = texture(color_texture, texCoords);

  if (discard_transparent && fragColor.a == 0.0)
    discard;

#ifdef Multiply_Alpha
  fragColor.rgb *= fragColor.a;
#endif
#ifdef Divide_By_Alpha
  if (fragColor.a > 0.0)
    fragColor.rgb /= fragColor.a;
#endif

  FragData0 = fragColor;

  gl_FragDepth = texture(depth_texture, texCoords).r;
}
