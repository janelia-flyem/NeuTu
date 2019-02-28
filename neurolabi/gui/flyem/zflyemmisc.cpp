//#define _NEUTU_USE_REF_KEY_
#include "zflyemmisc.h"

#include <unistd.h>
#include <iostream>
#include <QString>
#include <QProcess>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include "tz_math.h"
#include "tz_utilities.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_neighborhood.h"

#include "zjsondef.h"
#include "neutubeconfig.h"
#include "zglobal.h"
#include "zmatrix.h"
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"
#include "zstring.h"
#include "geometry/zcuboid.h"
#include "geometry/zaffinerect.h"
#include "geometry/zintcuboidarray.h"

#include "mvc/zstackdoc.h"
#include "mvc/zstackdochelper.h"
#include "z3dgraphfactory.h"
#include "zstackobjectsourcefactory.h"
#include "z3dwindow.h"
#include "zcubearray.h"

#include "dvid/zdvidreader.h"
#include "dvid/zdvidinfo.h"
#include "dvid/zdvidsparsestack.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidurl.h"
#include "dvid/zdvidsynapse.h"

#include "zstackviewparam.h"
#include "zobject3dfactory.h"
#include "zstroke2d.h"
#include "zfileparser.h"
#include "zjsonfactory.h"
#include "zobject3d.h"
#include "zarbsliceviewparam.h"
#include "zmainwindowcontroller.h"
#include "zswctree.h"
#include "zarray.h"
#include "zstack.hxx"
#include "zstackfactory.h"
#include "misc/miscutility.h"
#include "zmeshfactory.h"
#include "zflyembodyannotation.h"

void flyem::NormalizeSimmat(ZMatrix &simmat)
{
  for (int j = 0; j < simmat.getColumnNumber(); ++j) {
    double maxC = simmat.getValue(j, j);
    for (int i = 0; i < simmat.getRowNumber(); ++i) {
      if (i != j) {
        double maxR = simmat.getValue(i, i);
        simmat.set(i, j, simmat.getValue(i, j) / dmax2(maxC, maxR));
      }
    }
  }
  simmat.setDiag(1);
}

flyem::HackathonEvaluator::HackathonEvaluator(
    const std::string &sourceDir, const std::string &workDir) :
  m_sourceDir(sourceDir), m_workDir(workDir), m_accuracy(0)
{

}

std::string flyem::HackathonEvaluator::getNeuronInfoFile() const
{
  return getSourceDir() + "/neuronsinfo.json";
}

std::string flyem::HackathonEvaluator::getNeuronTypeFile() const
{
  return getWorkDir() + "/neuron_type.json";
}

std::string flyem::HackathonEvaluator::getSimmatFile() const
{
  return getWorkDir() + "/simmat.txt";
}

void flyem::HackathonEvaluator::processNeuronTypeFile()
{
  ZJsonObject jsonObject;
  jsonObject.load(getNeuronInfoFile());

  const char *key;
  json_t *value;
  std::map<std::string, int> typeLabelMap;
  std::vector<int> bodyIdArray;
  std::vector<int> labelArray;
  int maxLabel = 0;
  ZJsonArray idJson;
  ZJsonArray labelJson;

  ZJsonObject_foreach(jsonObject, key, value) {
    int bodyId = ZString(key).firstInteger();
    ZJsonObject bodyJson(value, ZJsonValue::SET_INCREASE_REF_COUNT);
    std::string type = ZJsonParser::stringValue(bodyJson["Type"]);
    if (typeLabelMap.count(type) == 0) {
      typeLabelMap[type] = ++maxLabel;
    }

    int label = typeLabelMap[type];

    bodyIdArray.push_back(bodyId);
    labelArray.push_back(label);
    idJson.append(bodyId);
    labelJson.append(label);
  }

  ZJsonObject rootJson;
  ZJsonArray typeArray;
  for (std::map<std::string, int>::const_iterator iter = typeLabelMap.begin();
       iter != typeLabelMap.end(); ++iter) {
    ZJsonObject typeJson;
    typeJson.setEntry(iter->first.c_str(), iter->second);
    typeArray.append(typeJson);
  }

  rootJson.setEntry("id", idJson);
  rootJson.setEntry("label", labelJson);
  rootJson.setEntry("label_type", typeArray);
  std::cout << rootJson.dumpString() << std::endl;

  rootJson.dump(getNeuronTypeFile());
}

void flyem::HackathonEvaluator::evalulate()
{
  processNeuronTypeFile();

//  std::cout << count << " named neurons." << std::endl;

//  QString simmatFile = m_hackathonConfigDlg->getWorkDir() + "/simmat.txt";

  ZMatrix matrix;
  matrix.importTextFile(getSimmatFile());

//  std::vector<int> idArray(matrix.getColumnNumber());
  m_idArray.resize(matrix.getColumnNumber());
  for (int i = 0; i < matrix.getColumnNumber(); ++i) {
    m_idArray[i] = iround(matrix.getValue(0, i));
  }

  ZMatrix simmat = matrix.makeRowSlice(1, matrix.getRowNumber() - 1);
  flyem::NormalizeSimmat(simmat);
  simmat.setDiag(0);

#ifdef _DEBUG_
  simmat.printInfo();
#endif

  std::map<int, int> idLabelMap;
  ZJsonObject typeJson;
//  QString typeFile = m_hackathonConfigDlg->getWorkDir() + "/neuron_type.json";
  typeJson.load(getNeuronTypeFile());
  ZJsonArray idArrayJson(typeJson["id"], ZJsonValue::SET_INCREASE_REF_COUNT);
  ZJsonArray labelArrayJson(typeJson["label"], ZJsonValue::SET_INCREASE_REF_COUNT);
  for (size_t i = 0; i < idArrayJson.size(); ++i) {
    int id = ZJsonParser::integerValue(idArrayJson.at(i));
    int label = ZJsonParser::integerValue(labelArrayJson.at(i));
    idLabelMap[id] = label;
  }

  int count = 0;
  for (int i = 0; i < simmat.getRowNumber(); ++i) {
    int predictedIndex = 0;
    simmat.getRowMax(i, &predictedIndex);
    std::cout << i << " " << predictedIndex << std::endl;
    int trueLabel = idLabelMap[m_idArray[i]];
    int predictedLabel = idLabelMap[m_idArray[predictedIndex]];
    if (trueLabel == predictedLabel) {
      ++count;
    }
  }

  m_accuracy = count;

//  QString information =
//      QString("Accuracy: %1 / %2").arg(count).arg(idArray.size());

//  report("Evaluation", information.toStdString(), neutube::EMessageType::MSG_INFORMATION);
}

Z3DGraph* flyem::MakeBoundBoxGraph(const ZDvidInfo &dvidInfo)
{
  ZCuboid box;
  box.setFirstCorner(dvidInfo.getStartCoordinates().toPoint());
  box.setLastCorner(dvidInfo.getEndCoordinates().toPoint());
  Z3DGraph *graph = Z3DGraphFactory::MakeBox(
        box, dmax2(1.0, dmax3(box.width(), box.height(), box.depth()) / 1000.0));
  graph->setSource(ZStackObjectSourceFactory::MakeFlyEmBoundBoxSource());

  return graph;
}

Z3DGraph* flyem::MakePlaneGraph(ZStackDoc *doc, const ZDvidInfo &dvidInfo)
{
  Z3DGraph *graph = NULL;
  if (doc != NULL) {
    ZRect2d rect;
    ZStackDocHelper docHelper;
    docHelper.extractCurrentZ(doc);
    if (docHelper.hasCurrentZ()) {
      rect.setZ(docHelper.getCurrentZ());
      //    rect.setZ(getCurrentZ());
      rect.setFirstCorner(dvidInfo.getStartCoordinates().getX(),
                          dvidInfo.getStartCoordinates().getY());
      rect.setLastCorner(dvidInfo.getEndCoordinates().getX(),
                         dvidInfo.getEndCoordinates().getY());
      ZCuboid box;
      box.setFirstCorner(dvidInfo.getStartCoordinates().toPoint());
      box.setLastCorner(dvidInfo.getEndCoordinates().toPoint());
      double lineWidth = box.depth() / 2000.0;
      graph = Z3DGraphFactory::MakeGrid(rect, 100, lineWidth);
      graph->setSource(ZStackObjectSourceFactory::MakeFlyEmPlaneObjectSource());
    }
  }

  return graph;
}

Z3DGraph* flyem::MakeSliceViewGraph(const ZArbSliceViewParam &param)
{
  Z3DGraph *graph = NULL;
  if (param.isValid()) {
    ZPoint center = param.getCenter().toPoint();
    int rx = param.getWidth() / 2;
    int ry = param.getHeight() / 2;
    ZPoint pt1 = center - param.getPlaneV1() * rx - param.getPlaneV2() * ry;
    ZPoint pt2 = center + param.getPlaneV1() * rx - param.getPlaneV2() * ry;
    ZPoint pt3 = center + param.getPlaneV1() * rx + param.getPlaneV2() * ry;
    ZPoint pt4 = center - param.getPlaneV1() * rx + param.getPlaneV2() * ry;

    graph = Z3DGraphFactory::MakeQuadCross(pt1, pt2, pt3, pt4);
    graph->setSource(ZStackObjectSourceFactory::MakeSlicViewObjectSource());
  }

  return graph;
}

