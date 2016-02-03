#ifndef ZFLYEMORTHOWIDGET_H
#define ZFLYEMORTHOWIDGET_H

#include <QWidget>

class ZDvidTarget;
class ZFlyEmOrthoMvc;
class ZFlyEmOrthoDoc;

class ZFlyEmOrthoWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ZFlyEmOrthoWidget(const ZDvidTarget &target, QWidget *parent = 0);

  ZFlyEmOrthoDoc *getDocument() const;

signals:

public slots:

private:
  void init(const ZDvidTarget &target);

private:
  ZFlyEmOrthoMvc *m_xyMvc;
  ZFlyEmOrthoMvc *m_yzMvc;
  ZFlyEmOrthoMvc *m_xzMvc;
};

#endif // ZFLYEMORTHOWIDGET_H
