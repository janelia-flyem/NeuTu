#include "zcubearray.h"

//
Mesh::Mesh()
{

}

Mesh::~Mesh()
{

}

void Mesh::setPositions(glm::vec3 a, glm::vec3 b)
{
    ePositions.push_back(a);
    ePositions.push_back(b);
}

void Mesh::setPositions(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    positions.push_back(a);
    positions.push_back(b);
    positions.push_back(c);
}

void Mesh::setPositions(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d)
{
    setPositions(a,b,c);
    setPositions(c,d,a);
}

void Mesh::setColor(std::vector<glm::vec4> &color, glm::vec4 c, int n)
{
    for(int i=0; i<n; i++)
    {
        color.push_back(c);
    }
}

//
Triangle::Triangle() : Mesh()
{

}

Triangle::~Triangle()
{

}

void Triangle::init(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    glm::vec3 norm = glm::normalize( glm::cross(b - a, c - b) );

    setPositions(a,b,c);

    for(int i=0; i<3; i++)
    {
        normals.push_back(norm);
    }
}

void Triangle::setColor(glm::vec4 c)
{
    Mesh::setColor(colors,c,3);
}

//
Quadrilateral::Quadrilateral() : Triangle()
{

}

Quadrilateral::~Quadrilateral()
{

}

void Quadrilateral::init(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d)
{
    glm::vec3 norm = glm::normalize( glm::cross(b - a, c - b) );

    setPositions(a,b,c,d);

    for(int i=0; i<4; i++)
    {
        normals.push_back(norm);
    }
}

void Quadrilateral::setColor(glm::vec4 c)
{
    Mesh::setColor(colors,c,6);
}

//
Cube::Cube() : Quadrilateral()
{
    norm.push_back( glm::vec3(1.0f, 0.0f, 0.0f) );  // +x
    norm.push_back( glm::vec3(-1.0f, 0.0f, 0.0f) ); // -x
    norm.push_back( glm::vec3(0.0f, 1.0f, 0.0f) );  // +y
    norm.push_back( glm::vec3(0.0f, -1.0f, 0.0f) ); // -y
    norm.push_back( glm::vec3(0.0f, 0.0f, 1.0f) );  // +z
    norm.push_back( glm::vec3(0.0f, 0.0f, -1.0f) ); // -z

    normIndex.push_back(0);
    normIndex.push_back(1);
    normIndex.push_back(2);
    normIndex.push_back(3);
    normIndex.push_back(4);
    normIndex.push_back(5);

    // a,b,c
    texc.push_back( glm::vec2(0, 0) );
    texc.push_back( glm::vec2(1, 0) );
    texc.push_back( glm::vec2(1, 1) );
    // c,d,a
    texc.push_back( glm::vec2(1, 1) );
    texc.push_back( glm::vec2(0, 1) );
    texc.push_back( glm::vec2(0, 0) );

//    for (int i=0; i<6; i++)
//    {
//        for (int j=0; j<6; j++)
//        {
//            normals.push_back( norm[i] );
//            texCoords.push_back( texc[j] );
//            normalIndices.push_back(normIndex[i]);
//        }
//    }

    //
    for(int i=0; i<6; i++)
        b_visible.push_back(true);

    //
    nVertices = 0;
}

Cube::~Cube()
{
}

void Cube::init(double sx, double sy, double sz, double tx, double ty, double tz)
{
    double cx = sx/2.0;
    double cy = sy/2.0;
    double cz = sz/2.0;

    double l = -cx - tx;
    double r = cx - tx;
    double u = cy - ty;
    double d = -cy - ty;
    double f = -cz - tz;
    double b = cz - tz;

    x = tx;
    y = ty;
    z = tz;

    //
    points.push_back( glm::vec3( l, d, f ) ); // 0
    points.push_back( glm::vec3( l, u, f ) ); // 1
    points.push_back( glm::vec3( r, u, f ) ); // 2
    points.push_back( glm::vec3( r, d, f ) ); // 3
    points.push_back( glm::vec3( l, d, b ) ); // 4
    points.push_back( glm::vec3( l, u, b ) ); // 5
    points.push_back( glm::vec3( r, u, b ) ); // 6
    points.push_back( glm::vec3( r, d, b ) ); // 7

    //
    faces();

    //
    edges();

    //
    for (int i=0; i<6; i++)
    {
        if(b_visible[i])
        {
            for (int j=0; j<6; j++)
            {
                normals.push_back( norm[i] );
                texCoords.push_back( texc[j] );
            }
        }
    }
}

int Cube::init(std::vector<glm::vec3> nodes)
{
    if(nodes.size()<8)
    {
        std::cout<<"Invalid nodes."<<std::endl;
        return -1;
    }

    //
    for(size_t i=0; i<8; i++)
    {
        points.push_back(nodes[i]);
    }

    //
    faces();

    //
    //edges();

    //
    float idx = 0;
    for (int i=0; i<6; i++)
    {
        if(b_visible[i])
        {
//            for (int j=0; j<6; j++)
//            {
////                normals.push_back( norm[i] );
////                texCoords.push_back( texc[j] );
//                normalIndices.push_back(normIndex[i]);
//            }

            idx += pow(2, i);
        }
    }

    for (int i=0; i<6; i++)
    {
        if(b_visible[i])
        {
            for (int j=0; j<6; j++)
            {
                normalIndices.push_back(idx);
            }
        }
    }

    //
    return 0;

}

