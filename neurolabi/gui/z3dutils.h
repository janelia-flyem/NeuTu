#ifndef Z3DUTILS_H
#define Z3DUTILS_H

#include "z3dmesh.h"
#include "z3ddef.h"


class Z3DUtils
{
public:
  template<typename Real>
  static bool isApproxEqual(const glm::tvec3<Real,glm::highp> &vertex1, const glm::tvec3<Real,glm::highp> &vertex2, Real epsilon = 1e-6);

  template<typename Real>
  static Real vertexPlaneDistance(const glm::tvec3<Real,glm::highp> &vertex, const glm::tvec4<Real,glm::highp>& plane, Real epsilon = 1e-6);

  //
  // Determine the distance of the current vertex to the edge defined by
  // the vertices provided.  Returns distance squared. Note: line is assumed
  // infinite in extent.
  template<typename Real>
  static Real vertexLineSquaredDistance(const glm::tvec3<Real,glm::highp> &x, const glm::tvec3<Real,glm::highp> &p1, const glm::tvec3<Real,glm::highp> &p2);

  // Compute distance to finite line. Returns parametric coordinate t
  // and point location on line.
  template<typename Real>
  static Real vertexLineSegmentSquaredDistance(const glm::tvec3<Real,glm::highp> &x, const glm::tvec3<Real,glm::highp> &p1, const glm::tvec3<Real,glm::highp> &p2,
                                               Real &t, glm::tvec3<Real,glm::highp> &closestPoint);

  // Note: This method assume that P is on triangle plane. P = A + u * (B - A) + v * (C - A)
  // vertex inside tirangle if (u >= 0) && (v >= 0) && (u + v < 1)
  template<typename Real>
  static bool vertexInsideTriangle(const glm::tvec3<Real,glm::highp> &P, const glm::tvec3<Real,glm::highp> &A, const glm::tvec3<Real,glm::highp> &B,
                                   const glm::tvec3<Real,glm::highp> &C, Real epsilon, Real &u, Real &v);

  // distance between P and Triangle A + s * (B - A) + t * (C - A)
  // return value s and t can be used for intepolation
  template<typename Real>
  static Real vertexTriangleSquaredDistance(glm::tvec3<Real,glm::highp> P, glm::tvec3<Real,glm::highp> A,
                                            glm::tvec3<Real,glm::highp> B, glm::tvec3<Real,glm::highp> C, Real &s, Real &t);

  // from VTK
  static Z3DTriangleList clipClosedSurface(const Z3DTriangleList &mesh, std::vector<glm::vec4> clipPlanes, double epsilon = 1e-6);

  static std::string GetVolumeRenderingName(NeuTube3D::EVolumeRenderingMode mode);
};


// -------------------------------------------------------------------------------------------

template<typename Real>
bool Z3DUtils::isApproxEqual(const glm::tvec3<Real,glm::highp> &vertex1, const glm::tvec3<Real,glm::highp> &vertex2, Real epsilon)
{
  return glm::length(vertex1 - vertex2) <= epsilon;
}

template<typename Real>
Real Z3DUtils::vertexPlaneDistance(const glm::tvec3<Real,glm::highp> &vertex, const glm::tvec4<Real,glm::highp> &plane, Real epsilon)
{
  double distance = glm::dot(plane.xyz(), vertex) - plane.w;
  if (std::abs(distance) <= epsilon)
    return 0;
  else
    return distance;
}

