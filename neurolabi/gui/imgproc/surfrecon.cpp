// surfrecon: to reconstruct a surface from a point cloud and voxelization
// Yang Yu (gnayuy@gmail.com)

//
#ifdef _ENABLE_SURFRECON_
#include "surfrecon.h"

Surf::Surf()
{
}

Surf::~Surf()
{
}

/*
void Surf::test(std::vector<int> a)
{
}
*/
void Surf::setPoints(VoxelSet pointcloud)
{
    points.clear();
    
    for(int i=0; i<pointcloud.size(); i++)
    {
        points.push_back(Point(pointcloud[i].x, pointcloud[i].y, pointcloud[i].z));
    }
}

/*
 * as for A=B assigning the value of a point to another
 */
void Surf::assign(Vertex *to, Vertex *from) {
    to->x = from->x;
    to->y = from->y;
    to->z = from->z;
}

/*
 * A+B when B is implicitly a Vector3d ({x,y,z})
 */
void Surf::addTo(Vertex *result, Vertex *v, double x, double y, double z) {
    result->x = v->x + x;
    result->y = v->y + y;
    result->z = v->z + z;
}

/*
 * A-B when B is implicitly a Vector3d ({x,y,z})
 */
void Surf::subtractFrom(Vertex *result, Vertex *v, double x, double y, double z) {
    result->x = v->x - x;
    result->y = v->y - y;
    result->z = v->z - z;
}

/*
 * A+B
 */
void Surf::add(Vertex *result, Vertex *v1, Vertex *v2) {
    addTo(result, v1, v2->x, v2->y, v2->z);
}

/*
 * A-B
 */
void Surf::subtract(Vertex *result, Vertex *v1, Vertex *v2) {
    subtractFrom(result, v1, v2->x, v2->y, v2->z);
}

/*
 * C={x,y,z}=AxB| x=u2v3-u3v2, y=u3v1-u1v3, z=u1v2-u2v1
 */
void Surf::crossProduct(Vertex *result, Vertex *v1, Vertex *v2) {
    result->x = v1->y*v2->z - v2->y*v1->z;
    result->y = v2->x*v1->z - v1->x*v2->z;
    result->z = v1->x*v2->y - v1->y*v2->x;
}

/*
 * C=A.B=u1v1+u2v2+u3v3
 */
double Surf::dotProduct(Vertex *v1, Vertex *v2) {
    return v1->x*v2->x + v1->y*v2->y + v1->z*v2->z;
}

/*
 * scaling a Vector3d or Point3d
 */
void Surf::product(Vertex *result, double s, Vertex *v) {
    result->x = s*v->x;
    result->y = s*v->y;
    result->z = s*v->z;
}

/*
 * Euclidean distance(i.e. Sqrt(x2+y2+z2))
 */
double Surf::distance(Vertex *v1, Vertex *v2) {
    return sqrt((v1->x-v2->x)*(v1->x-v2->x) + (v1->y-v2->y)*(v1->y-v2->y) + (v1->z-v2->z)*(v1->z-v2->z));
}

/*
 * the bounding box is defined as a tuple(of Point3d,Point3d) respectively minimum and
 * maximum corners of the boundingbox. Haveing min corner means having min values for X,
 * Y and Z intervals and having max corner means having max values for X,Y and Z intervals.
 * This module finds the min and max values in x, y and z
 */
void Surf::getRBoundingBox(Vertex *vertices, unsigned int nVertices, Vertex *min, Vertex *max) {
    unsigned int currentVertex = 0;
    assign(min, &vertices[0]);
    assign(max, &vertices[0]);
    for (currentVertex = 1; currentVertex < nVertices; ++currentVertex) {
        if (vertices[currentVertex].x < min->x) min->x = vertices[currentVertex].x;
        if (vertices[currentVertex].x > max->x) max->x = vertices[currentVertex].x;
        if (vertices[currentVertex].y < min->y) min->y = vertices[currentVertex].y;
        if (vertices[currentVertex].y > max->y) max->y = vertices[currentVertex].y;
        if (vertices[currentVertex].z < min->z) min->z = vertices[currentVertex].z;
        if (vertices[currentVertex].z > max->z) max->z = vertices[currentVertex].z;
    }
}

