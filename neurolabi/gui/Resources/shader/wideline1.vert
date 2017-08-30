#if GLSL_VERSION >= 130
in vec3 attr_p0;
in vec3 attr_p1;
#ifndef USE_1DTEXTURE
in vec4 attr_p0color;
in vec4 attr_p1color;
#endif
in float attr_flags;
#else
attribute vec3 attr_p0;
attribute vec3 attr_p1;
#ifndef USE_1DTEXTURE
attribute vec4 attr_p0color;
attribute vec4 attr_p1color;
#endif
attribute float attr_flags;
#endif

uniform float line_width;
uniform float size_scale;
uniform mat4 viewport_matrix;
uniform mat4 viewport_matrix_inverse;
uniform mat4 projection_view_matrix;
uniform mat4 pos_transform = mat4(1.0);

#if GLSL_VERSION >= 130 && defined(HAS_CLIP_PLANE)
uniform vec4 clip_planes[CLIP_PLANE_COUNT];
out float gl_ClipDistance[CLIP_PLANE_COUNT];
#endif

#ifndef USE_1DTEXTURE
#if GLSL_VERSION >= 130
out vec4 color;
#else
varying vec4 color;
#endif
#endif

#if GLSL_VERSION >= 130
flat out vec3 plane1;
flat out vec3 plane2;
flat out vec3 plane3;
flat out vec3 plane4;
#ifdef ROUND_CAP
flat out vec4 p0p1;
#endif
#else
varying vec3 plane1;
varying vec3 plane2;
varying vec3 plane3;
varying vec3 plane4;
#ifdef ROUND_CAP
varying vec4 p0p1;
#endif
#endif

void ClipSegmentToPlane(inout vec4 p0, inout vec4 p1, vec4 plane)
{
  float dist0 = dot(p0, plane);
  float dist1 = dot(p1, plane);
  bool in0 = dist0 >= 0.0;
  bool in1 = dist1 >= 0.0;
  if (!in0 && !in1)
  {
    p0 = vec4(0, 0, 0, -1);
    p1 = vec4(0, 0, 0, -1);
  }
  else if (in0 != in1)
  {
    float t = dist0 / (dist0 - dist1);
    if (in1) p0 = mix(p0, p1, t);
    else     p1 = mix(p0, p1, t);
  }
}

void main()
{
  vec2 flags = mod(floor(vec2(attr_flags/16.0, attr_flags)), 16.0);
  // either -1 or 1, -1 -> left, 1 -> right
  float rightFlag = flags.x - 1.;
  // either -1 or 1, -1 -> down, 1 -> up
  float upFlag = flags.y - 1.;

  vec4 p0vertex = pos_transform * vec4(attr_p0, 1.0);
  vec4 p1vertex = pos_transform * vec4(attr_p1, 1.0);

  vec4 p0 = projection_view_matrix * p0vertex;
  vec4 p1 = projection_view_matrix * p1vertex;
  ClipSegmentToPlane(p0, p1, vec4(0, 0, 1, 1));

  p0 = viewport_matrix * p0;
  p0 /= p0.w;
  p1 = viewport_matrix * p1;
  p1 /= p1.w;
#ifdef ROUND_CAP
  p0p1 = vec4(p0.xy, p1.xy);
#endif

  // Compute segment direction L and perpendicular P then scale for line width
  float R = (line_width * size_scale / 2.0 + 1.0);
  vec2 L = normalize(p1.xy - p0.xy);
  vec2 P = vec2(-L.y, L.x);
#ifdef LINE_SCREEN_ALIGNED
  vec2 LR = vec2(0, 0);
#else
  vec2 LR = L * R;
#endif
  vec2 PR = P * R;

#ifdef LINE_SCREEN_ALIGNED
  // Sheer PR so that it's orthogonal to L's major axis (so that ends are screen-aligned)
  vec2 Lmajor = abs(L.y) >= abs(L.x) ? vec2(0, sign(L.y)) : vec2(sign(L.x), 0);
  PR -= dot(PR, Lmajor) / dot(L, Lmajor) * L;
#endif

  // Compute the screen-space quad corners
  vec2 qcorner = upFlag * PR + (rightFlag > 0 ? (p1.xy + LR) : (p0.xy - LR));

  // Compute the screen-space planes used to determine fragment distances from the quad's boundary
  plane1 = vec3(+P, -(dot(p0.xy, +P) - R));
  plane2 = vec3(-P, -(dot(p1.xy, -P) - R));
#ifndef LINE_SCREEN_ALIGNED
  plane3 = vec3(+L, -(dot(p0.xy, +L) - 1.0));
  plane4 = vec3(-L, -(dot(p1.xy, -L) - 1.0));
#else
  plane3 = vec3(+Lmajor, -(dot(p0.xy, +Lmajor) - 1.0));
  plane4 = vec3(-Lmajor, -(dot(p1.xy, -Lmajor) - 1.0));
#endif

  vec4 worldVertex = vec4(rightFlag > 0 ? p1vertex.xyz : p0vertex.xyz, 1.0);
  gl_Position = viewport_matrix_inverse * vec4(qcorner, rightFlag > 0 ? p1.zw : p0.zw);
#ifndef USE_1DTEXTURE
  color = rightFlag > 0 ? attr_p1color : attr_p0color;
#endif
#if defined(HAS_CLIP_PLANE)
#if GLSL_VERSION >= 130
  for (int i=0; i<CLIP_PLANE_COUNT; ++i)
    gl_ClipDistance[i] = dot(clip_planes[i], worldVertex);
#else
  gl_ClipVertex = worldVertex;
#endif   // version 130 or up
#endif  // has clipplane
}
