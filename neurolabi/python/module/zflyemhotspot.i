%{
namespace FlyEm {
  #include "flyem/zhotspotarray.h"
}
%}

%include "flyem/zhotspotarray.h"

%inline %{
    FlyEm::ZHotSpotArray* CreateHotSpotArray() {
      return new FlyEm::ZHotSpotArray;
    }

    void DeleteHotSpotArray(FlyEm::ZHotSpotArray *obj) {
      delete obj;
    }
%}

