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

//  report("Evaluation", information.toStdString(), ZMessageReporter::Information);
}

Z3DGraph* ZFlyEmMisc::MakeBoundBoxGraph(const ZDvidInfo &dvidInfo)
{
  ZCuboid box;
  box.setFirstCorner(dvidInfo.getStartCoordinates().toPoint());
  box.setLastCorner(dvidInfo.getEndCoordinates().toPoint());
  Z3DGraph *graph = Z3DGraphFactory::MakeBox(
        box, dmax2(1.0, dmax3(box.width(), box.height(), box.depth()) / 500.0));
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
