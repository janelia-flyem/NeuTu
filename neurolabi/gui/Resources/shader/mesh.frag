uniform vec4 scene_ambient;
uniform vec4 material_ambient;
uniform vec4 material_specular;
uniform float material_shininess;
uniform float alpha;
uniform vec4 custom_color;
uniform bool use_custom_color = false;
uniform bool use_two_sided_lighting = true;

#if GLSL_VERSION >= 130
#if defined(USE_MESH_COLOR)
in vec4 color;
#elif defined(USE_MESH_1DTEXTURE)
in float texCoord0;
uniform sampler1D texture;
#elif defined(USE_MESH_2DTEXTURE)
in vec2 texCoord0;
uniform sampler2D texture;
#elif defined(USE_MESH_3DTEXTURE)
in vec3 texCoord0;
uniform sampler3D texture;
#endif

in vec3 normal;
in vec3 point;
#else
#if defined(USE_MESH_COLOR)
varying vec4 color;
#elif defined(USE_MESH_1DTEXTURE)
varying float texCoord0;
uniform sampler1D texture;
#elif defined(USE_MESH_2DTEXTURE)
varying vec2 texCoord0;
uniform sampler2D texture;
#elif defined(USE_MESH_3DTEXTURE)
varying vec3 texCoord0;
uniform sampler3D texture;
#endif

varying vec3 normal;
varying vec3 point;
#endif

#if GLSL_VERSION >= 330
layout(location = 0) out vec4 FragData0;
#elif GLSL_VERSION >= 130
out vec4 FragData0;  // call glBindFragDataLocation before linking
#else
#define FragData0 gl_FragData[0]
#endif

vec4 apply_lighting_and_fog(const in vec4 sceneAmbient,
                            const in float materialShininess, const in vec4 materialAmbient, const in vec4 materialSpecular,
                            const in vec3 normalDirection, const in vec3 position, const in vec4 color, const in float alpha);

void main(void)
{
  vec3 sidedNormal = (use_two_sided_lighting && !gl_FrontFacing) ? -normal : normal;

  if (use_custom_color) {
    FragData0 = apply_lighting_and_fog(scene_ambient, material_shininess, material_ambient, material_specular,
                                       sidedNormal, point, custom_color, alpha);
  } else {
#if defined(USE_MESH_2DTEXTURE)
#if GLSL_VERSION >= 130
    vec4 color = texture(texture, texCoord0);
#else
    vec4 color = texture2D(texture, texCoord0);
#endif
#elif defined(USE_MESH_3DTEXTURE)
#if GLSL_VERSION >= 130
    vec4 color = texture(texture, texCoord0);
#else
    vec4 color = texture3D(texture, texCoord0);
#endif
#elif defined(USE_MESH_1DTEXTURE)
#if GLSL_VERSION >= 130
    vec4 color = texture(texture, texCoord0);
#else
    vec4 color = texture1D(texture, texCoord0);
#endif
#endif

#if defined(USE_MESH_COLOR) || defined(USE_MESH_2DTEXTURE) || defined(USE_MESH_1DTEXTURE) || defined(USE_MESH_3DTEXTURE)
    FragData0 = apply_lighting_and_fog(scene_ambient, material_shininess, material_ambient, material_specular,
                                       sidedNormal, point, color, alpha);
#endif
  }
}

