void fragment_func(out vec4 fragColor0, out vec4 fragColor1, out float fragDepth);

void main(void)
{
    fragment_func(FragData0, FragData1, gl_FragDepth);
}

