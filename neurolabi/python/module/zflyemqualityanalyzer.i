%{
#include "flyem/zflyemqualityanalyzer.h"
%}

%include "flyem/zflyemqualityanalyzer.h"

%inline %{
  ZFlyEmQualityAnalyzer* CreateZFlyEmQualityAnalyzer() {
    return new ZFlyEmQualityAnalyzer;
  }

  void DeleteZFlyEmQualityAnalyzer(ZFlyEmQualityAnalyzer *obj) {
    delete obj;
  }
%}
