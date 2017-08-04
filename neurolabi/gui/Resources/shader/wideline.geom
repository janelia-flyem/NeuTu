#if GLSL_VERSION >= 150
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;
#endif

uniform float line_width;
uniform float size_scale;
uniform mat4 viewport_matrix;
uniform mat4 viewport_matrix_inverse;

#if GLSL_VERSION >= 150
#ifndef USE_1DTEXTURE
in vec4 colorIn[2];
out vec4 color;
#endif
flat out vec3 FragPlanes[4];
#ifdef ROUND_CAP
flat out vec2 p0pos;
flat out vec2 p1pos;
#endif
#else
#ifndef USE_1DTEXTURE
varying in vec4 colorIn[2];
varying out vec4 color;
#endif
varying out vec3 FragPlanes[4];
#ifdef ROUND_CAP
varying out vec2 p0pos;
varying out vec2 p1pos;
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
  // Clip the input clip-space segment to the near plane
#if GLSL_VERSION >= 150
  vec4 p0 = gl_in[0].gl_Position;
  vec4 p1 = gl_in[1].gl_Position;
#else
  vec4 p0 = gl_PositionIn[0];
  vec4 p1 = gl_PositionIn[1];
#endif
  ClipSegmentToPlane(p0, p1, vec4(0, 0, 1, 1));

  // Project clipped segment into screen space
  p0 = viewport_matrix * p0; p0 /= p0.w;
  p1 = viewport_matrix * p1; p1 /= p1.w;

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
  vec2 q0 = p0.xy - LR - PR;
  vec2 q1 = p1.xy + LR - PR;
  vec2 q2 = p1.xy + LR + PR;
  vec2 q3 = p0.xy - LR + PR;

  // Move the final screen-space quad back into clip-space
  vec4 v0 = viewport_matrix_inverse * vec4(q0, p0.zw);
  vec4 v1 = viewport_matrix_inverse * vec4(q1, p1.zw);
  vec4 v2 = viewport_matrix_inverse * vec4(q2, p1.zw);
  vec4 v3 = viewport_matrix_inverse * vec4(q3, p0.zw);

  // Compute the screen-space planes used to determine fragment distances from the quad's boundary
  vec3 planes[4] = vec3[] (
    vec3(+P, -(dot(p0.xy, +P) - R)),
    vec3(-P, -(dot(p1.xy, -P) - R)),
#ifndef LINE_SCREEN_ALIGNED
    vec3(+L, -(dot(p0.xy, +L) - 1.0)),
    vec3(-L, -(dot(p1.xy, -L) - 1.0))
#else
    vec3(+Lmajor, -(dot(p0.xy, +Lmajor) - 1.0)),
    vec3(-Lmajor, -(dot(p1.xy, -Lmajor) - 1.0))
#endif
    );

  // Emit the quad's tri-strip
#ifndef USE_1DTEXTURE
  color = colorIn[0];
#endif
  FragPlanes = planes;
#ifdef ROUND_CAP
  p0pos = p0.xy; p1pos = p1.xy;
#endif
  gl_Position = v0; EmitVertex();

#ifndef USE_1DTEXTURE
  color = colorIn[0];
#endif
  FragPlanes = planes;
#ifdef ROUND_CAP
  p0pos = p0.xy; p1pos = p1.xy;
#endif
  gl_Position = v3; EmitVertex();

#ifndef USE_1DTEXTURE
  color = colorIn[1];
#endif
  FragPlanes = planes;
#ifdef ROUND_CAP
  p0pos = p0.xy; p1pos = p1.xy;
#endif
  gl_Position = v1; EmitVertex();

#ifndef USE_1DTEXTURE
  color = colorIn[1];
#endif
  FragPlanes = planes;
#ifdef ROUND_CAP
  p0pos = p0.xy; p1pos = p1.xy;
#endif
  gl_Position = v2; EmitVertex();
}