// "Weighted Blended Order-Independent Transparency" technique by Morgan McGuire and Louis Bavoil

attribute vec3 vPosition;
attribute vec3 vNormal;
attribute vec4 vColor;

varying vec3 position;
varying vec3 normal;
varying vec4 color;
varying float depth;

uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

void main()
{
    normal = normalize( normalMatrix * vNormal);
    position = vec3( modelviewMatrix * vec4(vPosition,1.0) );
    color = vColor;

    gl_Position = projectionMatrix * modelviewMatrix * vec4(vPosition,1.0);

    // eye coord
    depth = -(modelviewMatrix * vec4(vPosition,1.0)).z;
}
