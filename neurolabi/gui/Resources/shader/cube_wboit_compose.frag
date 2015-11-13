// "Weighted Blended Order-Independent Transparency" technique by Morgan McGuire and Louis Bavoil

// sum(rgb * a, a)
uniform sampler2D accumTexture;
// prod(1 - a)
uniform sampler2D revealageTexture;

varying vec2 vTexcoord;

void main(void)
{
    vec4 accum = texture2D(accumTexture, vTexcoord);
    float r = accum.a;
    accum.a = texture2D(revealageTexture, vTexcoord).r;
    if (r >= 1.0)
    {
        discard;
    }

#if defined(FragData0)
    FragData0 = vec4(accum.rgb / clamp(accum.a, 1e-4, 5e4), r);
#else
    gl_FragColor = vec4(accum.rgb / clamp(accum.a, 1e-4, 5e4), r);
#endif

}
