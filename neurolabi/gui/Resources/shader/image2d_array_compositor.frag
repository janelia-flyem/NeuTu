#if GLSL_VERSION < 130
#extension GL_EXT_texture_array : enable
uniform vec2 screen_dim_RCP;
#endif

uniform sampler2DArray color_texture;
uniform sampler2DArray depth_texture;

#if GLSL_VERSION >= 330
layout(location = 0) out vec4 FragData0;
#elif GLSL_VERSION >= 130
out vec4 FragData0;  // call glBindFragDataLocation before linking
#else
#define FragData0 gl_FragData[0]
#endif

void main()
{
#if NUM_VOLUMES < 2
#if GLSL_VERSION < 130
  FragData0 = texture2DArray(color_texture, vec3(gl_FragCoord.xy * screen_dim_RCP, 0));;
  gl_FragDepth = texture2DArray(depth_texture, vec3(gl_FragCoord.xy * screen_dim_RCP, 0)).r;;
#else
  FragData0 = texelFetch(color_texture, ivec3(gl_FragCoord.xy, 0), 0);;
  gl_FragDepth = texelFetch(depth_texture, ivec3(gl_FragCoord.xy, 0), 0).r;;
#endif
#elif defined(MAX_PROJ_MERGE)
#if GLSL_VERSION < 130
  FragData0 = texture2DArray(color_texture, vec3(gl_FragCoord.xy * screen_dim_RCP, 0));
  gl_FragDepth = texture2DArray(depth_texture, vec3(gl_FragCoord.xy * screen_dim_RCP, 0)).r;
  for (int i = 1; i < NUM_VOLUMES; ++i) {
    FragData0 = max(FragData0, texture2DArray(color_texture, vec3(gl_FragCoord.xy * screen_dim_RCP, i)));
#ifdef RESULT_OPAQUE
    gl_FragDepth = max(gl_FragDepth, texture2DArray(depth_texture, vec3(gl_FragCoord.xy * screen_dim_RCP, i)).r);
#else
    gl_FragDepth = min(gl_FragDepth, texture2DArray(depth_texture, vec3(gl_FragCoord.xy * screen_dim_RCP, i)).r);
#endif
#else
  FragData0 = texelFetch(color_texture, ivec3(gl_FragCoord.xy, 0), 0);
  gl_FragDepth = texelFetch(depth_texture, ivec3(gl_FragCoord.xy, 0), 0).r;
  for (int i = 1; i < NUM_VOLUMES; ++i) {
    FragData0 = max(FragData0, texelFetch(color_texture, ivec3(gl_FragCoord.xy, i), 0));
#ifdef RESULT_OPAQUE
    gl_FragDepth = max(gl_FragDepth, texelFetch(depth_texture, ivec3(gl_FragCoord.xy, i), 0).r);
#else
    gl_FragDepth = min(gl_FragDepth, texelFetch(depth_texture, ivec3(gl_FragCoord.xy, i), 0).r);
#endif
#endif
  }
#else
  vec4 colors[NUM_VOLUMES];
  float depths[NUM_VOLUMES];
  vec4 color;
  float depth;

  for (int i = 0; i < NUM_VOLUMES; ++i) {
#if GLSL_VERSION < 130
    colors[i] = texture2DArray(color_texture, vec3(gl_FragCoord.xy * screen_dim_RCP, i));
    depths[i] = texture2DArray(depth_texture, vec3(gl_FragCoord.xy * screen_dim_RCP, i)).r;
#else
    colors[i] = texelFetch(color_texture, ivec3(gl_FragCoord.xy, i), 0);
    depths[i] = texelFetch(depth_texture, ivec3(gl_FragCoord.xy, i), 0).r;
#endif
  }

  for (int j = 1; j < NUM_VOLUMES; ++j) {
    color = colors[j];
    depth = depths[j];
    int i = j-1;
    while (i >= 0 && depth > depths[i]) {
      depths[i+1] = depths[i];
      colors[i+1] = colors[i];
      --i;
    }
    depths[i+1] = depth;
    colors[i+1] = color;
  }

  color = colors[1] + (1 - colors[1].a) * colors[0];
  for (int i = 2; i < NUM_VOLUMES; ++i) {
    color = colors[i] + (1 - colors[i].a) * color;
  }
  FragData0 = color;
  gl_FragDepth = depths[NUM_VOLUMES-1];
#endif
}
