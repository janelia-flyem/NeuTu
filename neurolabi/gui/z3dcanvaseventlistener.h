#ifndef Z3DCANVASEVENTLISTENER_H
#define Z3DCANVASEVENTLISTENER_H

#include <QEvent>

// interface for all class that can receive z3dcanvas event
// listener can be registed with the addEventListener(tofront or toback) function of Z3DCanvas
// note: the listeners are called from front to back until event is accepted, so the order does matter.
//       If you want other listeners to receive the event too, don't accept it (set the sharing of ZEventListenerParameter
//       to true).
//
class Z3DCanvasEventListener
{
public:
  virtual void onEvent(QEvent *e, int w, int h) = 0;
  void setListenerName(const std::string &name) {
    m_name = name;
  }
  std::string getListnerName() const {
    return m_name;
  }

private:
  std::string m_name;
};

#endif // Z3DCANVASEVENTLISTENER_H
