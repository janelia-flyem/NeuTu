#ifndef ZFLYEMROIPROJECT_H
#define ZFLYEMROIPROJECT_H

#include <QObject>
#include "dvid/zdvidtarget.h"
#include "zclosedcurve.h"
#include <QString>
#include "dvid/zdvidinfo.h"

class ZStackFrame;
class ZSwcTree;
class ZObject3dScan;

class ZFlyEmRoiProject : public QObject
{
  Q_OBJECT
public:
  explicit ZFlyEmRoiProject(const std::string &name, QObject *parent = 0);
  ~ZFlyEmRoiProject();

  void initRoi();
  void clear();
  void shallowClear();
  void setDvidTarget(const ZDvidTarget &target);
  void showDataFrame() const;
  void closeDataFrame();
  bool hasDataFrame() const;
  void setDataFrame(ZStackFrame *frame);
  void shallowClearDataFrame();
  bool addRoi();
  void setRoi(ZClosedCurve *roi, int z);
  int getCurrentZ() const;
  int getDataZ() const;

  void setZ(int z);

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  const ZClosedCurve* getRoi() const;
  ZSwcTree* getRoiSwc() const;

  bool hasRoi(int z) const;
  bool hasRoi() const;

  const ZClosedCurve* getRoi(int z) const;
  ZSwcTree* getRoiSwc(int z) const;
  ZSwcTree* getAllRoiSwc() const;

  double getMarkerRadius() const;

  bool hasOpenedRoi() const;
  int uploadRoi();
  int uploadRoi(int z);
  void downloadRoi();
  void downloadRoi(int z);
  void downloadRoi(const std::string &key);
  void downloadAllRoi();
  ZClosedCurve estimateRoi(int z);
  ZClosedCurve* estimateRoi(int z, ZClosedCurve *result) const;
  void estimateRoi();
  inline const ZDvidInfo& getDvidInfo() const {
    return m_dvidInfo;
  }

  int findSliceToCreateRoi(int z0) const;
  int findSliceToCreateRoi() const;

  void setRoiUploaded(int z, bool uploaded);
  bool isRoiCurveUploaded(int z) const;
  bool isAllRoiCurveUploaded() const;

  ZObject3dScan getFilledRoi(int z) const;
  ZObject3dScan* getFilledRoi(int z, ZObject3dScan *result) const;

  ZObject3dScan getRoiObject() const;

  int getFirstRoiZ() const;
  int getLastRoiZ() const;

  void scaleRoiSwc(double sx, double sy);
  void rotateRoiSwc(double theta);
  void translateRoiSwc(double dx, double dy);

  static ZIntCuboid estimateBoundBox(const ZStack &stack);

  bool isRoiSaved() const;
  void setRoiSaved(bool state);

  inline void setName(const std::string &name) {
    m_name = name;
  }

  inline std::string getName() const { return m_name; }

  std::string getRoiKey(int z) const;

private:
  ZObject3dScan* getFilledRoi(
      const ZClosedCurve *curve, int z, ZObject3dScan *result) const;

signals:

public slots:

private:
  std::string m_name;
  ZDvidTarget m_dvidTarget;
  ZDvidInfo m_dvidInfo;
  int m_z;
  ZStackFrame *m_dataFrame;
  std::vector<bool> m_isRoiCurveUploaded;
  std::vector<ZClosedCurve*> m_curveArray; //curve array sorted by z position
};

#endif // ZFLYEMROIPROJECT_H
