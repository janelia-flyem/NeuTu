// "Weighted Blended Order-Independent Transparency" technique by Morgan McGuire and Louis Bavoil

#if GLSL_VERSION >= 130
in vec3 position;
in vec3 normal;
in float depth;
#else
varying vec3 position;
varying vec3 normal;
varying float depth;
#endif

uniform vec4 scene_ambient;
uniform vec4 material_ambient;
uniform vec4 material_specular;
uniform float material_shininess;
uniform float alpha;

uniform vec4 uColor;

//
vec4 apply_lighting_and_fog(const in vec4 sceneAmbient,
                            const in float materialShininess, const in vec4 materialAmbient, const in vec4 materialSpecular,
                            const in vec3 normalDirection, const in vec3 position, const in vec4 color, const in float alpha);
//
void main()
{
    // the object lies between -40 and -60 z coordinates
    float weight = pow(alpha + 0.01f, 4.0f) + max(0.01f, min(3000.0f, 0.3f / (0.00001f + pow(abs(depth) / 200.0f, 4.0f))));

    vec4 wbColor = vec4(uColor.rgb*alpha*weight, alpha*weight);

     //RGBA32F texture (accumulation), a synonym of gl_FragColor attached to GL_COLOR_ATTACHMENT0
     FragData0  = apply_lighting_and_fog(scene_ambient, material_shininess, material_ambient, material_specular, normal, position, wbColor, alpha);

    // R32F texture (revealage), attached to GL_COLOR_ATTACHMENT1
    FragData1.r = alpha * weight;
}
