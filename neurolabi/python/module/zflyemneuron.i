%{
#include "flyem/zflyemneuron.h"
%}

%include "flyem/zflyemneuron.h"

%inline %{
  ZFlyEmNeuron* CreateZFlyEmNeuron() {
    return new ZFlyEmNeuron;
  }

  ZFlyEmNeuron* CreateZFlyEmNeuron(int id, ZSwcTree *model) {
    return new ZFlyEmNeuron(id, model, NULL);
  }

  ZFlyEmNeuron* CreateZFlyEmNeuron(
    int id, ZSwcTree *model, ZObject3dScan* body) {
    return new ZFlyEmNeuron(id, model, body);
  }

  void DeleteZFlyEmNeuron(ZFlyEmNeuron *obj) {
    delete obj;
  }
%}
