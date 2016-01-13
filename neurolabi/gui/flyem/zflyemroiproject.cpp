#include "zflyemroiproject.h"
#include <QStringList>
#include <ostream>
#include <fstream>

#include "neutubeconfig.h"
#include "zstackframe.h"
#include "zswcgenerator.h"
#include "zflyemutilities.h"
#include "zstack.hxx"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidreader.h"
#include "swctreenode.h"
#include "zobject3dscan.h"
#include "zstroke2d.h"
#include "zstackfactory.h"
#include "zstring.h"
#include "dvid/zdviddata.h"
#include "zfiletype.h"
#include "zswcforest.h"
#include "zswctree.h"

const double ZFlyEmRoiProject::m_defaultSynpaseRadius = 20.0;

ZFlyEmRoiProject::ZFlyEmRoiProject(const std::string &name, QObject *parent) :
  QObject(parent), m_name(name), m_z(-1), m_dataFrame(NULL)
{
  m_punctaColorMap.push_back(QColor(255, 255, 255, 255));
  m_punctaColorMap.push_back(QColor(0, 255, 255, 255));
  m_punctaColorMap.push_back(QColor(255, 128, 0, 255));
  m_punctaColorMap.push_back(QColor(0, 0, 255, 255));
  m_punctaColorMap.push_back(QColor(255, 0, 255, 255));
  m_punctaColorMap.push_back(QColor(127, 0, 255, 255));
  m_punctaColorMap.push_back(QColor(0, 255, 0, 255));
  m_punctaColorMap.push_back(QColor(255, 255, 0, 255));

  m_punctaColorMap.push_back(QColor(128, 255, 255, 255));
  m_punctaColorMap.push_back(QColor(255, 128, 128, 255));
  m_punctaColorMap.push_back(QColor(128, 128, 255, 255));
  m_punctaColorMap.push_back(QColor(255, 128, 255, 255));
  m_punctaColorMap.push_back(QColor(127, 128, 255, 255));
  m_punctaColorMap.push_back(QColor(128, 255, 128, 255));
  m_punctaColorMap.push_back(QColor(255, 255, 128, 255));
}

ZFlyEmRoiProject::~ZFlyEmRoiProject()
{
  clear();
}

void ZFlyEmRoiProject::clear()
{
  if (m_dataFrame != NULL) {
    m_dataFrame->hide();
    delete m_dataFrame;
    m_dataFrame = NULL;
  }

  for (std::vector<ZClosedCurve*>::const_iterator iter = m_curveArray.begin();
       iter != m_curveArray.end(); ++iter) {
    delete *iter;
  }
  m_curveArray.clear();

  for (std::vector<ZPunctum*>::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    delete *iter;
  }
  m_puncta.clear();

  shallowClear();
}

void ZFlyEmRoiProject::deleteAllData()
{
  //Delete data from DVID server
  ZDvidWriter writer;
  if (writer.open(m_dvidTarget)) {
    writer.deleteKey(ZDvidData::GetName(ZDvidData::ROLE_ROI_CURVE),
                     getMinRoiKey(), getMaxRoiKey());
  }

  clear();
}

void ZFlyEmRoiProject::applyTranslate()
{
  if (m_dataFrame != NULL) {
    QList<ZSwcTree*> swcList = m_dataFrame->document()->getSwcList();
    if (swcList.size() == 1) {
      ZSwcTree *tree = swcList[0];

      const ZClosedCurve *roiCurve = getRoi(getDataZ());
      if (roiCurve != NULL) {
        ZPoint newCenter = tree->computeCentroid();

        ZPoint oldCenter = roiCurve->computeCenter();

        ZPoint offset = newCenter - oldCenter;
        for (std::vector<ZClosedCurve*>::iterator iter = m_curveArray.begin();
             iter != m_curveArray.end(); ++iter) {
          ZClosedCurve *curve = *iter;
          if (curve != NULL) {
            curve->translate(offset.x(), offset.y());
          }
        }
        m_isRoiCurveUploaded.clear();
        setRoiSaved(true);
      }
    }
  }
}

void ZFlyEmRoiProject::shallowClear()
{
  m_dataFrame = NULL;
}

bool ZFlyEmRoiProject::setDvidTarget(const ZDvidTarget &target)
{
  bool succ = false;

  clear();

  m_dvidTarget = target;
  ZDvidReader reader;
  if (reader.open(target)) {
    if (m_dvidWriter.open(target)) {
      m_dvidWriter.createKeyvalue(ZDvidData::GetName(ZDvidData::ROLE_ROI_CURVE));
      m_dvidInfo = reader.readGrayScaleInfo();
      downloadAllRoi();
    }
  }

  return succ;
}

