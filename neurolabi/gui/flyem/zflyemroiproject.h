#ifndef ZFLYEMROIPROJECT_H
#define ZFLYEMROIPROJECT_H

#include <QObject>
#include "dvid/zdvidtarget.h"
#include "zclosedcurve.h"
#include <QString>
#include "dvid/zdvidinfo.h"
#include "flyem/zsynapseannotationarray.h"
#include "dvid/zdvidwriter.h"

class ZStackFrame;
class ZSwcTree;
class ZObject3dScan;
class ZStackDocReader;
class ZStackDoc;

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
   * \brief Clone a project with a specified name
   */
  ZFlyEmRoiProject* clone(const std::string &name) const;

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
  void loadSynapse(const std::string &filePath, bool isVisible);
  void shallowClearDataFrame();
  bool addRoi();
  void setRoi(ZClosedCurve *roi, int z);
  int getCurrentZ() const;
  int getDataZ() const;
  void updateSynapse();
  QList<ZPunctum*> makePunctumList(bool dsScaled) const;

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

  void clearRoi();

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

  /*!
   * \brief Get downsampled ROI object
   */
  ZObject3dScan getRoiObject(int xIntv, int yIntv, int zIntv) const;
  ZObject3dScan getRoiObject() const;
  ZObject3dScan getRoiSlice() const;

  int getFirstRoiZ() const;
  int getLastRoiZ() const;

  void scaleRoiSwc(double sx, double sy);
  void rotateRoiSwc(double theta);
  void translateRoiSwc(double dx, double dy);

  static ZIntCuboid estimateBoundBox(const ZStack &stack, int bgValue);

  bool isRoiSaved() const;
  void setRoiSaved(bool state);

  inline void setName(const std::string &name) {
    m_name = name;
  }

  inline std::string getName() const { return m_name; }

  std::string getMinRoiKey() const;
  std::string getMaxRoiKey() const;
  std::string getRoiKey(int z) const;

  double estimateRoiVolume(char unit = 'p') const;

  void setDsIntv(int xintv, int yintv, int zintv);

  void setSynapseVisible(bool isVisible);

  ZStackFrame* makeAllSynapseFrame() const;
  ZStackDoc* makeAllSynapseDoc() const;

  const ZIntPoint& getCurrentDsIntv() const {
    return m_currentDsIntv;
  }

  void importRoiFromSwc(ZSwcTree *tree);

  void deleteAllData();

  void applyTranslate();

  void test();

private:
  ZObject3dScan* getFilledRoi(
      const ZClosedCurve *curve, int z, ZObject3dScan *result) const;

signals:

public slots:

private:
  std::string m_name;
  ZDvidTarget m_dvidTarget;
  ZDvidInfo m_dvidInfo;
  ZDvidWriter m_dvidWriter;
  int m_z;
  ZIntPoint m_currentDsIntv;
  ZStackFrame *m_dataFrame;
  std::vector<bool> m_isRoiCurveUploaded;
  std::vector<ZClosedCurve*> m_curveArray; //curve array sorted by z position
  FlyEm::ZSynapseAnnotationArray m_synapseArray;
  std::vector<ZPunctum*> m_puncta;
  std::vector<QColor> m_punctaColorMap;

  static const double m_defaultSynpaseRadius;
};

#endif // ZFLYEMROIPROJECT_H