template<typename Real>
Real Z3DUtils::vertexTriangleSquaredDistance(glm::tvec3<Real,glm::highp> P, glm::tvec3<Real,glm::highp> A,
                                                      glm::tvec3<Real,glm::highp> B, glm::tvec3<Real,glm::highp> C, Real &s, Real &t)
{
  glm::tvec3<Real,glm::highp> diff = A - P;
  glm::tvec3<Real,glm::highp> edge0 = B - A;
  glm::tvec3<Real,glm::highp> edge1 = C - A;
  Real a00 = glm::dot(edge0, edge0);
  Real a01 = glm::dot(edge0, edge1);
  Real a11 = glm::dot(edge1, edge1);
  Real b0 = glm::dot(diff, edge0);
  Real b1 = glm::dot(diff, edge1);
  Real c = glm::dot(diff, diff);
  Real det = std::abs(a00*a11 - a01*a01);
  s = a01*b1 - a11*b0;
  t = a01*b0 - a00*b1;
  Real sqrDistance;

  if (s + t <= det)
  {
    if (s < (Real)0)
    {
      if (t < (Real)0)  // region 4
      {
        if (b0 < (Real)0)
        {
          t = (Real)0;
          if (-b0 >= a00)
          {
            s = (Real)1;
            sqrDistance = a00 + ((Real)2)*b0 + c;
          }
          else
          {
            s = -b0/a00;
            sqrDistance = b0*s + c;
          }
        }
        else
        {
          s = (Real)0;
          if (b1 >= (Real)0)
          {
            t = (Real)0;
            sqrDistance = c;
          }
          else if (-b1 >= a11)
          {
            t = (Real)1;
            sqrDistance = a11 + ((Real)2)*b1 + c;
          }
          else
          {
            t = -b1/a11;
            sqrDistance = b1*t + c;
          }
        }
      }
      else  // region 3
      {
        s = (Real)0;
        if (b1 >= (Real)0)
        {
          t = (Real)0;
          sqrDistance = c;
        }
        else if (-b1 >= a11)
        {
          t = (Real)1;
          sqrDistance = a11 + ((Real)2)*b1 + c;
        }
        else
        {
          t = -b1/a11;
          sqrDistance = b1*t + c;
        }
      }
    }
    else if (t < (Real)0)  // region 5
    {
      t = (Real)0;
      if (b0 >= (Real)0)
      {
        s = (Real)0;
        sqrDistance = c;
      }
      else if (-b0 >= a00)
      {
        s = (Real)1;
        sqrDistance = a00 + ((Real)2)*b0 + c;
      }
      else
      {
        s = -b0/a00;
        sqrDistance = b0*s + c;
      }
    }
    else  // region 0
    {
      // minimum at interior point
      Real invDet = ((Real)1)/det;
      s *= invDet;
      t *= invDet;
      sqrDistance = s*(a00*s + a01*t + ((Real)2)*b0) +
          t*(a01*s + a11*t + ((Real)2)*b1) + c;
    }
  }
  else
  {
    Real tmp0, tmp1, numer, denom;

    if (s < (Real)0)  // region 2
    {
      tmp0 = a01 + b0;
      tmp1 = a11 + b1;
      if (tmp1 > tmp0)
      {
        numer = tmp1 - tmp0;
        denom = a00 - ((Real)2)*a01 + a11;
        if (numer >= denom)
        {
          s = (Real)1;
          t = (Real)0;
          sqrDistance = a00 + ((Real)2)*b0 + c;
        }
        else
        {
          s = numer/denom;
          t = (Real)1 - s;
          sqrDistance = s*(a00*s + a01*t + ((Real)2)*b0) +
              t*(a01*s + a11*t + ((Real)2)*b1) + c;
        }
      }
      else
      {
        s = (Real)0;
        if (tmp1 <= (Real)0)
        {
          t = (Real)1;
          sqrDistance = a11 + ((Real)2)*b1 + c;
        }
        else if (b1 >= (Real)0)
        {
          t = (Real)0;
          sqrDistance = c;
        }
        else
        {
          t = -b1/a11;
          sqrDistance = b1*t + c;
        }
      }
    }
    else if (t < (Real)0)  // region 6
    {
      tmp0 = a01 + b1;
      tmp1 = a00 + b0;
      if (tmp1 > tmp0)
      {
        numer = tmp1 - tmp0;
        denom = a00 - ((Real)2)*a01 + a11;
        if (numer >= denom)
        {
          t = (Real)1;
          s = (Real)0;
          sqrDistance = a11 + ((Real)2)*b1 + c;
        }
        else
        {
          t = numer/denom;
          s = (Real)1 - t;
          sqrDistance = s*(a00*s + a01*t + ((Real)2)*b0) +
              t*(a01*s + a11*t + ((Real)2)*b1) + c;
        }
      }
      else
      {
        t = (Real)0;
        if (tmp1 <= (Real)0)
        {
          s = (Real)1;
          sqrDistance = a00 + ((Real)2)*b0 + c;
        }
        else if (b0 >= (Real)0)
        {
          s = (Real)0;
          sqrDistance = c;
        }
        else
        {
          s = -b0/a00;
          sqrDistance = b0*s + c;
        }
      }
    }
    else  // region 1
    {
      numer = a11 + b1 - a01 - b0;
      if (numer <= (Real)0)
      {
        s = (Real)0;
        t = (Real)1;
        sqrDistance = a11 + ((Real)2)*b1 + c;
      }
      else
      {
        denom = a00 - ((Real)2)*a01 + a11;
        if (numer >= denom)
        {
          s = (Real)1;
          t = (Real)0;
          sqrDistance = a00 + ((Real)2)*b0 + c;
        }
        else
        {
          s = numer/denom;
          t = (Real)1 - s;
          sqrDistance = s*(a00*s + a01*t + ((Real)2)*b0) +
              t*(a01*s + a11*t + ((Real)2)*b1) + c;
        }
      }
    }
  }

  // Account for numerical round-off error.
  if (sqrDistance < (Real)0)
  {
    sqrDistance = (Real)0;
  }

  return sqrDistance;
}