void Surf::centerOfVoxel_(int x, int y, int z, Vertex *rMin, Vertex *vSize, Vertex *voxelCenter) {
    voxelCenter->x = rMin->x + ((double)x)*vSize->x + 0.5*vSize->x;
    voxelCenter->y = rMin->y + ((double)y)*vSize->y + 0.5*vSize->y;
    voxelCenter->z = rMin->z + ((double)z)*vSize->z + 0.5*vSize->z;
}

/*
 * embeds a voxel defined as {i,j,k} (three integers in Z3) in R3 as a Point3d, which
 * is a {x,y,z} or a point with three "double"s
 */
void Surf::centerOfVoxel(surfrecon::Voxel *voxel, Vertex *rMin, Vertex *vSize, Vertex *voxelCenter) {
    voxelCenter->x = rMin->x + ((double)voxel->x)*vSize->x + 0.5*vSize->x;
    voxelCenter->y = rMin->y + ((double)voxel->y)*vSize->y + 0.5*vSize->y;
    voxelCenter->z = rMin->z + ((double)voxel->z)*vSize->z + 0.5*vSize->z;
}

double Surf::distancePointSegment(LineSegment *l, Vertex *p) {
    Vertex p0, p1, v, w;
    assign(&p0, &l->start);
    assign(&p1, &l->end);
    subtract(&v, &l->end, &p0);
    subtract(&w, p, &p0);
    double c1 = dotProduct(&w, &v);
    /* the point is at the left side of the line segment {p0,p1} so it is closest to p0 */
    if (c1 <= 0)
        return distance(p, &p0);
    double c2 = dotProduct(&v, &v);
    /* the point is at the right side of the line segment {p0,p1} so it is closest to p1 */
    if (c2 <= c1)
        return distance(p, &p1);
    double b = c1/c2;
    Vertex b_v, pb;
    product(&b_v, b, &v);
    /* Pb = P0 + b * V; the point on the line segment closest to the point in question */
    add(&pb, &p0, &b_v);
    return distance(p, &pb);
}

double Surf::distancePointTriangle(Face *f, Vertex *p) {
    Vertex o, u, v;
    assign(&o, f->p1);
    subtract(&u, f->p2, &o);
    subtract(&v, f->p3, &o);
    /* considers a parametric equation for every point on the plane corresponding to the triangle (face) as P(s,t) */
    Vertex w;
    subtract(&w, p, &o);
    /* here computes five double values to be used throughout the function */
    double uu = dotProduct(&u, &u);
    double vv = dotProduct(&v, &v);
    double uv = dotProduct(&u, &v);
    double wu = dotProduct(&w, &u);
    double wv = dotProduct(&w, &v);
    /*
     * the determinant is actually the size of the cross product of two edges
     * of the triangle. If the determinant is 0 then the triangle is degenerate
     * as its area is zero (because the two edges are paralell)
     */
    double det = abs(uv*uv-uu*vv);
    if (det > 0) {
        double s = -(uv*wv-vv*wu);
        double t = -(uv*wu-uu*wv);
        /*
         t
         \R2|
         \ |
         \|
         |\
         | \R1
         R3|R0\
         __|___\_____s
         R4|R5  \R6
         */
        if (s + t <= det) {
            if (s < 0) {
                if (t < 0) {
                    /* Region4, distance to V0 */
                    return distance(f->p1, p);
                } else {
                    /* Region3, distance to line(V0,V2) */
                    LineSegment l;
                    assign(&l.start, f->p1);
                    assign(&l.end, f->p3);
                    return distancePointSegment(&l, p);
                }
            } else if (t < 0) {
                /* Region5, distance to line(V0,V1) */
                LineSegment l;
                assign(&l.start, f->p1);
                assign(&l.end, f->p2);
                return distancePointSegment(&l, p);
            } else {
                /* Region0, distance to point(s,t) */
                s /= det;
                t /= det;
                Vertex vertex, s_u, t_v;
                product(&s_u, s, &u);
                product(&t_v, t, &v);
                add(&vertex, &o, &s_u);
                add(&vertex, &vertex, &t_v);
                return distance(&vertex, p);
            }
        } else {
            if (s < 0) {
                /* Region2, distance to V2 */
                return distance(f->p3, p);
            } else if (t < 0) {
                /* Region6, distance to V1 */
                return distance(f->p2, p);
            } else {
                /* Region1, distance to line(V1,V2) */
                LineSegment l;
                assign(&l.start, f->p2);
                assign(&l.end, f->p3);
                return distancePointSegment(&l, p);
            }
        }
    } else {
        return DBL_MAX;
    }
}

