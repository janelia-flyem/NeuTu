// "Weighted Blended Order-Independent Transparency" technique by Morgan McGuire and Louis Bavoil

uniform vec4 scene_ambient;
uniform vec4 material_ambient;
uniform vec4 material_specular;
uniform float material_shininess;
uniform float alpha;

varying vec3 position;
varying vec3 normal;
varying vec4 color;
varying float depth;

vec4 apply_lighting_and_fog(const in vec4 sceneAmbient,
                            const in float materialShininess, const in vec4 materialAmbient, const in vec4 materialSpecular,
                            const in vec3 normalDirection, const in vec3 position, const in vec4 color, const in float alpha);

void main()
{
    //float alpha = color.a;

    // the object lies between -40 and -60 z coordinates
    float weight = pow(alpha + 0.01, 4.0) + max(0.01, min(3000.0, 0.3 / (0.00001 + pow(abs(depth) / 200.0, 4.0))));

     //RGBA32F texture (accumulation), a synonym of gl_FragColor attached to GL_COLOR_ATTACHMENT0
     vec4 lightcolor = apply_lighting_and_fog(scene_ambient, material_shininess, material_ambient, material_specular, normal, position, color, alpha);

#if defined(FragData0)
    //FragData0 = mix(vec4(color.rgb * alpha * weight, alpha), lightcolor, alpha);
     FragData0 = lightcolor;
#else
    //gl_FragData[0] = mix(vec4(color.rgb * alpha * weight, alpha), lightcolor, alpha);
     gl_FragData[0] = lightcolor;
#endif

    // R32F texture (revealage), attached to GL_COLOR_ATTACHMENT1

#if defined(FragData1)
    FragData1.r = alpha * weight;
#else
    gl_FragData[1].r = alpha * weight;
#endif

}