void ZFlyEmRoiProject::downloadAllRoi()
{
  //Download All ROIs
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    QStringList roiIdArray = reader.readKeys(
          ZDvidData::GetName(ZDvidData::ROLE_ROI_CURVE),
          QString("%1").arg(getRoiKey(m_dvidInfo.getMinZ()).c_str()),
          QString("%1").arg(getRoiKey(m_dvidInfo.getMaxZ()).c_str()));
    foreach (const QString &roiKey, roiIdArray) {
      int roiId = ZString(roiKey.toStdString()).lastInteger();
      downloadRoi(roiId);
    }
  }
}

void ZFlyEmRoiProject::showDataFrame() const
{
  if (m_dataFrame != NULL) {
    m_dataFrame->show();
  }
}

void ZFlyEmRoiProject::closeDataFrame()
{
  if (m_dataFrame != NULL) {
    m_dataFrame->hide();
    delete m_dataFrame;
    m_dataFrame = NULL;
  }
}

bool ZFlyEmRoiProject::hasDataFrame() const
{
  return m_dataFrame != NULL;
}

void ZFlyEmRoiProject::setDataFrame(ZStackFrame *frame)
{
  if (m_dataFrame != NULL) {
    frame->setObjectStyle(m_dataFrame->getObjectStyle());
    closeDataFrame();
  }

  m_dataFrame = frame;
  updateSynapse();
}

QList<ZPunctum*> ZFlyEmRoiProject::makePunctumList(bool dsScaled) const
{
  QList<ZPunctum*> punctumList;
  for (std::vector<ZPunctum*>::const_iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    const ZPunctum* punctum = *iter;
    ZPunctum *docPunctum = new ZPunctum(*punctum);
    if (dsScaled) {
      docPunctum->scale(1.0 / (m_currentDsIntv.getX() + 1),
                        1.0 / (m_currentDsIntv.getY() + 1),
                        1.0 / (m_currentDsIntv.getZ() + 1));
    }
    docPunctum->useCosmeticPen(true);
    punctumList.append(docPunctum);
  }

  return punctumList;
}

void ZFlyEmRoiProject::updateSynapse()
{
  int z = getDataZ();
  int range = m_defaultSynpaseRadius;

  ZPunctum markPunctum;
  markPunctum.setZ(z - range);
  //Find the puncta range
  std::vector<ZPunctum*>::const_iterator beginIter =
      std::lower_bound(m_puncta.begin(), m_puncta.end(),
                       &markPunctum, ZPunctum::ZCompare());
  markPunctum.setZ(z + range);
  std::vector<ZPunctum*>::const_iterator endIter =
      std::upper_bound(m_puncta.begin(), m_puncta.end(),
                       &markPunctum, ZPunctum::ZCompare());

  //Add to document
  QList<ZPunctum*> punctumList;
  for (std::vector<ZPunctum*>::const_iterator iter = beginIter;
       iter != endIter; ++iter) {
    const ZPunctum* punctum = *iter;
    ZPunctum *docPunctum = new ZPunctum(*punctum);
    docPunctum->scale(1.0 / (m_currentDsIntv.getX() + 1),
                      1.0 / (m_currentDsIntv.getY() + 1),
                      1.0 / (m_currentDsIntv.getZ() + 1));
    docPunctum->setZScale(m_currentDsIntv.getX() + 1);
    docPunctum->useCosmeticPen(true);
    punctumList.append(docPunctum);
  }

  if (m_dataFrame != NULL) {
    m_dataFrame->document()->addPunctum(punctumList);
  }
}

void ZFlyEmRoiProject::shallowClearDataFrame()
{
  shallowClear();
}

void ZFlyEmRoiProject::setRoi(ZClosedCurve *roi, int z)
{
  if (z >= 0) {
    if (roi != NULL) {
      if (z >= (int) m_curveArray.size()) {
        m_curveArray.resize(z + 1, NULL);
      }
      if (m_curveArray[z] != NULL) {
        delete m_curveArray[z];
      }
      m_curveArray[z] = roi;
      setRoiUploaded(z, false);
    }
  }
#if 0
  else {
    if (m_curveArray[z] != NULL) {
      m_curveArray[z].clear();
      /*
      delete m_curveArray[z];
      m_curveArray[z] = NULL;
      */
      setRoiUploaded(z, false);
    }
  }
#endif
}

int ZFlyEmRoiProject::getDataZ() const
{
  if (m_dataFrame == NULL) {
    return -1;
  }

  return m_dataFrame->document()->getStackOffset().getZ();
}
int ZFlyEmRoiProject::getCurrentZ() const
{
  return m_z;
}

