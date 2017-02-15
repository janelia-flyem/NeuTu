#include "zsandboxproject.h"
#include "zsandbox.h"
#include "zexamplemodule.h"

/***Include your module headers here***/
#include "zsandboxmodule.h"
#include "zaboutmodule.h"
#include "zrgb2graymodule.h"
#include "ztracemodule.h"
#include "zimageinfomodule.h"
#include "zmultiscalewatershedmodule.h"
#include "zgradientmagnitudemodule.h"
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
  RegisterModule<ZTraceModule>();
  RegisterModule<ZImageInfoModule>();
  RegisterModule<ZMultiscaleWaterShedModule>();
  RegisterModule<ZGradientMagnitudeModule>();
}
