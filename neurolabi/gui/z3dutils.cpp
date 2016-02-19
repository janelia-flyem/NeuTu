#include "z3dutils.h"
#include <limits>
#include <queue>
#include "zrandom.h"

namespace {

//----------------------------------------------------------------------------
// A helper class to quickly locate an edge, given the endpoint ids.
// It uses an stl map rather than a table partitioning scheme, since
// we have no idea how many entries there will be when we start.  So
// the performance is approximately log(n).

class vtkCCSEdgeLocatorNode
{
public:
  vtkCCSEdgeLocatorNode() :
    ptId0(-1), ptId1(-1), edgeId(-1), next(0) {}

  ~vtkCCSEdgeLocatorNode()
  {
    vtkCCSEdgeLocatorNode *ptr = this->next;
    while (ptr)
    {
      vtkCCSEdgeLocatorNode *tmp = ptr;
      ptr = ptr->next;
      tmp->next = 0;
      delete tmp;
    }
  }

  int64_t ptId0;
  int64_t ptId1;
  int64_t edgeId;
  vtkCCSEdgeLocatorNode *next;
};


class vtkCCSEdgeLocator
{
private:
  typedef std::map<int64_t, vtkCCSEdgeLocatorNode> MapType;
  MapType EdgeMap;

public:
  static vtkCCSEdgeLocator *New()
  {
    return new vtkCCSEdgeLocator;
  }

  void Delete()
  {
    delete this;
  }

  // Description:
  // Initialize the locator.
  void Initialize();