void ZFlyEmRoiProject::setZ(int z)
{
  m_z = z;
}

bool ZFlyEmRoiProject::isRoiSaved() const
{
  if (m_dataFrame != NULL) {
    return !m_dataFrame->document()->isSwcSavingRequired();
  }

  return true;
}

void ZFlyEmRoiProject::setRoiSaved(bool state)
{
  if (m_dataFrame != NULL) {
    m_dataFrame->document()->setSaved(ZStackObject::TYPE_SWC, state);
  }
}

bool ZFlyEmRoiProject::addRoi()
{
  if (m_dataFrame != NULL) {
    QList<ZSwcTree*> swcList = m_dataFrame->document()->getSwcList();
    ZClosedCurve *roi = NULL;

    if (swcList.size() == 1) {
      ZClosedCurve curve = swcList.front()->toClosedCurve();
      //if(!curve.isEmpty()) {
      roi = new ZClosedCurve(curve);
      if (!m_currentDsIntv.isZero()) {
        roi->scale(m_currentDsIntv.getX() + 1, m_currentDsIntv.getY() + 1,
                   m_currentDsIntv.getZ() + 1);
      }
    } else if (swcList.isEmpty()) {
      roi = new ZClosedCurve;
    }

    setRoi(roi, getDataZ());
    setRoiSaved(true);

#ifdef _DEBUG_2
      ZObject3dScan *obj = getFilledRoi(getDataZ(), NULL);
      obj->save(GET_DATA_DIR + "/test.sobj");
      delete obj;
#endif

      return true;
      //}

  /* else if (swcList.isEmpty()) {
      setRoi(NULL, getDataZ());
      setRoiSaved(true);
      return true;
    }*/
  }

  return false;
}

bool ZFlyEmRoiProject::hasRoi(int z) const
{
  const ZClosedCurve *curve = getRoi(z);
  if (curve == NULL) {
    return false;
  }

  return !curve->isEmpty();
}

bool ZFlyEmRoiProject::hasRoi() const
{
  return hasRoi(getDataZ());
}

const ZClosedCurve* ZFlyEmRoiProject::getRoi(int z) const
{
  const ZClosedCurve *curve = NULL;
  if (z >= 0 && z < (int) m_curveArray.size()) {
    curve = m_curveArray[z];
  }

  return curve;
}

const ZClosedCurve* ZFlyEmRoiProject::getRoi() const
{
  return getRoi(getDataZ());
}

double ZFlyEmRoiProject::getMarkerRadius() const
{
  double s = 0;

  if (m_dataFrame != NULL) {
    s = imin2(m_dataFrame->document()->getStack()->width(),
              m_dataFrame->document()->getStack()->height());
  }

  return FlyEm::GetFlyEmRoiMarkerRadius(s);
}

ZSwcTree* ZFlyEmRoiProject::getRoiSwc() const
{
  return getRoiSwc(getDataZ());
}

ZSwcTree* ZFlyEmRoiProject::getRoiSwc(int z, double radius) const
{
  ZSwcTree *tree = NULL;

  const ZClosedCurve *curve = getRoi(z);
  if (curve != NULL) {
    if (radius > 0.0) {
      tree = ZSwcGenerator::createSwc(*curve, radius);
    } else {
      tree = ZSwcGenerator::createSwc(*curve, getMarkerRadius());
    }
    if (!m_currentDsIntv.isZero()) {
      tree->rescale(1.0 / (m_currentDsIntv.getX() + 1),
                    1.0 / (m_currentDsIntv.getY() + 1),
                    1.0 / (m_currentDsIntv.getZ() + 1), false);
    }
    tree->useCosmeticPen(true);
    tree->setRole(ZStackObjectRole::ROLE_ROI);
  }

  return tree;
}

ZSwcTree* ZFlyEmRoiProject::getAllRoiSwc() const
{
  ZSwcTree *tree = new ZSwcTree();

  for (size_t i = 0; i < m_curveArray.size(); ++i) {
    const ZClosedCurve *curve = m_curveArray[i];
    if (curve != NULL) {
      ZSwcTree *curveTree = ZSwcGenerator::createSwc(*curve, getMarkerRadius());
      if (curveTree != NULL) {
        tree->merge(curveTree, true);
      }
    }
  }

  if (tree->isEmpty()) {
    delete tree;
    tree = NULL;
  }

  return tree;
}

bool ZFlyEmRoiProject::hasOpenedRoi() const
{
  return getRoi(getDataZ()) != NULL;
}

