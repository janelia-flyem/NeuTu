#ifndef ZPUNCTA_H
#define ZPUNCTA_H

#include <QList>

#include "zstackobject.h"
#include "zpunctum.h"

class ZPuncta : public ZStackObject
{
public:
  ZPuncta();
  virtual ~ZPuncta();

  virtual const std::string& className() const;


public:
  void clear();
  void addPunctum(ZPunctum *p, bool ignoreNull = true);

  void display(ZPainter &painter, int slice, EDisplayStyle option,
               NeuTube::EAxis sliceAxis) const;

  bool load(const std::string &filePath, double radius = 3.0);
#if defined(_FLYEM_)
  bool load(const ZJsonObject &obj, double radius = 3.0);
#endif
  void sort() const;

  template<typename InputIterator>
  void addPunctum(InputIterator first, InputIterator last);

  void pushCosmeticPen(bool state);
  void pushColor(const QColor &color);
  void pushVisualEffect(NeuTube::Display::TVisualEffect effect);

private:
  void setSorted(bool state) const;

private:
  mutable QList<ZPunctum*> m_puncta;
  mutable bool m_isSorted;
};

template<typename InputIterator>
void ZPuncta::addPunctum(InputIterator first, InputIterator last)
{
  for(InputIterator iter = first; iter != last; ++iter) {
    addPunctum(*iter);
  }
}

#endif // ZPUNCTA_H
