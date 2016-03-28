//#version 120
// "Weighted Blended Order-Independent Transparency" technique by Morgan McGuire and Louis Bavoil

attribute vec3 vPosition;
attribute float vNormal;

varying vec3 position;
varying vec3 normal;
varying float depth;

uniform mat4 view_matrix;
uniform mat4 projection_view_matrix;
uniform mat3 normal_matrix;

uniform vec3 pos_scale;

void main()
{
    vec4 vertex = vec4(vPosition*pos_scale,1.0);

    vec3 vertNormal = vec3(0.0f, 0.0f, 0.0f);

    //
    float d,r;
    if(vNormal>=32) // +z
    {
        vertNormal += vec3(0.0f, 0.0f, 1.0f);

        d = vNormal/2.0f;
        r = mod(vNormal,2.0f);

        if(r==1) // -z
        {
            vertNormal += vec3(0.0f, 0.0f, -1.0f);
        }

        d = vNormal/2.0f;
        r = mod(vNormal,2.0f);

        if(r==1) // +y
        {
            vertNormal += vec3(0.0f, 1.0f, 0.0f);
        }

        d = vNormal/2.0f;
        r = mod(vNormal,2.0f);

        if(r==1) // -y
        {
            vertNormal += vec3(0.0f, -1.0f, 0.0f);
        }

        d = vNormal/2.0f;
        r = mod(vNormal,2.0f);

        if(r==1) // +x
        {
            vertNormal += vec3(1.0f, 0.0f, 0.0f);
        }

        d = vNormal/2.0f;
        r = mod(vNormal,2.0f);

        if(r==1) // -x
        {
            vertNormal += vec3(-1.0f, 0.0f, 0.0f);
        }

    }
    else if(vNormal>=16) // -z
    {
        vertNormal += vec3(0.0f, 0.0f, -1.0f);

        d = vNormal/2.0f;
        r = mod(vNormal,2.0f);

        if(r==1) // +y
        {
            vertNormal += vec3(0.0f, 1.0f, 0.0f);
        }

        d = vNormal/2.0f;
        r = mod(vNormal,2.0f);

        if(r==1) // -y
        {
            vertNormal += vec3(0.0f, -1.0f, 0.0f);
        }

        d = vNormal/2.0f;
        r = mod(vNormal,2.0f);

        if(r==1) // +x
        {
            vertNormal += vec3(1.0f, 0.0f, 0.0f);
        }

        d = vNormal/2.0f;
        r = mod(vNormal,2.0f);

        if(r==1) // -x
        {
            vertNormal += vec3(-1.0f, 0.0f, 0.0f);
        }
    }
    else if(vNormal>=8) // +y
    {
        vertNormal += vec3(0.0f, 1.0f, 0.0f);

        d = vNormal/2.0f;
        r = mod(vNormal,2.0f);

        if(r==1) // -y
        {
            vertNormal += vec3(0.0f, -1.0f, 0.0f);
        }

        d = vNormal/2.0f;
        r = mod(vNormal,2.0f);

        if(r==1) // +x
        {
            vertNormal += vec3(1.0f, 0.0f, 0.0f);
        }

        d = vNormal/2.0f;
        r = mod(vNormal,2.0f);

        if(r==1) // -x
        {
            vertNormal += vec3(-1.0f, 0.0f, 0.0f);
        }
    }
    else if(vNormal>=4) // -y
    {
        vertNormal += vec3(0.0f, -1.0f, 0.0f);

        d = vNormal/2.0f;
        r = mod(vNormal,2.0f);

        if(r==1) // +x
        {
            vertNormal += vec3(1.0f, 0.0f, 0.0f);
        }

        d = vNormal/2.0f;
        r = mod(vNormal,2.0f);

        if(r==1) // -x
        {
            vertNormal += vec3(-1.0f, 0.0f, 0.0f);
        }
    }
    else if(vNormal>=2) // +x
    {
        vertNormal += vec3(1.0f, 0.0f, 0.0f);

        d = vNormal/2.0f;
        r = mod(vNormal,2.0f);

        if(r==1) // -x
        {
            vertNormal += vec3(-1.0f, 0.0f, 0.0f);
        }
    }
    else // -x
    {
        r = mod(vNormal,2.0f);

        if(r==1) // -x
        {
            vertNormal += vec3(-1.0f, 0.0f, 0.0f);
        }
    }
    vertNormal = normalize(vertNormal);

    //
    normal = normalize( normal_matrix * vertNormal);
    position = vec3( view_matrix * vertex);
    gl_Position = projection_view_matrix * vertex;

    // eye coord
    depth = -(view_matrix * vertex).z;
}