int ZFlyEmRoiProject::uploadRoi(int z)
{
  int count = 0;
  if (m_dvidWriter.good()) {
    const ZClosedCurve *curve = m_curveArray[z];
    if (curve != NULL) {
      if (!isRoiCurveUploaded(z)) {
        m_dvidWriter.writeRoiCurve(*curve, getRoiKey(z));
        setRoiUploaded(z, true);
        ++count;
      }
    }
  }

  return count;
}

int ZFlyEmRoiProject::uploadRoi()
{
  int count = 0;
  for (size_t i = 0; i < m_curveArray.size(); ++i) {
    count += uploadRoi(i);
  }

  return count;
}

void ZFlyEmRoiProject::downloadRoi()
{
  downloadRoi(getDataZ());
}
/*
void ZFlyEmRoiProject::downloadRoi(const std::string &key)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    ZClosedCurve *curve = reader.readRoiCurve(key, NULL);
    if (curve != NULL) {
      setRoi(curve, getDataZ());
      setRoiUploaded(z, true);
    }
  }
}
*/
void ZFlyEmRoiProject::downloadRoi(int z)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    ZClosedCurve *curve = reader.readRoiCurve(getRoiKey(z), NULL);
    if (curve != NULL) {
      if (curve->isEmpty()) {
        delete curve;
      } else {
        setRoi(curve, z);
        setRoiUploaded(z, true);
      }
    }
  }
}

ZClosedCurve* ZFlyEmRoiProject::estimateRoi(int z, ZClosedCurve *result) const
{
  if (result != NULL) {
    result->clear();
  }

  int minZ = -1;
  int maxZ = -1;
  for (int tz = z + 1; tz < (int) m_curveArray.size(); ++tz) {
    if (m_curveArray[tz] != NULL) {
      maxZ = tz;
      break;
    }
  }

  for (int tz = imin2(z - 1, (int) m_curveArray.size() - 1); tz >= 0; --tz) {
    if (m_curveArray[tz] != NULL) {
      minZ = tz;
      break;
    }
  }

  if (minZ >= 0 || maxZ >= 0) {
    if (minZ < 0) {
      minZ = maxZ;
      maxZ = -1;
      for (int tz = minZ + 1; tz < (int) m_curveArray.size(); ++tz) {
        if (m_curveArray[tz] != NULL) {
          maxZ = tz;
          break;
        }
      }
    } else if (maxZ < 0) {
      maxZ = minZ;
      minZ = -1;
      for (int tz = maxZ - 1; tz >= 0; --tz) {
        if (m_curveArray[tz] != NULL) {
          minZ = tz;
          break;
        }
      }
    }
  }

  if (minZ >= 0 && maxZ >= 0) {
    const ZClosedCurve *lowerCurve = getRoi(minZ);
    const ZClosedCurve *upperCurve = getRoi(maxZ);
    int sampleNumber = imax3(100, lowerCurve->getLandmarkNumber(),
                             upperCurve->getLandmarkNumber());
    ZClosedCurve curve1 = lowerCurve->resampleF(sampleNumber);
    ZClosedCurve curve2 = upperCurve->resampleF(sampleNumber);

    if (curve1.computeDirection().dot(curve2.computeDirection()) < 0) {
      curve2.reverse();
    }
    int shift = curve1.findMatchShift(curve2);

    result = curve1.interpolate(
          curve2, ((double) maxZ - z) / (maxZ - minZ), shift, result);
    if (result != NULL) {
      result->resampleF(50);
    }
  }

  return result;

}

ZClosedCurve ZFlyEmRoiProject::estimateRoi(int z)
{
  ZClosedCurve roiCurve;

  estimateRoi(z, &roiCurve);

  return roiCurve;
}

void ZFlyEmRoiProject::estimateRoi()
{
  if (m_dataFrame != NULL) {
    ZClosedCurve roiCurve = estimateRoi(getDataZ());
    if (!roiCurve.isEmpty()) {
      //m_dataFrame->document()->removeObject(ZDocPlayer::ROLE_ROI, true);
      ZSwcTree *tree = ZSwcGenerator::createSwc(roiCurve, getMarkerRadius());
      tree->setRole(ZStackObjectRole::ROLE_ROI);

      if (!m_currentDsIntv.isZero()) {
        tree->rescale(1.0 / (m_currentDsIntv.getX() + 1),
                      1.0 / (m_currentDsIntv.getY() + 1),
                      1.0 / (m_currentDsIntv.getZ() + 1), false);
      }
      tree->useCosmeticPen(true);
      m_dataFrame->document()->executeReplaceSwcCommand(tree);
    }
  }
}

