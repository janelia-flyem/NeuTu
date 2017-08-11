#if GLSL_VERSION >= 130
in vec3 attr_vertex;
in vec3 attr_normal;
#if defined(USE_MESH_COLOR)
in vec4 attr_color;
#elif defined(USE_MESH_1DTEXTURE)
in vec2 attr_1dTexCoord0;
#elif defined(USE_MESH_2DTEXTURE)
in vec2 attr_2dTexCoord0;
#elif defined(USE_MESH_3DTEXTURE)
in vec3 attr_3dTexCoord0;
#endif
#else
attribute vec3 attr_vertex;
attribute vec3 attr_normal;
#if defined(USE_MESH_COLOR)
attribute vec4 attr_color;
#elif defined(USE_MESH_1DTEXTURE)
attribute vec2 attr_1dTexCoord0;
#elif defined(USE_MESH_2DTEXTURE)
attribute vec2 attr_2dTexCoord0;
#elif defined(USE_MESH_3DTEXTURE)
attribute vec3 attr_3dTexCoord0;
#endif
#endif

uniform mat4 view_matrix;
uniform mat4 projection_view_matrix;
//uniform vec3 pos_scale = vec3(1.0, 1.0, 1.0);
uniform mat4 pos_transform = mat4(1.0);
uniform mat3 pos_transform_normal_matrix = mat3(1.0);
//uniform mat3 normal_matrix;

#if GLSL_VERSION >= 130 && defined(HAS_CLIP_PLANE)
uniform vec4 clip_planes[CLIP_PLANE_COUNT];
out float gl_ClipDistance[CLIP_PLANE_COUNT];
#endif

#if GLSL_VERSION >= 130
#if defined(USE_MESH_COLOR)
out vec4 color;
#elif defined(USE_MESH_1DTEXTURE)
out float texCoord0;
#elif defined(USE_MESH_2DTEXTURE)
out vec2 texCoord0;
#elif defined(USE_MESH_3DTEXTURE)
out vec3 texCoord0;
#endif
out vec3 normal;
out vec3 point;
#else
#if defined(USE_MESH_COLOR)
varying vec4 color;
#elif defined(USE_MESH_1DTEXTURE)
varying float texCoord0;
#elif defined(USE_MESH_2DTEXTURE)
varying vec2 texCoord0;
#elif defined(USE_MESH_3DTEXTURE)
varying vec3 texCoord0;
#endif
varying vec3 normal;
varying vec3 point;
#endif

void main()
{
  // Get surface normal in eye coordinates
  normal = normalize(pos_transform_normal_matrix * attr_normal);

  //vec4 vertex = vec4(attr_vertex*pos_scale, 1.0);
  vec4 vertex = pos_transform * vec4(attr_vertex, 1.0);
  // Calculate vertex position in modelview space
  vec4 eyeSpacePos = view_matrix * vertex;
  point = eyeSpacePos.xyz / eyeSpacePos.w;
  gl_Position = projection_view_matrix * vertex;
#ifdef USE_MESH_COLOR
  color = attr_color;
#endif

#if defined(HAS_CLIP_PLANE)
#if GLSL_VERSION >= 130
  for (int i=0; i<CLIP_PLANE_COUNT; ++i)
    gl_ClipDistance[i] = dot(clip_planes[i], vertex);
#else
  gl_ClipVertex = vertex;
#endif   // version 130 or up
#endif  // has clipplane
}
