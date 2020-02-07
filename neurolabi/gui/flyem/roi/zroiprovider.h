#ifndef ZROIPROVIDER_H
#define ZROIPROVIDER_H

#include <vector>
#include <string>
#include <memory>

#include <QObject>
#include <QColor>

#include "zcolorscheme.h"

class ZAbstractRoiFactory;
class ZRoiMesh;

class ZRoiProvider : public QObject
{
  Q_OBJECT

public:
  ZRoiProvider(QObject *parent = nullptr);

  size_t getRoiCount() const;
  std::string getRoiName(size_t index) const;
  std::string getRoiStatus(size_t index) const;
  bool isVisible(size_t index) const;
  QColor getRoiColor(size_t index) const;

  ZRoiMesh* getRoiMesh(size_t index) const;

signals:
  void roiUpdated();

private:
  ZAbstractRoiFactory *m_roiFactory;
  std::vector<std::shared_ptr<ZRoiMesh>> m_roiList;
  ZColorScheme m_colorScheme;
};

#endif // ZROIPROVIDER_H