/*
 * returns 1 on success and 0 on failure; success means that the distance of
 * the point to one of the triangles of the mesh is smaller than half of the
 * length(norm) of the VSize vector (the longest diagon of a voxel cube)
 */
int Surf::isNear(int num_threads, Face **mesh, unsigned int nTriangles, Vertex *voxelCenter, Vertex *vSize) {
    int res = 0;
    unsigned int currentTriangle = 0;
    double threshold = 0.5*sqrt(vSize->x*vSize->x + vSize->y*vSize->y + vSize->z*vSize->z);
    for (currentTriangle = 0; currentTriangle < nTriangles; ++currentTriangle) {
        if (distancePointTriangle(mesh[currentTriangle], voxelCenter) <= threshold) {
            res = 1;
            break;
        }
    }
    
    return res;
}

/*
 * using the parametric equation of a point as P(s,t) in reference  to two edges of a triangle
 * we find out if the intersection of a point and the plane corresponding to the triangle lies within the triangle;
 * this would correspond to s and t belonging to [0,1] and s+t,=1
 */
int Surf::intersectsTriangleLine(Face *f, LineSegment *l) {
    Vertex o, u, v, n;
    assign(&o, f->p1);
    subtract(&u, f->p2, &o);
    subtract(&v, f->p3, &o);
    crossProduct(&n, &u, &v);
    
    Vertex ps, pe;
    assign(&ps, &l->start);
    assign(&pe, &l->end);
    
    Vertex o_ps, pe_ps;
    subtract(&o_ps, &o, &ps);
    double nomin = dotProduct(&o_ps, &n);
    subtract(&pe_ps, &pe, &ps);
    double denom = dotProduct(&n, &pe_ps);
    
    if (denom != 0) {
        double alpha = nomin/denom;
        Vertex scaled_pe_ps, p, w;
        product(&scaled_pe_ps, alpha, &pe_ps);
        add(&p, &ps, &scaled_pe_ps);
        subtract(&w, &p, &o);
        double uu = dotProduct(&u, &u);
        double vv = dotProduct(&v, &v);
        double uv = dotProduct(&u, &v);
        double wu = dotProduct(&w, &u);
        double wv = dotProduct(&w, &v);
        double stdenom = uv*uv-uu*vv;
        double s = (uv*wv-vv*wu)/stdenom;
        double t = (uv*wu-uu*wv)/stdenom;
        Vertex point, s_u, t_v;
        product(&s_u, s, &u);
        product(&t_v, t, &v);
        add(&point, &o, &s_u);
        add(&point, &point, &t_v);
        
        if (s >= 0 && t >= 0 && s + t <= 1) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

/*
 * using the function triangle line intersection we iterate over all triangular faces of a mesh
 */
int Surf::intersectsMeshLine(Face **mesh, unsigned int nTriangles, LineSegment *l) {
    unsigned int currentTriangle = 0;
    for (currentTriangle = 0; currentTriangle < nTriangles; ++currentTriangle) {
        if (intersectsTriangleLine(mesh[currentTriangle], l)) return 1;
    } return 0;
}

/*
 * we intersect a "connectivity target"(Laine, 2013) for 26-connected results; this is a 3D
 * crosshair composed of 6 lines. If intersection is not null then the voxel should be included
 */
int Surf::intersectsMesh26(Face **mesh, unsigned int nTriangles, Vertex *voxelCenter, Vertex *vSize) {
    LineSegment testLine;
    assign(&testLine.start, voxelCenter);
    
    assign(&testLine.end, voxelCenter);
    testLine.end.x += 0.5*vSize->x;
    if (intersectsMeshLine(mesh, nTriangles, &testLine)) return 1;
    
    assign(&testLine.end, voxelCenter);
    testLine.end.y += 0.5*vSize->y;
    if (intersectsMeshLine(mesh, nTriangles, &testLine)) return 1;
    
    assign(&testLine.end, voxelCenter);
    testLine.end.z += 0.5*vSize->z;
    if (intersectsMeshLine(mesh, nTriangles, &testLine)) return 1;
    
    assign(&testLine.end, voxelCenter);
    testLine.end.x -= 0.5*vSize->x;
    if (intersectsMeshLine(mesh, nTriangles, &testLine)) return 1;
    
    assign(&testLine.end, voxelCenter);
    testLine.end.y -= 0.5*vSize->y;
    if (intersectsMeshLine(mesh, nTriangles, &testLine)) return 1;
    
    assign(&testLine.end, voxelCenter);
    testLine.end.z -= 0.5*vSize->z;
    if (intersectsMeshLine(mesh, nTriangles, &testLine)) return 1;
    
    return 0;
}

/*
 * we intersect a "connectivity target"(Laine, 2013) for 6-connected results with the mesh in
 * question; this is a the outline of a voxel cube composed of 12 lines. If intersection is not
 * null then the voxel should be included
 */
int Surf::intersectsMesh6(Face **mesh, unsigned int nTriangles, Vertex *voxelCenter, Vertex *vSize) {
    unsigned int currentEdge = 0;
    Vertex vertices[8];
    LineSegment edges[12];
    
    Vertex halfSize;
    halfSize.x = 0.5*vSize->x;
    halfSize.y = 0.5*vSize->y;
    halfSize.z = 0.5*vSize->z;
    
    addTo(&vertices[0], voxelCenter, +halfSize.x, +halfSize.y, +halfSize.z);
    addTo(&vertices[1], voxelCenter, -halfSize.x, +halfSize.y, +halfSize.z);
    addTo(&vertices[2], voxelCenter, -halfSize.x, -halfSize.y, +halfSize.z);
    addTo(&vertices[3], voxelCenter, +halfSize.x, -halfSize.y, +halfSize.z);
    addTo(&vertices[4], voxelCenter, -halfSize.x, -halfSize.y, -halfSize.z);
    addTo(&vertices[5], voxelCenter, +halfSize.x, -halfSize.y, -halfSize.z);
    addTo(&vertices[6], voxelCenter, +halfSize.x, +halfSize.y, -halfSize.z);
    addTo(&vertices[7], voxelCenter, -halfSize.x, +halfSize.y, -halfSize.z);
    
    assign(&edges[0].start, &vertices[0]); assign(&edges[0].end, &vertices[1]);
    assign(&edges[1].start, &vertices[1]); assign(&edges[1].end, &vertices[2]);
    assign(&edges[2].start, &vertices[2]); assign(&edges[2].end, &vertices[3]);
    assign(&edges[3].start, &vertices[3]); assign(&edges[3].end, &vertices[0]);
    assign(&edges[4].start, &vertices[0]); assign(&edges[4].end, &vertices[6]);
    assign(&edges[5].start, &vertices[6]); assign(&edges[5].end, &vertices[5]);
    assign(&edges[6].start, &vertices[5]); assign(&edges[6].end, &vertices[4]);
    assign(&edges[7].start, &vertices[4]); assign(&edges[7].end, &vertices[7]);
    assign(&edges[8].start, &vertices[5]); assign(&edges[8].end, &vertices[3]);
    assign(&edges[9].start, &vertices[4]); assign(&edges[9].end, &vertices[2]);
    assign(&edges[10].start, &vertices[1]); assign(&edges[10].end, &vertices[7]);
    assign(&edges[11].start, &vertices[6]); assign(&edges[11].end, &vertices[7]);
    
    for (currentEdge = 0; currentEdge < 12; ++currentEdge) {
        if (intersectsMeshLine(mesh, nTriangles, &edges[currentEdge])) return 1;
    }
    
    return 0;
}

// Zlatanova's 3D raster engine
int Surf::voxelizeMesh(VoxelSet &voxels, Vertex *vertices, unsigned int nVertices, Face **mesh, unsigned int nTriangles, Vertex *vSize, int co, int num_threads)
{
    Vertex rMin, rMax;
    surfrecon::Voxel vMax;
    
    // Bounding box already in Z3
    getRBoundingBox(vertices, nVertices, &rMin, &rMax);
    rMin.x = floor(rMin.x);
    rMin.y = floor(rMin.y);
    rMin.z = floor(rMin.z);

    vMax.x = (unsigned int)ceil((rMax.x-rMin.x)/vSize->x);
    vMax.y = (unsigned int)ceil((rMax.y-rMin.y)/vSize->y);
    vMax.z = (unsigned int)ceil((rMax.z-rMin.z)/vSize->z);

    // Go voxel by voxel, in the bounding box of the mesh. note that
    // if the mesh is big it will slow down the whole process. It is
    // better to put in many small mesh objects that one big mesh object
    int x, y, z;
    int i=0;
    voxels.clear();
    
#ifdef Use_OpenMP
    omp_set_num_threads(num_threads);
#pragma omp parallel for private(x, y, z)
#endif
    for (x = 0; x <= vMax.x; ++x) {
        for (y = 0; y <= vMax.y; ++y) {
            for (z = 0; z <= vMax.z; ++z) {
                Vertex voxelCenter;
                centerOfVoxel_(x, y, z, &rMin, vSize, &voxelCenter);
                
                // Check nearness
                int inTarget = 1;
                if (!isNear(num_threads, mesh, nTriangles, &voxelCenter, vSize)) inTarget = 0;
                
                // Check intersection
                if (inTarget)
                {
                    if (co == 26)
                    {
                        inTarget = intersectsMesh26(mesh, nTriangles, &voxelCenter, vSize);
                    }
                    else if (co == 6)
                    {
                        inTarget = intersectsMesh6(mesh, nTriangles, &voxelCenter, vSize);
                    }
                    else
                    {
                        printf("connectivity target undefined!");
                        inTarget = 0;
                    }
                }
                
                //
                if (inTarget)
                {
                    voxels.push_back(VoxelType(voxelCenter.x, voxelCenter.y, voxelCenter.z));
                }
            }
        }
    }
    
    return 0;
}

// surfrecon func
void Surf::surfrecon(VoxelSet pcIn, VoxelSet &voxelOut, int co, int num_threads)
{
    // init
    setPoints(pcIn);
    
    //
    Timer t;
    t.start();
    
    // CGAL's implementation of scale space surface reconstrcution
    // Digne, Julie, et al. "Scale space meshing of raw data point sets." 
    // Computer Graphics Forum. Vol. 30. No. 6. Blackwell Publishing Ltd, 2011.
    //
    Reconstruction reconstruct( 10, 200 );
    
    reconstruct.reconstruct_surface( points.begin(), points.end(), 4,
                                    false, // Do not separate shells
                                    true // Force manifold output
                                    );
    
    std::cerr << " Surface reconstruction done in " << t.time() << " sec." << std::endl;

    //
    faces.clear();
    for( Triple_iterator it = reconstruct.surface_begin( ); it != reconstruct.surface_end(  ); ++it )
    {
        int c=0;
        surfrecon::Voxel v;
        for (auto i:*it)
        {
            switch (c++)
            {
                case 0:
                    v.x = i;
                    break;
                    
                case 1:
                    v.y = i;
                    break;
                    
                case 2:
                    v.z = i;
                    break;
                    
                default:
                    break;
            }
        }
        faces.push_back(v);
    }
    
    //
    int nVertices = points.size(), nTriangles = faces.size(), v=0;
    Vertex *vertices = NULL;
    Face *mesh = NULL;
    
    try {
        vertices = new Vertex [nVertices];
        mesh = new Face [nTriangles];
    } catch (...) {
        cout<<"Fail to allocate memory for Vertices and Faces."<<endl;
        return;
    }
    
    Face **mesh_ptr = NULL;
    try {
        mesh_ptr = new Face* [nTriangles];
    } catch (...) {
        cout<<"Fail to allocate memory for Vertices and Faces."<<endl;
        return;
    }
    
    for(long i=0; i<nVertices; i++)
    {
        vertices[i].x = points[i].x();
        vertices[i].y = points[i].y();
        vertices[i].z = points[i].z();
    }
    
    for(long i=0; i<nTriangles; i++)
    {
        mesh[i].p1 = &vertices[faces[i].x];
        mesh[i].p2 = &vertices[faces[i].y];
        mesh[i].p3 = &vertices[faces[i].z];
        
        mesh_ptr[i] = &mesh[i];
    }
    
    Vertex vSize;
    vSize.x = 1.0;
    vSize.y = 1.0;
    vSize.z = 1.0;
    
    //
    t.reset();
    if (voxelizeMesh(voxelOut, vertices, nVertices, mesh_ptr, nTriangles, &vSize, co, num_threads))
    {
        if(mesh_ptr) {delete []mesh_ptr; mesh_ptr=NULL;}
        if(mesh) {delete []mesh; mesh=NULL;}
        if(vertices) {delete []vertices; vertices=NULL;}
        cout << "Fail to execute voxelization."<<endl;
        return;
    }
    cout<<" Convert mesh "<<nTriangles<<" triangles to "<<voxelOut.size()<<" voxels in "<< t.time() << " sec." <<endl;

    //
    return;
}
#endif
