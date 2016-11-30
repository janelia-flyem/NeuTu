#ifndef ZGRADIENTMAGNITUDEMODULE_H
#define ZGRADIENTMAGNITUDEMODULE_H
#include <map>
#include <QWidget>
#include <QString>
#include <QMessageBox>
#include "zsandboxmodule.h"
#include "zstackdoc.h"


typedef struct _GradientStrategyParam
{
  _GradientStrategyParam(uint _w,uint _h,uint _d,bool _i):inverse(_i),width(_w),height(_h),depth(_d){}
  bool inverse;
  uint width;
  uint height;
  uint depth;
}GradientStrategyParam;

/* GradientStrategy
 * abstract base class of computing gradient
 * and if magnitude is not null it computes magnitude*/
template<typename T>
class GradientStrategy
{
public:
  virtual void run(const T* in,T* out_x,T* out_y,T* out_z,T* mag,GradientStrategyParam p)=0;
  //virtual ~GradientStrategy();
};


/* simple gradient strategy which uses df/dx=[f(x+1)-f(x-1)]/2*/
template<typename T>
class GradientStrategySimple:public GradientStrategy<T>
{
public:
  void run(const T* in,T* out_x,T* out_y,T* out_z,T* mag,GradientStrategyParam p);
};


class GradientStrategyType
{
public:
  typedef enum _StrategyType
  {
    SIMPLE=0
  }StrategyType;
};


/*GradientStrategyContext is the user interface to use gradient strategy classes*/
class GradientStrategyContext
{
public:
  GradientStrategyContext(const ZStack* input_stack,ZStack* output_x,ZStack* output_y,
                          ZStack* output_z,ZStack* mag,bool inverse,GradientStrategyType::StrategyType strategy_type);
  GradientStrategyContext(const ZStack* in,ZStack* out,bool inverse,GradientStrategyType::StrategyType strategy_type);
  ~GradientStrategyContext();
  void run();
private:
  template<typename T>
  void _run();
private:
  const ZStack* _in;
  ZStack  *_outx,*_outy,*_outz,*_mag;
  GradientStrategyType::StrategyType _type;
  bool delete_out_xyz,_inverse;
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
  QRadioButton*   inverse;
  std::map<QString,GradientStrategyType::StrategyType>strategy_map;
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
