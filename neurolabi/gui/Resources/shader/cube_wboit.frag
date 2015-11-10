// "Weighted Blended Order-Independent Transparency" technique by Morgan McGuire and Louis Bavoil

varying vec3 position;
varying vec3 normal;
varying vec4 color;
varying float depth;

vec3 ads(vec3 position, vec3 normal);
vec4 mixcolor(vec4 c1, vec4 c2, float a);

void main()
{
    float alpha = color.a;

    // the object lies between -40 and -60 z coordinates
    float weight = pow(alpha + 0.01, 4.0) + max(0.01, min(3000.0, 0.3 / (0.00001 + pow(abs(depth) / 200.0, 4.0))));

    // RGBA32F texture (accumulation), a synonym of gl_FragColor attached to GL_COLOR_ATTACHMENT0
    gl_FragData[0] = mixcolor(vec4(color.rgb * alpha * weight, alpha), vec4(ads(position, normal), 1.0), alpha);

    // R32F texture (revealage), attached to GL_COLOR_ATTACHMENT1
    gl_FragData[1].r = alpha * weight;

}