void ZFlyEmRoiProject::setRoiUploaded(int z, bool uploaded)
{
  if (z >= (int) m_isRoiCurveUploaded.size()) {
    m_isRoiCurveUploaded.resize(z + 1, false);
  }

  m_isRoiCurveUploaded[z] = uploaded;
}

bool ZFlyEmRoiProject::isRoiCurveUploaded(int z) const
{
  if (z >= (int) m_isRoiCurveUploaded.size()) {
    return false;
  }

#ifdef _DEBUG_
  std::cout << z << " uploaded: " << m_isRoiCurveUploaded[z] << std::endl;
#endif

  return m_isRoiCurveUploaded[z];
}

bool ZFlyEmRoiProject::isAllRoiCurveUploaded() const
{
  for (size_t z = 0; z < m_curveArray.size(); ++z) {
    if (getRoi(z) != NULL) {
      if (!isRoiCurveUploaded(z)) {
        return false;
      }
    }
  }

  return true;
}

int ZFlyEmRoiProject::findSliceToCreateRoi(int z0) const
{
  int minZ = (getDvidInfo().getMinZ() + z0) / 2;
  int maxZ = (getDvidInfo().getMaxZ() + z0) / 2;
  std::pair<int, int> bestSeg(0, 1);

  int length = 0;

  int startZ = minZ;
  int z = startZ;
  for (; z <= maxZ; ++z) {
    const ZClosedCurve *curve = getRoi(z);
    if (curve == NULL) {
      ++length;
    } else {
      int maxLength = bestSeg.second - bestSeg.first - 1;
      if (length > maxLength) {
        bestSeg.first = startZ;
        bestSeg.second = z;
      }
      length = 0;
      startZ = z;
    }
  }

  if (z < maxZ) {
    length = maxZ - z - 1;
    int maxLength = bestSeg.second - bestSeg.first - 1;
    if (length > maxLength) {
      bestSeg.first = z;
      bestSeg.second = maxZ;
    }
  }

  int targetZ = -1;
  if (bestSeg.second - bestSeg.first > 1) {
    targetZ = (bestSeg.second + bestSeg.first) / 2;
  }

  return targetZ;
}

int ZFlyEmRoiProject::findSliceToCreateRoi() const
{
  return findSliceToCreateRoi(getDataZ());
}

ZObject3dScan* ZFlyEmRoiProject::getFilledRoi(
    const ZClosedCurve *curve, int z, ZObject3dScan *result) const
{
  if (result != NULL) {
    result->clear();
  }

  if (curve != NULL) {
    ZStroke2d stroke;
    stroke.setZ(z);
    stroke.setWidth(1.0);
    for (size_t i = 0; i < curve->getLandmarkNumber(); ++i) {
      ZPoint pt = curve->getLandmark(i);
      stroke.append(pt.x(), pt.y());
    }
    ZStack *stack = ZStackFactory::makePolygonPicture(stroke);
    if (stack != NULL) {
      if (result == NULL) {
        result = new ZObject3dScan;
      }

      result->loadStack(*stack);
      delete stack;
    }
  }

  return result;
}

ZObject3dScan* ZFlyEmRoiProject::getFilledRoi(int z, ZObject3dScan *result) const
{
  const ZClosedCurve *curve = getRoi(z);

  return getFilledRoi(curve, z, result);
}

ZObject3dScan ZFlyEmRoiProject::getFilledRoi(int z) const
{
  ZObject3dScan obj;

  getFilledRoi(z, &obj);

  return obj;
}

int ZFlyEmRoiProject::getFirstRoiZ() const
{
  for (int z = 0; z < (int) m_curveArray.size(); ++z) {
    if (getRoi(z) != NULL) {
      return z;
    }
  }

  return -1;
}

int ZFlyEmRoiProject::getLastRoiZ() const
{
  for (int z = (int) m_curveArray.size(); z >= 0; --z) {
    if (getRoi(z) != NULL) {
      return z;
    }
  }

  return -1;
}

ZObject3dScan ZFlyEmRoiProject::getRoiObject(
    int xIntv, int yIntv, int zIntv) const
{
  ZObject3dScan obj;

  ZObject3dScan sliceObj;

  int minZ = getFirstRoiZ();
  int maxZ = getLastRoiZ();

  for (int z = minZ; z <= maxZ; ++z) {
    const ZClosedCurve *originalRoiCurve = getRoi(z);
    ZClosedCurve *roiCurve = NULL;
    if (originalRoiCurve == NULL) {
      roiCurve = estimateRoi(z, NULL);
    } else {
      roiCurve = originalRoiCurve->clone();
    }

    if (roiCurve != NULL) {
      if (!roiCurve->isEmpty()) {
        roiCurve->scale(1.0 / (1 + xIntv), 1.0 / (1 + yIntv), 1);
        getFilledRoi(roiCurve, z, &sliceObj);
        obj.concat(sliceObj);
      }

      delete roiCurve;
    }
  }

  obj.downsampleMax(0, 0, zIntv);

  return obj;
}

