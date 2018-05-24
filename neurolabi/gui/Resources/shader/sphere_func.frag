uniform float ortho;

uniform vec4 scene_ambient;
uniform vec4 material_ambient;
uniform vec4 material_specular;
uniform float material_shininess;
uniform float alpha;
uniform mat4 projection_matrix;

#if GLSL_VERSION >= 130
in vec4 color;
in vec3 sphere_center;
in float radius2;
in vec3 point;
#ifdef DYNAMIC_MATERIAL_PROPERTY
in float va_material_shininess;
in vec4 va_material_specular;
#endif
#else
varying vec4 color;
varying vec3 sphere_center;
varying float radius2;
varying vec3 point;
#ifdef DYNAMIC_MATERIAL_PROPERTY
varying float va_material_shininess;
varying vec4 va_material_specular;
#endif
#endif

vec4 apply_lighting_and_fog(const in vec4 sceneAmbient,
                            const in float materialShininess, const in vec4 materialAmbient, const in vec4 materialSpecular,
                            const in vec3 normalDirection, const in vec3 position, const in vec4 color, const in float alpha);

void fragment_func(out vec4 fragColor, out float fragDepth)
{
  vec3 rayOrigin = point;
  vec3 rayDirection = mix(normalize(point), vec3(0.0, 0.0, -1.0), ortho);

  vec3 sphereVector = sphere_center - rayOrigin;
  float b = dot(sphereVector, rayDirection);

  float position = b * b + radius2 - dot(sphereVector, sphereVector);
#ifdef ANTI_ALIASING
  float delta = fwidth(position);
  float edgeAlpha = smoothstep(0.0, delta, position);
#endif
  if (position < 0.0)
    discard;

  float dist = b - sqrt(position);
  vec3 ipoint = dist * rayDirection + rayOrigin;
  vec2 clipZW = ipoint.z * projection_matrix[2].zw + projection_matrix[3].zw;

  float depth = 0.5 + 0.5 * clipZW.x / clipZW.y;

  if (depth <= 0.0)
    discard;

  if (depth >= 1.0)
    discard;

  fragDepth = depth;

  vec3 normalDirection = normalize(ipoint - sphere_center);

#ifdef DYNAMIC_MATERIAL_PROPERTY
fragColor = apply_lighting_and_fog(scene_ambient, va_material_shininess, material_ambient, va_material_specular,
#ifdef ANTI_ALIASING
                                     normalDirection, ipoint, color, alpha * edgeAlpha);
#else
                                  normalDirection, ipoint, color, alpha);
#endif
#else
fragColor = apply_lighting_and_fog(scene_ambient, material_shininess, material_ambient, material_specular,
#ifdef ANTI_ALIASING
                                   normalDirection, ipoint, color, alpha * edgeAlpha);
#else
                                    normalDirection, ipoint, color, alpha);
#endif
#endif

}
