#ifndef ZFFNSKELETON_H
#define ZFFNSKELETON_H

#include <vector>
#include "zsandboxmodule.h"
#include "zintcuboid.h"
#include "zswctree.h"

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

public:

private:

  //ZIntCuboid m_working_box;
  ZSwcTree* m_skeleton;
  std::vector<ZStack*> m_segs;
};
#endif // ZFFNSKELETON_H