Z3DGraph* flyem::MakeRoiGraph(
    const ZObject3dScan &roi, const ZDvidInfo &dvidInfo)
{
  int sampleInterval = 1;

//  int intv = 0;

  Z3DGraph *graph = new Z3DGraph;
  //For each voxel, create a graph
  int startCoord[3];
  Stack *stack = roi.toStackWithMargin(startCoord, 1, 1);

  size_t offset = 0;
  int i, j, k;
  int n;
  int neighbor[26];
  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);
  int cwidth = width - 1;
  int cheight = height - 1;
  int cdepth = depth - 1;

  Stack_Neighbor_Offset(
        6, C_Stack::width(stack), C_Stack::height(stack), neighbor);
  uint8_t *array = C_Stack::array8(stack);

  Z3DGraphFactory factory;
  factory.setNodeRadiusHint(0);
  factory.setShapeHint(GRAPH_LINE);
  factory.setEdgeColorHint(roi.getColor());
//  factory.setEdgeColorHint(QColor(128, 64, 64));

  for (k = 0; k <= cdepth; k ++) {
    for (j = 0; j <= cheight; j++) {
      for (i = 0; i <= cwidth; i++) {
        if (k % sampleInterval == 0) {
          if (array[offset] > 0) {
            std::vector<int> faceArray;
            for (n = 0; n < 6; n++) {
              if (array[offset + neighbor[n]] == 0) {
                faceArray.push_back(n);
              }
            }
            if (!faceArray.empty()) {
              ZIntCuboid box = dvidInfo.getBlockBox(
                    i + startCoord[0], j + startCoord[1], k + startCoord[2]);
              box.setLastCorner(box.getLastCorner() + ZIntPoint(1, 1, 1));
              Z3DGraph *subgraph = factory.makeFaceGraph(box, faceArray);
              graph->append(*subgraph);
              delete subgraph;
            }
          }
        }
        offset++;
      }
    }
  }

  C_Stack::kill(stack);

#if 0
  Stack *surface = Stack_Perimeter(stack, NULL, 6);
  C_Stack::kill(stack);

  int width = C_Stack::width(surface);
  int height = C_Stack::height(surface);
  int depth = C_Stack::depth(surface);
  size_t offset = 0;

  C_Stack::write("/Users/zhaot/Work/neutube/neurolabi/data/test.tif", surface);

  Z3DGraphFactory factory;
  factory.setNodeRadiusHint(0);
  factory.setShapeHint(GRAPH_LINE);
  factory.setEdgeColorHint(QColor(128, 64, 64));
  int sampleInterval = 5;
  int intv = 0;
  for (int k = 0; k < depth; ++k) {
    for (int j = 0; j < height; ++j) {
      for (int i = 0; i < width; ++i) {
        if ((intv++) % sampleInterval == 0) {
          if (surface->array[offset] > 0) {
            ZIntCuboid box = dvidInfo.getBlockBox(
                  i + startCoord[0], j + startCoord[1], k + startCoord[2]);
            Z3DGraph *subgraph = factory.makeBox(box);
            graph->append(*subgraph);
            delete subgraph;
          }
        }
        ++offset;
      }
    }
  }

  C_Stack::kill(surface);
#endif

  return graph;
}

ZCubeArray* flyem::MakeRoiCube(
    const ZObject3dScan &roi, QColor color, int dsIntv)
{
  ZCubeArray *cubeArray = new ZCubeArray;
  ZMesh *mesh = MakeRoiMesh(roi, color, dsIntv);
  cubeArray->setMesh(mesh);

  return cubeArray;
#if 0
  ZObject3dScan dsRoi = roi;
//  ZDvidInfo dsInfo = dvidInfo;

  if (dsIntv > 0) {
    dsRoi.downsampleMax(dsIntv, dsIntv, dsIntv);
//    dsInfo.downsampleBlock(dsIntv, dsIntv, dsIntv);
  }

  ZIntPoint roiDsIntv = dsRoi.getDsIntv();
//  int sampleInterval = dsIntv;

  ZCubeArray *cubes = new ZCubeArray;
  //For each voxel, create a graph
  int startCoord[3];
  Stack *stack = dsRoi.toStackWithMargin(startCoord, 1, 1);
  //Stack *stack = roi.getSlice(49).toStackWithMargin(startCoord, 1, 1); // rendering only one slice of ROIs for test

  size_t offset = 0;
  int i, j, k;
  int n;
  int neighbor[26];
  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);
  int cwidth = width - 1;
  int cheight = height - 1;
  int cdepth = depth - 1;

  Stack_Neighbor_Offset(6, C_Stack::width(stack), C_Stack::height(stack), neighbor);
  uint8_t *array = C_Stack::array8(stack);

  //
  //glm::vec4 color = glm::vec4(0.5, 0.25, 0.25, 1.0);

  //
  for (k = 0; k <= cdepth; k ++) {
    for (j = 0; j <= cheight; j++) {
      for (i = 0; i <= cwidth; i++) {
//        bool goodCube = (sampleInterval == 0);
//        if (!goodCube) {
//          goodCube = (k % sampleInterval == 0);
//        }
        bool goodCube = true;
        if (goodCube) {
          if (array[offset] > 0) {
            std::vector<int> faceArray;
            for (n = 0; n < 6; n++) {
              if (array[offset + neighbor[n]] == 0) {
                faceArray.push_back(n);
              }
            }
            if (!faceArray.empty()) {
              ZIntCuboid box;
              box.setFirstCorner((i + startCoord[0]) * (roiDsIntv.getX() + 1),
                                 (j + startCoord[1]) * (roiDsIntv.getY() + 1),
                                 (k + startCoord[2]) * (roiDsIntv.getZ() + 1));
              box.setSize(
                    roiDsIntv.getX() + 2, roiDsIntv.getY() + 2, roiDsIntv.getZ() + 2);
//              ZIntCuboid box = dsInfo.getBlockBox(
//                    i * dsInt, j + startCoord[1], k + startCoord[2]);
//              box.setLastCorner(box.getLastCorner() + ZIntPoint(1, 1, 1));

              qreal r,g,b,a;
              color.getRgbF(&r, &g, &b, &a); // QColor -> glm::vec4

              Z3DCube *cube = cubes->makeCube(box, glm::vec4(r,g,b,a), faceArray);
              cubes->append(*cube);
              delete cube;
            }
          }
        }
        offset++;
      }
    }
  }

  C_Stack::kill(stack);

  //
  return cubes;
#endif
}

ZStack* flyem::MakeColorSegmentation(const ZDvidReader &reader, const ZAffineRect &ar)
{
  return MakeColorSegmentation(reader, ar, 0, 0);
}

namespace {

ZStack* LabelToColorStack(const ZArray *array)
{
  ZStack *stack = NULL;
  if (array != NULL) {
    uint64_t *labelArray = array->getDataPointer<uint64_t>();

    ZObjectColorScheme colorScheme;
    colorScheme.setColorScheme(ZColorScheme::CONV_RANDOM_COLOR);

    stack = ZStackFactory::MakeZeroStack(
          GREY, misc::GetBoundBox(array), 3);
    //    ZStack *stack = new ZStack(COLOR, width, height, 1, 1);
    uint8_t *array1 = stack->array8(0);
    uint8_t *array2 = stack->array8(1);
    uint8_t *array3 = stack->array8(2);

    size_t volume = stack->getVoxelNumber();
    for (size_t i = 0; i < volume; ++i) {
      QColor color = colorScheme.getColor(labelArray[i]);
      array1[i] = color.red();
      array2[i] = color.green();
      array3[i] = color.blue();
    }

    delete array;
  }

  return stack;
}

} //namespace

ZStack* flyem::MakeColorSegmentation(
    const ZDvidReader &reader, const ZAffineRect &ar, int ccx, int ccy)
{
  ZArray *array = reader.readLabels64Lowtis(ar, 0, ccx, ccy, true);

  ZStack *stack = LabelToColorStack(array);

  return stack;
}

ZStack* flyem::MakeColorSegmentation(
    const ZDvidReader &reader, int x0, int y0, int z0, int width, int height,
    int zoom, int ccx, int ccy)
{
  ZArray *array = reader.readLabels64Lowtis(
        x0, y0, z0, width, height, zoom, ccx, ccy, true);

  ZStack *stack = LabelToColorStack(array);

  return stack;
}


