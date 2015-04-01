#ifndef ZSTACKMVC_H
#define ZSTACKMVC_H

#include <QObject>
#include <QWidget>
#include <QFrame>
#include <QLayout>

#include "zsharedpointer.h"

class ZStackDoc;
class ZStackView;
class ZStackPresenter;

class ZStackMvc : public QWidget
{
  Q_OBJECT
public:
  explicit ZStackMvc(QWidget *parent = 0);

  static ZStackMvc* Make(QWidget *parent, ZSharedPointer<ZStackDoc> doc);

  void attachDocument(ZStackDoc *doc);
  void attachDocument(ZSharedPointer<ZStackDoc> doc);
  void detachDocument();

  ZSharedPointer<ZStackDoc> getDocument() const {
    return m_doc;
  }

  inline ZStackPresenter* getPresenter() const {
    return m_presenter;
  }

  inline ZStackView* getView() const {
    return m_view;
  }

  void connectSignalSlot();
  void disconnectAll();

protected: // Events
  virtual void keyPressEvent(QKeyEvent *event);

signals:
  void stackChanged();
  void objectChanged();
  void objectSelectionChanged();

public slots:

private:
  void createView();
  void createPresenter();
  void dropDocument(ZSharedPointer<ZStackDoc> doc);
  void updateDocument();
  void construct(ZSharedPointer<ZStackDoc> doc);

  static void BaseConstruct(ZStackMvc *frame, ZSharedPointer<ZStackDoc> doc);

private:
  ZSharedPointer<ZStackDoc> m_doc;
  ZStackPresenter *m_presenter;
  ZStackView *m_view;
  QLayout *m_layout;
};

#endif // ZSTACKMVC_H