void Cube::faces()
{
    // 12 triangles: 36 vertices and 36 colors

    //
    if(b_visible[1])
    {
        //setPositions(points[7], points[3], points[2], points[6]); // GL_TEXTURE_CUBE_MAP_POSITIVE_X 	0 +x Right
        setPositions(points[5], points[1], points[3], points[7]);
        nVertices += 6;
    }
    if(b_visible[0])
    {
        //setPositions(points[0], points[4], points[5], points[1]); // GL_TEXTURE_CUBE_MAP_NEGATIVE_X 	1 -x Left
        setPositions(points[0], points[4], points[6], points[2]);
        nVertices += 6;
    }
    if(b_visible[3])
    {
        //setPositions(points[1], points[5], points[6], points[2]); // GL_TEXTURE_CUBE_MAP_POSITIVE_Y 	2 +y Up (Top)
        setPositions(points[2], points[6], points[7], points[3]);
        nVertices += 6;
    }
    if(b_visible[2])
    {
        //setPositions(points[4], points[0], points[3], points[7]); // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 	3 -y Down (Bottom)
        setPositions(points[4], points[0], points[1], points[5]);
        nVertices += 6;
    }
    if(b_visible[5])
    {
        //setPositions(points[6], points[5], points[4], points[7]); // GL_TEXTURE_CUBE_MAP_POSITIVE_Z 	4 +z Back
        setPositions(points[7], points[6], points[4], points[5]);
        nVertices += 6;
    }
    if(b_visible[4])
    {
        //setPositions(points[3], points[0], points[1], points[2]); // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 	5 -z Front
        setPositions(points[1], points[0], points[2], points[3]);
        nVertices += 6;
    }

}

void Cube::edges()
{
    // 12 lines: 24 vertices and 24 colors

    // edges of the top face
    setPositions(points[0], points[1]);
    setPositions(points[1], points[2]);
    setPositions(points[2], points[3]);
    setPositions(points[3], points[0]);

    // edges of the bottom face
    setPositions(points[4], points[5]);
    setPositions(points[5], points[6]);
    setPositions(points[6], points[7]);
    setPositions(points[7], points[4]);

    // edges connecting top face to bottom face
    setPositions(points[1], points[5]);
    setPositions(points[0], points[2]);
    setPositions(points[2], points[6]);
    setPositions(points[3], points[7]);
}

void Cube::setFaceColor(glm::vec4 c)
{
    Mesh::setColor(colors, c, nVertices);
}

void Cube::setEdgeColor(glm::vec4 c)
{
    Mesh::setColor(eColors, c, 24);
}

void Cube::setVisible(std::vector<bool> v)
{
    if(v.size()<6)
    {
        std::cout<<"Invalid visible input."<<std::endl;
        return;
    }

    for(size_t i=0; i<v.size(); i++)
        b_visible[i] = v[i];
}

//
Z3DCube::Z3DCube()
{
    initByNodes = false;
    length = 1;
    x = y = z = 0;
}

Z3DCube::~Z3DCube()
{
}

//
ZCubeArray::ZCubeArray()
{
    m_type = ZStackObject::TYPE_3D_CUBE;
    m_target = ZStackObject::TARGET_3D_ONLY;
}

ZCubeArray::~ZCubeArray()
{

}

bool ZCubeArray::isEmpty() const
{
    return (m_cubeArray.size()==0);
}

Z3DCube* ZCubeArray::makeCube(const ZIntCuboid &box, glm::vec4 color, const std::vector<int> &faceArray)
{
    Z3DCube *cube = new Z3DCube;

    //
    cube->b_visible.clear();
    for(int i=0; i<6; i++)
        cube->b_visible.push_back(false);

    //
    for (size_t i = 0; i < faceArray.size(); ++i)
    {
        cube->b_visible[ faceArray[i] ] = true;
    }

    //
    ZPoint p;
    QVector<int> vertexOrder;
    vertexOrder << 0 << 2 << 3 << 1 << 4 << 6 << 7 << 5;  // convert ZCuboid -> Cube

    for (int i = 0; i < 8; ++i)
    {
        //p = box.getCorner( vertexOrder[i] ).toPoint();
        p = box.getCorner( i ).toPoint();
        cube->nodes.push_back(glm::vec3(p.x(), p.y(), p.z()));
    }

    //
    cube->color = color;

    //
    cube->initByNodes = true;

    //
    return cube;
}

void ZCubeArray::append(Z3DCube cube)
{
    m_cubeArray.push_back(cube);
}

std::vector<Z3DCube> ZCubeArray::getCubeArray()
{
    return m_cubeArray;
}

size_t ZCubeArray::size()
{
    return m_cubeArray.size();
}

void ZCubeArray::setCubeArray(std::vector<Z3DCube> cubeArray)
{    
    m_cubeArray = cubeArray;
}

void ZCubeArray::display(
    ZPainter &/*painter*/, int /*slice*/, EDisplayStyle /*option*/,
    NeuTube::EAxis /*sliceAxis*/) const
{
}

void ZCubeArray::clear()
{
  m_cubeArray.clear();
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZCubeArray)

