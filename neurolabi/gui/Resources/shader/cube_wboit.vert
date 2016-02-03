// "Weighted Blended Order-Independent Transparency" technique by Morgan McGuire and Louis Bavoil

attribute vec3 vPosition;
attribute vec3 vNormal;
//attribute vec4 vColor;

varying vec3 position;
varying vec3 normal;
varying vec4 color;
varying float depth;

uniform vec4 vColor;
uniform mat4 view_matrix;
uniform mat4 projection_view_matrix;
uniform mat3 normal_matrix;

uniform vec3 pos_scale;

void main()
{
    vec4 vertex = vec4(vPosition*pos_scale,1.0);

    normal = normalize( normal_matrix * vNormal);
    position = vec3( view_matrix * vertex);
    color = vColor;

    gl_Position = projection_view_matrix * vertex;

    // eye coord
    depth = -(view_matrix * vertex).z;
}
