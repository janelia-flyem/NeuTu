#ifndef ZFLYEMARBMVC_H
#define ZFLYEMARBMVC_H

#include "flyem/zflyemproofmvc.h"

class ZFlyEmArbDoc;
class ZArbSliceViewParam;
class ZFlyEmArbPresenter;

class ZFlyEmArbMvc : public ZFlyEmProofMvc
{
  Q_OBJECT

public:
  explicit ZFlyEmArbMvc(QWidget *parent = 0);

  static ZFlyEmArbMvc* Make(QWidget *parent, ZSharedPointer<ZFlyEmArbDoc> doc);
  static ZFlyEmArbMvc* Make(const ZDvidTarget &target);

  ZFlyEmArbDoc* getCompleteDocument() const;
  ZFlyEmArbPresenter* getCompletePresenter() const;

  void setDvidTarget(const ZDvidTarget &target);

  void updateViewParam(const ZArbSliceViewParam &param);

protected:
  void createPresenter();

private:
  void init();
};

#endif // ZFLYEMARBMVC_H
