#include "zpuncta.h"
#include "zpainter.h"
#if defined(_FLYEM_)
#include "flyem/zsynapseannotationarray.h"
#endif

#include "zfiletype.h"
#include "zstring.h"
#include "zcolorscheme.h"
#include "zjsonobject.h"

ZPuncta::ZPuncta()
{
  m_type = ZStackObject::TYPE_PUNCTA;
  m_isSorted = false;
}

ZPuncta::~ZPuncta()
{
  for (QList<ZPunctum*>::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    delete *iter;
  }
}

void ZPuncta::addPunctum(ZPunctum *p, bool ignoreNull)
{
  if (p != NULL || ignoreNull) {
//    p->useCosmeticPen(m_usingCosmeticPen);
    m_puncta.append(p);
    m_isSorted = false;
  }
}

void ZPuncta::sort() const
{
  if (!m_isSorted) {
    std::sort(m_puncta.begin(), m_puncta.end(), ZPunctum::ZCompare());
    m_isSorted = true;
  }
}

void ZPuncta::display(ZPainter &painter, int slice, EDisplayStyle option,
                      NeuTube::EAxis sliceAxis) const
{
  if (m_puncta.isEmpty() || slice < 0 || sliceAxis != NeuTube::Z_AXIS) {
    return;
  }

  sort();

  int z = painter.getZ(slice);

  double range = m_puncta.first()->getRadius();

  ZPunctum markPunctum;
  markPunctum.setZ(z - range);
  //Find the puncta range
  QList<ZPunctum*>::const_iterator beginIter =
      std::lower_bound(m_puncta.begin(), m_puncta.end(),
                       &markPunctum, ZPunctum::ZCompare());

  markPunctum.setZ(z + range);
  QList<ZPunctum*>::const_iterator endIter =
      std::upper_bound(m_puncta.begin(), m_puncta.end(),
                       &markPunctum, ZPunctum::ZCompare());

  //Add to document
  for (QList<ZPunctum*>::const_iterator iter = beginIter;
       iter != endIter; ++iter) {
    const ZPunctum* punctum = *iter;
    punctum->display(painter, slice, option, sliceAxis);
  }
}

void ZPuncta::clear()
{
  for (QList<ZPunctum*>::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    delete *iter;
  }

  m_puncta.clear();
  m_isSorted = false;
}

void ZPuncta::setSorted(bool state) const
{
  m_isSorted = state;
}
#if defined(_FLYEM_)
bool ZPuncta::load(const ZJsonObject &obj, double radius)
{
  clear();
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(obj);
  std::vector<ZPunctum*> puncta = synapseArray.toTBarPuncta(radius);
  addPunctum(puncta.begin(), puncta.end());

  return true;
}
#endif
bool ZPuncta::load(const std::string &filePath, double radius)
{
  clear();

  bool succ = false;

  switch (ZFileType::fileType(filePath)) {
#if defined(_FLYEM_)
  case ZFileType::JSON_FILE:
  {
    ZJsonObject obj;
    obj.load(filePath);
    succ = load(obj, radius);
  }
    break;
#endif
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
        m_puncta.push_back(punctum);
      }
    }
    fclose(fp);
  }
    succ = true;
    break;
  default:
    break;
  }

  sort();

  return succ;
}

void ZPuncta::pushCosmeticPen(bool state)
{
  for (QList<ZPunctum*>::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    ZPunctum *p = *iter;
    p->useCosmeticPen(state);
  }
}

void ZPuncta::pushColor(const QColor &color)
{
  for (QList<ZPunctum*>::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    ZPunctum *p = *iter;
    p->setColor(color);
  }
}

void ZPuncta::pushVisualEffect(NeuTube::Display::TVisualEffect effect)
{
  for (QList<ZPunctum*>::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    ZPunctum *p = *iter;
    p->setVisualEffect(effect);
  }
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZPuncta)
