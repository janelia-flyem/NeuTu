%{
#include "zobject3dscan.h"
%}

%include zobject3dscan.h
%inline %{
  ZObject3dScan* CreateObject3dScan() {
    return new ZObject3dScan;
  }

  void DeleteObject3dScan(ZObject3dScan *obj) {
    delete obj;
  }
%}
