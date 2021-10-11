#ifndef ZDVIDDATASLICETASK_H
#define ZDVIDDATASLICETASK_H

#include <string>

#include "ztask.h"
#include "zstackviewparam.h"

class ZStackDoc;

class ZDvidDataSliceTask : public ZTask
{
  Q_OBJECT
public:
  explicit ZDvidDataSliceTask(QObject *parent = nullptr);

  void setViewParam(const ZStackViewParam &param);
  void setZoom(int zoom);
  void setCenterCut(int width, int height);
  void setViewId(int viewId);
  void setDoc(ZStackDoc *doc);
  void useCenterCut(bool on);
  void setSupervoxel(bool on);
  void setHandle(const std::string &handle);

  int getViewId() const;

signals:

public slots:

protected:
  ZStackViewParam m_viewParam;
  int m_zoom = 0;
  int m_centerCutWidth = 0;
  int m_centerCutHeight = 0;
  bool m_usingCenterCut = false;
  bool m_supervoxel = false;
  ZStackDoc *m_doc = nullptr;
  std::string m_handle;
};

#endif // ZDVIDDATASLICETASK_H
