// surfrecon: to reconstruct a surface from a point cloud and voxelization
// Yang Yu (gnayuy@gmail.com)


#ifndef __SURFRECON_H__
#define __SURFRECON_H__

#ifdef _ENABLE_SURFRECON_
//
#include <deque>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <functional>
#include <vector>
#include <cmath>
#include <ctime>
#include <limits>
#include <complex>
#include <float.h>
#include "glm/vec3.hpp"
#include <boost/foreach.hpp>

#ifdef Use_OpenMP
#include <omp.h>
#endif

#include <CGAL/Scale_space_surface_reconstruction_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/IO/read_off_points.h>
#include <CGAL/Timer.h>

//
using namespace std;

//
typedef CGAL::Exact_predicates_inexact_constructions_kernel     Kernel;

typedef CGAL::Scale_space_surface_reconstruction_3< Kernel >    Reconstruction;

typedef Reconstruction::Point                                   Point;
typedef std::vector< Point >                                    PointSet;

typedef Reconstruction::Triple_const_iterator                   Triple_iterator;
typedef CGAL::Timer Timer;

//
typedef glm::vec3                                               VoxelType;
typedef std::vector< VoxelType >                                VoxelSet;

//
typedef struct {
    double x, y, z;
} Vertex;

typedef struct {
    Vertex *p1, *p2, *p3;
} Face;

namespace surfrecon {
  typedef struct {
    unsigned int x, y, z;
  } Voxel;
}

typedef struct {
    Vertex start, end;
} LineSegment;

typedef std::vector<surfrecon::Voxel>     FaceSet;

// Surf
class Surf
{
public:
    Surf();
    ~Surf();
    
public:
    void setPoints(VoxelSet pointcloud);
    void surfrecon(VoxelSet pcIn, VoxelSet &voxelOut, int co = 26, int num_threads = 8);
//    void test(std::vector<int> a);
    
public:
    // Voxelization with Zlatanova's efficient implementation [1] of Laine's topological voxelization [2]
    // 1. Zlatanova, Sisi, et al. "Towards 3D raster GIS: On developing a raster engine for spatial DBMS."
    // ISPRS WG IV/2 Workshop "Global Geospatial Information and High Resolution Global Land Cover/Land Use Mapping"
    // Novosibirsk, Russian Federation, 21 April 2016.
    // 2. Laine, Samuli. "A topological approach to voxelization." Computer Graphics Forum. Vol. 32. No. 4. 
    // Blackwell Publishing Ltd, 2013.
    //
    int voxelizeMesh(VoxelSet &voxels, Vertex *vertices, unsigned int nVertices, Face **mesh, unsigned int nTriangles, Vertex *vSize, int co = 26, int num_threads = 8);
    
    void assign(Vertex *to, Vertex *from);
    void addTo(Vertex *result, Vertex *v, double x, double y, double z);
    void subtractFrom(Vertex *result, Vertex *v, double x, double y, double z);
    void add(Vertex *result, Vertex *v1, Vertex *v2);
    void subtract(Vertex *result, Vertex *v1, Vertex *v2);
    void crossProduct(Vertex *result, Vertex *v1, Vertex *v2);
    double dotProduct(Vertex *v1, Vertex *v2);
    void product(Vertex *result, double s, Vertex *v);
    double distance(Vertex *v1, Vertex *v2);
    void getRBoundingBox(Vertex *vertices, unsigned int nVertices, Vertex *min, Vertex *max);
    void centerOfVoxel_(int x, int y, int z, Vertex *rMin, Vertex *vSize, Vertex *voxelCenter);
    void centerOfVoxel(surfrecon::Voxel *voxel, Vertex *rMin, Vertex *vSize, Vertex *voxelCenter);
    double distancePointSegment(LineSegment *l, Vertex *p);
    double distancePointTriangle(Face *f, Vertex *p);
    int isNear(int num_threads, Face **mesh, unsigned int nTriangles, Vertex *voxelCenter, Vertex *vSize);
    int intersectsTriangleLine(Face *f, LineSegment *l);
    int intersectsMeshLine(Face **mesh, unsigned int nTriangles, LineSegment *l);
    int intersectsMesh26(Face **mesh, unsigned int nTriangles, Vertex *voxelCenter, Vertex *vSize);
    int intersectsMesh6(Face **mesh, unsigned int nTriangles, Vertex *voxelCenter, Vertex *vSize);
    
public:
    PointSet points;
    FaceSet faces;
};
#endif

#endif // __SURFRECON_H__