ZCubeArray* flyem::MakeRoiCube(
    const ZObject3dScan &roi, const ZDvidInfo &dvidInfo, QColor color, int dsIntv)
{
  ZCubeArray *cubeArray = new ZCubeArray;
  ZMesh *mesh = MakeRoiMesh(roi, dvidInfo, color, dsIntv);
  cubeArray->setMesh(mesh);

  return cubeArray;
#if 0
  ZObject3dScan dsRoi = roi;
  ZDvidInfo dsInfo = dvidInfo;

  if (dsIntv > 0) {
    dsRoi.downsampleMax(dsIntv, dsIntv, dsIntv);
    dsInfo.downsampleBlock(dsIntv, dsIntv, dsIntv);
  }

//  int sampleInterval = dsIntv;

  ZCubeArray *cubes = new ZCubeArray;
  //For each voxel, create a graph
  int startCoord[3];
  Stack *stack = dsRoi.toStackWithMargin(startCoord, 1, 1);
  //Stack *stack = roi.getSlice(49).toStackWithMargin(startCoord, 1, 1); // rendering only one slice of ROIs for test

  size_t offset = 0;
  int i, j, k;
  int neighbor[26];
  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);
  int cwidth = width - 1;
  int cheight = height - 1;
  int cdepth = depth - 1;

  Stack_Neighbor_Offset(6, C_Stack::width(stack), C_Stack::height(stack), neighbor);
  uint8_t *array = C_Stack::array8(stack);

  //
  //glm::vec4 color = glm::vec4(0.5, 0.25, 0.25, 1.0);

  //
  for (k = 0; k <= cdepth; k ++) {
    for (j = 0; j <= cheight; j++) {
      for (i = 0; i <= cwidth; i++) {
//        bool goodCube = (sampleInterval == 0);
//        if (!goodCube) {
//          goodCube = (k % sampleInterval == 0);
//        }
        bool goodCube = true;
        if (goodCube) {
          if (array[offset] > 0) {
            std::vector<int> faceArray;
            for (int n = 0; n < 6; n++) {
              if (array[offset + neighbor[n]] == 0) {
                int f = n;
//                if (f % 2 == 0) { //Remapping to match face defination (temporary fix)
//                  f += 1;
//                } else {
//                  f -= 1;
//                }
                faceArray.push_back(f);
              }
            }
            if (!faceArray.empty()) {
              ZIntCuboid box = dsInfo.getBlockBox(
                    i + startCoord[0], j + startCoord[1], k + startCoord[2]);
              box.setLastCorner(box.getLastCorner() + ZIntPoint(1, 1, 1));

              qreal r,g,b,a;
              color.getRgbF(&r, &g, &b, &a); // QColor -> glm::vec4

              Z3DCube *cube = cubes->makeCube(box, glm::vec4(r,g,b,a), faceArray);
              cubes->append(*cube);
              delete cube;
            }
          }
        }
        offset++;
      }
    }
  }

  C_Stack::kill(stack);

  //
  return cubes;
#endif
}

ZMesh* flyem::MakeRoiMesh(const ZObject3dScan &roi, QColor color, int dsIntv)
{
  ZObject3dScan dsRoi = roi;

  if (dsIntv > 0) {
    dsRoi.downsampleMax(dsIntv, dsIntv, dsIntv);
  }

  ZMesh *mesh = new ZMesh;

  //For each voxel, create a graph
  int startCoord[3];
  Stack *stack = dsRoi.toStackWithMargin(startCoord, 1, 1);

  size_t offset = 0;
  int i, j, k;
  int neighbor[26];
  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);
  int cwidth = width - 1;
  int cheight = height - 1;
  int cdepth = depth - 1;

  Stack_Neighbor_Offset(6, C_Stack::width(stack), C_Stack::height(stack), neighbor);
  uint8_t *array = C_Stack::array8(stack);

  std::vector<glm::vec3> coordLlfs;
  std::vector<glm::vec3> coordUrbs;
  std::vector<std::vector<bool> > faceVisbility;
  std::vector<glm::vec4> cubeColors;

  qreal r,g,b,a;
  color.getRgbF(&r, &g, &b, &a); // QColor -> glm::vec4

  int bw = dsRoi.getDsIntv().getX() + 1;
  int bh = dsRoi.getDsIntv().getY() + 1;
  int bd = dsRoi.getDsIntv().getZ() + 1;

  for (k = 0; k <= cdepth; k ++) {
    for (j = 0; j <= cheight; j++) {
      for (i = 0; i <= cwidth; i++) {
        bool goodCube = true;
        if (goodCube) {
          if (array[offset] > 0) {
            std::vector<bool> fv(6, false);
            bool visible = false;
            for (int n = 0; n < 6; n++) {
              if (array[offset + neighbor[n]] == 0) {
                fv[n] = true;
                visible = true;
              }
            }
            if (visible) {
              ZIntCuboid box;
              box.setFirstCorner(
                    (i + startCoord[0]) * bw, (j + startCoord[1]) * bh,
                  (k + startCoord[2]) * bd);
              box.setLastCorner(box.getFirstCorner() + ZIntPoint(bw, bh, bd));

              coordLlfs.emplace_back(box.getFirstCorner().getX(),
                                     box.getFirstCorner().getY(),
                                     box.getFirstCorner().getZ());
              coordUrbs.emplace_back(box.getLastCorner().getX(),
                                     box.getLastCorner().getY(),
                                     box.getLastCorner().getZ());
              cubeColors.emplace_back(r, g, b, a);
              faceVisbility.push_back(fv);
            }
          }
        }
        offset++;
      }
    }
  }

  C_Stack::kill(stack);

  mesh->createCubesWithNormal(coordLlfs, coordUrbs, faceVisbility, &cubeColors);


  return mesh;
}

ZMesh* flyem::MakeRoiMesh(
    const ZObject3dScan &roi, const ZDvidInfo &dvidInfo, QColor color, int dsIntv)
{
  ZObject3dScan dsRoi = roi;
  ZDvidInfo dsInfo = dvidInfo;

  if (dsIntv > 0) {
    dsRoi.downsampleMax(dsIntv, dsIntv, dsIntv);
    dsInfo.downsampleBlock(dsIntv, dsIntv, dsIntv);
  }

  ZMesh *mesh = new ZMesh;

  //For each voxel, create a graph
  int startCoord[3];
  Stack *stack = dsRoi.toStackWithMargin(startCoord, 1, 1);

  size_t offset = 0;
  int i, j, k;
  int neighbor[26];
  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);
  int cwidth = width - 1;
  int cheight = height - 1;
  int cdepth = depth - 1;

  Stack_Neighbor_Offset(6, C_Stack::width(stack), C_Stack::height(stack), neighbor);
  uint8_t *array = C_Stack::array8(stack);

  std::vector<glm::vec3> coordLlfs;
  std::vector<glm::vec3> coordUrbs;
  std::vector<std::vector<bool> > faceVisbility;
  std::vector<glm::vec4> cubeColors;

  qreal r,g,b,a;
  color.getRgbF(&r, &g, &b, &a); // QColor -> glm::vec4

  for (k = 0; k <= cdepth; k ++) {
    for (j = 0; j <= cheight; j++) {
      for (i = 0; i <= cwidth; i++) {
        bool goodCube = true;
        if (goodCube) {
          if (array[offset] > 0) {
            std::vector<bool> fv(6, false);
            bool visible = false;
            for (int n = 0; n < 6; n++) {
              if (array[offset + neighbor[n]] == 0) {
                fv[n] = true;
                visible = true;
              }
            }
            if (visible) {
              ZIntCuboid box = dsInfo.getBlockBox(
                    i + startCoord[0], j + startCoord[1], k + startCoord[2]);
              box.setLastCorner(box.getLastCorner() + ZIntPoint(1, 1, 1));

              coordLlfs.emplace_back(box.getFirstCorner().getX(),
                                     box.getFirstCorner().getY(),
                                     box.getFirstCorner().getZ());
              coordUrbs.emplace_back(box.getLastCorner().getX(),
                                     box.getLastCorner().getY(),
                                     box.getLastCorner().getZ());
              cubeColors.emplace_back(r, g, b, a);
              faceVisbility.push_back(fv);
            }
          }
        }
        offset++;
      }
    }
  }

  C_Stack::kill(stack);

  mesh->createCubesWithNormal(coordLlfs, coordUrbs, faceVisbility, &cubeColors);


  return mesh;
}

void flyem::Decorate3dBodyWindowPlane(Z3DWindow *window, const ZDvidInfo &dvidInfo,
    const ZStackViewParam &viewParam, bool visible)
{
  if (window != NULL) {
    ZRect2d rect;
    rect.setZ(viewParam.getZ());
    //    rect.setZ(getCurrentZ());
    rect.setFirstCorner(dvidInfo.getStartCoordinates().getX(),
                        dvidInfo.getStartCoordinates().getY());
    rect.setLastCorner(dvidInfo.getEndCoordinates().getX(),
                       dvidInfo.getEndCoordinates().getY());
    ZCuboid box;
    box.setFirstCorner(dvidInfo.getStartCoordinates().toPoint());
    box.setLastCorner(dvidInfo.getEndCoordinates().toPoint());
//    double lineWidth = box.depth() / 2000.0;
    double lineWidth = 4.0;
    Z3DGraph *graph = Z3DGraphFactory::MakeGrid(rect, 50, lineWidth);

    if (viewParam.getViewPort().isValid()) {
      Z3DGraphNode node1;
      Z3DGraphNode node2;

      node1.setColor(QColor(0, 255, 0));
      node2.setColor(QColor(0, 255, 0));

      double x = viewParam.getViewPort().center().x();
      double y = viewParam.getViewPort().center().y();

      double width = lineWidth * 0.5;
      node1.set(x, rect.getFirstY(), rect.getZ(), width);
      node2.set(x, rect.getLastY(), rect.getZ(), width);

      double edgeWidth = NeutubeConfig::Get3DCrossWidth();
      graph->addNode(node1);
      graph->addNode(node2);
      graph->addEdge(node1, node2, edgeWidth, GRAPH_LINE);

      node1.setColor(QColor(255, 0, 0));
      node2.setColor(QColor(255, 0, 0));
      node1.set(rect.getFirstX(), y, rect.getZ(), width);
      node2.set(rect.getLastX(), y, rect.getZ(), width);

      graph->addNode(node1);
      graph->addNode(node2);
      graph->addEdge(node1, node2, edgeWidth, GRAPH_LINE);
    }

    graph->setSource(ZStackObjectSourceFactory::MakeFlyEmPlaneObjectSource());
    graph->setVisible(visible);
    window->getDocument()->addObject(graph, true);
#if 0
    ZStackObject *replaced =
        window->getDocument()->getObjectGroup().replaceFirstSameSource(graph);
    if (replaced == NULL) {
      window->getDocument()->addObject(graph, true);
    } else {
      window->getDocument()->beginObjectModifiedMode(ZStackDoc::EObjectModifiedMode::OBJECT_MODIFIED_CACHE);
      window->getDocument()->bufferObjectModified(replaced);
      window->getDocument()->bufferObjectModified(graph);
      delete replaced;
      window->getDocument()->endObjectModifiedMode();
      window->getDocument()->notifyObjectModified();
    }
#endif
  }
}


