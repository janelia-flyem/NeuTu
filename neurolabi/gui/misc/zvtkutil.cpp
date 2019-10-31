#include "zvtkutil.h"

#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkDataArray.h>
#include <vtkIdTypeArray.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>

#include "zmesh.h"
#include "logging/zqslog.h"

ZMesh vtkPolyDataToMesh(vtkPolyData* polyData)
{
  CHECK(polyData);
  vtkPoints* points = polyData->GetPoints();
  vtkCellArray* polys = polyData->GetPolys();
  vtkDataArray* pointsNormals = polyData->GetPointData()->GetNormals();

  std::vector<glm::dvec3> vertices(points->GetNumberOfPoints());

  std::vector<glm::dvec3> normals;
  if (pointsNormals) {
    normals.resize(pointsNormals->GetNumberOfTuples());
    CHECK(vertices.size() == normals.size());
  }

  std::vector<gl::GLuint> indices;
  for (vtkIdType id = 0; id < points->GetNumberOfPoints(); ++id) {
    points->GetPoint(id, &vertices[id][0]);
    if (pointsNormals) {
      pointsNormals->GetTuple(id, &normals[id][0]);
    }
  }
  vtkIdType npts;
  vtkIdType* pts;
  polys->InitTraversal();
  for (int i = 0; i < polyData->GetNumberOfPolys(); ++i) {
    int h = polys->GetNextCell(npts, pts);
    if (h == 0) {
      break;
    }
    if (npts == 3) {
      indices.push_back(pts[0]);
      indices.push_back(pts[1]);
      indices.push_back(pts[2]);
    }
  }

  ZMesh msh;
  msh.setVertices(vertices);
  msh.setIndices(indices);
  if (pointsNormals) {
    msh.setNormals(normals);
  }
  return msh;
}

vtkSmartPointer<vtkPolyData> meshToVtkPolyData(const ZMesh& mesh)
{
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetDataType(VTK_FLOAT);
  const std::vector<glm::vec3>& vertices = mesh.vertices();
  points->Allocate(vertices.size());
  for (size_t i = 0; i < vertices.size(); ++i) {
    points->InsertPoint(i, vertices[i].x, vertices[i].y, vertices[i].z);
  }


  vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
  size_t numTriangles = mesh.numTriangles();
  polys->Allocate(numTriangles * 3);
  vtkIdType pts[3];
  for (size_t i = 0; i < numTriangles; ++i) {
    glm::uvec3 tri = mesh.triangleIndices(i);
    pts[0] = tri[0];
    pts[1] = tri[1];
    pts[2] = tri[2];
    polys->InsertNextCell(3, pts);
  }

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);

  const std::vector<glm::vec3>& normals = mesh.normals();
  if (!normals.empty()) {
    vtkSmartPointer<vtkFloatArray> nrmls = vtkSmartPointer<vtkFloatArray>::New();

    if (normals.size() != vertices.size()) {
      LWARN() << "Unmatched normal size" << normals.size() << vertices.size();
    }
    //  CHECK(normals.size() == vertices.size());
    nrmls->SetNumberOfComponents(3);
    nrmls->Allocate(3 * normals.size());
    nrmls->SetName("Normals");
    for (size_t i = 0; i < normals.size(); ++i) {
      nrmls->InsertTuple(i, &normals[i][0]);
    }
    polyData->GetPointData()->SetNormals(nrmls);
  }

  polyData->SetPolys(polys);

  return polyData;
}
