#ifndef MARCHING_CUBES_H
#define MARCHING_CUBES_H

//Adapted from https://github.com/ilastik/marching_cubes

#include <map>
#include <vector>

namespace ilastik {

typedef float Point[3];

struct IdPoint
{
	size_t id;
	float x, y, z;
};

typedef std::map<size_t, IdPoint> PointIdMapping;

struct Triangle
{
	size_t pointId[3];
};

/**
 * the primary struct used to pass around the components of a mesh
 * vertices:    the vertex positions as an array of points
 * normals:     the normal direction of each vertex as an array of points
 * vertexCount: the number of vertices and normals
 * faces:       the faces given by 3 vertex indices (length = faceCount * 3)
 * faceCount:   the number of faces
 */
struct Mesh
{
	size_t vertexCount;
    Point* vertices = NULL;
    Point* normals = NULL;
	size_t faceCount;
    size_t* faces = NULL;

	Mesh(size_t, Point*, Point*, size_t, size_t*);
	Mesh();
    ~Mesh() {
      delete []vertices;
      delete []normals;
      delete []faces;
    }
};

/**
 * the marching cubes algorithm as described here: http://paulbourke.net/geometry/polygonise/
 * volume:   contains the data (size = xDim * yDim * zDim)
 * [xyz]Dim: the dimensions in each direction
 * isoLevel: the minimum isoLevel, all values >= isoLevel will contribute to the mesh
 * the mesh is returned, the caller takes ownership over the pointers
 */
Mesh march(const uint8_t*, size_t, size_t, size_t, uint8_t);

}

#endif