void flyem::Decorate3dBodyWindow(
    Z3DWindow *window, const ZDvidInfo &dvidInfo,
    const ZStackViewParam &viewParam, bool visible)
{
  if (window != NULL) {
    Decorate3dBodyWindowPlane(window, dvidInfo, viewParam, visible);
    ZCuboid box;
    box.setFirstCorner(dvidInfo.getStartCoordinates().toPoint());
    box.setLastCorner(dvidInfo.getEndCoordinates().toPoint());
    double radius =
        dmax2(1.0, dmax3(box.width(), box.height(), box.depth()) / 1000.0);
    if (!visible) {
      radius = 0.0;
    }
    Z3DGraph *graph = Z3DGraphFactory::MakeBox(box, radius);
    graph->setSource(ZStackObjectSourceFactory::MakeFlyEmBoundBoxSource());
//    if (window->getWindowType() == neutube3d::EWindowType::TYPE_NEU3) {
//      graph->setVisible(visible);
//    }

    window->getDocument()->addObject(graph, true);
    window->resetCamera();
    if (window->isBackgroundOn()) {
      window->setOpacity(neutu3d::ERendererLayer::GRAPH, 0.4);
    }
  }
}

void flyem::Decorate3dBodyWindowRoi(
    Z3DWindow *window, const ZDvidInfo &dvidInfo, const ZDvidTarget &dvidTarget)
{
  if (window != NULL) {
    const std::vector<std::string> &roiList = dvidTarget.getRoiList();
    if (!roiList.empty()) {
      ZDvidReader reader;
      if (reader.open(dvidTarget)) {
        ZColorScheme colorScheme;
        colorScheme.setColorScheme(ZColorScheme::UNIQUE_COLOR);
        int index = 0;
        for (std::vector<std::string>::const_iterator iter = roiList.begin();
             iter != roiList.end(); ++iter, ++index) {
          ZObject3dScan roi = reader.readRoi(*iter);
          roi.setColor(colorScheme.getColor(index));
          if (!roi.isEmpty()) {
            Z3DGraph *graph = MakeRoiGraph(roi, dvidInfo);
//            graph->setColor(colorScheme.getColor(index));
//            graph->syncNodeColor();
            graph->setSource(
                  ZStackObjectSourceFactory::MakeFlyEmRoiSource(*iter));
            window->getDocument()->addObject(graph, true);
          }
        }
      }
    }
  }
}

void flyem::Decorate3dBodyWindowRoiCube(
    Z3DWindow *window, const ZDvidInfo &/*dvidInfo*/, const ZDvidTarget &dvidTarget)
{
  if (window != NULL) {
/*
    ZDvidReader reader;
    if (reader.open(dvidTarget)) {
      const std::vector<std::string>& roiList = dvidTarget.getRoiList();
      for (std::vector<std::string>::const_iterator iter = roiList.begin();
           iter != roiList.end(); ++iter) {
        const std::string &roiName = *iter;
        ZObject3dScan roi = reader.readRoi(roiName);
        if (!roi.isEmpty()) {
          ZDvidInfo info = dvidInfo;
          info.downsampleBlock(1, 1, 1);
          roi.downsample(1, 1, 1);

          ZCubeArray *cubes = MakeRoiCube(roi, info);
          cubes->setSource(
                ZStackObjectSourceFactory::MakeFlyEmRoiSource(roiName));
          window->getDocument()->addObject(cubes, true);
        }
*/
//    if (!dvidTarget.getRoiName().empty()) {
      ZDvidReader reader;
      if (reader.open(dvidTarget)) {

          // test
          ZJsonObject meta = reader.readInfo();
//          std::vector<std::string> keys = meta.getAllKey();

//          for(int i=0; i<keys.size(); i++)
//          {
//              qDebug()<<keys.at(i);
//          }
//          qDebug()<<"~~~~~~~~~~~~ test dvid roi reading ~~~~~~~~~~~~~"<<dvidTarget.getRoiName();


          ZJsonValue datains = meta.value("DataInstances");
          qDebug()<<"~~~~~~"<<datains.isObject()<<datains.isArray()<<datains.isString();

          if(datains.isObject())
          {
              ZJsonObject insList(datains);
              std::vector<std::string> keys = insList.getAllKey();

              for(size_t i=0; i<keys.size(); i++)
              {
                  //qDebug()<<keys.at(i);


                  std::size_t found = keys.at(i).find("roi");

                  if(found!=std::string::npos)
                  {
                    qDebug()<<" rois: "<<keys.at(i);
                  }


//                  ZJsonObject submeta(insList.value(keys.at(i).c_str()));
//                  std::vector<std::string> subkeys = submeta.getAllKey();

//                  for(int j=0; j<subkeys.size(); j++)
//                  {
//                      qDebug()<<" ... "<<subkeys.at(i);
//                  }






              }
              qDebug()<<"~~~~~~~~~~~~ test dvid roi reading ~~~~~~~~~~~~~";
          }






//       ZObject3dScan roi = reader.readRoi(dvidTarget.getRoiName());
//        if (!roi.isEmpty()) {
//          ZCubeArray *cubes = MakeRoiCube(roi, dvidInfo);
//          cubes->setSource(
//                ZStackObjectSourceFactory::MakeFlyEmRoiSource(
//                  dvidTarget.getRoiName()));
//          window->getDocument()->addObject(cubes, true);
//        }
      }
    }
//  }
}

void flyem::SubtractBodyWithBlock(
    ZObject3dScan *body, const ZObject3dScan &coarsePart,
    const ZDvidInfo& dvidInfo)
{
  if (body != NULL) {
//    tic();
    ZObject3dScan expandedPart;

    ZObject3dScan::ConstSegmentIterator segIter(&coarsePart);
    while (segIter.hasNext()) {
      const ZObject3dScan::Segment &seg = segIter.next();

      ZIntCuboid box = dvidInfo.getBlockBox(
            seg.getStart(), seg.getEnd(), seg.getY(), seg.getZ());

      expandedPart.concat(ZObject3dFactory::MakeObject3dScan(box));
    }

    body->subtract(expandedPart);
//    std::cout << "Subtracting time: " << toc() << std::endl;
  }
}

void flyem::MakeStar(
    const QPointF &center, double radius, QPointF *ptArray)
{
  const double shapeFactor = 0.25;
  double sw = radius * shapeFactor;
  double left = center.x() - radius;
  double right = center.x() + radius;
  double top = center.y() - radius;
  double bottom = center.y() + radius;

  ptArray[0] = QPointF(center.x(), top);
  ptArray[1] = QPointF(center.x() + sw, center.y() - sw);
  ptArray[2] = QPointF(right, center.y());
  ptArray[3] = QPointF(center.x() + sw, center.y() + sw);
  ptArray[4] = QPointF(center.x(), bottom);
  ptArray[5] = QPointF(center.x() - sw, center.y() + sw);
  ptArray[6] = QPointF(left, center.y());
  ptArray[7] = QPointF(center.x() - sw, center.y() - sw);
  ptArray[8] = ptArray[0];
}

void flyem::MakeStar(const QRectF &rect, QPointF *ptArray)
{
  QPointF center = rect.center();
  double width= rect.width();
  double height = rect.height();
  const double shapeFactor = 0.125;
  double sw = width * shapeFactor;
  double sh = height * shapeFactor;
  ptArray[0] = QPointF(center.x(), rect.top());
  ptArray[1] = QPointF(center.x() + sw, center.y() - sh);
  ptArray[2] = QPointF(rect.right(), center.y());
  ptArray[3] = QPointF(center.x() + sw, center.y() + sh);
  ptArray[4] = QPointF(center.x(), rect.bottom());
  ptArray[5] = QPointF(center.x() - sw, center.y() + sh);
  ptArray[6] = QPointF(rect.left(), center.y());
  ptArray[7] = QPointF(center.x() - sw, center.y() - sh);
  ptArray[8] = ptArray[0];
}

