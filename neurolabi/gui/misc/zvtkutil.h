#ifndef ZVTKUTIL_H
#define ZVTKUTIL_H

#include <vtkSmartPointer.h>

class vtkPolyData;
class ZMesh;

ZMesh vtkPolyDataToMesh(vtkPolyData* polyData);
vtkSmartPointer<vtkPolyData> meshToVtkPolyData(const ZMesh& mesh);

#endif // ZVTKUTIL_H
