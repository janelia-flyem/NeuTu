#include "zflyemmisc.h"

#include <iostream>
#include <QString>

#include "zmatrix.h"
#include "tz_math.h"
#include "tz_utilities.h"
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"
#include "zstring.h"
#include "zcuboid.h"
#include "dvid/zdvidinfo.h"
#include "zstackdoc.h"
#include "z3dgraphfactory.h"
#include "zstackobjectsourcefactory.h"
#include "zstackdochelper.h"
#include "z3dwindow.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidreader.h"
#include "zstackviewparam.h"
#include "zintcuboidarray.h"
#include "zobject3dfactory.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_neighborhood.h"

void ZFlyEmMisc::NormalizeSimmat(ZMatrix &simmat)
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

ZFlyEmMisc::HackathonEvaluator::HackathonEvaluator(
    const std::string &sourceDir, const std::string &workDir) :
  m_sourceDir(sourceDir), m_workDir(workDir), m_accuracy(0)
{

}

std::string ZFlyEmMisc::HackathonEvaluator::getNeuronInfoFile() const
{
  return getSourceDir() + "/neuronsinfo.json";
}

std::string ZFlyEmMisc::HackathonEvaluator::getNeuronTypeFile() const
{
  return getWorkDir() + "/neuron_type.json";
}

std::string ZFlyEmMisc::HackathonEvaluator::getSimmatFile() const
{
  return getWorkDir() + "/simmat.txt";
}

void ZFlyEmMisc::HackathonEvaluator::processNeuronTypeFile()
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

void ZFlyEmMisc::HackathonEvaluator::evalulate()
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
  ZFlyEmMisc::NormalizeSimmat(simmat);
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

//  report("Evaluation", information.toStdString(), NeuTube::MSG_INFORMATION);
}

Z3DGraph* ZFlyEmMisc::MakeBoundBoxGraph(const ZDvidInfo &dvidInfo)
{
  ZCuboid box;
  box.setFirstCorner(dvidInfo.getStartCoordinates().toPoint());
  box.setLastCorner(dvidInfo.getEndCoordinates().toPoint());
  Z3DGraph *graph = Z3DGraphFactory::MakeBox(
        box, dmax2(1.0, dmax3(box.width(), box.height(), box.depth()) / 1000.0));
  graph->setSource(ZStackObjectSourceFactory::MakeFlyEmBoundBoxSource());

  return graph;
}

Z3DGraph* ZFlyEmMisc::MakePlaneGraph(ZStackDoc *doc, const ZDvidInfo &dvidInfo)
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

Z3DGraph* ZFlyEmMisc::MakeRoiGraph(
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
  factory.setEdgeColorHint(QColor(128, 64, 64));

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

/*
void ZFlyEmMisc::Decorate3DWindow(Z3DWindow *window, const ZDvidInfo &dvidInfo)
{
  if (window != NULL) {
    ZStackDoc *doc = window->getDocument();
    if (doc != NULL) {
      doc->addObject(MakeBoundBoxGraph(dvidInfo), true);
      doc->addObject(MakePlaneGraph(doc, dvidInfo), true);
    }
  }
}

void ZFlyEmMisc::Decorate3DWindow(Z3DWindow *window, const ZDvidReader &reader)
{
  if (window != NULL) {
    Decorate3DWindow(window, reader.readGrayScaleInfo());
  }
}
*/

void ZFlyEmMisc::Decorate3dBodyWindowPlane(
    Z3DWindow *window, const ZDvidInfo &dvidInfo,
    const ZStackViewParam &viewParam)
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
    double lineWidth = box.depth() / 2000.0;
    Z3DGraph *graph = Z3DGraphFactory::MakeGrid(rect, 100, lineWidth);

    if (viewParam.getViewPort().isValid()) {
      Z3DGraphNode node1;
      Z3DGraphNode node2;

      node1.setColor(QColor(255, 0, 0));
      node2.setColor(QColor(255, 0, 0));

      double x = viewParam.getViewPort().center().x();
      double y = viewParam.getViewPort().center().y();

      double width = lineWidth * 0.3;
      node1.set(x, rect.getFirstY(), rect.getZ(), width);
      node2.set(x, rect.getLastY(), rect.getZ(), width);

      graph->addNode(node1);
      graph->addNode(node2);
      graph->addEdge(node1, node2, GRAPH_LINE);

      node1.set(rect.getFirstX(), y, rect.getZ(), width);
      node2.set(rect.getLastX(), y, rect.getZ(), width);

      graph->addNode(node1);
      graph->addNode(node2);
      graph->addEdge(node1, node2, GRAPH_LINE);
    }

    graph->setSource(ZStackObjectSourceFactory::MakeFlyEmPlaneObjectSource());
    window->getDocument()->addObject(graph, true);
#if 0
    ZStackObject *replaced =
        window->getDocument()->getObjectGroup().replaceFirstSameSource(graph);
    if (replaced == NULL) {
      window->getDocument()->addObject(graph, true);
    } else {
      window->getDocument()->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
      window->getDocument()->bufferObjectModified(replaced);
      window->getDocument()->bufferObjectModified(graph);
      delete replaced;
      window->getDocument()->endObjectModifiedMode();
      window->getDocument()->notifyObjectModified();
    }
#endif
  }
}


void ZFlyEmMisc::Decorate3dBodyWindow(
    Z3DWindow *window, const ZDvidInfo &dvidInfo,
    const ZStackViewParam &viewParam)
{
  if (window != NULL) {
    Decorate3dBodyWindowPlane(window, dvidInfo, viewParam);
    ZCuboid box;
    box.setFirstCorner(dvidInfo.getStartCoordinates().toPoint());
    box.setLastCorner(dvidInfo.getEndCoordinates().toPoint());
    Z3DGraph *graph = Z3DGraphFactory::MakeBox(
          box, dmax2(1.0, dmax3(box.width(), box.height(), box.depth()) / 1000.0));
    graph->setSource(ZStackObjectSourceFactory::MakeFlyEmBoundBoxSource());

    window->getDocument()->addObject(graph, true);
    if (window->isBackgroundOn()) {
      window->setOpacity(Z3DWindow::LAYER_GRAPH, 0.4);
    }
  }
}

void ZFlyEmMisc::Decorate3dBodyWindowRoi(
    Z3DWindow *window, const ZDvidInfo &dvidInfo, const ZDvidTarget &dvidTarget)
{
  if (window != NULL) {
    const std::vector<std::string> &roiList = dvidTarget.getRoiList();
    if (!roiList.empty()) {
      ZDvidReader reader;
      if (reader.open(dvidTarget)) {
        for (std::vector<std::string>::const_iterator iter = roiList.begin();
             iter != roiList.end(); ++iter) {
          ZObject3dScan roi = reader.readRoi(*iter);
          if (!roi.isEmpty()) {
            Z3DGraph *graph = MakeRoiGraph(roi, dvidInfo);
            graph->setSource(
                  ZStackObjectSourceFactory::MakeFlyEmRoiSource(*iter));
            window->getDocument()->addObject(graph, true);
          }
        }
      }
    }
  }
}

void ZFlyEmMisc::SubtractBodyWithBlock(
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