void flyem::PrepareBodyStatus(QComboBox *box)
{
  if (box != NULL) {
    box->clear();
    box->addItem("---");
    box->addItem("Not examined");
    box->addItem("Traced");
    box->addItem("Traced in ROI");
    box->addItem("Partially traced");
    box->addItem("Orphan");
    box->addItem("Hard to trace");
    box->addItem("Finalized");
  }
}

QList<QString> flyem::GetDefaultBodyStatus()
{
  return QList<QString>() << "Not examined" << "Traced" << "Traced in ROI"
                          << "Partially traced" << "Orphan" << "Hard to trace"
                          << "Finalized";
}

void flyem::MakeTriangle(
    const QRectF &rect, QPointF *ptArray, neutu::ECardinalDirection direction)
{
  switch (direction) {
  case neutu::ECardinalDirection::EAST:
    ptArray[0] = QPointF(rect.right(), rect.center().y());
    ptArray[1] = rect.topLeft();
    ptArray[2] = rect.bottomLeft();
    break;
  case neutu::ECardinalDirection::WEST:
    ptArray[0] = QPointF(rect.left(), rect.center().y());
    ptArray[1] = rect.topRight();
    ptArray[2] = rect.bottomRight();
    break;
  case neutu::ECardinalDirection::NORTH:
    ptArray[0] = QPointF(rect.center().x(), rect.top());
    ptArray[1] = rect.bottomLeft();
    ptArray[2] = rect.bottomRight();
    break;
  case neutu::ECardinalDirection::SOUTH:
    ptArray[0] = QPointF(rect.center().x(), rect.bottom());
    ptArray[1] = rect.topLeft();
    ptArray[2] = rect.topRight();
    break;
  }

  ptArray[3] = ptArray[0];
}