template<typename Real>
Real Z3DUtils::vertexLineSquaredDistance(const glm::tvec3<Real,glm::highp> &x, const glm::tvec3<Real,glm::highp> &p1, const glm::tvec3<Real,glm::highp> &p2)
{
  Real proj, den;
  glm::tvec3<Real,glm::highp> np1, p1p2;

  np1 = x - p1;
  p1p2 = p1 - p2;

  if ( (den=glm::length(p1p2)) != 0.0 )
  {
    for (int i=0; i<3; i++)
    {
      p1p2[i] /= den;
    }
  }
  else
  {
    return glm::dot(np1,np1);
  }

  proj = glm::dot(np1,p1p2);

  return (glm::dot(np1,np1) - proj*proj);
}

template<typename Real>
Real Z3DUtils::vertexLineSegmentSquaredDistance(const glm::tvec3<Real,glm::highp> &x, const glm::tvec3<Real,glm::highp> &p1, const glm::tvec3<Real,glm::highp> &p2,
                                                Real &t, glm::tvec3<Real,glm::highp> &closestPoint)
{
  Real denom, num;
  glm::tvec3<Real,glm::highp> closest;
  Real tolerance;
  //
  //   Determine appropriate vectors
  //
  glm::tvec3<Real,glm::highp> p21 = p2 - p1;

  //
  //   Get parametric location
  //
  //num = p21[0]*(x[0]-p1[0]) + p21[1]*(x[1]-p1[1]) + p21[2]*(x[2]-p1[2]);
  num = glm::dot(p21, x - p1);
  denom = glm::dot(p21,p21);

  // trying to avoid an expensive fabs
  tolerance = 1e-5*num;
  if (tolerance < 0.0)
  {
    tolerance = -tolerance;
  }
  if ( -tolerance < denom && denom < tolerance ) //numerically bad!
  {
    closest = p1; //arbitrary, point is (numerically) far away
  }
  //
  // If parametric coordinate is within 0<=p<=1, then the point is closest to
  // the line.  Otherwise, it's closest to a point at the end of the line.
  //
  else if ( denom <= 0.0 || (t=num/denom) < 0.0 )
  {
    closest = p1;
  }
  else if ( t > 1.0 )
  {
    closest = p2;
  }
  else
  {
    closest = p21;
    p21 = p1 + p21 * t;
    //p21[0] = p1[0] + t*p21[0];
    //p21[1] = p1[1] + t*p21[1];
    //p21[2] = p1[2] + t*p21[2];
  }

  closestPoint = closest;
  return glm::dot(closest-x, closest-x);
}

template<typename Real>
bool Z3DUtils::vertexInsideTriangle(const glm::tvec3<Real,glm::highp> &P, const glm::tvec3<Real,glm::highp> &A, const glm::tvec3<Real,glm::highp> &B,
                                    const glm::tvec3<Real,glm::highp> &C, Real epsilon, Real &u, Real &v)
{
  // Compute vectors
  glm::tvec3<Real,glm::highp> v0 = C - A;
  glm::tvec3<Real,glm::highp> v1 = B - A;
  glm::tvec3<Real,glm::highp> v2 = P - A;

  // Compute dot products
  Real dot00 = glm::dot(v0, v0);
  Real dot01 = glm::dot(v0, v1);
  Real dot02 = glm::dot(v0, v2);
  Real dot11 = glm::dot(v1, v1);
  Real dot12 = glm::dot(v1, v2);

  // Compute barycentric coordinates
  Real invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
  v = (dot11 * dot02 - dot01 * dot12) * invDenom;
  u = (dot00 * dot12 - dot01 * dot02) * invDenom;

  // Check if point is in triangle
  return (u >= -epsilon) && (v >= -epsilon) && (u + v <= 1+epsilon);
}


#endif // Z3DUTILS_H
