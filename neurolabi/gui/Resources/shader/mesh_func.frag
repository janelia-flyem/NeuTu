uniform vec4 scene_ambient;
uniform vec4 material_ambient;
uniform vec4 material_specular;
uniform float material_shininess;
uniform float alpha;
uniform vec4 custom_color;
uniform bool use_custom_color = false;

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

vec4 apply_lighting_and_fog(const in vec4 sceneAmbient,
                            const in float materialShininess, const in vec4 materialAmbient, const in vec4 materialSpecular,
                            const in vec3 normalDirection, const in vec3 position, const in vec4 color, const in float alpha);

void fragment_func(out vec4 fragColor, out float fragDepth)
{
  if (use_custom_color) {
    fragDepth = gl_FragCoord.z;
    fragColor = apply_lighting_and_fog(scene_ambient, material_shininess, material_ambient, material_specular,
                                       normal, point, custom_color, alpha);
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

    fragDepth = gl_FragCoord.z;

#if defined(USE_MESH_COLOR) || defined(USE_MESH_2DTEXTURE) || defined(USE_MESH_1DTEXTURE) || defined(USE_MESH_3DTEXTURE)
    fragColor = apply_lighting_and_fog(scene_ambient, material_shininess, material_ambient, material_specular,
                                       normal, point, color, alpha);
#endif
  }
}