QString flyem::GetMemoryUsage()
{
  QString memInfo;

#if defined(__APPLE__) || defined(_LINUX_)
  QProcess p;

#if defined(__APPLE__)
  p.start(QString("top -l 1 -pid %1").arg(getpid()));
#else
  p.start(QString("ps v -p %1").arg(getpid()));
#endif

  if (p.waitForFinished(1000)) {
    QString output = p.readAllStandardOutput();
    QStringList lines = output.split("\n", QString::SkipEmptyParts);
//    qDebug() << lines;
    QString fieldLine;
    QString infoLine;
    for (int i = 0; i < lines.size(); ++i) {
      QString line = lines[i].trimmed();
      if (line.startsWith("PID")) {
        fieldLine = line;
        if (i + 1 < lines.size()) {
          infoLine = lines[i + 1];
        }
        break;
      }
    }

    QStringList fields = fieldLine.split(" ", QString::SkipEmptyParts);
//    qDebug() << fields;
    QStringList infoList = infoLine.split(" ", QString::SkipEmptyParts);
//    qDebug() << infoList;


    for (int i = 0; i < fields.size(); ++i) {
#if defined(__APPLE__)
      if (fields[i] == "MEM" || fields[i] == "RPRVT") {
#else
      if (fields[i] == "RSS") {
#endif
        if (infoList.size() > i) {
          memInfo = infoList[i];
        }
        break;
      }
    }

//    qDebug() << "Memory usage:" << memInfo;
  } else {
    qDebug() << p.readAllStandardError();
  }

  p.close();
#endif
  return memInfo;
}

ZStroke2d* flyem::MakeSplitSeed(const ZObject3dScan &slice, int label)
{
  ZVoxel voxel = slice.getMarker();
  ZStroke2d *stroke = new ZStroke2d;
  stroke->setWidth(10);
  stroke->setZ(voxel.z());
  stroke->append(voxel.x(), voxel.y());
  stroke->addRole(ZStackObjectRole::ROLE_SEED);
  stroke->setLabel(label);
  stroke->setPenetrating(false);

  return stroke;
}

std::vector<ZStroke2d*> flyem::MakeSplitSeedList(const ZObject3dScan &obj)
{
  std::vector<ZStroke2d*> seedList;

  ZVoxel voxel = obj.getMarker();

  ZObject3dScan slice = obj.getSlice(voxel.z());
  seedList.push_back(flyem::MakeSplitSeed(slice, 1));

  slice = obj.getSlice(voxel.z() + 1);
  seedList.push_back(flyem::MakeSplitSeed(slice, 2));

  return seedList;
}

void flyem::SetSplitTaskSignalUrl(
    ZJsonObject &taskObj, uint64_t bodyId, const ZDvidTarget &target)
{
  ZDvidUrl dvidUrl(target);
  std::string signalUrl = dvidUrl.getSparsevolUrl(bodyId);
  if (!signalUrl.empty()) {
    taskObj.setEntry("signal", signalUrl);
  }
}

ZStroke2d flyem::SyGlassSeedToStroke(const ZJsonObject &obj)
{
  ZStroke2d stroke;
  stroke.setLabel(0);
  if (obj.hasKey("color")) {
    stroke.setLabel(obj.value("color").toInteger() + 1);
    stroke.setZ(obj.value("z").toInteger());
    stroke.append(obj.value("x").toInteger(), obj.value("y").toInteger());
    stroke.setWidth(30);
  }

  return stroke;
}

ZStroke2d flyem::SyGlassSeedToStroke(
    const ZJsonObject &obj, const ZIntPoint &offset, const ZIntPoint &dsIntv)
{
  ZStroke2d stroke = SyGlassSeedToStroke(obj);

  stroke.translate(offset);
  stroke.scale(dsIntv.getX() + 1, dsIntv.getY() + 1, dsIntv.getZ() + 1);

  return stroke;
}

ZJsonObject flyem::MakeSplitSeedJson(const ZStroke2d &stroke)
{
  ZJsonObject json;

  json.setEntry("label", stroke.getLabel());
  json.setEntry("type", "ZStroke2d");
  ZJsonObject obj = stroke.toJsonObject();
  json.setEntry("data", obj);

  return json;
}

ZJsonObject flyem::MakeSplitSeedJson(const ZObject3d &seed)
{
  ZJsonObject json;

  json.setEntry("label", seed.getLabel());
  json.setEntry("type", "ZObject3d");
  ZJsonObject obj = seed.toJsonObject();
  json.setEntry("data", obj);

  return json;
}

template<typename T>
void flyem::AddSplitTaskSeedG(ZJsonObject &taskObj, const T& obj)
{
  ZJsonArray array;

  if (!taskObj.hasKey("seeds")) {
    taskObj.setEntry("seeds", array);
  } else {
    array = ZJsonArray(taskObj.value("seeds"));
  }

  array.append(MakeSplitSeedJson(obj));
}

void flyem::AddSplitTaskSeed(ZJsonObject &taskObj, const ZStroke2d &stroke)
{
  AddSplitTaskSeedG(taskObj, stroke);
}

void flyem::AddSplitTaskSeed(ZJsonObject &taskObj, const ZObject3d &obj)
{
  AddSplitTaskSeedG(taskObj, obj);
}

ZJsonArray flyem::GetSeedJson(ZStackDoc *doc)
{
  QList<ZDocPlayer*> playerList =
      doc->getPlayerList(ZStackObjectRole::ROLE_SEED);
  ZJsonArray jsonArray;
  foreach (const ZDocPlayer *player, playerList) {
    ZJsonObject jsonObj = player->toSeedJson();
    if (!jsonObj.isEmpty()) {
      jsonArray.append(jsonObj);
    }
  }

  return jsonArray;
}

void flyem::UploadSyGlassTask(
    const std::string &filePath, const ZDvidTarget &target)
{
  ZJsonArray rootObj;
  rootObj.load(filePath);

  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriterFromUrl(
        GET_FLYEM_CONFIG.getTaskServer());
  ZDvidUrl dvidUrl(target);

  for (size_t i = 0; i < rootObj.size(); ++i) {
    ZJsonObject obj(rootObj.value(i));
    ZJsonArray markerJson(obj.value("pointMarkers"));
    if (!markerJson.isEmpty()) {
      uint64_t bodyId =
          ZString(obj.value("file").toString()).firstUint64();

      if (bodyId > 0) {
        ZJsonObject taskJson;
        flyem::SetSplitTaskSignalUrl(taskJson, bodyId, target);

        for (size_t i = 0; i < markerJson.size(); ++i) {
          ZJsonObject markerObj(markerJson.value(i));
          ZStroke2d stroke =
              flyem::SyGlassSeedToStroke(markerObj);
          flyem::AddSplitTaskSeed(taskJson, stroke);
        }
        std::string location = writer->writeServiceTask("split", taskJson);

        ZJsonObject entryJson;
        entryJson.setEntry(neutu::json::REF_KEY, location);
        QString taskKey = dvidUrl.getSplitTaskKey(bodyId).c_str();
        writer->writeSplitTask(taskKey, taskJson);
      }
    }
  }
}

QString flyem::ReadLastLines(const QString &filePath, int maxCount)
{
  QString str;

  QFile file(filePath);

  if (file.exists()) {
    file.open(QFile::ReadOnly);

    file.seek(file.size() - 1);

    int count = 0;

    while ((count <= maxCount) && (file.pos() > 0))
    {
      char ch;
      file.getChar(&ch);
      file.seek(file.pos() - 2);
      if (ch == '\n') {
        count++;
      }
    }

    str = file.readAll();

    file.close();
  }

  return str;
}

ZIntCuboid flyem::EstimateSplitRoi(const ZIntCuboid &boundBox)
{
  ZIntCuboid newBox = boundBox;

  newBox.expandZ(10);
  size_t v = newBox.getVolume();

//  double s = Cube_Root(ZSparseStack::GetMaxStackVolume() / 2 / v);
  double s = Cube_Root(neutu::BIG_STACK_VOLUME_HINT / 2 / v);
  if (s > 1) {
    double ds = s - 1.0;
    int dw = iround(newBox.getWidth() * ds);
    int dh = iround(newBox.getHeight() * ds);
    int dd = iround(newBox.getDepth() * ds);

    const int xMargin = dw / 2;
    const int yMargin = dh / 2;
    const int zMargin = dd / 2;
    newBox.expandX(xMargin);
    newBox.expandY(yMargin);
    newBox.expandZ(zMargin);
  }

  return newBox;
}

QString flyem::GetNeuroglancerPath(
    const ZDvidTarget &target, const ZIntPoint &pos, const ZWeightedPoint &quat,
    const QSet<uint64_t> &bodySet)
{
  if (GET_FLYEM_CONFIG.getNeuroglancerServer().empty()) {
    return "";
  }


  QString path = QString("http://%1/neuroglancer/#!{'layers':"
                         "{'grayscale':{'type':'image'_'source':'dvid://"
                         "http://%2/%3/%4'}").
      arg(GET_FLYEM_CONFIG.getNeuroglancerServer().c_str()).
      arg(target.getGrayScaleSource().getAddressWithPort().c_str()).
      arg(target.getGrayScaleSource().getUuid().c_str()).
      arg(target.getGrayScaleName().c_str());

  if (target.hasSegmentation()) {
    path += QString("_'segmentation':{'type':'segmentation'_"
                    "'source':'dvid://http://%1/%2/%3'").
        arg(target.getAddressWithPort().c_str()).
        arg(target.getUuid().c_str()).
        arg(target.getSegmentationName().c_str());

    if (!bodySet.empty() && bodySet.size() < 4) {
      path += "_'segments':[";
      uint64_t firstId = *(bodySet.begin());
      path += QString("'%1'").arg(firstId);

      foreach (uint64_t bodyId, bodySet) {
        if (bodyId != firstId) {
          path += QString("_'%1'").arg(bodyId);
        }
      }
      path += "]";
    }
    path += "}";
  }

  path += QString("}_'navigation':{'pose':{'position':"
                  "{'voxelSize':[8_8_8]_'voxelCoordinates':[%1_%2_%3]}_"
                  "'orientation':[%4_%5_%6_%7]}_"
                  "'zoomFactor':8}_"
                  "'perspectiveOrientation':"
                  "[%4_%5_%6_%7]_"
                  "'perspectiveZoom':64_'layout':'xy'}").
      arg(pos.getX()).arg(pos.getY()).arg(pos.getZ()).
      arg(quat.getX()).arg(quat.getY()).arg(quat.getZ()).arg(quat.weight());

  return path;
}

#if 0
namespace {

ZIntPoint load_point_from_json_zyx(const ZJsonArray &v)
{
  ZIntPoint pt;

  pt.setX(ZJsonParser::integerValue(v, 0));
  pt.setX(ZJsonParser::integerValue(v, 1));
  pt.setX(ZJsonParser::integerValue(v, 2));

  return pt;
}

}
#endif

ZObject3dScan* flyem::LoadRoiFromJson(const std::string &filePath)
{
  ZObject3dScan *sobj = nullptr;

  ZJsonObject obj;
  obj.load(filePath);
  if (ZJsonParser::stringValue(obj["type"]) == "points" && obj.hasKey("roi")
      && ZJsonParser::stringValue(obj["order"]) == "zyx") {
    if (obj.hasKey("resolution")) {
      int res = ZJsonParser::integerValue(obj["resolution"]);
      if (res > 0) {
        sobj = new ZObject3dScan;
        sobj->setSource(filePath);
        sobj->setDsIntv(res - 1);
        ZJsonArray roiJson(obj.value("roi"));
        ZObject3dScan::Appender appender(sobj);
        for (size_t i = 0; i < roiJson.size(); ++i) {
          ZJsonArray v(roiJson.value(i));
          if (v.size() == 3) {
//            ZIntPoint pt = load_point_from_json_zyx(v);
            int z = ZJsonParser::integerValue(v.at(0));
            int y = ZJsonParser::integerValue(v.at(1));
            int x = ZJsonParser::integerValue(v.at(2));
            appender.addSegment(z, y, x, x);
          }
        }
      }
    }
  }

  return sobj;
}

/*
void flyem::UpdateBodyStatus(
    const ZIntPoint &pos, const std::string &newStatus, ZDvidWriter *writer)
{
  if (writer) {
    uint64_t bodyId = writer->getDvidReader().readBodyIdAt(pos);
    ZFlyEmBodyAnnotation annot = writer->getDvidReader().readBodyAnnotation(bodyId);
#ifdef _DEBUG_
    std::cout << "Old annotation:" << std::endl;
    annot.print();
#endif
    annot.setStatus(newStatus);
    writer->writeBodyAnntation(annot);

#ifdef _DEBUG_
    ZFlyEmBodyAnnotation newAnnot = writer->getDvidReader().readBodyAnnotation(bodyId);
    std::cout << "New annotation:" << std::endl;
    newAnnot.print();
#endif
  }
}
*/
void flyem::UploadRoi(
    const QString &dataDir, const QString &roiNameFile, ZDvidWriter *writer)
{
  if (writer) {
    std::ifstream stream(roiNameFile.toStdString());

    std::string line;
    std::unordered_map<std::string, std::string> nameMap;
    while (std::getline(stream, line)) {
      std::cout << line << std::endl;
      std::vector<std::string> tokens = ZString::Tokenize(line, ',');
      for (auto &str : tokens) {
        std::cout << str << " --- ";
      }
      std::cout << std::endl;
      if (tokens.size() >= 3) {
        ZString name(tokens[2]);
        name.trim();
        nameMap[tokens[1] + ".obj"] = name;
        nameMap[tokens[1] + ".sobj"] = name;
      }
    }

//    for (auto &entry : nameMap) {
//      std::cout << entry.first << ": " << entry.second << std::endl;
//    }

    //Upload: ZDvidWriter::uploadRoiMesh
    {
      QDir dir(dataDir);
      QStringList fileList = dir.entryList(QStringList() << "*.obj");
      for (QString &file : fileList) {
        std::string name = nameMap[file.toStdString()];
        std::cout << file.toStdString() << ": " << name << std::endl;
        writer->uploadRoiMesh((dataDir + "/" + file).toStdString(), name);
        //    break;
      }
    }

    //Write ROI data: ZDvidWriter::writeRoi

    {
      QDir dir(dataDir);
      QStringList fileList = dir.entryList(QStringList() << "*.sobj");
      for (QString &file : fileList) {
        std::string name = nameMap[file.toStdString()];
        std::cout << file.toStdString() << ": " << name << std::endl;
        ZObject3dScan roi;
        roi.load((dataDir + "/" + file).toStdString());
        if (!writer->getDvidReader().hasData(name)) {
          writer->createData("roi", name);
        }
        writer->writeRoi(roi, name);
      }
    }
  }
}

void flyem::UpdateSupervoxelMesh(ZDvidWriter &writer, uint64_t svId)
{
  ZObject3dScan obj;
  writer.getDvidReader().readSupervoxel(svId, true, &obj);
  ZMeshFactory factory;
  ZMesh *mesh = factory.makeMesh(obj);
  if (mesh != NULL) {
    writer.writeSupervoxelMesh(*mesh, svId);
  }

  delete mesh;
}

std::vector<uint64_t> flyem::LoadBodyList(const std::string &input)
{
  std::vector<uint64_t> bodyList;

  FILE *fp = fopen(input.c_str(), "r");
  if (fp != NULL) {
    ZString str;
    while (str.readLine(fp)) {
      std::vector<uint64_t> bodyArray = str.toUint64Array();
      std::copy(bodyArray.begin(), bodyArray.end(), std::back_inserter(bodyList));
//      bodyList.insert(bodyArray.begin(), bodyArray.end());
    }
    fclose(fp);
  } else {
    std::cout << "Failed to open " << input << std::endl;
  }

  return bodyList;
}

ZStack* flyem::GenerateExampleStack(const ZJsonObject &obj)
{
  ZDvidTarget target;
  target.loadJsonObject(ZJsonObject(obj.value("dvid")));

  ZStack *stack = NULL;

  ZDvidReader reader;
  if (reader.open(target)) {
    ZString bodyStr = ZJsonParser::stringValue(obj["body"]);
    std::vector<uint64_t> bodyIdArray = bodyStr.toUint64Array();
    uint64_t bodyId = bodyIdArray.front();

    ZIntCuboid box;
    if (obj.hasKey("range")) {
      box.loadJson(ZJsonArray(obj.value("range")));
    }

    ZDvidSparseStack *spStack = reader.readDvidSparseStack(bodyId, box);
    spStack->shakeOff();
    stack = spStack->makeIsoDsStack(MAX_INT32, false);

    delete spStack;
  }

  return stack;
}


ZStack* flyem::GenerateExampleStack(
    const ZDvidTarget &target, uint64_t bodyId, const ZIntCuboid &range)
{
  ZStack *stack = NULL;

  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader(target);
  if (reader != NULL) {
    ZDvidSparseStack *spStack = reader->readDvidSparseStack(bodyId, range);
    spStack->shakeOff();
    stack = spStack->makeIsoDsStack(MAX_INT32, false);

    delete spStack;
  }

  return stack;
}

QList<ZStackObject*> flyem::LoadSplitTask(const ZJsonObject &taskJson)
{
  ZJsonArray seedArrayJson(taskJson.value("seeds"));
  QList<ZStackObject*> seedList;
  for (size_t i = 0; i < seedArrayJson.size(); ++i) {
    ZJsonObject seedJson(seedArrayJson.value(i));
    if (seedJson.hasKey("type")) {
//      std::string seedUrl = ZJsonParser::stringValue(seedJson["url"]);
      std::string type = ZJsonParser::stringValue(seedJson["type"]);
      if (type == "ZStroke2d") {
        ZStroke2d *stroke = new ZStroke2d;
        stroke->loadJsonObject(ZJsonObject(seedJson.value("data")));

        if (!stroke->isEmpty()) {
          seedList.append(stroke);
        } else {
          delete stroke;
        }
      } else if (type == "ZObject3d") {
        ZObject3d *obj = new ZObject3d;
        obj->loadJsonObject(ZJsonObject(seedJson.value("data")));
        if (!obj->isEmpty()) {
          seedList.append(obj);
        } else {
          delete obj;
        }
      }
    }
  }
  foreach (ZStackObject *seed, seedList) {
    seed->addRole(ZStackObjectRole::ROLE_SEED |
                  ZStackObjectRole::ROLE_3DGRAPH_DECORATOR);
    ZLabelColorTable colorTable;
    seed->setColor(colorTable.getColor(seed->getLabel()));
  }

  return seedList;
}

QList<ZStackObject*> flyem::LoadSplitTask(
    const ZDvidTarget &target, uint64_t bodyId)
{
  ZDvidUrl dvidUrl(target);
  std::string taskKey =dvidUrl.getSplitTaskKey(bodyId);
  ZDvidReader *reader =
      ZGlobal::GetInstance().getDvidReaderFromUrl(
        GET_FLYEM_CONFIG.getTaskServer());
  QList<ZStackObject*> seedList;
  if (reader != NULL) {
    ZJsonObject taskJson =
        reader->readJsonObjectFromKey(ZDvidData::GetTaskName("split").c_str(),
                                      taskKey.c_str());
    if (taskJson.hasKey(neutu::json::REF_KEY)) {
      taskJson =
          reader->readJsonObject(
            ZJsonParser::stringValue(taskJson[neutu::json::REF_KEY]));
    }

    seedList = LoadSplitTask(taskJson);
    foreach (ZStackObject *seed, seedList) {
      seed->setSource(ZStackObjectSourceFactory::MakeFlyEmSeedSource(bodyId));
    }
  }

  return seedList;
}

void flyem::RemoveSplitTask(const ZDvidTarget &target, uint64_t bodyId)
{
  ZDvidUrl dvidUrl(target);
  std::string taskKey =dvidUrl.getSplitTaskKey(bodyId);
  ZDvidWriter *writer =
      ZGlobal::GetInstance().getDvidWriterFromUrl(GET_FLYEM_CONFIG.getTaskServer());
  writer->deleteKey(ZDvidData::GetTaskName("split"), taskKey);
}

ZJsonObject flyem::MakeSplitTask(
    const ZDvidTarget &target, uint64_t bodyId,
    ZJsonArray seedJson, ZJsonArray roiJson)
{
  ZJsonObject task;

  ZDvidUrl dvidUrl(target);
  std::string bodyUrl = dvidUrl.getSparsevolUrl(bodyId);
  task.setEntry("signal", bodyUrl);
  task.setEntry("seeds", seedJson);

  if (!roiJson.isEmpty()) {
    task.setEntry("range", roiJson);
  }

  if (target.hasGrayScaleData()) {
    ZJsonObject signalInfo;
    signalInfo.setEntry(
          ZDvidTarget::m_grayScaleNameKey, target.getGrayScaleName());
    ZJsonObject sourceConfig = target.getSourceConfigJson();
    if (!sourceConfig.isEmpty()) {
      signalInfo.setEntry(ZDvidTarget::m_sourceConfigKey, sourceConfig);
    }
    task.setEntry("signal info", signalInfo);
  }

  return task;
}

ZDvidReader* flyem::GetTaskReader()
{
  ZDvidReader *reader =
      ZGlobal::GetInstance().getDvidReaderFromUrl(
        GET_FLYEM_CONFIG.getTaskServer());

  return reader;
}

ZDvidWriter* flyem::GetTaskWriter()
{
  ZDvidWriter *writer =
      ZGlobal::GetInstance().getDvidWriterFromUrl(
        GET_FLYEM_CONFIG.getTaskServer());

  return writer;
}

bool flyem::IsTaskOpen(const QString &taskKey)
{
  ZDvidReader *reader = GetTaskReader();
  if (reader != NULL) {
    if (reader->hasKey("task_test", taskKey)) {
      if (!reader->hasKey("result_test", taskKey)) {
        return true;
      }
    }
  }

  return false;
}


QSet<uint64_t> flyem::MB6Paper::ReadBodyFromSequencer(const QString &filePath)
{
  QStringList fileList;
  fileList << filePath;

  return ReadBodyFromSequencer(fileList);
}

QSet<uint64_t> flyem::MB6Paper::ReadBodyFromSequencer(
    const QDir &dir, const QStringList &fileList)
{
  QStringList fullFilePathList;
  foreach (const QString &filePath, fileList) {
    fullFilePathList << dir.absoluteFilePath(filePath);
  }

  return ReadBodyFromSequencer(fullFilePathList);
}

QSet<uint64_t> flyem::MB6Paper::ReadBodyFromSequencer(
    const QDir &dir, const QString &filePath)
{
  return ReadBodyFromSequencer(dir.absoluteFilePath(filePath));
}

QSet<uint64_t> flyem::MB6Paper::ReadBodyFromSequencer(
    const QStringList &fileList)
{
  QSet<uint64_t> bodySet;
  foreach (const QString &filePath, fileList) {
    FILE *fp = fopen(filePath.toStdString().c_str(), "r");
    if (fp != NULL) {
      ZString str;
      while (str.readLine(fp)) {
        if (!str.startsWith("Body")) {
          std::vector<uint64_t> numArray = str.toUint64Array();
          if (!numArray.empty()) {
            bodySet.insert(numArray[0]);
          }
        }
      }

      fclose(fp);
    }
  }

  return bodySet;
}

QString flyem::FIB19::GetRootDir()
{
  return (GET_FLYEM_DATA_DIR + "/FIB/FIB19/movies").c_str();
}

QString flyem::FIB19::GetMovieDir(const QString &folder)
{
  return QDir(GetRootDir()).absoluteFilePath(folder);
}

ZDvidTarget flyem::FIB19::MakeDvidTarget()
{
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "@FIB19", 7000);
  target.useDefaultDataSetting(true);

  return target;
}

