// "Weighted Blended Order-Independent Transparency" technique by Morgan McGuire and Louis Bavoil

attribute vec3 vPosition;

//uniform vec3 pos_scale;

varying vec2 vTexcoord;

void main(void)
{   
//    gl_Position = vec4(vPosition*pos_scale,1.0);

    gl_Position = vec4(vPosition,1.0);

    vTexcoord = (vPosition.xy+1.0)/2.0;
}
