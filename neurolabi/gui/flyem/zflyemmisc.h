#ifndef ZFLYEMMISC_H
#define ZFLYEMMISC_H

#include <string>
#include <vector>
#include <QList>
#include <QSet>
#include <QColor>
#include <QRectF>
#include <QVector>

#include "common/neutube_def.h"
#include "dvid/libdvidheader.h"
#include "common/zsharedpointer.h"

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
class QComboBox;
class ZStroke2d;
class QDir;
class ZStack;
class ZVaa3dMarker;
class ZObject3d;
class ZMesh;
class ZArbSliceViewParam;
class ZDvidWriter;
class ZAffineRect;
class ZCubeArray;
class ZIntCuboid;
class ZJsonObject;
class ZJsonArray;
class ZWeightedPoint;
class ZIntPoint;
class ZStackObject;

namespace flyem {
void NormalizeSimmat(ZMatrix &simmat);

Z3DGraph* MakeBoundBoxGraph(const ZDvidInfo &dvidInfo);
Z3DGraph* MakePlaneGraph(ZStackDoc *doc, const ZDvidInfo &dvidInfo);
Z3DGraph* MakeSliceViewGraph(const ZArbSliceViewParam &param);
Z3DGraph* MakeRoiGraph(const ZObject3dScan &roi, const ZDvidInfo &dvidInfo);
ZCubeArray* MakeRoiCube(
    const ZObject3dScan &roi, const ZDvidInfo &dvidInfo, QColor color, int dsIntv);
ZCubeArray* MakeRoiCube(const ZObject3dScan &roi, QColor color, int dsIntv);
ZMesh* MakeRoiMesh(
    const ZObject3dScan &roi, const ZDvidInfo &dvidInfo, QColor color, int dsIntv);

ZMesh* MakeRoiMesh(const ZObject3dScan &roi, QColor color, int dsIntv);

//void Decorate3DWindow(Z3DWindow *window, const ZDvidInfo &dvidInfo);
//void Decorate3DWindow(Z3DWindow *window, const ZDvidReader &reader);
void Decorate3dBodyWindow(Z3DWindow *window, const ZDvidInfo &dvidInfo,
                          const ZStackViewParam &viewParam, bool visible = true);
void Decorate3dBodyWindowPlane(Z3DWindow *window, const ZDvidInfo &dvidInfo,
                               const ZStackViewParam &viewParam, bool visible = true);
void Decorate3dBodyWindowRoi(Z3DWindow *window, const ZDvidInfo &dvidInfo,
                             const ZDvidTarget &dvidTarget);
void Decorate3dBodyWindowRoiCube(Z3DWindow *window, const ZDvidInfo &dvidInfo,
                             const ZDvidTarget &dvidTarget);

void SubtractBodyWithBlock(
    ZObject3dScan *body, const ZObject3dScan &coarsePart,
    const ZDvidInfo& dvidInfo);

void MakeTriangle(const QRectF &rect, QPointF *ptArray,
                  neutu::ECardinalDirection direction);
void MakeStar(const QRectF &rect, QPointF *ptArray);
void MakeStar(const QPointF &center, double radius, QPointF *ptArray);

void PrepareBodyStatus(QComboBox *box);
QList<QString> GetDefaultBodyStatus();

QString GetMemoryUsage();

QString ReadLastLines(const QString &filePath, int maxCount);

ZStroke2d* MakeSplitSeed(const ZObject3dScan &slice, int label);
std::vector<ZStroke2d*> MakeSplitSeedList(const ZObject3dScan &obj);

ZStack* GenerateExampleStack(const ZJsonObject &obj);
ZStack* GenerateExampleStack(
    const ZDvidTarget &target, uint64_t bodyId, const ZIntCuboid &range);

ZIntCuboid EstimateSplitRoi(const ZIntCuboid &boundBox);

void SetSplitTaskSignalUrl(
    ZJsonObject &taskObj, uint64_t bodyId, const ZDvidTarget &target);
ZStroke2d SyGlassSeedToStroke(const ZJsonObject &obj);
ZStroke2d SyGlassSeedToStroke(
    const ZJsonObject &obj, const ZIntPoint &offset, const ZIntPoint &dsIntv);
ZJsonObject MakeSplitSeedJson(const ZStroke2d &stroke);
ZJsonObject MakeSplitSeedJson(const ZObject3d &seed);
void AddSplitTaskSeed(ZJsonObject &taskObj, const ZStroke2d &stroke);
void AddSplitTaskSeed(ZJsonObject &taskObj, const ZObject3d &obj);
template<typename T>
void AddSplitTaskSeedG(ZJsonObject &taskObj, const T& obj);
ZJsonArray GetSeedJson(ZStackDoc *doc);

void UploadSyGlassTask(const std::string &filePath, const ZDvidTarget &target);

QList<ZStackObject*> LoadSplitTask(const ZDvidTarget &target, uint64_t bodyId);
QList<ZStackObject*> LoadSplitTask(const ZJsonObject &taskJson);
ZJsonObject MakeSplitTask(
    const ZDvidTarget &target, uint64_t bodyId, ZJsonArray seedJson,
    ZJsonArray roiJson);

//Call it in the main thread ONLY
void RemoveSplitTask(const ZDvidTarget &target, uint64_t bodyId);

bool IsTaskOpen(const QString &taskKey);
//bool HasOpenTestTask();
//void StartOpenTestTask();

ZDvidReader* GetTaskReader();
ZDvidWriter* GetTaskWriter();

ZStack* MakeColorSegmentation(const ZDvidReader &reader, const ZAffineRect &ar);
ZStack* MakeColorSegmentation(
    const ZDvidReader &reader, const ZAffineRect &ar, int ccx, int ccy);
ZStack* MakeColorSegmentation(const ZDvidReader &reader, int x0, int y0, int z0,
                              int width, int height, int zoom, int ccx, int ccy);

QString GetNeuroglancerPath(
    const ZDvidTarget &target, const ZIntPoint &pos, const ZWeightedPoint &quat,
    const QSet<uint64_t> &bodySet);

void UploadRoi(
    const QString &dataDir, const QString &roiNameFile, ZDvidWriter *writer);

void UpdateBodyStatus(
    const ZIntPoint &pos, const std::string &newStatus, ZDvidWriter *writer);

void UpdateSupervoxelMesh(ZDvidWriter &writer, uint64_t svId);

std::vector<uint64_t> LoadBodyList(const std::string &input);

ZObject3dScan* LoadRoiFromJson(const std::string &filePath);

namespace MB6Paper {
ZDvidTarget MakeDvidTarget();
QSet<uint64_t> ReadBodyFromSequencer(const QString &filePath);
QSet<uint64_t> ReadBodyFromSequencer(const QStringList &fileList);
QSet<uint64_t> ReadBodyFromSequencer(
    const QDir &dir, const QStringList &fileList);
QSet<uint64_t> ReadBodyFromSequencer(
    const QDir &dir, const QString &filePath);
QString GenerateMBONConvCast(const QString &movieDir);
QString GenerateNeuronCast(
    const ZDvidTarget &target, const QString &movieDir,
    QVector<uint64_t> neuronArray = QVector<uint64_t>());
std::vector<ZVaa3dMarker> GetLocationMarker(const ZJsonArray &json);
}

namespace FIB19 {
ZDvidTarget MakeDvidTarget();
QString GetRootDir();
QString GetMovieDir(const QString &folder);
QString GenerateFIB19VsCast(const QString &movieDir);
QString GenerateRoiCast(
    const QVector<QString> &roiList, const QString &movieDir);
QString GenerateFIB19VsSynapseCast(
    const QString &movieDir, const QString &neuronType);
QString GenerateFIB19VsSynapseCast(const QString &movieDir);
}

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
