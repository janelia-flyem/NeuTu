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
#include "zshowsegresult.h"
#include "zbrowseropener.h"
#include "zmultiscalesegmentationmanagement.h"
#if defined(_ENABLE_SURFRECON_)
#include "zsurfreconmodule.h"
#endif
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
  RegisterModule<ZShowSegResultModule>();
  RegisterModule<ZBrowserOpenerModule>();
  RegisterModule<ZMultiscaleSegManagementModule>();

#if defined(_ENABLE_SURFRECON_)
  RegisterModule<ZSurfReconModule>();
#endif
}