  // Description:
  // If edge (i0, i1) is not in the list, then it will be added and
  // a pointer for storing the new edgeId will be returned.
  // If edge (i0, i1) is in the list, then edgeId will be set to the
  // stored value and a null pointer will be returned.
  int64_t *InsertUniqueEdge(int64_t i0, int64_t i1, int64_t &edgeId);
};

void vtkCCSEdgeLocator::Initialize()
{
  this->EdgeMap.clear();
}

int64_t *vtkCCSEdgeLocator::InsertUniqueEdge(
    int64_t i0, int64_t i1, int64_t &edgeId)
{
  // Ensure consistent ordering of edge
  if (i1 < i0)
  {
    int64_t tmp = i0;
    i0 = i1;
    i1 = tmp;
  }

  // Generate a integer key, try to make it unique
  int64_t key = ((i1 << 32) ^ i0);

  vtkCCSEdgeLocatorNode *node = &this->EdgeMap[key];

  if (node->ptId1 < 0)
  {
    // Didn't find key, so add a new edge entry
    node->ptId0 = i0;
    node->ptId1 = i1;
    return &node->edgeId;
  }

  // Search through the list for i0 and i1
  if (node->ptId0 == i0 && node->ptId1 == i1)
  {
    edgeId = node->edgeId;
    return 0;
  }

  int i = 1;
  while (node->next != 0)
  {
    i++;
    node = node->next;

    if (node->ptId0 == i0 && node->ptId1 == i1)
    {
      edgeId = node->edgeId;
      return 0;
    }
  }

  // No entry for i1, so make one and return
  node->next = new vtkCCSEdgeLocatorNode;
  node = node->next;
  node->ptId0 = i0;
  node->ptId1 = i1;
  node->edgeId = this->EdgeMap.size()-1;
  return &node->edgeId;
}

//----------------------------------------------------------------------------
// Description:
// A helper function for interpolating a new point along an edge.  It
// stores the index of the interpolated point in "i", and returns 1 if
// a new point was added to the points.  The values i0, i1, v0, v1 are
// the edge enpoints and scalar values, respectively.
// Point interpolation for clipping and contouring, given the scalar
// values (v0, v1) for the two endpoints (p0, p1).  The use of this
// function guarantees perfect consistency in the results.
int InterpolateEdge(
    std::vector<glm::dvec3> &vertices, vtkCCSEdgeLocator *locator,
    double tol, int64_t i0, int64_t i1, double v0, double v1,
    int64_t &i)
{
  // This swap guarantees that exactly the same point is computed
  // for both line directions, as long as the endpoints are the same.
  if (v1 > 0)
  {
    std::swap(i0, i1);
    std::swap(v0, v1);
  }

  // After the above swap, i0 will be kept, and i1 will be clipped

  // Check to see if this point has already been computed
  int64_t *iptr = locator->InsertUniqueEdge(i0, i1, i);
  if (iptr == 0)
  {
    return 0;
  }

  // Get the edge and interpolate the new point
  glm::dvec3 p0 = vertices[i0];
  glm::dvec3 p1 = vertices[i1];
  double f = v0/(v0 - v1);
  glm::dvec3 p = glm::mix(p0, p1, f);

  double tol2 = tol*tol;

  // Make sure that new point is far enough from kept point
  if (glm::dot(p - p0, p - p0) < tol2)
  {
    i = i0;
    *iptr = i0;
    return 0;
  }

  if (glm::dot(p - p1, p - p1) < tol2)
  {
    i = i1;
    *iptr = i1;
    return 0;
  }

  vertices.push_back(p);
  i = static_cast<int64_t>(vertices.size() - 1);

  // Store the new index in the locator
  *iptr = i;

  return 1;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Everything below this point is support code for MakePolysFromContours().
// It could be separated out into its own class for generating
// polygons from contours.
//
// MakePolysFromContours uses the following steps:
// 1) Join line segments into contours, never change line directions
// 2) If any contours aren't closed, and if a loose end is on the hull
//    of the point set, try to connect it with another loose end on the hull
// 3) Remove degenerate points and points at 180 degree vertices
// 4) Group polygons according to which polygons are inside others
// 5) Cut the "hole" polygons to make simple polygons
// 6) Check for pinch-points to ensure that polygons are simple polygons
// 7) Triangulate polygons with vtkPolygon::Triangulate()
// 8) Add triangles for each point removed in Step 3
//
// In other words, this routine does a lot of work to process the contours
// so that vtkPolygon can be used to triangulate them (vtkPolygon only does
// simple polygons and even then it will fail on degenerate vertices or
// vertices with 180 degree angles).
//
// The whole mess below could be replaced by any robust triangulation code
// that can deal with holes.  Also, it is O(n^2) while available algorithms
// are O(n log n).  The vtkDelaunay2D filter will go into infinite recursion
// for some triangulations, hence it cannot be used.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// A helper class: a bitfield that is always as large as needed.
// For our purposes this is much more convenient than a bool vector,
// which would have to be resized and range-checked externally.

class vtkCCSBitArray
{
public:
  void set(size_t bit, int val)
  {
    size_t n = (bit >> 5);
    size_t i = (bit & 0x1f);
    if (n >= bitstorage.size()) { bitstorage.resize(n+1); }
    unsigned int chunk = bitstorage[n];
    int bitval = 1;
    bitval <<= i;
    if (val) { chunk = chunk | bitval; }
    else { chunk = chunk & ~bitval; }
    bitstorage[n] = chunk;
  }

  int get(size_t bit)
  {
    size_t n = (bit >> 5);
    size_t i = (bit & 0x1f);
    if (n >= bitstorage.size()) { return 0; }
    unsigned int chunk = bitstorage[n];
    return ((chunk >> i) & 1);
  }

  void clear()
  {
    bitstorage.clear();
  }

private:
  std::vector<unsigned int> bitstorage;
};

//----------------------------------------------------------------------------
// Simple typedefs for stl-based polygons.

// A poly type that is just a vector of int64_t
typedef std::vector<int64_t> vtkCCSPoly;

// A poly group type that holds indices into a vector of polys.
// A poly group is used to represent a polygon with holes.
// The first member of the group is the outer poly, and all
// other members are the holes.
typedef std::vector<size_t> vtkCCSPolyGroup;

// Extra info for each edge in a poly
typedef std::vector<int64_t> vtkCCSPolyEdges;

//----------------------------------------------------------------------------
// These are the prototypes for helper functions for manipulating
// polys that are stored in stl vectors.

// Tolerances are relative to polygon size
#define VTK_CCS_POLYGON_TOLERANCE 1e-5

// ---------------------------------------------------
// Insert a triangle, and subdivide that triangle if one of
// its edges originally had more than two points before
// vtkCCSFindTrueEdges was called.
// Add a triangle to the output, and subdivide the triangle if one
// of the edges originally had more than two points, as indicated
// by originalEdges.  If scalars is not null, then add a scalar for
// each triangle.
void vtkCCSInsertTriangle(
    std::vector<glm::i64vec3> &polys, const std::vector<int64_t> &poly, const size_t trids[3],
    const std::vector<int64_t> &polyEdges, std::vector<std::vector<int64_t> > &originalEdges)
{
  static const size_t nextVert[3] = { 1, 2, 0 };

  // To store how many of originalEdges match
  int edgeCount = 0;
  int edgeLocs[3];
  edgeLocs[0] = -1;
  edgeLocs[1] = -1;
  edgeLocs[2] = -1;

  // Check for original edge matches
  for (int vert = 0; vert < 3; vert++)
  {
    size_t currId = trids[vert];
    int64_t edgeLoc = polyEdges[currId];
    if (edgeLoc >= 0)
    {
      size_t nextId = currId+1;
      if (nextId == poly.size()) { nextId = 0; }

      // Is the triangle edge a polygon edge?
      if (nextId == trids[nextVert[vert]])
      {
        edgeLocs[vert] = edgeLoc;
        edgeCount++;
      }
    }
  }

  if (edgeCount == 0)
  {
    // No special edge handling, so just do one triangle
    polys.push_back(glm::i64vec3(poly[trids[0]], poly[trids[1]], poly[trids[2]]));
  }
  else
  {
    // Make triangle fans for edges with extra points

    int64_t edgePtIds[4];
    edgePtIds[0] = poly[trids[0]];
    edgePtIds[1] = poly[trids[1]];
    edgePtIds[2] = poly[trids[2]];
    edgePtIds[3] = poly[trids[0]];

    int64_t *edgePts[3];
    edgePts[0] = &edgePtIds[0];
    edgePts[1] = &edgePtIds[1];
    edgePts[2] = &edgePtIds[2];

    int64_t edgeNPts[3];
    edgeNPts[0] = 2;
    edgeNPts[1] = 2;
    edgeNPts[2] = 2;

    // Find out which edge has the most extra points
    int64_t maxPoints = 0;
    int currSide = 0;
    for (int i = 0; i < 3; i++)
    {
      if (edgeLocs[i] >= 0)
      {
        int64_t npts = originalEdges[edgeLocs[i]].size();
        int64_t *pts = &(originalEdges[edgeLocs[i]][0]);
        assert(edgePts[i][0] == pts[0]);
        assert(edgePts[i][1] == pts[npts-1]);
        if (npts > maxPoints)
        {
          maxPoints = npts;
          currSide = i;
        }
        edgeNPts[i] = npts;
        edgePts[i] = pts;
      }
    }

    // Find the edges before/after the edge with most points
    int prevSide = (currSide+2)%3;
    int nextSide = (currSide+1)%3;

    // If other edges have only 2 points, nothing to do with them
    int prevNeeded = (edgeNPts[prevSide] > 2);
    int nextNeeded = (edgeNPts[nextSide] > 2);

    // The tail is the common point in the triangle fan
    int64_t tailPtIds[3];
    tailPtIds[prevSide] = edgePts[currSide][1];
    tailPtIds[currSide] = edgePts[prevSide][0];
    tailPtIds[nextSide] = edgePts[currSide][edgeNPts[currSide]-2];

    // Go through the sides and make the fans
    for (int side = 0; side < 3; side++)
    {
      if ((side != prevSide || prevNeeded) &&
          (side != nextSide || nextNeeded))
      {
        int64_t m = 0;
        int64_t n = edgeNPts[side]-1;

        if (side == currSide)
        {
          m += prevNeeded;
          n -= nextNeeded;
        }

        for (int k = m; k < n; k++)
        {
          polys.push_back(glm::i64vec3(edgePts[side][k], edgePts[side][k+1], tailPtIds[side]));
        }
      }
    }
  }
}

glm::dvec3 ComputePolygonNormal(const std::vector<int64_t> &poly, const std::vector<glm::dvec3> &vertices)
{
  glm::dvec3 normal(0.0);
  if (poly.empty())
    return normal;
  glm::dvec3 anchor = vertices[poly[0]];
  for (size_t i=1; i<poly.size()-1; i++) {
    glm::dvec3 v1 = vertices[poly[i]] - anchor;
    glm::dvec3 v2 = vertices[poly[i+1]] - anchor;
    normal += glm::cross(v1, v2);
  }
  return normal;
}

//----------------------------------------------------------------------------
// The measure is the ratio of triangle perimeter^2 to area;
// the sign of the measure is determined by dotting the local
// vector with the normal (concave features return a negative
// measure).
double ComputePolygonTriangleMeasure(size_t centerVertexIdx, glm::dvec3 normal, const std::vector<int64_t> &poly, const std::vector<glm::dvec3> &vertices)
{
  size_t prev = centerVertexIdx - 1;
  size_t next = centerVertexIdx + 1;
  if (centerVertexIdx == 0)
    prev = poly.size() - 1;
  if (next >= poly.size())
    next = 0;
  glm::dvec3 v1 = vertices[poly[centerVertexIdx]] - vertices[poly[prev]];
  glm::dvec3 v2 = vertices[poly[next]] - vertices[poly[centerVertexIdx]];
  glm::dvec3 v3 = vertices[poly[prev]] - vertices[poly[next]];
  glm::dvec3 v4 = glm::cross(v1, v2); //|v4| is twice the area
  double area;
  if ( (area=glm::dot(v4, normal)) < 0.0 ) {
    return -1.0; //concave or bad triangle
  } else if ( area == 0.0 ) {
    return -std::numeric_limits<double>::max(); //concave or bad triangle
  } else {
    double perimeter = glm::length(v1) + glm::length(v2) +
        glm::length(v3);
    return perimeter*perimeter/area;
  }
}

//----------------------------------------------------------------------------
// Performs intersection of two finite 3D lines. An intersection is found if
// the projection of the two lines onto the plane perpendicular to the cross
// product of the two lines intersect. The parameters (u,v) are the
// parametric coordinates of the lines at the position of closest approach.
int IntersectionLine(glm::dvec3 a1, glm::dvec3 a2, glm::dvec3 b1, glm::dvec3 b2,
                     double& u, double& v)
{
  glm::dvec3 a21, b21, b1a1;
  double c[2];
  double *A[2], row1[2], row2[2];

  //  Initialize
  u = v = 0.0;

  //   Determine line vectors.
  a21 = a2 - a1;
  b21 = b2 - b1;
  b1a1 = b1 - a1;

  //   Compute the system (least squares) matrix.
  A[0] = row1;
  A[1] = row2;
  row1[0] = glm::dot( a21, a21 );
  row1[1] = -glm::dot( a21, b21 );
  row2[0] = row1[1];
  row2[1] = glm::dot( b21, b21 );

  //   Compute the least squares system constant term.
  c[0] = glm::dot( a21, b1a1 );
  c[1] = -glm::dot( b21, b1a1 );


  //  Solve the system of equations
  double det = A[0][0] * A[1][1] - A[0][1] * A[1][0];

  if (det == 0.0)
  {
    // Unable to solve linear system
    return 3;
  }

  u = (A[1][1]*c[0] - A[0][1]*c[1]) / det;
  v = (-A[1][0]*c[0] + A[0][0]*c[1]) / det;

  //  Check parametric coordinates for intersection.
  if ( (0.0 <= u) && (u <= 1.0) && (0.0 <= v) && (v <= 1.0) )
  {
    return 2;
  }
  else
  {
    return 0;
  }
}

#define VTK_POLYGON_FAILURE -1
#define VTK_POLYGON_OUTSIDE 0
#define VTK_POLYGON_INSIDE 1
#define VTK_POLYGON_INTERSECTION 2
#define VTK_POLYGON_ON_LINE 3
#define VTK_POLYGON_CERTAIN 1
#define VTK_POLYGON_UNCERTAIN 0
#define VTK_POLYGON_RAY_TOL 1.e-03 //Tolerance for ray firing
#define VTK_POLYGON_MAX_ITER 10    //Maximum iterations for ray-firing
#define VTK_POLYGON_VOTE_THRESHOLD 2
#define 	VTK_TOL   1.e-05

//----------------------------------------------------------------------------
// Determine whether point is inside polygon. Function uses ray-casting
// to determine if point is inside polygon. Works for arbitrary polygon shape
// (e.g., non-convex). Returns 0 if point is not in polygon; 1 if it is.
// Can also return -1 to indicate degenerate polygon. Note: a point in
// bounding box check is NOT performed prior to in/out check. You may want
// to do this to improve performance.
int PointInPolygon(glm::dvec3 x, const std::vector<glm::dvec3> &pts, const double bounds[6], glm::dvec3 normal)
{
  double u, v;
  double rayMag, mag=1;
  glm::dvec3 ray, xray;
  int testResult, status, numInts;
  int iterNumber;
  int maxComp, comps[2];
  int deltaVotes;

  // do a quick bounds check
  if ( x[0] < bounds[0] || x[0] > bounds[1] ||
       x[1] < bounds[2] || x[1] > bounds[3] ||
       x[2] < bounds[4] || x[2] > bounds[5])
  {
    return VTK_POLYGON_OUTSIDE;
  }

  //
  //  Define a ray to fire.  The ray is a random ray normal to the
  //  normal of the face.  The length of the ray is a function of the
  //  size of the face bounding box.
  //
  for (int i=0; i<3; i++)
  {
    ray[i] = ( bounds[2*i+1] - bounds[2*i] )*1.1 +
        std::abs((bounds[2*i+1] + bounds[2*i])/2.0 - x[i]);
  }

  if ( (rayMag = glm::length(ray)) == 0.0 )
  {
    return VTK_POLYGON_OUTSIDE;
  }

  //  Get the maximum component of the normal.
  //
  if ( std::abs(normal[0]) > std::abs(normal[1]) )
  {
    if ( std::abs(normal[0]) > std::abs(normal[2]) )
    {
      maxComp = 0;
      comps[0] = 1;
      comps[1] = 2;
    }
    else
    {
      maxComp = 2;
      comps[0] = 0;
      comps[1] = 1;
    }
  }
  else
  {
    if ( std::abs(normal[1]) > std::abs(normal[2]) )
    {
      maxComp = 1;
      comps[0] = 0;
      comps[1] = 2;
    }
    else
    {
      maxComp = 2;
      comps[0] = 0;
      comps[1] = 1;
    }
  }

  //  Check that max component is non-zero
  //
  if ( normal[maxComp] == 0.0 )
  {
    return VTK_POLYGON_FAILURE;
  }

  //  Enough information has been acquired to determine the random ray.
  //  Random rays are generated until one is satisfactory (i.e.,
  //  produces a ray of non-zero magnitude).  Also, since more than one
  //  ray may need to be fired, the ray-firing occurs in a large loop.
  //
  //  The variable iterNumber counts the number of iterations and is
  //  limited by the defined variable VTK_POLYGON_MAX_ITER.
  //
  //  The variable deltaVotes keeps track of the number of votes for
  //  "in" versus "out" of the face.  When delta_vote > 0, more votes
  //  have counted for "in" than "out".  When delta_vote < 0, more votes
  //  have counted for "out" than "in".  When the delta_vote exceeds or
  //  equals the defined variable VTK_POLYGON_VOTE_THRESHOLD, than the
  //  appropriate "in" or "out" status is returned.
  //
  for (deltaVotes = 0, iterNumber = 1;
       (iterNumber < VTK_POLYGON_MAX_ITER)
       && (std::abs(deltaVotes) < VTK_POLYGON_VOTE_THRESHOLD);
       iterNumber++)
  {
    //
    //  Generate ray
    //
    bool rayOK;
    for (rayOK = false; rayOK == false; )
    {
      ray[comps[0]] = ZRandomInstance.randDouble(rayMag, -rayMag);
      ray[comps[1]] = ZRandomInstance.randDouble(rayMag, -rayMag);
      ray[maxComp] = -(normal[comps[0]]*ray[comps[0]] +
                       normal[comps[1]]*ray[comps[1]]) / normal[maxComp];
      if ( (mag = glm::length(ray)) > rayMag*VTK_TOL )
      {
        rayOK = true;
      }
    }

    //  The ray must be appropriately sized.
    //
    for (int i=0; i<3; i++)
    {
      xray[i] = x[i] + (rayMag/mag)*ray[i];
    }

    //  The ray may now be fired against all the edges
    //
    size_t i;
    for (numInts=0, testResult=VTK_POLYGON_CERTAIN, i=0; i<pts.size(); i++)
    {
      glm::dvec3 x1 = pts[i];
      glm::dvec3 x2 = pts[(i+1)%pts.size()];

      //   Fire the ray and compute the number of intersections.  Be careful
      //   of degenerate cases (e.g., ray intersects at vertex).
      //
      if ((status=IntersectionLine(x,xray,x1,x2,u,v)) == VTK_POLYGON_INTERSECTION)
      {
        if ( (VTK_POLYGON_RAY_TOL < v) && (v < 1.0-VTK_POLYGON_RAY_TOL) )
        {
          numInts++;
        }
        else
        {
          testResult = VTK_POLYGON_UNCERTAIN;
        }
      }
      else if ( status == VTK_POLYGON_ON_LINE )
      {
        testResult = VTK_POLYGON_UNCERTAIN;
      }
    }
    if ( testResult == VTK_POLYGON_CERTAIN )
    {
      if ( (numInts % 2) == 0)
      {
        --deltaVotes;
      }
      else
      {
        ++deltaVotes;
      }
    }
  } //try another ray

  //   If the number of intersections is odd, the point is in the polygon.
  //
  if ( deltaVotes < 0 )
  {
    return VTK_POLYGON_OUTSIDE;
  }
  else
  {
    return VTK_POLYGON_INSIDE;
  }
}

//----------------------------------------------------------------------------
// returns != 0 if vertex can be removed. Uses half-space
// comparison to determine whether ear-cut is valid, and may
// resort to line-plane intersections to resolve possible
// instersections with ear-cut.
int CanRemovePolygonVertex(size_t idx, double tolerance, glm::dvec3 normal, const std::vector<int64_t> &poly, const std::vector<glm::dvec3> &vertices)
{
  int sign, currentSign;
  double val, s, t;

  // Check for simple case
  if ( poly.size() <= 3 )
  {
    return 1;
  }

  // Compute split plane, the point to be cut off
  // is always on the positive side of the plane.
  size_t prevIdx = idx - 1;
  size_t nextIdx = idx + 1;
  if (idx == 0)
    prevIdx = poly.size() - 1;
  if (nextIdx >= poly.size())
    nextIdx = 0;
  size_t nextNextIdx = nextIdx+1;
  if (nextNextIdx >= poly.size())
    nextNextIdx = 0;
  int64_t previous = poly[prevIdx];
  int64_t next = poly[nextIdx];

  glm::dvec3 sPt = vertices[previous]; //point on plane
  glm::dvec3 v = vertices[next] - vertices[previous];
  glm::dvec3 sN = glm::cross(v, normal);

  if ( (glm::dot(sN, sN)) == 0.0 )
  {
    return 0; //bad split, indeterminant
  }

  // Traverse the other points to see if a) they are all on the
  // other side of the plane; and if not b) whether they intersect
  // the split line.
  int oneNegative=0;
  val = glm::dot(sN, vertices[poly[nextNextIdx]] - sPt);
  //val = vtkPlane::Evaluate(sN,sPt,next->next->x);
  currentSign = (val > tolerance ? 1 : (val < -tolerance ? -1 : 0));
  oneNegative = (currentSign < 0 ? 1 : 0); //very important

  // Intersections are only computed when the split half-space is crossed
  for (size_t vtxIdx=nextNextIdx+1; vtxIdx != static_cast<size_t>(previous); vtxIdx++)
  {
    if (vtxIdx >= poly.size())
      vtxIdx = 0;
    val = glm::dot(sN, vertices[poly[vtxIdx]] - sPt);
    //val = vtkPlane::Evaluate(sN,sPt,vtxIdx->x);
    sign = (val > tolerance ? 1 : (val < -tolerance ? -1 : 0));
    if ( sign != currentSign )
    {
      if ( !oneNegative )
      {
        oneNegative = (sign < 0 ? 1 : 0); //very important
      }
      size_t vtxIdxPrevious = vtxIdx - 1;
      if (vtxIdx == 0)
        vtxIdxPrevious = poly.size() - 1;
      if (IntersectionLine(sPt,vertices[next],vertices[poly[vtxIdx]],vertices[poly[vtxIdxPrevious]],s,t) != 0 )
      {
        return 0;
      }
      else
      {
        currentSign = sign;
      }
    }//if crossing occurs
  }//for the rest of the loop

  if ( !oneNegative )
  {
    return 0; //entire loop is on this side of plane
  }
  else
  {
    return 1;
  }
}

#define VTK_POLYGON_TOLERANCE 1.0e-06

class MeasureIDComparison
{
public:
  MeasureIDComparison() {}
  bool operator() (const std::pair<double, size_t>& lhs, const std::pair<double, size_t>& rhs) const
  {
    return lhs.first < rhs.first;
  }
};

// ---------------------------------------------------
// Simple utility method for computing polygon bounds.
// Returns the sum of the squares of the dimensions.
// Requires a poly with at least one  point.
// Compute polygon bounds.  Poly must have at least one point.
double vtkCCSPolygonBounds(
    const std::vector<int64_t> &poly, const std::vector<glm::dvec3> &vertices, double bounds[6])
{
  size_t n = poly.size();
  glm::dvec3 p = vertices[poly[0]];

  bounds[0] = bounds[1] = p[0];
  bounds[2] = bounds[3] = p[1];
  bounds[4] = bounds[5] = p[2];

  for (size_t j = 1; j < n; j++)
  {
    glm::dvec3 p = vertices[poly[j]];
    if (p[0] < bounds[0]) { bounds[0] = p[0]; };
    if (p[0] > bounds[1]) { bounds[1] = p[0]; };
    if (p[1] < bounds[2]) { bounds[2] = p[1]; };
    if (p[1] > bounds[3]) { bounds[3] = p[1]; };
    if (p[2] < bounds[4]) { bounds[4] = p[2]; };
    if (p[2] > bounds[5]) { bounds[5] = p[2]; };
  }

  double bx = (bounds[1] - bounds[0]);
  double by = (bounds[3] - bounds[2]);
  double bz = (bounds[5] - bounds[4]);

  return (bx*bx + by*by + bz*bz);
}

//----------------------------------------------------------------------------
// Triangulation method based on ear-cutting. Triangles, or ears, are
// cut off from the polygon based on the angle of the vertex. Small
// angles (narrow triangles) are cut off first. This implementation uses
// a priority queue to cut off ears with smallest angles. Also, the
// algorithm works in 3D (the points don't have to be projected into
// 2D, and the ordering direction of the points is nor important as
// long as the polygon edges do not self intersect).
bool TriangulatePolygon2(const std::vector<int64_t> &poly, const std::vector<glm::dvec3> &vertices, std::vector<size_t> &triangles)
{
  double bounds[6];
  double d = std::sqrt(vtkCCSPolygonBounds(poly, vertices, bounds));
  double tolerance = VTK_POLYGON_TOLERANCE * d;

  glm::dvec3 normal = ComputePolygonNormal(poly, vertices);
  if (glm::dot(normal, normal) == 0.0) {
    LERROR() << "Degenerate polygon encountered during triangulation in TriangulatePolygon2 1";
    return false;
  }

  // Now compute the angles between edges incident to each
  // vertex. Place the structure into a priority queue (those
  // vertices with smallest angle are to be removed first).
  //
  std::priority_queue<std::pair<double, size_t>, std::vector<std::pair<double, size_t> >, MeasureIDComparison> VertexQueue;
  for (size_t i=0; i < poly.size(); i++)
  {
    //concave (negative measure) vertices are not elgible for removal
    double measure = ComputePolygonTriangleMeasure(i, normal, poly, vertices);
    if ( measure > 0.0)
    {
      VertexQueue.push(std::make_pair(measure, i));
    }
  }

  // For each vertex in priority queue, and as long as there
  // are three or more vertices, remove the vertex (if possible)
  // and create a new triangle. If the number of vertices in the
  // queue is equal to the number of vertices, then the polygon
  // is convex and triangle removal can proceed without intersection
  // checks.
  //
  size_t numInQueue;
  std::vector<int64_t> polyCopy = poly;
  while ( polyCopy.size() > 2 &&
          (numInQueue=VertexQueue.size()) > 0)
  {
    if ( numInQueue == polyCopy.size() ) //convex, pop away
    {
      size_t idx = VertexQueue.top().second;
      VertexQueue.pop();
      size_t nextIdx = idx+1;
      if (nextIdx >= polyCopy.size())
        nextIdx = 0;
      size_t prevIdx = idx-1;
      if (idx == 0)
        prevIdx = polyCopy.size() - 1;
      triangles.push_back(polyCopy[idx]);
      triangles.push_back(polyCopy[nextIdx]);
      triangles.push_back(polyCopy[prevIdx]);
      polyCopy.erase(polyCopy.begin() + idx);
      // rebuild VertexQueue
      while(!VertexQueue.empty())
        VertexQueue.pop();
      for (size_t i=0; i < polyCopy.size(); i++)
      {
        //concave (negative measure) vertices are not elgible for removal
        double measure = ComputePolygonTriangleMeasure(i, normal, polyCopy, vertices);
        if ( measure > 0.0)
        {
          VertexQueue.push(std::make_pair(measure, i));
        }
      }
    }//convex
    else
    {
      size_t idx = VertexQueue.top().second;
      VertexQueue.pop(); //removes it, even if can't be split
      if (CanRemovePolygonVertex(idx, tolerance, normal, polyCopy, vertices))
      {
        size_t nextIdx = idx+1;
        if (nextIdx >= polyCopy.size())
          nextIdx = 0;
        size_t prevIdx = idx-1;
        if (idx == 0)
          prevIdx = polyCopy.size() - 1;
        triangles.push_back(polyCopy[idx]);
        triangles.push_back(polyCopy[nextIdx]);
        triangles.push_back(polyCopy[prevIdx]);
        polyCopy.erase(polyCopy.begin() + idx);
        // rebuild VertexQueue
        while(!VertexQueue.empty())
          VertexQueue.pop();
        for (size_t i=0; i < polyCopy.size(); i++)
        {
          //concave (negative measure) vertices are not elgible for removal
          double measure = ComputePolygonTriangleMeasure(i, normal, polyCopy, vertices);
          if ( measure > 0.0)
          {
            VertexQueue.push(std::make_pair(measure, i));
          }
        }
      }
    }//concave
  }//while

  if ( polyCopy.size() > 2 ) //couldn't triangulate
  {
    LERROR() << "Degenerate polygon encountered during triangulation in TriangulatePolygon2 2";
    return false;
  }
  return true;
}

// ---------------------------------------------------
// Triangulate a polygon that has been simplified by FindTrueEdges.
// This will re-insert the original edges.  The output triangles are
// appended to "polys" and, for each stored triangle, "color" will
// be added to "scalars".  The final two arguments (polygon and
// triangles) are only for temporary storage.
// The return value is true if triangulation was successful.
bool vtkCCSTriangulate(
    const std::vector<int64_t> &poly, const std::vector<glm::dvec3> &vertices,
    const std::vector<int64_t> &polyEdges, std::vector<std::vector<int64_t> > &originalEdges,
    std::vector<glm::i64vec3> &triangles)
{
  int triangulationFailure = false;
  size_t n = poly.size();

  // If the poly is a line, then skip it
  if (n < 3)
  {
    return true;
  }
  // If the poly is a triangle, then pass it
  else if (n == 3)
  {
    size_t trids[3];
    trids[0] = 0;
    trids[1] = 1;
    trids[2] = 2;

    vtkCCSInsertTriangle(triangles, poly, trids, polyEdges, originalEdges);
  }
  // If the poly has 4 or more points, triangulate it
  else
  {
    std::vector<size_t> tris;
    if (!TriangulatePolygon2(poly, vertices, tris))
    {
      triangulationFailure = true;
    }

    for (size_t k = 0; k < tris.size(); k += 3)
    {
      size_t trids[3];
      for (size_t i=0; i<3; i++) {
        size_t id = tris[k+i];
        for (size_t m=0; m<poly.size(); m++) {
          if (static_cast<size_t>(poly[m]) == id) {
            trids[i] = m;
            break;
          }
        }
      }

      vtkCCSInsertTriangle(triangles, poly, trids, polyEdges, originalEdges);
    }
  }

  return !triangulationFailure;
}

// ---------------------------------------------------
// Here is the code for creating polygons from line segments.
// Take a set of lines, join them tip-to-tail to create polygons
void vtkCCSMakePolysFromLines(
    std::vector<glm::i64vec2> &lines,
    std::vector<std::vector<int64_t> > &newPolys,
    std::vector<size_t> &incompletePolys)
{
  // Bitfield for marking lines as used
  vtkCCSBitArray usedLines;

  size_t numNewPolys = 0;
  int64_t remainingLines = lines.size();

  while (remainingLines > 0)
  {
    // Create a new poly
    size_t polyId = numNewPolys++;
    newPolys.push_back(std::vector<int64_t>());
    std::vector<int64_t> &poly = newPolys[polyId];

    size_t lineId = 0;
    bool completePoly = false;

    // start the poly
    for (lineId = 0; lineId < lines.size(); lineId++)
    {
      if (!usedLines.get(lineId))
      {
        poly.push_back(lines[lineId].x);
        poly.push_back(lines[lineId].y);
        break;
      }
    }

    usedLines.set(lineId, 1);
    remainingLines--;

    bool noLinesMatch = false;

    while (!completePoly && !noLinesMatch && remainingLines > 0)
    {
      // This is cleared if a match is found
      noLinesMatch = true;



      glm::i64vec2 endPts, reverseEndPts;

      // For both open ends of the polygon
      for (int endIdx = 0; endIdx < 2; endIdx++)
      {
        // Number of points in the poly
        size_t npoly = poly.size();
        endPts[0] = poly[npoly-1];
        endPts[1] = poly[0];
        reverseEndPts.x = endPts.y;
        reverseEndPts.y = endPts.x;

        std::vector<int64_t> matches;

        // Go through all lines that contain this endpoint
        for (size_t lineIdx = 0; lineIdx < lines.size(); lineIdx++) {
          if (!usedLines.get(lineIdx)) {
            if (lines[lineIdx].x == endPts[endIdx] || lines[lineIdx].y == endPts[endIdx]) {
              matches.push_back(lineIdx);
            }
          }
        }

        if (matches.size() > 0)
        {
          // Multiple matches mean we need to decide which path to take
          if (matches.size() > 1)
          {
            // Remove double-backs
            size_t k = matches.size();
            do
            {
              lineId = matches[--k];
              if ((endIdx == 0 && (poly[npoly-2] == lines[lineId][1] || poly[npoly-2] == lines[lineId][0])) ||
                  (endIdx == 1 && (poly[1] == lines[lineId][0] || poly[1] == lines[lineId][1])))
              {
                matches.erase(matches.begin()+k);
              }
            }
            while (k > 0 && matches.size() > 1);

            // If there are multiple matches due to intersections,
            // they should be dealt with here.
          }

          lineId = matches[0];

          // Do both ends match?
          if (lines[lineId] == endPts || lines[lineId] == reverseEndPts)
          {
            completePoly = true;
          }

          if (!completePoly) {
            if (endIdx == 0)
            {
              if (lines[lineId].x == endPts[endIdx])
                poly.insert(poly.end(), lines[lineId].y);
              else
                poly.insert(poly.end(), lines[lineId].x);
            }
            else
            {
              if (lines[lineId].x == endPts[endIdx])
                poly.insert(poly.begin(), lines[lineId].y);
              else
                poly.insert(poly.begin(), lines[lineId].x);
            }
          }

          usedLines.set(lineId, 1);
          remainingLines--;
          noLinesMatch = false;
        }
      }
    }

    // Check for incomplete polygons
    if (noLinesMatch)
    {
      incompletePolys.push_back(polyId);
    }
  }
}

// ---------------------------------------------------
// Join polys that have loose ends, as indicated by incompletePolys.
// Any polys created will have a normal opposite to the supplied normal,
// and any new edges that are created will be on the hull of the point set.
// Shorter edges will be preferred over long edges.
// Finish any incomplete polygons by trying to join loose ends
void vtkCCSJoinLooseEnds(
    std::vector<std::vector<int64_t> > &polys, std::vector<size_t> &incompletePolys,
    std::vector<glm::dvec3> &vertices, const glm::dvec3 normal)
{
  // Relative tolerance for checking whether an edge is on the hull
  const double tol = VTK_CCS_POLYGON_TOLERANCE;

  // A list of polys to remove when everything is done
  std::vector<size_t> removePolys;

  size_t n;
  while ( (n = incompletePolys.size()) )
  {
    vtkCCSPoly &poly1 = polys[incompletePolys[n-1]];
    int64_t pt1 = poly1[poly1.size()-1];
    glm::dvec3 p2;
    glm::dvec3 p1 = vertices[pt1];

    double dMin = std::numeric_limits<double>::max();
    size_t iMin = 0;

    for (size_t i = 0; i < n; i++)
    {
      std::vector<int64_t> &poly2 = polys[incompletePolys[i]];
      int64_t pt2 = poly2[0];
      p2 = vertices[pt2];

      // The next few steps verify that edge [p1, p2] is on the hull
      glm::dvec3 v = p2 - p1;
      double d = glm::length(v);
      v[0] /= d; v[1] /= d; v[2] /= d;

      // Compute the midpoint of the edge
      glm::dvec3 pm = glm::mix(p1, p2, 0.5);

      // Create a plane equation
      double pc[4];
      glm::dvec3 glmpc = glm::cross(v, normal);
      //vtkMath::Cross(v, normal, pc);
      pc[0] = glmpc[0];
      pc[1] = glmpc[1];
      pc[2] = glmpc[2];
      pc[3] = -glm::dot(glmpc, pm);

      // Check that all points are inside the plane.  If they aren't, then
      // the edge is not on the hull of the pointset.
      int badPoint = 0;
      size_t m = polys.size();
      for (size_t j = 0; j < m && !badPoint; j++)
      {
        vtkCCSPoly &poly = polys[j];
        size_t npts = poly.size();
        for (size_t k = 0; k < npts; k++)
        {
          int64_t ptId = poly[k];
          if (ptId != pt1 && ptId != pt2)
          {
            glm::dvec3 p = vertices[ptId];
            double val = p[0]*pc[0] + p[1]*pc[1] + p[2]*pc[2] + pc[3];
            double r2 = glm::dot(p - pm, p - pm);

            // Check distance from plane against the tolerance
            if (val < 0 && val*val > tol*tol*r2)
            {
              badPoint = 1;
              break;
            }
          }
        }

        // If no bad points, then this edge is a candidate
        if (!badPoint && d < dMin)
        {
          dMin = d;
          iMin = i;
        }
      }
    }

    // If a match was found, append the polys
    if (dMin < std::numeric_limits<double>::max())
    {
      // Did the poly match with itself?
      if (iMin == n-1)
      {
        // Mark the poly as closed
        incompletePolys.pop_back();
      }
      else
      {
        size_t id2 = incompletePolys[iMin];

        // Combine the polys
        poly1.insert(poly1.end(), polys[id2].begin(), polys[id2].end());

        // Erase the second poly
        removePolys.push_back(id2);
        incompletePolys.erase(incompletePolys.begin() + iMin);
      }
    }
    else
    {
      // If no match, erase this poly from consideration
      removePolys.push_back(incompletePolys[n-1]);
      incompletePolys.pop_back();
    }
  }

  // Remove polys that couldn't be completed
  std::sort(removePolys.begin(), removePolys.end());
  size_t i = removePolys.size();
  while (i > 0)
  {
    // Remove items in reverse order
    polys.erase(polys.begin() + removePolys[--i]);
  }

  // Clear the incompletePolys vector, it's indices are no longer valid
  incompletePolys.clear();
}

// ---------------------------------------------------
// Given three vectors p->p1, p->p2, and p->p3, this routine
// checks to see if progressing from p1 to p2 to p3 is a clockwise
// or counterclockwise progression with respect to the normal.
// The return value is -1 for clockwise, +1 for counterclockwise,
// and 0 if any two of the vectors are coincident.
int vtkCCSVectorProgression(
    const glm::dvec3 p, const glm::dvec3 p1,
    const glm::dvec3 p2, const glm::dvec3 p3, const glm::dvec3 normal)
{
  glm::dvec3 v1, v2, v3;
  v1 = p1 - p;
  v2 = p2 - p;
  v3 = p3 - p;

  glm::dvec3 w1 = glm::cross(v2, v1);
  glm::dvec3 w2 = glm::cross(v2, v3);
  double s1 = glm::dot(w1, normal);
  double s2 = glm::dot(w2, normal);

  if (s1 != 0 && s2 != 0)
  {
    int sb1 = (s1 < 0);
    int sb2 = (s2 < 0);

    // if sines have different signs
    if ( (sb1 ^ sb2) )
    {
      // return -1 if s2 is -ve
      return (1 - 2*sb2);
    }

    double c1 = glm::dot(v2, v1);
    double l1 = glm::length(v1);
    double c2 = glm::dot(v2, v3);
    double l2 = glm::length(v3);

    // ck is the difference of the cosines, flipped in sign if sines are +ve
    double ck = (c2*l2 - c1*l1)*(1 - sb1*2);

    if (ck != 0)
    {
      // return the sign of ck
      return (1 - 2*(ck < 0));
    }
  }

  return 0;
}

// ---------------------------------------------------
// Check for self-intersection. Split the figure-eights.
// This assumes that all intersections occur at existing
// vertices, i.e. no new vertices will be created. Returns
// the number of splits made.
// Check for polygons that contain multiple loops, and split the loops apart.
// Returns the number of splits made.
int vtkCCSSplitAtPinchPoints(
    std::vector<std::vector<int64_t> > &polys, std::vector<glm::dvec3> &vertices,
    std::vector<std::vector<size_t> > &polyGroups,
    std::vector<std::vector<int64_t> > &polyEdges,
    const glm::dvec3 normal)
{
  int splitCount = 0;

  for (size_t i = 0; i < polys.size(); i++)
  {
    std::vector<int64_t> &poly = polys[i];
    size_t n = poly.size();

    double bounds[6];
    double tol = VTK_CCS_POLYGON_TOLERANCE;
    tol *= std::sqrt(vtkCCSPolygonBounds(poly, vertices, bounds));

    if (tol == 0)
    {
      continue;
    }

    double tol2 = tol*tol;
    std::vector<size_t> locator;

    int foundMatch = 0;
    size_t idx1 = 0;
    size_t idx2 = 0;
    int unique = 0;

    for (idx2 = 0; idx2 < n; idx2++)
    {
      int64_t firstId = poly[idx2];
      glm::dvec3 point = vertices[firstId];

      //
      double minsqdist = std::numeric_limits<double>::max();
      int64_t vertIdx = 0;
      for (size_t locatorIdx = 0; locatorIdx < locator.size(); ++locatorIdx) {
        double sqdist = glm::dot(vertices[locator[locatorIdx]] - point,
                                 vertices[locator[locatorIdx]] - point);
        if (sqdist < minsqdist) {
          minsqdist = sqdist;
          vertIdx = locator[locatorIdx];
        }
      }
      if (minsqdist <= tol2)
      {
        // Need vertIdx to match poly indices, so force point insertion
        locator.push_back(firstId);

        // Do the points have different pointIds?
        for (size_t polyIdx = 0; polyIdx < poly.size(); ++polyIdx) {
          if (poly[polyIdx] == vertIdx) {
            idx1 = polyIdx;
            break;
          }
        }
        unique = (poly[idx2] != poly[idx1]);

        if ((idx2 > idx1 + 2 - unique) && (n + idx1 > idx2 + 2 - unique))
        {
          // Make sure that splitting this poly won't create a hole poly
          size_t prevIdx = n + idx1 - 1;
          size_t midIdx = idx1 + 1;
          size_t nextIdx = idx2 + 1;
          if (prevIdx >= n) { prevIdx -= n; }
          if (midIdx >= n) { midIdx -= n; }
          if (nextIdx >= n) { nextIdx -= n; }

          glm::dvec3 p1 = vertices[poly[prevIdx]];
          glm::dvec3 p2 = vertices[poly[midIdx]];
          glm::dvec3 p3 = vertices[poly[nextIdx]];

          if (vtkCCSVectorProgression(point, p1, p2, p3, normal) < 0)
          {
            foundMatch = 1;
            break;
          }
        }
      } else {
        locator.push_back(firstId);
      }
    }

    if (foundMatch)
    {
      splitCount++;

      // Split off a new poly
      size_t m = idx2 - idx1;

      std::vector<int64_t> &oldPoly = polys[i];
      std::vector<int64_t> &oldEdges = polyEdges[i];
      std::vector<int64_t> newPoly1(m + unique);
      std::vector<int64_t> newEdges1(m + unique);
      std::vector<int64_t> newPoly2(n - m + unique);
      std::vector<int64_t> newEdges2(n - m + unique);

      // The current poly, which is now intersection-free
      for (size_t l = 0; l < m+unique; l++)
      {
        newPoly1[l] = oldPoly[l+idx1];
        newEdges1[l] = oldEdges[l+idx1];
      }
      if (unique)
      {
        newEdges1[m] = -1;
      }

      // The poly that is split off, which might have more intersections
      for (size_t j = 0; j < idx1+unique; j++)
      {
        newPoly2[j] = oldPoly[j];
        newEdges2[j] = oldEdges[j];
      }
      if (unique)
      {
        newEdges2[idx1] = -1;
      }
      for (size_t k = idx2; k < n; k++)
      {
        newPoly2[k - m + unique] = oldPoly[k];
        newEdges2[k - m + unique] = oldEdges[k];
      }

      polys[i] = newPoly1;
      polyEdges[i] = newEdges1;
      polys.push_back(newPoly2);
      polyEdges.push_back(newEdges2);

      // Unless polygroup was clear (because poly was reversed),
      // make a group with one entry for the new poly
      polyGroups.resize(polys.size());
      if (polyGroups[i].size())
      {
        polyGroups[polys.size()-1].push_back(polys.size()-1);
      }
    }
  }

  return splitCount;
}

// ---------------------------------------------------
// The polygons might have a lot of extra points, i.e. points
// in the middle of the edges.  Remove those points, but keep
// the original edges as polylines in the originalEdges array.
// Only original edges with more than two points will be kept.
// Remove points that are not vertices of the polygon,
// i.e. remove any points that are on an edge but not at a corner.
// This simplifies all remaining steps and improves the triangulation.
// The original edges are appended to the originalEdges cell array,
// where each cell in this array will be a polyline consisting of two
// corner vertices and all the points in between.
void vtkCCSFindTrueEdges(
    std::vector<std::vector<int64_t> > &polys, std::vector<glm::dvec3> &vertices,
    std::vector<std::vector<int64_t> > &polyEdges, std::vector<std::vector<int64_t> > &originalEdges)
{
  // Tolerance^2 for angle to see if line segments are parallel
  const double atol2 = (VTK_CCS_POLYGON_TOLERANCE*VTK_CCS_POLYGON_TOLERANCE);

  for (size_t polyId = 0; polyId < polys.size(); polyId++)
  {
    std::vector<int64_t> &oldPoly = polys[polyId];
    size_t n = oldPoly.size();
    polyEdges.push_back(std::vector<int64_t>());

    // Only useful if poly has more than three sides
    if (n < 4)
    {
      polyEdges[polyId].resize(3);
      polyEdges[polyId][0] = -1;
      polyEdges[polyId][1] = -1;
      polyEdges[polyId][2] = -1;
      continue;
    }

    // While we remove points, m keeps track of how many points are left
    size_t m = n;

    // Compute bounds for tolerance
    double bounds[6];
    double tol2 = vtkCCSPolygonBounds(oldPoly, vertices, bounds)*atol2;

    // The new poly
    std::vector<int64_t> newPoly;
    std::vector<int64_t> &newEdges = polyEdges[polyId];
    int64_t cornerPointId = 0;
    int64_t oldOriginalId = -1;

    // Allocate space
    newPoly.reserve(n);
    newEdges.reserve(n);

    // Keep the partial edge from before the first corner is found
    std::vector<int64_t> partialEdge;
    int cellCount = 0;

    glm::dvec3 p0 = vertices[oldPoly[n-1]];
    glm::dvec3 p1 = vertices[oldPoly[0]];
    glm::dvec3 v1 = p1 - p0;
    double l1 = glm::dot(v1, v1);
    double l2;
    glm::dvec3 p2, v2;

    for (size_t j = 0; j < n; j++)
    {
      size_t k = j+1;
      if (k >= n) { k -= n; }

      p2 = vertices[oldPoly[k]];
      v2 = p2 - p1;
      l2 = glm::dot(v2, v2);

      // Dot product is |v1||v2|cos(theta)
      double c = glm::dot(v1, v2);
      // sin^2(theta) = (1 - cos^2(theta))
      // and   c*c = l1*l2*cos^2(theta)
      double s2 = (l1*l2 - c*c);

      // In the small angle approximation, sin(theta) == theta, so
      // s2/(l1*l2) is the angle that we want to check, but it's not
      // a valid check if l1 or l2 is very close to zero.

      int64_t pointId = oldPoly[j];

      // Keep the point if:
      // 1) removing it would create a 2-point poly OR
      // 2) it's more than "tol" distance from the prev point AND
      // 3) the angle is greater than atol:
      if (m <= 3 ||
          (l1 > tol2 &&
           (c < 0 || l1 < tol2 || l2 < tol2 || s2 > l1*l2*atol2)))
      {
        // Complete the previous edge only if the final point count
        // will be greater than two
        if (cellCount > 1)
        {
          if (pointId != oldOriginalId)
          {
            originalEdges[originalEdges.size()-1].push_back(pointId);
            cellCount++;
          }
          newEdges.push_back(originalEdges.size()-1);
        }
        else if (cellCount == 0)
        {
          partialEdge.push_back(pointId);
        }
        else
        {
          newEdges.push_back(-1);
        }

        newPoly.push_back(pointId);

        // Start a new edge with cornerPointId as a "virtual" point
        cornerPointId = pointId;
        oldOriginalId = pointId;
        cellCount = 1;

        // Rotate to the next point
        p0 = p2;
        p1 = p2;
        v1 = v2;
        l1 = l2;
      }
      else
      {
        if (cellCount > 0 && pointId != oldOriginalId)
        {
          // First check to see if we have to add cornerPointId
          if (cellCount == 1)
          {
            originalEdges.push_back(std::vector<int64_t>());
            originalEdges[originalEdges.size()-1].push_back(cornerPointId);
          }
          // Then add the new point
          originalEdges[originalEdges.size()-1].push_back(pointId);
          oldOriginalId = pointId;
          cellCount++;
        }
        else
        {
          // No corner yet, so save the point
          partialEdge.push_back(pointId);
        }

        // Reduce the count
        m--;

        // Join the previous two segments, since the point was removed
        p1 = p2;
        v1 = p2 - p0;
        l1 = glm::dot(v1, v1);
      }
    }

    // Add the partial edge to the end
    for (size_t ii = 0; ii < partialEdge.size(); ii++)
    {
      int64_t pointId = partialEdge[ii];
      if (pointId != oldOriginalId)
      {
        if (cellCount == 1)
        {
          originalEdges.push_back(std::vector<int64_t>());
          originalEdges[originalEdges.size()-1].push_back(cornerPointId);
        }
        originalEdges[originalEdges.size()-1].push_back(pointId);
        oldOriginalId = pointId;
        cellCount++;
      }
    }

    // Finalize
    if (cellCount > 1)
    {
      newEdges.push_back(originalEdges.size()-1);
    }

    polys[polyId] = newPoly;
  }
}



// ---------------------------------------------------
// Check the sense of the polygon against the given normal.  Returns
// zero if the normal is zero.
// Set sense to 1 if the poly's normal matches the specified normal, and
// zero otherwise. Returns zero if poly is degenerate.
int vtkCCSCheckPolygonSense(
    std::vector<int64_t> &poly, std::vector<glm::dvec3> &vertices, const glm::dvec3 normal,
    int &sense)
{
  // Compute the normal
  glm::dvec3 pnormal, p0, p1, p2, v1, v2, v;
  pnormal[0] = 0.0; pnormal[1] = 0.0; pnormal[2] = 0.0;

  p0 = vertices[poly[0]];
  p1 = vertices[poly[1]];
  v1 = p1 - p0;

  size_t n = poly.size();
  for (size_t jj = 2; jj < n; jj++)
  {
    p2 = vertices[poly[jj]];
    v2 = p2 - p0;
    v = glm::cross(v1, v2);
    pnormal += v;
    p1 = p2;
    v1 = v2;
  }

  // Check the normal
  double d = glm::dot(pnormal, normal);

  sense = (d > 0);

  return (d != 0);
}

// ---------------------------------------------------
// Check whether innerPoly is inside outerPoly.
// The normal is needed to verify the polygon orientation.
// The values of pp, bounds, and tol2 must be precomputed
// by calling vtkCCSPrepareForPolyInPoly() on outerPoly.

int vtkCCSPolyInPoly(
    const std::vector<int64_t> &outerPoly, const std::vector<int64_t> &innerPoly,
    std::vector<glm::dvec3> &vertices, const glm::dvec3 normal,
    const std::vector<glm::dvec3> &pp, const double bounds[6],
    double tol2)
{
  // Find a vertex of poly "j" that isn't on the edge of poly "i".
  // This is necessary or the PointInPolygon might return "true"
  // based only on roundoff error.

  size_t n = outerPoly.size();
  size_t m = innerPoly.size();

  for (size_t jj = 0; jj < m; jj++)
  {
    // Semi-randomize the point order
    size_t kk = (jj>>1) + (jj&1)*((m+1)>>1);
    glm::dvec3 p = vertices[innerPoly[kk]];

    if (PointInPolygon(p, pp, bounds, normal))
    {
      int pointOnEdge = 0;
      glm::dvec3 q1 = vertices[outerPoly[n-1]];
      glm::dvec3 q2;

      for (size_t ii = 0; ii < n; ii++)
      {
        q2 = vertices[outerPoly[ii]];
        double t;
        glm::dvec3 dummy;
        // This method returns distance squared
        if (Z3DUtils::vertexLineSegmentSquaredDistance(p, q1, q2, t, dummy) < tol2)
        {
          pointOnEdge = 1;
          break;
        }
        q1 = q2;
      }

      if (!pointOnEdge)
      {
        // Good result, point is in polygon
        return 1;
      }
    }
  }

  // No matches found
  return 0;
}

// ---------------------------------------------------
// Precompute values needed for the PolyInPoly check.
// The values that are returned are as follows:
// pp: an array of the polygon vertices
// bounds: the polygon bounds
// tol2: a tolerance value based on the size of the polygon
// (note: pp must be pre-allocated to the 3*outerPoly.size())

void vtkCCSPrepareForPolyInPoly(
    const std::vector<int64_t> &outerPoly, std::vector<glm::dvec3> &vertices,
    std::vector<glm::dvec3> &pp, double bounds[6], double &tol2)
{
  size_t n = outerPoly.size();

  if (n == 0)
  {
    tol2=0.0; // to avoid false positive warning about uninitialized value.
    return;
  }

  // Pull out the points
  for (size_t k = 0; k < n; k++)
  {
    //double *p = &pp[3*k];
    //vertices->GetPoint(outerPoly[k], p);
    pp.push_back(vertices[outerPoly[k]]);
  }

  // Find the bounding box and tolerance for the polygon
  tol2 = (vtkCCSPolygonBounds(outerPoly, vertices, bounds)*
          (VTK_CCS_POLYGON_TOLERANCE * VTK_CCS_POLYGON_TOLERANCE));
}

// ---------------------------------------------------
// Check for polygons within polygons.  Group the polygons
// if they are within each other.  Reverse the sense of
// the interior "hole" polygons.  A hole within a hole
// will be reversed twice and will become its own group.
// Check for polys within other polys, i.e. find polys that are holes and
// add them to the "polyGroup" of the poly that they are inside of.
void vtkCCSMakeHoleyPolys(
    std::vector<std::vector<int64_t> > &newPolys, std::vector<glm::dvec3> &vertices,
    std::vector<std::vector<size_t> > &polyGroups,
    const glm::dvec3 normal)
{
  size_t numNewPolys = newPolys.size();
  if (numNewPolys <= 1)
  {
    return;
  }

  // Use bit arrays to keep track of inner polys
  vtkCCSBitArray polyReversed;
  vtkCCSBitArray innerPolys;

  // Find the maximum poly size
  size_t nmax = 1;
  for (size_t kk = 0; kk < numNewPolys; kk++)
  {
    size_t n = newPolys[kk].size();
    if (n > nmax) { nmax = n; }
  }

  // These are some values needed for poly-in-poly checks
  std::vector<glm::dvec3> pp;
  double bounds[6];
  double tol2;

  // Go through all polys
  for (size_t i = 0; i < numNewPolys; i++)
  {
    size_t n = newPolys[i].size();

    if (n < 3) { continue; }

    // Check if poly is reversed
    int sense = 0;
    if (vtkCCSCheckPolygonSense(newPolys[i], vertices, normal, sense))
    {
      polyReversed.set(i, sense);
    }

    // Precompute some values needed for poly-in-poly checks
    vtkCCSPrepareForPolyInPoly(newPolys[i], vertices, pp, bounds, tol2);

    // Look for polygons inside of this one
    for (size_t j = 0; j < numNewPolys; j++)
    {
      size_t m = newPolys[j].size();
      if (j == i || m < 3) { continue; }

      // Make sure polygon i is not in polygon j
      int isInteriorPoly = 0;
      for (size_t k = 1; k < polyGroups[j].size(); k++)
      {
        if (polyGroups[j][k] == i)
        {
          isInteriorPoly = 1;
          break;
        }
      }

      if (isInteriorPoly)
      {
        continue;
      }

      if (vtkCCSPolyInPoly(newPolys[i], newPolys[j], vertices,
                           normal, pp, bounds, tol2))
      {
        // Add to group
        polyGroups[i].push_back(j);
      }
    }
  }

  for (size_t j = 0; j < numNewPolys; j++)
  {
    // Remove the groups for reversed polys
    if (polyReversed.get(j))
    {
      polyGroups[j].clear();
    }
    // Polys inside the interior polys have their own groups, so remove
    // them from this group
    else if (polyGroups[j].size() > 1)
    {
      // Convert the group into a bit array, to make manipulation easier
      innerPolys.clear();
      for (size_t k = 1; k < polyGroups[j].size(); k++)
      {
        innerPolys.set(polyGroups[j][k], 1);
      }

      // Look for non-reversed polys inside this one
      for (size_t kk = 1; kk < polyGroups[j].size(); kk++)
      {
        // jj is the index of the inner poly
        size_t jj = polyGroups[j][kk];
        // If inner poly is not reversed then
        if (!polyReversed.get(jj))
        {
          // Remove that poly and all polys inside of it from the group
          for (size_t ii = 0; ii < polyGroups[jj].size(); ii++)
          {
            innerPolys.set(polyGroups[jj][ii], 0);
          }
        }
      }

      // Use the bit array to recreate the polyGroup
      polyGroups[j].clear();
      polyGroups[j].push_back(j);
      for (size_t jj = 0; jj < numNewPolys; jj++)
      {
        if (innerPolys.get(jj) != 0)
        {
          polyGroups[j].push_back(jj);
        }
      }
    }
  }
}

// ---------------------------------------------------
// Check line segment with point Ids (i, j) to make sure that it
// doesn't cut through the edges of any polys in the group.
// Return value of zero means check failed and the cut is not
// usable.

int vtkCCSCheckCut(
    const std::vector<std::vector<int64_t> > &polys, std::vector<glm::dvec3> &vertices,
    const glm::dvec3 normal, const std::vector<size_t> &polyGroup,
    size_t outerPolyId, size_t innerPolyId,
    int64_t outerIdx, int64_t innerIdx)
{
  int64_t ptId1 = polys[outerPolyId][outerIdx];
  int64_t ptId2 = polys[innerPolyId][innerIdx];

  const double tol = VTK_CCS_POLYGON_TOLERANCE;

  glm::dvec3 p1 = vertices[ptId1];
  glm::dvec3 p2 = vertices[ptId2];

  glm::dvec3 w = p2 - p1;
  double l = glm::length(w);

  // Cuts between coincident points are good
  if (l == 0)
  {
    return 1;
  }

  // Define a tolerance with units of distance squared
  double tol2 = l*l*tol*tol;

  // Check the sense of the cut: it must be pointing "in" for both polys.
  size_t polyId = outerPolyId;
  size_t polyIdx = outerIdx;
  glm::dvec3 r = p1;
  glm::dvec3 r2 = p2;

  for (int ii= 0; ii < 2; ii++)
  {
    const std::vector<int64_t> &poly = polys[polyId];
    size_t n = poly.size();
    size_t prevIdx = n - polyIdx - 1;
    size_t nextIdx = polyIdx + 1;
    if (prevIdx >= n) { prevIdx -= n; }
    if (nextIdx >= n) { nextIdx -= n; }

    glm::dvec3 r1 = vertices[poly[prevIdx]];
    glm::dvec3 r3 = vertices[poly[nextIdx]];

    if (vtkCCSVectorProgression(r, r1, r2, r3, normal) < 0)
    {
      return 0;
    }

    polyId = innerPolyId;
    polyIdx = innerIdx;
    r = p2;
    r2 = p1;
  }

  // Check for intersections of the cut with polygon edges.
  // First, create a cut plane that divides space at the cut line.
  double pc[4];
  glm::dvec3 glmpc = glm::cross(normal, w);
  pc[0] = glmpc[0];
  pc[1] = glmpc[1];
  pc[2] = glmpc[2];
  pc[3] = -glm::dot(glmpc, p1);

  for (size_t i = 0; i < polyGroup.size(); i++)
  {
    const vtkCCSPoly &poly = polys[polyGroup[i]];
    size_t n = poly.size();

    int64_t qtId1 = poly[n-1];
    glm::dvec3 q1 = vertices[qtId1];
    double v1 = pc[0]*q1[0] + pc[1]*q1[1] + pc[2]*q1[2] + pc[3];
    int c1 = (v1 > 0);

    for (size_t j = 0; j < n; j++)
    {
      int64_t qtId2 = poly[j];
      glm::dvec3 q2 = vertices[qtId2];
      double v2 = pc[0]*q2[0] + pc[1]*q2[1] + pc[2]*q2[2] + pc[3];
      int c2 = (v2 > 0);

      // If lines share an endpoint, they can't intersect,
      // so don't bother with the check.
      if (ptId1 != qtId1 && ptId1 != qtId2 &&
          ptId2 != qtId1 && ptId2 != qtId2)
      {
        // Check for intersection
        if ( (c1 ^ c2) || v1*v1 < tol2 || v2*v2 < tol2)
        {
          w[0] = q2[0] - q1[0]; w[1] = q2[1] - q1[1]; w[2] = q2[2] - q1[2];
          if (glm::dot(w, w) > 0)
          {
            double qc[4];
            glm::dvec3 glmqc = glm::cross(normal, w);
            qc[0] = glmqc[0];
            qc[1] = glmqc[1];
            qc[2] = glmqc[2];
            qc[3] = -glm::dot(glmqc, q1);

            double u1 = qc[0]*p1[0] + qc[1]*p1[1] + qc[2]*p1[2] + qc[3];
            double u2 = qc[0]*p2[0] + qc[1]*p2[1] + qc[2]*p2[2] + qc[3];
            int d1 = (u1 > 0);
            int d2 = (u2 > 0);

            if ( (d1 ^ d2) )
            {
              // One final check to make sure endpoints aren't coincident
              glm::dvec3 p = p1;
              glm::dvec3 q = q1;
              if (v2*v2 < v1*v1) { p = p2; }
              if (u2*u2 < u1*u1) { q = q2; }
              if (glm::dot(p - q, p - q) > tol2)
              {
                return 0;
              }
            }
          }
        }
      }

      qtId1 = qtId2;
      q1[0] = q2[0]; q1[1] = q2[1]; q1[2] = q2[2];
      v1 = v2;
      c1 = c2;
    }
  }

  return 1;
}

// ---------------------------------------------------
// Check the quality of a cut between an outer and inner polygon.
// An ideal cut is one that forms a 90 degree angle with each
// line segment that it joins to.  Smaller values indicate a
// higher quality cut.

double vtkCCSCutQuality(
    const std::vector<int64_t> &outerPoly, const std::vector<int64_t> &innerPoly,
    size_t i, size_t j, std::vector<glm::dvec3> &vertices)
{
  size_t n = outerPoly.size();
  size_t m = innerPoly.size();

  size_t a = ((i > 0) ? i-1 : n-1);
  size_t b = ((i < n-1) ? i+1 : 0);

  size_t c = ((j > 0) ? j-1 : m-1);
  size_t d = ((j < m-1) ? j+1 : 0);

  glm::dvec3 p1 = vertices[outerPoly[i]];
  glm::dvec3 p2 = vertices[innerPoly[j]];

  glm::dvec3 v1 = p2 - p1;

  double l1 = glm::dot(v1, v1);
  double l2;
  double qmax = 0;
  double q;

  glm::dvec3 p0 = vertices[outerPoly[a]];
  glm::dvec3 v2 = p0 - p1;
  l2 = glm::dot(v2, v2);
  if (l2 > 0)
  {
    q = glm::dot(v1, v2);
    q *= q/l2;
    if (q > qmax) { qmax = q; }
  }

  p0 = vertices[outerPoly[b]];
  v2 = p0 - p1;
  l2 = glm::dot(v2, v2);
  if (l2 > 0)
  {
    q = glm::dot(v1, v2);
    q *= q/l2;
    if (q > qmax) { qmax = q; }
  }

  p0 = vertices[innerPoly[c]];
  v2 = p2 - p0;
  l2 = glm::dot(v2, v2);
  if (l2 > 0)
  {
    q = glm::dot(v1, v2);
    q *= q/l2;
    if (q > qmax) { qmax = q; }
  }

  p0 = vertices[innerPoly[d]];
  v2 = p2 - p0;
  l2 = glm::dot(v2, v2);
  if (l2 > 0)
  {
    q = glm::dot(v1, v2);
    q *= q/l2;
    if (q > qmax) { qmax = q; }
  }

  if (l1 > 0)
  {
    return qmax/l1; // also l1 + qmax, incorporates distance;
  }

  return std::numeric_limits<double>::max();
}

// ---------------------------------------------------
// Find the two sharpest verts on an inner (i.e. inside-out) poly.

void vtkCCSFindSharpestVerts(
    const std::vector<int64_t> &poly, std::vector<glm::dvec3> &vertices, const glm::dvec3 normal,
    size_t verts[2])
{
  glm::dvec3 v1, v2, v;
  double l1, l2;

  double minVal[2];
  minVal[0] = 0;
  minVal[1] = 0;

  verts[0] = 0;
  verts[1] = 0;

  size_t n = poly.size();
  glm::dvec3 p2 = vertices[poly[n-1]];
  glm::dvec3 p1 = vertices[poly[0]];

  v1 = p1 - p2;

  l1 = std::sqrt(glm::dot(v1, v1));

  for (size_t j = 0; j < n; j++)
  {
    size_t k = j+1;
    if (k == n) { k = 0; }

    p2 = vertices[poly[k]];
    v2 = p2 - p1;
    l2 = std::sqrt(glm::dot(v2, v2));

    v = glm::cross(v1, v2);
    double b = glm::dot(v, normal);

    if (b > 0 && l1*l2 > 0)
    {
      // Dot product is |v1||v2|cos(theta), range [-1, +1]
      double val = glm::dot(v1, v2)/(l1*l2);
      if (val < minVal[0])
      {
        minVal[1] = minVal[0];
        minVal[0] = val;
        verts[1] = verts[0];
        verts[0] = j;
      }
    }

    // Rotate to the next point
    p1 = p2;
    v1 = v2;
    l1 = l2;
  }
}

// ---------------------------------------------------
// Find two valid cuts between outerPoly and innerPoly.
// Used by vtkCCSCutHoleyPolys.

int vtkCCSFindCuts(
    const std::vector<std::vector<int64_t> > &polys,
    const std::vector<size_t> &polyGroup, size_t outerPolyId, size_t innerPolyId,
    std::vector<glm::dvec3> &vertices, const glm::dvec3 normal, size_t cuts[2][2],
    size_t exhaustive)
{
  const std::vector<int64_t> &outerPoly = polys[outerPolyId];
  const std::vector<int64_t> &innerPoly = polys[innerPolyId];
  size_t innerSize = innerPoly.size();
  // Find the two sharpest points on the inner poly
  size_t verts[2];
  vtkCCSFindSharpestVerts(innerPoly, vertices, normal, verts);

  // A list of cut locations according to quality
  typedef std::pair<double, size_t> vtkCCSCutLoc;
  std::vector<vtkCCSCutLoc> cutlist(outerPoly.size());

  // Search for potential cuts (need to find two cuts)
  int cutId = 0;
  cuts[0][0] = cuts[0][1] = 0;
  cuts[1][0] = cuts[1][1] = 0;

  for (cutId = 0; cutId < 2; cutId++)
  {
    int foundCut = 0;

    size_t count = (exhaustive ? innerSize : 3);

    for (size_t i = 0; i < count && !foundCut; i++)
    {
      // Semi-randomize the search order
      size_t j = (i>>1) + (i&1)*((innerSize+1)>>1);
      // Start at the best first point
      j = (j + verts[cutId])%innerSize;

      for (size_t kk = 0; kk < outerPoly.size(); kk++)
      {
        double q = vtkCCSCutQuality(outerPoly, innerPoly, kk, j, vertices);
        cutlist[kk].first = q;
        cutlist[kk].second = kk;
      }

      std::sort(cutlist.begin(), cutlist.end());

      for (size_t lid = 0; lid < cutlist.size(); lid++)
      {
        size_t k = cutlist[lid].second;

        // If this is the second cut, do extra checks
        if (cutId > 0)
        {
          // Make sure cuts don't share an endpoint
          if (j == cuts[0][1] || k == cuts[0][0])
          {
            continue;
          }

          // Make sure cuts don't intersect
          glm::dvec3 p1 = vertices[outerPoly[cuts[0][0]]];
          glm::dvec3 p2 = vertices[innerPoly[cuts[0][1]]];

          glm::dvec3 q1 = vertices[outerPoly[k]];
          glm::dvec3 q2 = vertices[innerPoly[j]];

          double u, v;
          if (IntersectionLine(p1, p2, q1, q2, u, v) == 2)
          {
            continue;
          }
        }

        // This check is done for both cuts
        if (vtkCCSCheckCut(polys, vertices, normal, polyGroup,
                           outerPolyId, innerPolyId, k, j))
        {
          cuts[cutId][0] = k;
          cuts[cutId][1] = j;
          foundCut = 1;
          break;
        }
      }
    }

    if (!foundCut)
    {
      return 0;
    }
  }

  return 1;
}

// ---------------------------------------------------
// Helper for vtkCCSCutHoleyPolys.  Change a polygon and a hole
// into two separate polygons by making two cuts between them.

void vtkCCSMakeCuts(
    std::vector<std::vector<int64_t> > &polys,
    std::vector<std::vector<int64_t> > &polyEdges,
    size_t outerPolyId, size_t innerPolyId,
    std::vector<glm::dvec3> &vertices, const size_t cuts[2][2])
{
  glm::dvec3 q, r;
  for (size_t bb = 0; bb < 2; bb++)
  {
    int64_t ptId1 = polys[outerPolyId][cuts[bb][0]];
    int64_t ptId2 = polys[innerPolyId][cuts[bb][1]];
    q = vertices[ptId1];
    r = vertices[ptId2];
  }

  std::vector<int64_t> &outerPoly = polys[outerPolyId];
  std::vector<int64_t> &innerPoly = polys[innerPolyId];

  std::vector<int64_t> &outerEdges = polyEdges[outerPolyId];
  std::vector<int64_t> &innerEdges = polyEdges[innerPolyId];

  // Generate new polys from the cuts
  size_t n = outerPoly.size();
  size_t m = innerPoly.size();
  size_t idx;

  // Generate poly1
  size_t n1 = n*(cuts[1][0] < cuts[0][0]) + cuts[1][0] - cuts[0][0] + 1;
  size_t n2 = n1 + m*(cuts[0][1] < cuts[1][1]) + cuts[0][1] - cuts[1][1] + 1;

  std::vector<int64_t> poly1(n2);
  std::vector<int64_t> edges1(n2);

  idx = cuts[0][0];
  for (size_t i1 = 0; i1 < n1; i1++)
  {
    size_t k = idx++;
    poly1[i1] = outerPoly[k];
    edges1[i1] = outerEdges[k];
    idx *= (idx != n);
  }
  edges1[n1-1] = -1;

  idx = cuts[1][1];
  for (size_t i2 = n1; i2 < n2; i2++)
  {
    size_t k = idx++;
    poly1[i2] = innerPoly[k];
    edges1[i2] = innerEdges[k];
    idx *= (idx != m);
  }
  edges1[n2-1] = -1;

  // Generate poly2
  size_t m1 = n*(cuts[0][0] < cuts[1][0]) + cuts[0][0] - cuts[1][0] + 1;
  size_t m2 = m1 + m*(cuts[1][1] < cuts[0][1]) + cuts[1][1] - cuts[0][1] + 1;

  std::vector<int64_t> poly2(m2);
  std::vector<int64_t> edges2(m2);

  idx = cuts[1][0];
  for (size_t j1 = 0; j1 < m1; j1++)
  {
    size_t k = idx++;
    poly2[j1] = outerPoly[k];
    edges2[j1] = outerEdges[k];
    idx *= (idx != n);
  }
  edges2[m1-1] = -1;

  idx = cuts[0][1];
  for (size_t j2 = m1; j2 < m2; j2++)
  {
    size_t k = idx++;
    poly2[j2] = innerPoly[k];
    edges2[j2] = innerEdges[k];
    idx *= (idx != m);
  }
  edges2[m2-1] = -1;

  // Replace outerPoly and innerPoly with these new polys
  polys[outerPolyId] = poly1;
  polys[innerPolyId] = poly2;
  polyEdges[outerPolyId] = edges1;
  polyEdges[innerPolyId] = edges2;
}

// ---------------------------------------------------
// After the holes have been identified, make cuts between the
// outer poly and each hole.  Make two cuts per hole.  The only
// strict requirement is that the cut must not intersect any
// edges, but it's best to make sure that no really sharp angles
// are created.
// For each poly that has holes, make two cuts between each hole and
// the outer poly in order to turn the polygon+hole into two polys.
int vtkCCSCutHoleyPolys(
    std::vector<std::vector<int64_t> > &polys, std::vector<glm::dvec3> &vertices,
    std::vector<std::vector<size_t> > &polyGroups,
    std::vector<std::vector<int64_t> > &polyEdges,
    const glm::dvec3 normal)
{
  int cutFailure = 0;

  // Go through all groups and cut out the first inner poly that is
  // found.  Every time an inner poly is cut out, the groupId counter
  // is reset because a cutting a poly creates a new group.
  size_t groupId = 0;
  while (groupId < polyGroups.size())
  {
    std::vector<size_t> &polyGroup = polyGroups[groupId];

    // Only need to make a cut if the group size is greater than 1
    if (polyGroup.size() > 1)
    {
      // The first member of the group is the outer poly
      size_t outerPolyId = polyGroup[0];

      // The second member of the group is the first inner poly
      size_t innerPolyId = polyGroup[1];

      // Sort the group by size, do largest holes first
      std::vector<std::pair<size_t, size_t> >
          innerBySize(polyGroup.size());

      for (size_t i = 1; i < polyGroup.size(); i++)
      {
        innerBySize[i].first = polys[polyGroup[i]].size();
        innerBySize[i].second = i;
      }

      std::sort(innerBySize.begin()+1, innerBySize.end());
      std::reverse(innerBySize.begin()+1, innerBySize.end());

      // Need to check all inner polys in sequence, until one succeeds.
      // Do a quick search first, then do an exhaustive search.
      int madeCut = 0;
      size_t inner = 0;
      for (int exhaustive = 0; exhaustive < 2 && !madeCut; exhaustive++)
      {
        for (size_t j = 1; j < polyGroup.size(); j++)
        {
          inner = innerBySize[j].second;
          innerPolyId = polyGroup[inner];

          size_t cuts[2][2];
          if (vtkCCSFindCuts(polys, polyGroup, outerPolyId, innerPolyId,
                             vertices, normal, cuts, exhaustive))
          {
            vtkCCSMakeCuts(polys, polyEdges, outerPolyId, innerPolyId,
                           vertices, cuts);
            madeCut = 1;
            break;
          }
        }
      }

      if (madeCut)
      {
        // Move successfuly cut innerPolyId into its own group
        polyGroup.erase(polyGroup.begin() + inner);
        polyGroups[innerPolyId].push_back(innerPolyId);
      }
      else
      {
        // Remove all failed inner polys from the group
        for (size_t k = 1; k < polyGroup.size(); k++)
        {
          innerPolyId = polyGroup[k];
          polyGroups[innerPolyId].push_back(innerPolyId);
        }
        polyGroup.resize(1);
        cutFailure = 1;
      }

      // If there are other interior polys in the group, find out whether
      // they are in poly1 or poly2
      if (polyGroup.size() > 1)
      {
        std::vector<int64_t> &poly1 = polys[outerPolyId];
        std::vector<glm::dvec3> pp;
        double bounds[6];
        double tol2;
        vtkCCSPrepareForPolyInPoly(poly1, vertices, pp, bounds, tol2);

        size_t ii = 1;
        while (ii < polyGroup.size())
        {
          if (vtkCCSPolyInPoly(poly1, polys[polyGroup[ii]],
                               vertices, normal, pp, bounds, tol2))
          {
            // Keep this poly in polyGroup
            ii++;
          }
          else
          {
            // Move this poly to poly2 group
            polyGroups[innerPolyId].push_back(polyGroup[ii]);
            polyGroup.erase(polyGroup.begin()+ii);

            // Reduce the groupId to ensure that this new group
            // will get cut
            if (innerPolyId < groupId)
            {
              groupId = innerPolyId;
            }
          }
        }

        // Continue without incrementing groupId
        continue;
      }
    }

    // Increment to the next group
    groupId++;
  }

  return !cutFailure;
}

//----------------------------------------------------------------------------
// This is a complex subroutine that takes a collection of lines that
// were formed by cutting a polydata with a plane, and generates
// a face that has those lines as its edges.  The lines must form one
// or more closed contours, but they need not be sorted.
//
// Only "numLine" lines starting from "firstLine" are used to create new
// polygons, and the new polygons are appended to "polys".  The normal of
// the cut plane must be provided so that polys will be correctly oriented.

// If this is defined, then the outlines of any failed polygons will be
// added to "data".  It is only meant as a debugging tool.
//#define VTK_CCS_SHOW_FAILED_POLYS

void MakePolysFromContours(
    std::vector<glm::i64vec2> &lines, std::vector<glm::dvec3> &vertices,
    std::vector<glm::i64vec3> &triangles, const glm::dvec3 normal)
{
  // If no cut lines were generated, there's nothing to do
  if (lines.empty())
  {
    return;
  }

  // Join all the new lines into connected groups, i.e. polygons.
  // If we are lucky these will be simple, convex polygons.  But
  // we can't count on that.

  std::vector<std::vector<int64_t> > newPolys;
  std::vector<size_t> incompletePolys;

  vtkCCSMakePolysFromLines(lines, newPolys, incompletePolys);

  // Join any loose ends.  If the input was a closed surface then there
  // will not be any loose ends, so this is provided as a service to users
  // who want to clip a non-closed surface.
  vtkCCSJoinLooseEnds(newPolys, incompletePolys, vertices, normal);

  // Some points might be in the middle of straight line segments.
  // These points can be removed without changing the shape of the
  // polys, and removing them makes triangulation more stable.
  // Unfortunately removing these points also means that the polys
  // will no longer form a watertight cap over the cut.

  std::vector<std::vector<int64_t> > polyEdges;
  std::vector<std::vector<int64_t> > originalEdges;
  vtkCCSFindTrueEdges(newPolys, vertices, polyEdges, originalEdges);

  // Next we have to check for polygons with holes, i.e. polygons that
  // have other polygons inside.  Each polygon is "grouped" with the
  // polygons that make up its holes.

  // Initialize each group to hold just one polygon.

  size_t numNewPolys = newPolys.size();
  std::vector<std::vector<size_t> > polyGroups(numNewPolys);
  for (size_t i = 0; i < numNewPolys; i++)
  {
    polyGroups[i].push_back(i);
  }

  // Find out which polys are holes in larger polys.  Create a group
  // for each poly where the first member of the group is the larger
  // poly, and all other members are the holes.  The number of polyGroups
  // will be the same as the number of polys, and any polys that are
  // holes will have a matching empty group.

  vtkCCSMakeHoleyPolys(newPolys, vertices, polyGroups, normal);

  // Make cuts to create simple polygons out of the holey polys.
  // After this is done, each polyGroup will have exactly 1 polygon,
  // and no polys will be holes.  This is currently the most computationally
  // expensive part of the process.

  if (!vtkCCSCutHoleyPolys(newPolys, vertices, polyGroups, polyEdges, normal))
  {
    LERROR() << "Triangulation failed, data may not be watertight.";
  }

  // Some polys might be self-intersecting.  Split the polys at each
  // intersection point.

  vtkCCSSplitAtPinchPoints(newPolys, vertices, polyGroups, polyEdges, normal);

  // ------ Triangulation code ------

  // Go through all polys and triangulate them
  int triangulationFailure = 0;
  for (size_t polyId = 0; polyId < polyGroups.size(); polyId++)
  {
    // If group is empty, then poly was a hole without a containing poly
    if (polyGroups[polyId].size() == 0)
    {
      continue;
    }

    if (!vtkCCSTriangulate(newPolys[polyId], vertices, polyEdges[polyId],
                           originalEdges, triangles))
    {
      triangulationFailure = 1;
    }
  }

  if (triangulationFailure)
  {
    LERROR() << "Triangulation failed, surface may not be watertight in MakePolysFromContours.";
  }
}

// ---------------------------------------------------
bool TriangulatePolygon(std::vector<int64_t> &polygon, std::vector<glm::dvec3> &vertices, std::vector<glm::i64vec3> &triangles)
{
  std::vector<std::vector<int64_t> > polys(1);
  std::vector<int64_t> &poly = polys[0];
  poly = polygon;

  std::vector<std::vector<int64_t> > originalEdges;

  std::vector<std::vector<int64_t> > polyEdges;
  vtkCCSFindTrueEdges(polys, vertices, polyEdges, originalEdges);
  std::vector<int64_t> &edges = polyEdges[0];

  return vtkCCSTriangulate(poly, vertices, edges, originalEdges, triangles);
}

//----------------------------------------------------------------------------
// Description:
// Clip and contour polys in one step, in order to guarantee
// that the contour lines exactly match the new free edges of
// the clipped polygons.  This exact correspondence is necessary
// in order to guarantee that the surface remains closed.
void ClipAndContourPolys(
    std::vector<glm::dvec3> &vertices, std::vector<double> &vertexDists,
    vtkCCSEdgeLocator *edgeLocator,
    std::vector<glm::i64vec3> &inputTriangles,
    std::vector<glm::i64vec3> &outputTriangles, std::vector<glm::i64vec2> &outputLines,
    double epsilon = 1e-6)
{
  std::vector<int64_t> polygon;

  // Go through all triangles and clip them.
  for (size_t triangleIdx = 0; triangleIdx < inputTriangles.size(); triangleIdx++)
  {
    glm::i64vec3 triangle = inputTriangles[triangleIdx];
    polygon.clear();

    int64_t i1 = triangle[2];
    double v1 = vertexDists[i1];
    int c1 = (v1 > 0);

    // The ids for the current edge: init j0 to -1 if i1 will be clipped
    int64_t j0 = (c1 ? i1 : -1);
    int64_t j1 = 0;

    // To store the ids of the contour line
    glm::i64vec2 linePts(0,0);

    for (int i = 0; i < 3; i++)
    {
      // Save previous point info
      int64_t i0 = i1;
      double v0 = v1;
      int c0 = c1;

      // Generate new point info
      i1 = triangle[i];
      v1 = vertexDists[i1];
      c1 = (v1 > 0);

      // If at least one edge end point wasn't clipped
      if ( (c0 | c1) )
      {
        // If only one end was clipped, interpolate new point
        if ( (c0 ^ c1) )
        {
          InterpolateEdge(vertices, edgeLocator, epsilon,
                i0, i1, v0, v1, j1);

          if (j1 != j0)
          {
            polygon.push_back(j1);
            j0 = j1;
          }

          // Save as one end of the contour line
          linePts[c0] = j1;
        }

        if (c1)
        {
          j1 = i1;

          if (j1 != j0)
          {
            polygon.push_back(j1);
            j0 = j1;
          }
        }
      }
    }

    // Insert the clipped poly
    size_t numPoints = polygon.size();

    if (numPoints > 3)
    {
      // Triangulate the poly and insert triangles into output.
      if (!TriangulatePolygon(polygon, vertices, outputTriangles))
      {
        LERROR() << "Triangulation failed, output may not be watertight";
      }
    }
    else if (numPoints == 3)
    {
      // Insert the polygon without triangulating it
      outputTriangles.push_back(glm::i64vec3(polygon[0], polygon[1], polygon[2]));
    }

    // Insert the contour line if one was created
    if (linePts[0] != linePts[1])
    {
      outputLines.push_back(linePts);
    }
  }
}

} // empty namespace


Z3DTriangleList Z3DUtils::clipClosedSurface(const Z3DTriangleList &mesh, std::vector<glm::vec4> clipPlanes, double epsilon)
{
  std::vector<glm::dvec3> vertices = mesh.getDoubleVertices();
  std::vector<glm::uvec3> tris = mesh.getTriangleIndices();
  std::vector<glm::i64vec3> inputTriangles;
  vtkCCSEdgeLocator *edgeLocator = vtkCCSEdgeLocator::New();
  for (size_t i=0; i<tris.size(); i++)
    inputTriangles.push_back(glm::i64vec3(tris[i]));

  bool clipped = false;

  for (size_t i=0; i<clipPlanes.size(); i++) {
    std::vector<glm::i64vec3> outputTriangles;
    glm::dvec4 plane = glm::dvec4(clipPlanes[i]);
    std::vector<double> vertexDists;
    bool needClip2 = false;
    for (size_t v=0; v<vertices.size(); v++) {
      vertexDists.push_back(-vertexPlaneDistance(vertices[v], plane, 0.0));
      if (vertexDists[vertexDists.size()-1] < 0)
        needClip2 = true;
    }

    if (needClip2) {
      edgeLocator->Initialize();
      std::vector<glm::i64vec2> outputLines;
      ClipAndContourPolys(vertices, vertexDists, edgeLocator, inputTriangles, outputTriangles, outputLines, epsilon);
      MakePolysFromContours(outputLines, vertices, outputTriangles, plane.xyz());
      inputTriangles.swap(outputTriangles);
      clipped = true;
    }
  }
  edgeLocator->Delete();

  if (clipped) {
    Z3DTriangleList result(GL_TRIANGLES);
    result.setVertices(vertices);
    std::vector<GLuint> indexes;
    for (size_t i=0; i<inputTriangles.size(); i++) {
      indexes.push_back(inputTriangles[i].x);
      indexes.push_back(inputTriangles[i].y);
      indexes.push_back(inputTriangles[i].z);
    }
    result.setIndices(indexes);

    result.interpolate(mesh);
    return result;
  } else
    return mesh;
}

std::string Z3DUtils::GetVolumeRenderingName(
    NeuTube3D::EVolumeRenderingMode mode)
{
  return NeuTube3D::GetVolumeRenderingModeName(mode);
}