ZObject3dScan ZFlyEmRoiProject::getRoiObject() const
{
  ZObject3dScan obj;

  ZObject3dScan sliceObj;

  int minZ = getFirstRoiZ();
  int maxZ = getLastRoiZ();

  for (int z = minZ; z <= maxZ; ++z) {
    const ZClosedCurve *roiCurve = getRoi(z);
    if (roiCurve == NULL) {
      roiCurve = estimateRoi(z, NULL);
    }
    if (!roiCurve->isEmpty()) {
      getFilledRoi(roiCurve, z, &sliceObj);
      obj.concat(sliceObj);
    }
    if (getRoi(z) == NULL) {
      delete roiCurve;
    }
  }

  return obj;
}

ZObject3dScan ZFlyEmRoiProject::getRoiSlice() const
{
  ZObject3dScan obj;

  ZObject3dScan sliceObj;

  int minZ = getFirstRoiZ();
  int maxZ = getLastRoiZ();

  for (int z = minZ; z <= maxZ; ++z) {
    const ZClosedCurve *roiCurve = getRoi(z);
    if (roiCurve != NULL) {
      if (!roiCurve->isEmpty()) {
        getFilledRoi(roiCurve, z, &sliceObj);
        obj.concat(sliceObj);
      }
    }
  }

  return obj;
}

void ZFlyEmRoiProject::translateRoiSwc(double dx, double dy)
{
  if (m_dataFrame != NULL) {
    if (m_dataFrame->document()->hasSelectedSwcNode()) {
      m_dataFrame->document()->executeMoveSwcNodeCommand(dx, dy, 0);
    } else {
      m_dataFrame->document()->executeMoveAllSwcCommand(dx, dy, 0);
    }
    /*
    ZSwcTree *tree = m_dataFrame->document()->getSwcTree(0);
    if (tree != NULL) {
      tree->translate(dx, dy, 0.0);
      m_dataFrame->document()->notifySwcModified();
    }
    */
  }
}

void ZFlyEmRoiProject::scaleRoiSwc(double sx, double sy)
{
  if (m_dataFrame != NULL) {
    if (m_dataFrame->document()->hasSelectedSwcNode()) {
      ZSwcTree *allTree = m_dataFrame->document()->getMergedSwc();
      m_dataFrame->document()->executeScaleSwcNodeCommand(
            sx, sy, 0, allTree->computeCentroid());
      delete allTree;
    } else {
      m_dataFrame->document()->executeScaleAllSwcCommand(sx, sy, 1.0, true);
    }
    /*
    ZSwcTree *tree = m_dataFrame->document()->getSwcTree(0);
    if (tree != NULL) {
      ZPoint center = tree->computeCentroid();
      tree->translate(-center);
      tree->scale(sx, sy, 1.0);
      tree->translate(center);
      m_dataFrame->document()->notifySwcModified();
    }
    */
  }
}

void ZFlyEmRoiProject::rotateRoiSwc(double theta)
{
  if (m_dataFrame != NULL) {
    if (m_dataFrame->document()->hasSelectedSwcNode()) {
      m_dataFrame->document()->executeRotateSwcNodeCommand(0.0, theta, true);
    } else {
      m_dataFrame->document()->executeRotateAllSwcCommand(0.0, theta, true);
    }
    /*
    ZSwcTree *tree = m_dataFrame->document()->getSwcTree(0);
    if (tree != NULL) {
      ZPoint center = tree->computeCentroid();
      tree->rotate(0.0, theta, center);
      m_dataFrame->document()->notifySwcModified();
    }
    */
  }
}

