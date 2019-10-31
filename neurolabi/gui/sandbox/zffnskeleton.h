#ifndef ZFFNSKELETON_H
#define ZFFNSKELETON_H

#include <vector>
#include <map>
#include <set>
#include "zsandboxmodule.h"
#include "geometry/zintcuboid.h"
#include "zswctree.h"
#include "mvc/zstackdoc.h"

class ZFFNSkeletonModule : public ZSandboxModule
{
  Q_OBJECT
public:
  explicit ZFFNSkeletonModule(QObject *parent = 0);

signals:

public slots:

private slots:
  void execute();

private:
  void init();
  std::vector<ZIntPoint> getSeedPos(ZStackDoc* doc);
  ZStack* getFFNSegmentation(ZIntPoint start_pos,ZIntPoint end_pos,ZIntPoint seed_pos);
  void getMask(std::set<ZIntPoint>& mask,ZStack* stackMask);
  void connectSwcTree(ZSwcTree* first,ZSwcTree* second);
  void trace(const ZIntPoint& pos);
public:

private:
  std::vector<ZSwcTree*> m_skeleton;
  std::vector<std::set<ZIntPoint> *> m_segs;
  ZStack* m_src;
  ZStackDoc* m_doc;
  int m_resolution;
};
#endif // ZFFNSKELETON_H
