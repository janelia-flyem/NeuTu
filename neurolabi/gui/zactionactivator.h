#ifndef ZACTIONACTIVATOR_H
#define ZACTIONACTIVATOR_H

#include <QAction>
#include <QMenu>
#include <QSet>

class ZStackFrame;

/*!
 * \brief A class of managing actions
 */
class ZActionActivator
{
public:
  ZActionActivator();

  /*!
   * \brief Test if the current doc context is positive
   */
  virtual bool isPositive(const ZStackFrame *frame) const = 0;

  void update(const ZStackFrame *frame);
  void registerAction(QAction *action, bool positive);

private:
  QSet<QAction*> m_postiveActionList;
  QSet<QAction*> m_negativeActionList;
};


/////////////ZStackActionActivator////////////////////
class ZStackActionActivator : public ZActionActivator
{
public:
  bool isPositive(const ZStackFrame *frame) const;
};


#endif // ZACTIONACTIVATOR_H