QString flyem::FIB19::GenerateFIB19VsSynapseCast(
    const QString &movieDir, const QString &neuronType)
{
  QString errMsg;

  ZDvidReader reader;
  if (reader.open(MakeDvidTarget())) {
    QDir outDir(movieDir + "/cast");

    ZJsonObject neuronJson;
    neuronJson.load((movieDir + "/neuron_list.json").toStdString());

    double radius = 30;

    ZJsonArray neuronArrayJson(neuronJson.value(neuronType.toStdString().c_str()));
    for (size_t i = 0; i < neuronArrayJson.size(); ++i) {
      ZJsonObject obj(neuronArrayJson.value(i));
      if (obj.hasKey("pos")) {
        ZIntPoint pos = ZJsonParser::toIntPoint(obj["pos"]);
        uint64_t bodyId = reader.readBodyIdAt(pos);
        QString name = ZJsonParser::stringValue(obj["name"]).c_str();
        std::vector<ZDvidSynapse> synapseArray = reader.readSynapse(
              bodyId, dvid::EAnnotationLoadMode::NO_PARTNER);
//            reader.readSynapse(
//              bodyId, dvid::EAnnotationLoadMode::NO_PARTNER);
        std::vector<ZVaa3dMarker> preMarkerArray;
        std::vector<ZVaa3dMarker> postMarkerArray;
        for (std::vector<ZDvidSynapse>::const_iterator iter = synapseArray.begin();
             iter != synapseArray.end(); ++iter) {
          const ZDvidSynapse &synapse = *iter;
          ZVaa3dMarker marker = synapse.toVaa3dMarker(radius);
          if (synapse.getKind() == ZDvidAnnotation::EKind::KIND_POST_SYN) {
            marker.setColor(255, 255, 255);
            postMarkerArray.push_back(marker);
          } else if (synapse.getKind() == ZDvidAnnotation::EKind::KIND_PRE_SYN) {
            marker.setColor(255, 255, 0);
            preMarkerArray.push_back(marker);
          }
        }
        flyem::ZFileParser::writeVaa3dMakerFile(
              outDir.absoluteFilePath(name + ".pre.marker").toStdString(),
              preMarkerArray);
        flyem::ZFileParser::writeVaa3dMakerFile(
              outDir.absoluteFilePath(name + ".post.marker").toStdString(),
              postMarkerArray);
      }
    }
  }

  return errMsg;
}

