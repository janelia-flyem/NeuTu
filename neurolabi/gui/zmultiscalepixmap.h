#ifndef ZMULTISCALEPIXMAP_H
#define ZMULTISCALEPIXMAP_H

#include <vector>
#include <QSize>
#include <QPoint>

class ZPixmap;

class ZMultiscalePixmap
{
public:
  ZMultiscalePixmap();
  ~ZMultiscalePixmap();

  void setSize(const QSize &size);
  void setOffset(const QPoint &offset);
  void clear();

  ZPixmap* getPixmap(int level);

  int getScale(int level) const;

  int getWidth() const;
  int getHeight() const;

  int getTx() const;
  int getTy() const;

  void setVisible(bool visible);

  bool isEmpty() const;

private:
  void init();

private:
  std::vector<ZPixmap*> m_pixmapArray;
  QSize m_originalSize;
  QPoint m_offset;

  bool m_visible;
};

#endif // ZMULTISCALEPIXMAP_H
