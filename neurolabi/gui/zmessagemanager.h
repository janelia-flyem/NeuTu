#ifndef ZMESSAGEMANAGER_H
#define ZMESSAGEMANAGER_H

#include <QObject>
#include "zsharedpointer.h"

class ZMessage;
class QWidget;
class ZMessageProcessor;
class ZTextLineCompositer;

class ZMessageManager : public QObject
{
  Q_OBJECT
public:
  explicit ZMessageManager(QObject *parent = 0);
  void processMessage(ZMessage *message, bool reporting);
  void dispatchMessage(ZMessage *message);
  void reportMessage(ZMessage *message);

  /*!
   * \brief Register widget
   *
   * \a parent is not NULL: the current manager will become a child of \a parent.
   * \a parent is NULL: when \a widget has a registered parent and
   *    \a updatingParent is set to true,
   *    the parent of the manager becomes the manager associated with its
   *    parent; otherwise its parent is set to the global manager root.
   *
   * To make the manager with NULL parent, set \a parent to NULL and
   * \a updateParent() to false.
   *
   * \param widget widget to register
   */
  void registerWidget(QWidget *widget, ZMessageManager *parent = NULL,
                      bool updatingParent = true);


  static ZMessageManager& getRootManager() {
    static ZMessageManager manager;

    return manager;
  }

  inline const QWidget* getWidget() const { return m_widget; }

  void updateParent();

  /*!
   * \brief Set message processor
   */
  void setProcessor(ZSharedPointer<ZMessageProcessor> processor);
  void setProcessor(ZMessageProcessor *processor);

  bool hasProcessor() const;

  void print() const;

  template <typename TProcessor>
  static ZMessageManager* Make(QWidget *widget, ZMessageManager *parent = NULL);

  template <typename TProcessor>
  static ZMessageManager* MakeNoParent(QWidget *widget);


  static ZMessageManager* Make(
      QWidget *widget, ZMessageManager *parent = NULL);

  bool hasParent() const;

signals:

public slots:
  void detachWidget();

private:
  ZMessageManager* findChildManager(const QWidget *widget) const;
  std::string toLine() const;
  ZTextLineCompositer toLineCompositer(int level = 0) const;
  ZMessageManager* getPotentialParent() const;

private:
  QWidget *m_widget;
  ZSharedPointer<ZMessageProcessor> m_processor;
};

template <typename TProcessor>
ZMessageManager* ZMessageManager::Make(QWidget *widget, ZMessageManager *parent)
{
  ZMessageManager *messageManager = new ZMessageManager();
  messageManager->setProcessor(new TProcessor);
  messageManager->registerWidget(widget, parent);

  return messageManager;
}

template <typename TProcessor>
ZMessageManager* ZMessageManager::MakeNoParent(QWidget *widget)
{
  ZMessageManager *messageManager = new ZMessageManager();
  messageManager->setProcessor(new TProcessor);
  messageManager->registerWidget(widget, NULL, false);

  return messageManager;
}

#endif // ZMESSAGEMANAGER_H
