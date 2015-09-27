#include "zslicedpuncta.h"
#include "zpainter.h"
#include "flyem/zsynapseannotationarray.h"
#include "zfiletype.h"
#include "zstring.h"
#include "zcolorscheme.h"

ZSlicedPuncta::ZSlicedPuncta()
{
  m_type = ZStackObject::TYPE_SLICED_PUNCTA;
  m_zStart = 0;
}


ZSlicedPuncta::~ZSlicedPuncta()
{
  clear();
}

void ZSlicedPuncta::addPunctum(ZPunctum *p, bool ignoreNull)
{
  if (p != NULL || ignoreNull) {
    int z = iround(p->getZ()) - m_zStart;
    if (m_puncta.size() <= z) {
      m_puncta.resize(z + 1);
    }

    QList<ZPunctum*> &punctumList = m_puncta[z];

    punctumList.append(p);
  }
}

void ZSlicedPuncta::display(ZPainter &painter, int slice, EDisplayStyle option) const
{
  if (m_puncta.isEmpty() || slice < 0) {
    return;
  }

  int z = painter.getZ(slice);

  int lowerIndex = std::max(0, z - m_zStart - 5);
  int upperIndex = std::min(m_puncta.size() - 1, z + m_zStart + 5);


  for (int index = lowerIndex; index <= upperIndex; ++index) {
    const QList<ZPunctum*> punctumArray = m_puncta[index];

    for (QList<ZPunctum*>::const_iterator iter = punctumArray.begin();
         iter != punctumArray.end(); ++iter) {
      const ZPunctum* punctum = *iter;
      punctum->display(painter, slice, option);
    }
  }

}

void ZSlicedPuncta::clear()
{
  for (QVector<QList<ZPunctum*> >::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    QList<ZPunctum*> &punctumList = *iter;
    for (QList<ZPunctum*>::iterator iter = punctumList.begin();
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
  std::vector<ZPunctum*> puncta = synapseArray.toTBarPuncta(radius);
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
        ZPunctum *punctum = new ZPunctum;
        punctum->setCenter(pt[0], pt[1], pt[2]);
        punctum->setRadius(radius);
        punctum->setColor(255, 255, 255, 255);

        if (pt.size() >= 4) {
          int label = pt[3];
          punctum->setName(QString("%1").arg(label));
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
  for (QVector<QList<ZPunctum*> >::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    QList<ZPunctum*> &punctumList = *iter;
    for (QList<ZPunctum*>::iterator iter = punctumList.begin();
         iter != punctumList.end(); ++iter) {
      ZPunctum *p = *iter;
      p->useCosmeticPen(state);
    }
  }
}

void ZSlicedPuncta::pushColor(const QColor &color)
{
  for (QVector<QList<ZPunctum*> >::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    QList<ZPunctum*> &punctumList = *iter;
    for (QList<ZPunctum*>::iterator iter = punctumList.begin();
         iter != punctumList.end(); ++iter) {
      ZPunctum *p = *iter;
      p->setColor(color);
    }
  }
}

void ZSlicedPuncta::pushVisualEffect(NeuTube::Display::TVisualEffect effect)
{
  for (QVector<QList<ZPunctum*> >::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    QList<ZPunctum*> &punctumList = *iter;
    for (QList<ZPunctum*>::iterator iter = punctumList.begin();
         iter != punctumList.end(); ++iter) {
      ZPunctum *p = *iter;
      p->setVisualEffect(effect);
    }
  }
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZSlicedPuncta)


