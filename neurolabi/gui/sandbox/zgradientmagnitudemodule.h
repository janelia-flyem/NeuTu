#ifndef ZGRADIENTMAGNITUDEMODULE_H
#define ZGRADIENTMAGNITUDEMODULE_H
#include <map>
#include <string>
#include <QWidget>
#include <QString>
#include <QMessageBox>
#include "zsandboxmodule.h"
#include "zstackdoc.h"


/* GradientStrategy
 * abstract base class of computing gradient
 * and subclasses should override _run method*/
template<typename T>
class GradientStrategy
{
public:
  GradientStrategy();
  //virtual ~GradientStrategy();
  void run(const T* in,T* out,uint width,uint height,uint depth,bool reverse);
protected:
  virtual void _run(const T* in,T* out)=0;
private:
  void reverse(T* begin,T* end);
protected:
  double _max;
  uint _width,_height,_depth;
  size_t _slice,_total;
};


/* simple gradient strategy which uses df/dx=[f(x+1)-f(x-1)]/2*/
template<typename T>
class GradientStrategySimple:public GradientStrategy<T>
{
protected:
  void _run(const T* in,T* out);
};


/*GradientStrategyContext is the user interface to use gradient strategy classes*/
class GradientStrategyContext
{
public:
  typedef enum
  {
    SIMPLE=0
  }StrategyType;
public:
  GradientStrategyContext(StrategyType strategy_type);
  ~GradientStrategyContext();
  void run(const ZStack* in,ZStack* out,bool reverse);
private:
  template<typename T>
  GradientStrategy<T>* getStrategy()
  {
    GradientStrategy<T>* strategy=0;
    switch(_type)
    {
      case SIMPLE:
          strategy=new GradientStrategySimple<T>;
          break;
      default:
          strategy=0;
          break;
    }
    return strategy;
  }
  template<typename T>
  void _run(const ZStack* in,ZStack* out,bool reverse);
private:
  StrategyType _type;
};


class QComboBox;
class QRadioButton;
class QPushButton;

class ZSelectGradientStrategyWindow:public QWidget
{
  Q_OBJECT
public:
  ZSelectGradientStrategyWindow(QWidget *parent = 0);
private slots:
  void onStart();
private:
  QPushButton* start_gradient_magnitude;
  QComboBox*   strategies;
  QRadioButton*   reverse;
  std::map<QString,GradientStrategyContext::StrategyType>strategy_map;
};



class ZGradientMagnitudeModule : public ZSandboxModule
{
  Q_OBJECT
public:
  explicit ZGradientMagnitudeModule(QObject *parent = 0);
  ~ZGradientMagnitudeModule();
signals:

public slots:

private slots:
  void execute();
private:
  void init();

private:
  ZSelectGradientStrategyWindow* select_strategy_window;
};

#endif // ZGRADIENTMAGNITUDEMODULE_H
