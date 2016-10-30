#include "zsandboxproject.h"
#include "zsandbox.h"
#include "zexamplemodule.h"

/***Include your module headers here***/
#include "zsandboxmodule.h"
#include "zaboutmodule.h"
#include "zrgb2graymodule.h"
#include"zpixelsmodule.h"
#include "ztracemodule.h"
#include"zimageinfomodule.h"
/*************************************/


template<typename T>
void RegisterModule()
{
  T *module = new T;
  ZSandbox::RegisterModule(module);
}

void ZSandboxProject::InitSandbox()
{
  //Add your modules here
  RegisterModule<ZAboutModule>();
  RegisterModule<ZExampleModule>();
  RegisterModule<ZRgb2GrayModule>();
<<<<<<< HEAD
  RegisterModule<ZPixelsModule>();
  RegisterModule<ZTraceModule>();
=======
  RegisterModule<ZImageInfoModule>();
>>>>>>> 7923b00bedbba53a4b1bb16084c723e59967adc5
}
