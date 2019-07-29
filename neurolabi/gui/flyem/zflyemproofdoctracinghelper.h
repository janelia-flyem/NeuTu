#ifndef ZFLYEMPROOFDOCTRACINGHELPER_H
#define ZFLYEMPROOFDOCTRACINGHELPER_H

#include <memory>

class ZFlyEmProofDoc;
class ZDvidTracingTaskFactory;
class ZIntPoint;
class ZPoint;

class ZFlyEmProofDocTracingHelper
{
public:
  ZFlyEmProofDocTracingHelper();

  void setDocument(ZFlyEmProofDoc *doc);

  void trace(double x, double y, double z);
  void trace(const ZIntPoint &pt);
  void trace(const ZPoint &pt);

  bool isReady() const;

private:
  ZFlyEmProofDoc *m_doc = nullptr;
  std::shared_ptr<ZDvidTracingTaskFactory> m_taskFactory;
};

#endif // ZFLYEMPROOFDOCTRACINGHELPER_H
