#ifndef ZDVIDGRAYSLICEHIGHRESTASK_H
#define ZDVIDGRAYSLICEHIGHRESTASK_H

#include "ztask.h"
#include "zstackviewparam.h"

class ZStackDoc;

class ZDvidGraySliceHighresTask : public ZTask
{
  Q_OBJECT
public:
  explicit ZDvidGraySliceHighresTask(QObject *parent = nullptr);

  void setViewParam(const ZStackViewParam &param);
  void setZoom(int zoom);
  void setCenterCut(int width, int height);
  void setDoc(ZStackDoc *doc);
  void useCenterCut(bool on);

  void execute();

signals:

public slots:

private:
  ZStackViewParam m_viewParam;
  int m_zoom = 0;
  int m_centerCutWidth = 0;
  int m_centerCutHeight = 0;
  bool m_usingCenterCut = false;
  ZStackDoc *m_doc = nullptr;
};

#endif // ZDVIDGRAYSLICEHIGHRESTASK_H
