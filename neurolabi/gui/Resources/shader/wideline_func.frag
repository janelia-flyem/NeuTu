uniform float alpha;
uniform bool lighting_enabled = true;
uniform float line_width;
uniform float size_scale;
#ifdef USE_1DTEXTURE
uniform sampler1D texture;
#endif

#if GLSL_VERSION >= 130
#ifndef USE_1DTEXTURE
in vec4 color;
#endif
flat in vec3 FragPlanes[4];
#ifdef ROUND_CAP
flat in vec2 p0pos;
flat in vec2 p1pos;
#endif
#else
#ifndef USE_1DTEXTURE
varying vec4 color;
#endif
varying vec3 FragPlanes[4];
#ifdef ROUND_CAP
varying vec2 p0pos;
varying vec2 p1pos;
#endif
#endif

void fragment_func(out vec4 fragColor, out float fragDepth)
{
  // Get fragment distances to quad boundary
  vec3 pos = vec3(gl_FragCoord.xy, 1.0);
#ifdef ROUND_CAP
  vec4 dist = vec4(dot(pos, FragPlanes[0]), dot(pos, FragPlanes[1]), dot(pos, FragPlanes[2]), dot(pos, FragPlanes[3]));
  if (dist.x < 0 || dist.y < 0)
    discard;
  if (dist.z < 0 && line_width * size_scale / 2 - distance(pos.xy, p0pos) < 0) {
    discard;
  }
  if (dist.w < 0 && line_width * size_scale / 2 - distance(pos.xy, p1pos) < 0) {
    discard;
  }
#else
  vec4 dist = vec4(dot(pos, FragPlanes[0]), dot(pos, FragPlanes[1]), dot(pos, FragPlanes[2]), dot(pos, FragPlanes[3]));
  if (any(lessThan(dist, vec4(0))))
    discard;
#endif
  float d = min(dist.x, dist.y);
  float f = smoothstep(0.0, 1.0, d);

#ifdef USE_1DTEXTURE
  float texCoord0 = 1.0 - (d - 1.0) * 2.0 * size_scale / line_width;
#if GLSL_VERSION >= 130
  vec4 color = texture(texture, texCoord0);
#else
  vec4 color = texture1D(texture, texCoord0);
#endif
#endif
  fragColor = !lighting_enabled ? color : vec4(color.rgb * color.a * f * alpha, color.a * f * alpha);
  fragDepth = gl_FragCoord.z;
}
