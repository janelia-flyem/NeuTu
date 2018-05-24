uniform vec2 screen_dim_RCP;

uniform sampler2D color_texture_0;
uniform sampler2D depth_texture_0;
uniform sampler2D color_texture_1;
uniform sampler2D depth_texture_1;

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

  vec4 color0 = texture(color_texture_0, texCoords);
  float depth0 = texture(depth_texture_0, texCoords).r;
  vec4 color1 = texture(color_texture_1, texCoords);
  float depth1 = texture(depth_texture_1, texCoords).r;

#if defined(DEPTH_TEST)
  if (depth0 < depth1) {
    FragData0 = color0;
    gl_FragDepth = depth0;
  } else {
    FragData0 = color1;
    gl_FragDepth = depth1;
  }
#elif defined(FIRST_ON_TOP)
  if (color0.a > 0.0) {
    FragData0 = color0;
    gl_FragDepth = depth0;
  } else if (color1.a > 0.0) {
    FragData0 = color1;
    gl_FragDepth = depth1;
  }
#elif defined(SECOND_ON_TOP)
  if (color1.a > 0.0) {
    FragData0 = color1;
    gl_FragDepth = depth1;
  } else if (color0.a > 0.0) {
    FragData0 = color0;
    gl_FragDepth = depth0;
  }
#elif defined(DEPTH_TEST_BLENDING)
  if (depth1 < depth0) {
    // use premultiplied alpha
    FragData0 = color1 + (1 - color1.a) * color0;
  } else {
    // use premultiplied alpha
    FragData0 = color0 + (1 - color0.a) * color1;
  }
  gl_FragDepth = min(depth0, depth1);
#elif defined(FIRST_ON_TOP_BLENDING)
  // use premultiplied alpha
  FragData0 = color0 + (1 - color0.a) * color1;
  gl_FragDepth = color0.a > 0.0 ? depth0 : depth1;
#elif defined(SECOND_ON_TOP_BLENDING)
  // use premultiplied alpha
  FragData0 = color1 + (1 - color1.a) * color0;
  gl_FragDepth = color1.a > 0.0 ? depth1 : depth0;
#endif
}
