#include "zslicedpuncta.h"
#include "zpainter.h"
#include "flyem/zsynapseannotationarray.h"
#include "zfiletype.h"
#include "zstring.h"
#include "zcolorscheme.h"
#include "zstackball.h"
#include "tz_math.h"

const int ZSlicedPuncta::m_visibleRange = 5.0;

ZSlicedPuncta::ZSlicedPuncta()
{
  m_type = ZStackObject::TYPE_SLICED_PUNCTA;
  m_zStart = 0;
}


ZSlicedPuncta::~ZSlicedPuncta()
{
  clear();
}

void ZSlicedPuncta::addPunctum(ZStackBall *p, bool ignoreNull)
{
  if (p != NULL || ignoreNull) {
    int z = iround(p->getZ()) - m_zStart;

#ifdef _DEBUG_2
    if (z == 9447) {
      std::cout << "debug here" << std::endl;
    }
#endif

    if (m_puncta.size() <= z) {
      m_puncta.resize(z + 1);
    }

    QList<ZStackBall*> &punctumList = m_puncta[z];

    punctumList.append(p);
  }
}

void ZSlicedPuncta::display(ZPainter &painter, int slice, EDisplayStyle option,
                            NeuTube::EAxis sliceAxis) const
{
  if (sliceAxis != NeuTube::Z_AXIS) {
    return;
  }

  if (m_puncta.isEmpty() || slice < 0) {
    return;
  }

  int z = painter.getZ(slice);

  int lowerIndex = std::max(0, z - m_zStart - m_visibleRange);
  int upperIndex = std::min(m_puncta.size() - 1, z + m_zStart + m_visibleRange);


  for (int index = lowerIndex; index <= upperIndex; ++index) {
    const QList<ZStackBall*> punctumArray = m_puncta[index];

    for (QList<ZStackBall*>::const_iterator iter = punctumArray.begin();
         iter != punctumArray.end(); ++iter) {
      const ZStackBall* punctum = *iter;
      punctum->display(painter, slice, option, sliceAxis);
    }
  }

}

void ZSlicedPuncta::clear()
{
  for (QVector<QList<ZStackBall*> >::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    QList<ZStackBall*> &punctumList = *iter;
    for (QList<ZStackBall*>::iterator iter = punctumList.begin();
         iter != punctumList.end(); ++iter) {
      delete *iter;
    }
  }
  m_puncta.clear();
}


bool ZSlicedPuncta::load(const ZJsonObject &obj, double radius)
{
  clear();
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(obj);
  std::vector<ZStackBall*> puncta = synapseArray.toTBarBall(radius);
  addPunctum(puncta.begin(), puncta.end());

  return true;
}

bool ZSlicedPuncta::load(const std::string &filePath, double radius)
{
  clear();

  bool succ = false;

  switch (ZFileType::fileType(filePath)) {
  case ZFileType::JSON_FILE:
  {
    ZJsonObject obj;
    obj.load(filePath);
    succ = load(obj, radius);
  }
    break;
  case ZFileType::TXT_FILE:
  {
    ZColorScheme scheme;
    scheme.setColorScheme(ZColorScheme::PUNCTUM_TYPE_COLOR);
    FILE *fp = fopen(filePath.c_str(), "r");
    ZString line;
    while (line.readLine(fp)) {
      std::vector<int> pt = line.toIntegerArray();
      if (pt.size() >= 3) {
        ZStackBall *punctum = new ZStackBall;
        punctum->setCenter(pt[0], pt[1], pt[2]);
        punctum->setRadius(radius);
        punctum->setColor(255, 255, 255, 255);

        if (pt.size() >= 4) {
          int label = pt[3];
//          punctum->setName(QString("%1").arg(label));
          punctum->setColor(scheme.getColor(label));
        }
        addPunctum(punctum);
//        m_puncta.push_back(punctum);
      }
    }
    fclose(fp);
  }
    succ = true;
    break;
  default:
    break;
  }
  return succ;
}

void ZSlicedPuncta::pushCosmeticPen(bool state)
{
  for (QVector<QList<ZStackBall*> >::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    QList<ZStackBall*> &punctumList = *iter;
    for (QList<ZStackBall*>::iterator iter = punctumList.begin();
         iter != punctumList.end(); ++iter) {
      ZStackBall *p = *iter;
      p->useCosmeticPen(state);
    }
  }
}

void ZSlicedPuncta::pushColor(const QColor &color)
{
  for (QVector<QList<ZStackBall*> >::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    QList<ZStackBall*> &punctumList = *iter;
    for (QList<ZStackBall*>::iterator iter = punctumList.begin();
         iter != punctumList.end(); ++iter) {
      ZStackBall *p = *iter;
      p->setColor(color);
    }
  }
}

void ZSlicedPuncta::pushVisualEffect(NeuTube::Display::TVisualEffect effect)
{
  for (QVector<QList<ZStackBall*> >::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    QList<ZStackBall*> &punctumList = *iter;
    for (QList<ZStackBall*>::iterator iter = punctumList.begin();
         iter != punctumList.end(); ++iter) {
      ZStackBall *p = *iter;
      p->setVisualEffect(effect);
    }
  }
}

QList<ZStackBall*> ZSlicedPuncta::getPunctaOnSlice(int z)
{
  int index = z - m_zStart;
  if (index >= 0 && index < m_puncta.size()) {
    return m_puncta[index];
  }

  return QList<ZStackBall*>();
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZSlicedPuncta)


