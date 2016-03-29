// "Weighted Blended Order-Independent Transparency" technique by Morgan McGuire and Louis Bavoil

#if GLSL_VERSION >= 130
in vec3 attr_vertex;
out vec2 vTexcoord;
#else
attribute vec3 attr_vertex;
varying vec2 vTexcoord;
#endif

void main(void)
{   
    gl_Position = vec4(attr_vertex,1.0);
    vTexcoord = (attr_vertex.xy+1.0)/2.0;
}