QString flyem::FIB19::GenerateFIB19VsSynapseCast(const QString &movieDir)
{
  QString errMsg = GenerateFIB19VsSynapseCast(movieDir, "VS");
  errMsg += GenerateFIB19VsSynapseCast(movieDir, "VS_branch");
  errMsg += GenerateFIB19VsSynapseCast(movieDir, "LPi");

  return errMsg;
}

QString flyem::FIB19::GenerateRoiCast(
    const QVector<QString> &roiList, const QString &movieDir)
{
  QString errMsg;

  ZDvidReader reader;
  if (reader.open(MakeDvidTarget())) {
    foreach (const QString &roi, roiList) {
      ZJsonArray roiJson = reader.readRoiJson(roi.toStdString());
      if (roiJson.isEmpty()) {
        errMsg += "Failed to read " + roi;
        return errMsg;
      }
      roiJson.dump((movieDir + "/cast/" + roi + ".json").toStdString());
    }
  }

  return errMsg;
}

QString flyem::FIB19::GenerateFIB19VsCast(const QString &movieDir)
{
  QString errMsg;
  QDir outDir(movieDir + "/cast");

  QString bodyListCsv = movieDir + "/bodylist_michael_170515_coord.csv";
  QFile file(bodyListCsv);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    errMsg += "Failed to open " + bodyListCsv;
    return errMsg;
  }

  QMap<QString, QString> nameTypeMap;
  QMap<QString, ZIntPoint> namePosMap;
  QList<QString> nameList;

  QList<QString> typeList =
      QList<QString>() << "VS" << "VS_branch" << "T4" << "T5" << "LPi";

  QTextStream in(&file);
  while (!in.atEnd()) {
    QString line = in.readLine();
    QStringList entryList = line.split(",");
    ZIntPoint pos;
    if (entryList.back() != "Z" && entryList.size() == 6) {
      pos.setX(entryList[entryList.size() - 3].toInt());
      pos.setY(entryList[entryList.size() - 2].toInt());
      pos.setZ(entryList[entryList.size() - 1].toInt());
      QString originalName = entryList[2];
      QString name = QString("%1_i%2").arg(entryList[2].replace(" ", "_").
          replace("-", "_")).arg(nameList.size());
      nameList.append(name);

      namePosMap[name] = pos;

      if (originalName == "LOP tan VS 01 Lop4") {
        nameTypeMap[name] = typeList[0];
      } else if (originalName == "LOP tan VS 01 Lop4 branch") {
        nameTypeMap[name] = typeList[1];
      } else {
        for (int i = 2; i < 5; ++i) {
          if (originalName.startsWith(typeList[i])) {
            nameTypeMap[name] = typeList[i];
            break;
          }
        }
      }
    }
  }


  ZJsonObject neuronJson;

  foreach(const QString &type, typeList) {
    ZJsonArray array;
    neuronJson.setEntry(type.toLocal8Bit(), array);
  }

  foreach (const QString &name, nameList) {
    ZJsonArray array(neuronJson.value(nameTypeMap[name].toLocal8Bit()));
    ZJsonObject infoJson;
    infoJson.setEntry("name", name.toStdString());
    ZJsonArray posJson = ZJsonFactory::MakeJsonArray(namePosMap[name]);
    infoJson.setEntry("pos", posJson);
    array.append(infoJson);
  }

  neuronJson.dump((movieDir + "/neuron_list.json").toStdString());

  ZDvidReader reader;
  if (reader.open(MakeDvidTarget())) {
    QFile neuronListFile(movieDir + "/neuron_list.json");
    ZJsonObject neuronJson;
    if (!neuronJson.load(neuronListFile.fileName().toStdString())) {
      errMsg += QString("Failed to load %1").arg(neuronListFile.fileName());
      return errMsg;
    }

    const char *key;
    json_t *value;
    ZJsonObject_foreach(neuronJson, key, value) {
      ZJsonArray neuronArray(neuronJson.value(key));
      for (size_t i = 0; i < neuronArray.size(); ++i) {
        uint64_t bodyId = 0;
        QString name;
        if (ZJsonParser::IsInteger(neuronArray.at(i))) {
          bodyId = ZJsonParser::integerValue(neuronArray.getData(), i);
        } else {
          ZJsonObject infoJson(neuronArray.value(i));
          if (infoJson.hasKey("pos")) {
            ZIntPoint pos = ZJsonParser::toIntPoint(infoJson["pos"]);
            bodyId = reader.readBodyIdAt(pos);
            name = ZJsonParser::stringValue(infoJson["name"]).c_str();
          }
        }

        if (bodyId > 0) {
          ZSwcTree *tree = reader.readSwc(bodyId);
          if (tree == NULL) {
            errMsg += QString("Failed to read skeleton of body %1").arg(bodyId);
            return errMsg;
          }
          QString outFileName = QString("%1.swc").arg(bodyId);
          if (!name.isEmpty()) {
            outFileName = name + ".swc";
          }
          QString outPath = outDir.absoluteFilePath(outFileName);
          tree->save(outPath.toStdString());
          delete tree;
        }
      }
    }
  }

  errMsg += GenerateRoiCast(
        QVector<QString>() << "Layer_Lop1" << "Layer_Lop2" << "Layer_Lop3"
        << "Layer_Lop4", movieDir);

  errMsg += GenerateFIB19VsSynapseCast(movieDir);

  return errMsg;
}

ZDvidTarget flyem::MB6Paper::MakeDvidTarget()
{
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "@MB6", 8500);
  target.setSynapseName("mb6_synapses_10062016");
  target.setBodyLabelName("bodies3");
  target.setSegmentationName("labels3");
  target.setGrayScaleName("grayscale");

  return target;
}

QString flyem::MB6Paper::GenerateNeuronCast(
    const ZDvidTarget &target, const QString &movieDir,
    QVector<uint64_t> neuronArray)
{
  QString errMsg;

  QDir outDir(movieDir + "/cast");

  if (neuronArray.isEmpty()) {
    QFile neuronListFile(movieDir + "/neuron_list.csv");
    if (!neuronListFile.open(QIODevice::ReadOnly)) {
      errMsg = "Failed to read " + neuronListFile.fileName() + ":" +
          neuronListFile.errorString();
      return errMsg;
    }

    while (!neuronListFile.atEnd()) {
      QString line = QString(neuronListFile.readLine());
      QStringList wordList = line.split(',');

      if (!wordList.isEmpty()) {
        ZString str(wordList[0].toStdString());
        std::vector<uint64_t> idArray = str.toUint64Array();
        if (!idArray.empty()) {
          neuronArray.append(idArray.front());
        }
      }
    }
  }

  ZDvidReader reader;
  if (reader.open(target)) {
    foreach (uint64_t bodyId, neuronArray) {
      ZSwcTree *tree = reader.readSwc(bodyId);

      QString outFileName = QString("%1.swc").arg(bodyId);
      QString outPath = outDir.absoluteFilePath(outFileName);
      tree->save(outPath.toStdString());
      delete tree;
    }
  }

  return errMsg;
}

std::vector<ZVaa3dMarker> flyem::MB6Paper::GetLocationMarker(
    const ZJsonArray &json)
{
  std::vector<ZVaa3dMarker> markerArray;

  for (size_t i = 0; i < json.size(); ++i) {
    ZJsonObject locationJson(json.value(i));
    ZJsonArray infoJson(locationJson.value("location"));
    std::string neuron = ZJsonParser::stringValue(infoJson.at(0));
    if (neuron == "MBON-14-A") {
      ZVaa3dMarker marker;
      marker.setCenter(ZJsonParser::integerValue(infoJson.at(1)),
                       ZJsonParser::integerValue(infoJson.at(2)),
                       ZJsonParser::integerValue(infoJson.at(3)));
      marker.setRadius(30.0);
      markerArray.push_back(marker);
    }
  }

  return markerArray;
}

QString flyem::MB6Paper::GenerateMBONConvCast(const QString &movieDir)
{
  ZDvidTarget target = MakeDvidTarget();

  QString errMsg = GenerateNeuronCast(
        target, movieDir, QVector<uint64_t>() << 54977);

  if (!errMsg.isEmpty()) {
    return errMsg;
  }

  QString synapseFile = movieDir + "/Shinya.json";

  ZJsonObject json;
  json.load(synapseFile.toStdString());


  ZJsonArray singleJson(json.value("single"));
  std::vector<ZVaa3dMarker> singleMarkerArray = GetLocationMarker(singleJson);
  flyem::ZFileParser::writeVaa3dMakerFile(
        (movieDir + "/cast/single.marker").toStdString(), singleMarkerArray);

  ZJsonArray multipleJson(json.value("multiple"));
  std::vector<ZVaa3dMarker> multileMarkerArray = GetLocationMarker(multipleJson);
  flyem::ZFileParser::writeVaa3dMakerFile(
        (movieDir + "/cast/multiple.marker").toStdString(), multileMarkerArray);



  return errMsg;
}


