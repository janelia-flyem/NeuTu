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
  explicit ZFlyEmArbMvc(QWidget *parent = nullptr);

  static ZFlyEmArbMvc* Make(QWidget *parent, ZSharedPointer<ZFlyEmArbDoc> doc);
  static ZFlyEmArbMvc* Make(const ZDvidEnv &env);

  ZFlyEmArbDoc* getCompleteDocument() const;
  ZFlyEmArbPresenter* getCompletePresenter() const;

//  void setDvidTarget(const ZDvidTarget &target);
  void setDvid(const ZDvidEnv &env) override;

//  void updateViewParam(const ZArbSliceViewParam &param);
  void resetViewParam(const ZArbSliceViewParam &param);
  void processViewChangeCustom(const ZStackViewParam &viewParam) override;

signals:
  void sliceViewChanged(const ZArbSliceViewParam &param);
  void locating(double x, double y, double z);

public slots:
  void zoomTo(const ZIntPoint &pt) override;

protected:
  void createPresenter() override;
  void makeViewButtons() override;

//private slots:
//  void updateViewParam(const ZStackViewParam &param);

private:
  void init();
};

#endif // ZFLYEMARBMVC_H
