#ifndef ZFLYEMMISC_H
#define ZFLYEMMISC_H

#include <string>
#include <vector>
#include "zcubearray.h"

#include "dvid/libdvidheader.h"
#include "zsharedpointer.h"

class ZMatrix;
class Z3DGraph;
class ZDvidInfo;
class ZStackDoc;
class Z3DWindow;
class ZDvidTarget;
class ZDvidReader;
class ZStackViewParam;
class ZObject3dScan;
class QPointF;

namespace ZFlyEmMisc {
void NormalizeSimmat(ZMatrix &simmat);

Z3DGraph* MakeBoundBoxGraph(const ZDvidInfo &dvidInfo);
Z3DGraph* MakePlaneGraph(ZStackDoc *doc, const ZDvidInfo &dvidInfo);
Z3DGraph* MakeRoiGraph(const ZObject3dScan &roi, const ZDvidInfo &dvidInfo);
ZCubeArray* MakeRoiCube(
    const ZObject3dScan &roi, const ZDvidInfo &dvidInfo, QColor color, int dsIntv);
//void Decorate3DWindow(Z3DWindow *window, const ZDvidInfo &dvidInfo);
//void Decorate3DWindow(Z3DWindow *window, const ZDvidReader &reader);
void Decorate3dBodyWindow(Z3DWindow *window, const ZDvidInfo &dvidInfo,
                          const ZStackViewParam &viewParam);
void Decorate3dBodyWindowPlane(Z3DWindow *window, const ZDvidInfo &dvidInfo,
                               const ZStackViewParam &viewParam);
void Decorate3dBodyWindowRoi(Z3DWindow *window, const ZDvidInfo &dvidInfo,
                             const ZDvidTarget &dvidTarget);
void Decorate3dBodyWindowRoiCube(Z3DWindow *window, const ZDvidInfo &dvidInfo,
                             const ZDvidTarget &dvidTarget);

void SubtractBodyWithBlock(
    ZObject3dScan *body, const ZObject3dScan &coarsePart,
    const ZDvidInfo& dvidInfo);

void MakeTriangle(const QRectF &rect, QPointF *ptArray,
                  NeuTube::ECardinalDirection direction);
void MakeStar(const QRectF &rect, QPointF *ptArray);
void MakeStar(const QPointF &center, double radius, QPointF *ptArray);


class HackathonEvaluator {
public:
  HackathonEvaluator(const std::string &sourceDir,
                     const std::string &workDir);
  void processNeuronTypeFile();
  void evalulate();

  const std::string& getSourceDir() const { return m_sourceDir; }
  const std::string& getWorkDir() const { return m_workDir; }
  std::string getNeuronTypeFile() const;
  std::string getNeuronInfoFile() const;
  std::string getSimmatFile() const;
  int getAccurateCount() const { return m_accuracy; }
  int getNeuronCount() const { return (int) m_idArray.size(); }

private:
  std::string m_sourceDir;
  std::string m_workDir;
  int m_accuracy;
  std::vector<int> m_idArray;
};

}
#endif // ZFLYEMMISC_H