ZIntCuboid ZFlyEmRoiProject::estimateBoundBox(const ZStack &stack, int bgValue)
{
  const static uint8_t fgValue = bgValue;
  int width = stack.width();
  int height = stack.height();
  const uint8_t *array = stack.array8();

  ZIntCuboid cuboid0 = stack.getBoundBox();
  ZIntCuboid cuboid;
  cuboid.set(cuboid0.getLastCorner(), cuboid0.getFirstCorner());
  //Left bound
  for (int y = 0; y < height; y += 100) {
    size_t offset = width * y;
    for (int x = 0; x < width; ++x) {
      if (array[offset++] != fgValue) {
        if (x < cuboid.getFirstCorner().getX()) {
          cuboid.setFirstX(x);
          break;
        }
      }
    }
  }

  //Right bound
  for (int y = 0; y < height; y += 100) {
    size_t offset = width * y + width - 1;
    for (int x = width - 1; x >= 0; --x) {
      if (array[offset--] != fgValue) {
        if (x > cuboid.getLastCorner().getX()) {
          cuboid.setLastX(x);
          break;
        }
      }
    }
  }

  //Up bound
  for (int x = 0; x < width; x += 100) {
    size_t offset =x;
    for (int y = 0; y < height; ++y) {
      if (array[offset] != fgValue) {
        if (y < cuboid.getFirstCorner().getY()) {
          cuboid.setFirstY(y);
          break;
        }
      }
      offset += width;
    }
  }

  //Down bound
  size_t area = width * height;
  for (int x = 0; x < width; x += 100) {
    size_t offset = area - 1 - x;
    for (int y = height - 1; y >= 0; --y) {
      if (array[offset] != fgValue) {
        if (y > cuboid.getLastCorner().getY()) {
          cuboid.setLastY(y);
          break;
        }
      }
      offset -= width;
    }
  }

  return cuboid;
}

std::string ZFlyEmRoiProject::getMinRoiKey() const
{
  return getRoiKey(m_dvidInfo.getMinZ());
}

std::string ZFlyEmRoiProject::getMaxRoiKey() const
{
  return getRoiKey(m_dvidInfo.getMaxZ());
}

std::string ZFlyEmRoiProject::getRoiKey(int z) const
{
  ZString key = getName();
  if (!key.empty()) {
    key += "_";
    key.appendNumber(z, 7);
  }

  return key;
}

double ZFlyEmRoiProject::estimateRoiVolume(char unit) const
{
//  double totalVolume = 0;
////  size_t z1 = 0;
////  size_t z2 = 0;
////  size_t v1 = 0;
////  size_t v2 = 0;

////  bool hitFirst = false;

//  int z1 = getFirstRoiZ();
//  int z2 = getLastRoiZ();

//  ZObject3dScan obj;
//  for (int z = z1; z <= z2; ++z) {
//    ZClosedCurve *curve = m_curveArray[z];
//    ZClosedCurve *tmpCurve = NULL;
//    if (curve == NULL) {
//      tmpCurve = estimateRoi(z, NULL);
//      curve = tmpCurve;
//    }
//    getFilledRoi(curve, z, &obj);
//    size_t v = obj.getVoxelNumber();
//    totalVolume += v;

//    delete tmpCurve;
//  }


////  for (size_t z = 0; z < m_curveArray.size(); ++z) {
////     else {
////      hitFirst = true;
////    }

////    if (curve != NULL) {
////      if (!curve->isEmpty()) {
////         = getFilledRoi(z);
//////        obj.downsampleMax(19, 19, 19);
////#ifdef _DEBUG_2
////        obj.save(GET_TEST_DATA_DIR + "/test.sobj");
////#endif
//////        size_t v = obj.getVoxelNumber() * 20 * 20;
////        totalVolume += v;
////        if (v1 == 0) {
////          v1 = v;
////          z1 = z;
////        } else {
////          v2 = v;
////          z2 = z;
////        }
////        if (v1 > 0 && v2 > 0) {
////          totalVolume += 0.5 * (v1 + v2) * (z2 - z1 - 1);
////          v1 = v2;
////          z1 = z2;
////          v2 = 0;
////          z2 = 0;
////        }
////      }
////    }
////  }


  ZObject3dScan obj = getRoiObject(19, 19, 19);

#ifdef _DEBUG_
  obj.save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

  size_t totalVolume = obj.getVoxelNumber() * 20 * 20 * 20;
  ZResolution res = m_dvidInfo.getVoxelResolution();
  res.convertUnit(unit);

  return totalVolume * res.voxelSize();
}

void ZFlyEmRoiProject::setDsIntv(int xintv, int yintv, int zintv)
{
  m_currentDsIntv.set(xintv, yintv, zintv);
}

void ZFlyEmRoiProject::setDocData(const ZStackDocReader &docReader)
{
  if (m_dataFrame != NULL) {
    m_dataFrame->clearData();
    m_dataFrame->addDocData(docReader);
  }
}

