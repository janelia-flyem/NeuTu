#if GLSL_VERSION >= 130
in vec3 attr_vertex;
in vec4 attr_color;
#else
attribute vec3 attr_vertex;
attribute vec4 attr_color;
#endif

uniform mat4 view_matrix;
uniform mat4 projection_view_matrix;
uniform vec3 pos_scale = vec3(1.0, 1.0, 1.0);

#if GLSL_VERSION >= 130 && defined(HAS_CLIP_PLANE)
uniform vec4 clip_planes[CLIP_PLANE_COUNT];
out float gl_ClipDistance[CLIP_PLANE_COUNT];
#endif

#if GLSL_VERSION >= 130
out vec4 color;
#else
varying vec4 color;
#endif

void main()
{
  vec4 vertex = vec4(attr_vertex*pos_scale, 1.0);
  gl_Position = projection_view_matrix * vertex;
  color = attr_color;
#if defined(HAS_CLIP_PLANE)
#if GLSL_VERSION >= 130
  for (int i=0; i<CLIP_PLANE_COUNT; ++i)
    gl_ClipDistance[i] = dot(clip_planes[i], vertex);
#else
  gl_ClipVertex = vertex;
#endif   // version 130 or up
#endif  // has clipplane
}
