#ifndef ZSTACKMVC_H
#define ZSTACKMVC_H

#include <QObject>

#include "zsharedpointer.h"

class ZStackDoc;
class ZStackView;
class ZStackPresenter;

class ZStackMvc : public QObject
{
  Q_OBJECT
public:
  explicit ZStackMvc(QObject *parent = 0);

  void attachDocument(ZSharedPointer<ZStackDoc> doc);
  void detachDocument();

signals:
  void stackChanged();
  void objectChanged();
  void objectSelectionChanged();

public slots:

private:
  void createView();
  void createPresenter();

private:
  ZSharedPointer<ZStackDoc> m_doc;
  ZStackPresenter *m_presenter;
  ZStackView *m_view;
};

#endif // ZSTACKMVC_H
