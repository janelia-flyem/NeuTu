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
  void processMessage(ZMessage *message);
  void dispatchMessage(ZMessage *message);
  void reportMessage(ZMessage *message);

  void registerWidget(QWidget *widget);

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

  bool hasProcessor() const;

  void print() const;

signals:

public slots:
  void detachWidget();

private:
  ZMessageManager* findChildManager(QWidget *widget) const;
  std::string toLine() const;
  ZTextLineCompositer toLineCompositer(int level = 0) const;

private:
  QWidget *m_widget;
  ZSharedPointer<ZMessageProcessor> m_processor;
};

#endif // ZMESSAGEMANAGER_H
