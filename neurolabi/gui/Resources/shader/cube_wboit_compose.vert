// "Weighted Blended Order-Independent Transparency" technique by Morgan McGuire and Louis Bavoil

#if GLSL_VERSION >= 130
in vec3 composeVertPos;
out vec2 vTexcoord;
#else
attribute vec3 composeVertPos;
varying vec2 vTexcoord;
#endif

void main(void)
{   
    gl_Position = vec4(composeVertPos,1.0);
    vTexcoord = (composeVertPos.xy+1.0)/2.0;
}
