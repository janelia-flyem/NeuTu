#ifndef ZFLYEMARBPRESENTER_H
#define ZFLYEMARBPRESENTER_H

#include "flyem/zflyemproofpresenter.h"

#include "zarbsliceviewparam.h"

class ZFlyEmArbPresenter : public ZFlyEmProofPresenter
{
  Q_OBJECT

public:
  explicit ZFlyEmArbPresenter(QWidget *parent = 0);
  static ZFlyEmArbPresenter* Make(QWidget *parent);


  const ZArbSliceViewParam& getSliceViewParam() const {
    return m_viewParam;
  }

//  void setViewParam(const ZArbSliceViewParam &param);


private:
  ZArbSliceViewParam m_viewParam;
};

#endif // ZFLYEMARBPRESENTER_H
