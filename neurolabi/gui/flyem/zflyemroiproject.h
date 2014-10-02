#ifndef ZFLYEMROIPROJECT_H
#define ZFLYEMROIPROJECT_H

#include <QObject>
#include "dvid/zdvidtarget.h"
#include "zclosedcurve.h"
#include <QString>
#include "dvid/zdvidinfo.h"
#include "flyem/zsynapseannotationarray.h"

class ZStackFrame;
class ZSwcTree;
class ZObject3dScan;
class ZStackDocReader;

/*!
 * \brief The class of managing a FlyEM ROI project
 */
class ZFlyEmRoiProject : public QObject
{
  Q_OBJECT
public:
  explicit ZFlyEmRoiProject(const std::string &name, QObject *parent = 0);
  ~ZFlyEmRoiProject();

  //void initRoi();
  /*!
   * \brief Remove all associated memory
   */
  void clear();

  /*!
   * \brief Set all member pointers to NULL
   */
  void shallowClear();

  /*!
   * \brief Set the dvid target
   *
   * It will create the project in the DVID key if necessary.
   *
   * \return true iff the DVID target is writable
   */
  bool setDvidTarget(const ZDvidTarget &target);

  void showDataFrame() const;
  void closeDataFrame();
  bool hasDataFrame() const;
  void setDataFrame(ZStackFrame *frame);
  void setDocData(const ZStackDocReader &docReader);
  void loadSynapse(const std::string &filePath);
  void shallowClearDataFrame();
  bool addRoi();
  void setRoi(ZClosedCurve *roi, int z);
  int getCurrentZ() const;
  int getDataZ() const;
  void updateSynapse();

  void setZ(int z);

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  const ZClosedCurve* getRoi() const;
  ZSwcTree* getRoiSwc() const;

  bool hasRoi(int z) const;
  bool hasRoi() const;

  const ZClosedCurve* getRoi(int z) const;
  ZSwcTree* getRoiSwc(int z, double radius = -1.0) const;
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

  double estimateRoiVolume(char unit = 'p') const;

  void setDsIntv(int xintv, int yintv, int zintv);

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
  ZIntPoint m_currentDsIntv;
  ZStackFrame *m_dataFrame;
  std::vector<bool> m_isRoiCurveUploaded;
  std::vector<ZClosedCurve*> m_curveArray; //curve array sorted by z position
  FlyEm::ZSynapseAnnotationArray m_synapseArray;
  std::vector<ZPunctum*> m_puncta;
};

#endif // ZFLYEMROIPROJECT_H
