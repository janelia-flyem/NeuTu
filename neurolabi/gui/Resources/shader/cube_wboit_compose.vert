#version 120
// "Weighted Blended Order-Independent Transparency" technique by Morgan McGuire and Louis Bavoil

attribute vec3 composeVertPos;

varying vec2 vTexcoord;

void main(void)
{   
    gl_Position = vec4(composeVertPos,1.0);

    vTexcoord = (composeVertPos.xy+1.0)/2.0;
}
