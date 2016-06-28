#ifndef ZSLICEDPUNCTA_H
#define ZSLICEDPUNCTA_H

#include <QList>
#include <QVector>

#include "zstackobject.h"
#include "zstackobjectselector.h"
//#include "zpunctum.h"

class ZStackBall;
class ZJsonObject;

class ZSlicedPuncta : public ZStackObject
{
public:
  ZSlicedPuncta();

  virtual ~ZSlicedPuncta();

  virtual const std::string& className() const;

public:
  void clear();
  void addPunctum(ZStackBall *p, bool ignoreNull = true);

  void display(ZPainter &painter, int slice, EDisplayStyle option,
               NeuTube::EAxis sliceAxis) const;

  bool load(const std::string &filePath, double radius = 3.0);
  bool load(const ZJsonObject &obj, double radius = 3.0);

  template<typename InputIterator>
  void addPunctum(InputIterator first, InputIterator last);

  void pushCosmeticPen(bool state);
  void pushColor(const QColor &color);
  void pushVisualEffect(NeuTube::Display::TVisualEffect effect);

  QList<ZStackBall*> getPunctaOnSlice(int z);

private:
  QVector<QList<ZStackBall*> > m_puncta;
  int m_zStart;
  static const int m_visibleRange;
  mutable ZStackObjectSelector m_selector;
};

template<typename InputIterator>
void ZSlicedPuncta::addPunctum(InputIterator first, InputIterator last)
{
  for(InputIterator iter = first; iter != last; ++iter) {
    addPunctum(*iter);
  }
}

#endif // ZSLICEDPUNCTA_H
