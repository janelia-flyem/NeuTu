//#version 120
// "Weighted Blended Order-Independent Transparency" technique by Morgan McGuire and Louis Bavoil

uniform vec4 scene_ambient;
uniform vec4 material_ambient;
uniform vec4 material_specular;
uniform float material_shininess;
uniform float alpha;

//uniform vec4 uColor;
uniform vec3 uColor;

varying vec3 position;
varying vec3 normal;
varying float depth;

vec4 apply_lighting_and_fog(const in vec4 sceneAmbient,
                            const in float materialShininess, const in vec4 materialAmbient, const in vec4 materialSpecular,
                            const in vec3 normalDirection, const in vec3 position, const in vec4 color, const in float alpha);

void main()
{
    //float alpha = color.a;

    // the object lies between -40 and -60 z coordinates
    float weight = pow(alpha + 0.01f, 4.0f) + max(0.01f, min(3000.0f, 0.3f / (0.00001f + pow(abs(depth) / 200.0f, 4.0f))));

    //vec4 wbColor = vec4(uColor.rgb*alpha*weight, alpha*weight);
    vec4 wbColor = vec4(uColor*alpha*weight, alpha*weight);

     //RGBA32F texture (accumulation), a synonym of gl_FragColor attached to GL_COLOR_ATTACHMENT0
     //vec4 lightcolor = apply_lighting_and_fog(scene_ambient, material_shininess, material_ambient, material_specular, normal, position, wbColor, alpha);

#if defined(FragData0)
     FragData0 = vec4(1,0,0,1); //lightcolor;
#else
     gl_FragData[0] = vec4(1,0,0,1); //lightcolor;
#endif

    // R32F texture (revealage), attached to GL_COLOR_ATTACHMENT1

#if defined(FragData1)
    FragData1.r = 0.5; //alpha * weight;
#else
    gl_FragData[1].r = 0.5; //alpha * weight;
#endif

}