void ZFlyEmRoiProject::loadSynapse(const std::string &filePath, bool isVisible)
{
  m_synapseArray.clear();
  m_puncta.clear();
  const double radius = m_defaultSynpaseRadius;
  switch (ZFileType::fileType(filePath)) {
  case ZFileType::JSON_FILE:
  {
    m_synapseArray.loadJson(filePath);
    m_puncta = m_synapseArray.toTBarPuncta(radius);
  }
    break;
  case ZFileType::TXT_FILE:
  {
    FILE *fp = fopen(filePath.c_str(), "r");
    ZString line;
    while (line.readLine(fp)) {
      std::vector<int> pt = line.toIntegerArray();
      if (pt.size() >= 3) {
        ZPunctum *punctum = new ZPunctum;
        punctum->setCenter(pt[0], pt[1], pt[2]);
        punctum->setRadius(radius);
        punctum->setColor(255, 255, 255, 255);
        punctum->setVisible(isVisible);
        if (pt.size() >= 4) {
          int label = pt[3];
          punctum->setName(QString("%1").arg(label));
          int colorLabel = label;
          if (colorLabel >= (int) m_punctaColorMap.size()) {
            colorLabel %= (m_punctaColorMap.size() - 1);
            colorLabel += 1;
          }
          punctum->setColor(m_punctaColorMap[colorLabel]);
        }
        m_puncta.push_back(punctum);
      }
    }
    fclose(fp);
  }
    break;
  default:
    break;
  }

  std::sort(m_puncta.begin(), m_puncta.end(), ZPunctum::ZCompare());
  updateSynapse();
}

void ZFlyEmRoiProject::setSynapseVisible(bool isVisible)
{
  if (m_dataFrame != NULL) {
    QList<ZPunctum*> puncta = m_dataFrame->document()->getPunctumList();
    for (QList<ZPunctum*>::iterator iter = puncta.begin();
         iter != puncta.end(); ++iter) {
      ZPunctum *punctum = *iter;
      punctum->setVisible(isVisible);
    }
    m_dataFrame->document()->notifyPunctumModified();
  }
}

ZStackFrame *ZFlyEmRoiProject::makeAllSynapseFrame() const
{
  ZStackFrame *frame = NULL;
  if (!m_puncta.empty()) {
    //frame = new ZStackFrame();
    frame = ZStackFrame::Make(NULL);
    frame->document()->addPunctum(makePunctumList(false));

    if (hasDataFrame()) {
      if (m_dataFrame->document()->hasStackData()) {
        frame->document()->loadStack(
              m_dataFrame->document()->getStack()->clone());
      }
    }
  }

  return frame;
}

ZStackDoc* ZFlyEmRoiProject::makeAllSynapseDoc() const
{
  ZStackDoc *doc = NULL;
  if (!m_puncta.empty()) {
    doc = new ZStackDoc;
    doc->addPunctumFast(makePunctumList(false));

    if (hasDataFrame()) {
      if (m_dataFrame->document()->hasStackData()) {
        doc->loadStack(m_dataFrame->document()->getStack()->clone());
      }
    }
  }

  return doc;
}

void ZFlyEmRoiProject::clearRoi()
{
  m_curveArray.clear();
}

void ZFlyEmRoiProject::importRoiFromSwc(ZSwcTree *tree)
{
  clearRoi();
  if (tree != NULL) {
    ZSwcForest *forest = tree->toSwcTreeArray();
    for (ZSwcForest::iterator iter = forest->begin();
         iter != forest->end(); ++iter) {
      ZSwcTree *roiSwc = *iter;
      ZSwcTree::DepthFirstIterator swcIter(roiSwc);
      ZClosedCurve *roiCurve = new ZClosedCurve;
      double z = 0.0;
      for (Swc_Tree_Node *tn = swcIter.begin(); tn != NULL; tn = swcIter.next()) {
        if (SwcTreeNode::isRegular(tn)) {
          z = SwcTreeNode::z(tn);
          roiCurve->append(SwcTreeNode::center(tn));
        }
      }
      setRoi(roiCurve, iround(z));
    }
    delete forest;
  }
}

ZFlyEmRoiProject* ZFlyEmRoiProject::clone(const std::string &name) const
{
  ZFlyEmRoiProject *project = new ZFlyEmRoiProject(name, parent());
  project->setDvidTarget(m_dvidTarget);
  project->m_curveArray.resize(m_curveArray.size());
  for (size_t i = 0; i < m_curveArray.size(); ++i) {
    if (m_curveArray[i] != NULL) {
      project->m_curveArray[i] = m_curveArray[i]->clone();
    }
  }

  return project;
}

void ZFlyEmRoiProject::test()
{
  std::ofstream stream((GET_TEST_DATA_DIR + "/flyem/AL/roi_area.txt").c_str());
  for (size_t z = 0; z < m_curveArray.size(); ++z) {
    const ZClosedCurve *curve = m_curveArray[z];
    if (curve != NULL) {
      if (!curve->isEmpty()) {
        size_t v = getFilledRoi(z).getVoxelNumber();
        stream << z << " " << v << std::endl;
      }
    }
  }
  stream.close();
}
