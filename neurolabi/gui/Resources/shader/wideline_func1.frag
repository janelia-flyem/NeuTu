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
flat in vec3 plane1;
flat in vec3 plane2;
flat in vec3 plane3;
flat in vec3 plane4;
#ifdef ROUND_CAP
flat in vec4 p0p1;
#endif
#else
#ifndef USE_1DTEXTURE
varying vec4 color;
#endif
varying vec3 plane1;
varying vec3 plane2;
varying vec3 plane3;
varying vec3 plane4;
#ifdef ROUND_CAP
varying vec4 p0p1;
#endif
#endif

void fragment_func(out vec4 fragColor, out float fragDepth)
{
  // Get fragment distances to quad boundary
  vec3 pos = vec3(gl_FragCoord.xy, 1.0);
  vec4 dist = vec4(dot(pos, plane1), dot(pos, plane2), dot(pos, plane3), dot(pos, plane4));
#ifdef ROUND_CAP
  if (dist.x < 0 || dist.y < 0)
    discard;
  if (dist.z < 0 && line_width * size_scale / 2 - distance(pos.xy, p0p1.xy) < 0) {
    discard;
  }
  if (dist.w < 0 && line_width * size_scale / 2 - distance(pos.xy, p0p1.zw) < 0) {
    discard;
  }
#else
  if (any(lessThan(dist, vec4(0))))
    discard;
#endif
  float d = min(dist.x, dist.y);

#ifdef USE_1DTEXTURE
  float texCoord0 = 1.0 - (d - 1.0) * 2.0 * size_scale / line_width;
#if GLSL_VERSION >= 130
  vec4 color = texture(texture, texCoord0);
#else
  vec4 color = texture1D(texture, texCoord0);
#endif
#endif
  if (lighting_enabled) {
    float f = smoothstep(0, 1.0, d);
    fragColor = vec4(color.rgb * color.a * f * alpha, color.a * f * alpha);
  } else {
    fragColor = color;
  }
  fragDepth = gl_FragCoord.z;
}