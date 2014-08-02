#include "zflyemroiproject.h"
#include <QStringList>
#include "zstackframe.h"
#include "zswcgenerator.h"
#include "zflyemutilities.h"
#include "zstack.hxx"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidreader.h"

ZFlyEmRoiProject::ZFlyEmRoiProject(QObject *parent) :
  QObject(parent), m_z(-1), m_dataFrame(NULL)
{
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

  shallowClear();
}

void ZFlyEmRoiProject::shallowClear()
{
  m_dataFrame = NULL;
}

void ZFlyEmRoiProject::setDvidTarget(const ZDvidTarget &target)
{
  clear();

  m_dvidTarget = target;
  ZDvidReader reader;
  if (reader.open(target)) {
    m_dvidInfo = reader.readGrayScaleInfo();
    downloadAllRoi();
  }
}

void ZFlyEmRoiProject::downloadAllRoi()
{
  //Download All ROIs
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    QStringList roiIdArray = reader.readKeys(
          "roi_curve", QString("%1").arg(m_dvidInfo.getMinZ()),
          QString("%1").arg(99999));
    foreach (const QString &roiId, roiIdArray) {
      downloadRoi(roiId.toInt());
    }
  }
}

void ZFlyEmRoiProject::showDataFrame() const
{
  if (m_dataFrame != NULL) {
    m_dataFrame->show();
  }
}

bool ZFlyEmRoiProject::hasDataFrame() const
{
  return m_dataFrame != NULL;
}

void ZFlyEmRoiProject::setDataFrame(ZStackFrame *frame)
{
  if (m_dataFrame != NULL) {
    m_dataFrame->hide();
    delete m_dataFrame;
    m_dataFrame = NULL;
  }

  m_dataFrame = frame;
}

void ZFlyEmRoiProject::shallowClearDataFrame()
{
  shallowClear();
}

void ZFlyEmRoiProject::setRoi(ZClosedCurve *roi, int z)
{
  if (roi != NULL && z >= 0) {
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

bool ZFlyEmRoiProject::addRoi()
{
  if (m_dataFrame != NULL) {
    QList<ZSwcTree*>& swcList = m_dataFrame->document()->getSwcList();
    if (swcList.size() == 1) {
      ZClosedCurve curve = swcList.front()->toClosedCurve();
      if(!curve.isEmpty()) {
        ZClosedCurve *roi = new ZClosedCurve(curve);
        setRoi(roi, getDataZ());
        return true;
      }
    }
  }

  return false;
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

ZSwcTree* ZFlyEmRoiProject::getRoiSwc(int z) const
{
  ZSwcTree *tree = NULL;

  const ZClosedCurve *curve = getRoi(z);
  if (curve != NULL) {
    tree = ZSwcGenerator::createSwc(*curve, getMarkerRadius());
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
  ZDvidWriter writer;
  int count = 0;
  if (writer.open(getDvidTarget())) {
    const ZClosedCurve *curve = m_curveArray[z];
    if (curve != NULL) {
      if (!isRoiCurveUploaded(z)) {
        writer.writeRoiCurve(*curve, z);
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

void ZFlyEmRoiProject::downloadRoi(int z)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    ZClosedCurve *curve = reader.readRoiCurve(z, NULL);
    if (curve != NULL) {
      setRoi(curve, z);
      setRoiUploaded(z, true);
    }
  }
}

void ZFlyEmRoiProject::estimateRoi()
{
  if (m_dataFrame != NULL) {
    int z = getDataZ();
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
      ZClosedCurve *lowerCurve = m_curveArray[minZ];
      ZClosedCurve *upperCurve = m_curveArray[maxZ];
      int sampleNumber = imax3(100, lowerCurve->getLandmarkNumber(),
                               upperCurve->getLandmarkNumber());
      ZClosedCurve curve1 = lowerCurve->resampleF(sampleNumber);
      ZClosedCurve curve2 = upperCurve->resampleF(sampleNumber);

      if (curve1.computeDirection().dot(curve2.computeDirection()) < 0) {
        curve2.reverse();
      }
      int shift = curve1.findMatchShift(curve2);
      ZClosedCurve curve3 = curve1.interpolate(
            curve2, ((double) maxZ - z) / (maxZ - minZ), shift).resampleF(50);

      m_dataFrame->document()->removeObject(ZDocPlayer::ROLE_ROI);
      ZSwcTree *tree = ZSwcGenerator::createSwc(curve3, getMarkerRadius());
      m_dataFrame->document()->addObject(tree, NeuTube::Documentable_SWC,
                                         ZDocPlayer::ROLE_ROI);
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

  return m_isRoiCurveUploaded[z];
}

int ZFlyEmRoiProject::findSliceToCreateRoi() const
{
  int minZ = getDvidInfo().getMinZ();
  int maxZ = getDvidInfo().getMaxZ();
  std::pair<int, int> bestSeg(0, 1);

  int length = 0;

  int startZ = minZ;
  int z = startZ;
  for (; z < (int) m_curveArray.size(); ++z) {
    const ZClosedCurve *curve = m_curveArray[z];
    if (curve == NULL) {
      ++length;
    } else {
      int maxLength = bestSeg.second - bestSeg.first - 1;
      if (length > maxLength) {
        length = 0;
        bestSeg.first = startZ;
        bestSeg.second = z;
        startZ = z;
      }
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
