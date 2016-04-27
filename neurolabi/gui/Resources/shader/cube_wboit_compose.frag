// "Weighted Blended Order-Independent Transparency" technique by Morgan McGuire and Louis Bavoil

// sum(rgb * a, a)
uniform sampler2D accumTexture;
// prod(1 - a)
uniform sampler2D revealageTexture;

#if GLSL_VERSION >= 130
in vec2 vTexcoord;
#else
#define texture texture2D
varying vec2 vTexcoord;
#endif

void main(void)
{
    vec4 accum = texture(accumTexture, vTexcoord);
    float alpha = accum.a;
    accum.a = texture(revealageTexture, vTexcoord).r;
    if (alpha >= 1.0)
    {
        discard;
    }
    FragData0 = vec4(accum.rgb / clamp(accum.a, 1e-4, 5e4), alpha);;
}
