%{
#include "zpointarray.h"
%}

%include zpointarray.h

%inline %{
  ZPointArray* CreateZPointArray() {
    return new ZPointArray;
  }

  void DeleteZPointArray(ZPointArray *obj) {
    delete obj;
  }
%}
