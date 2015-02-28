#include "zflyemmisc.h"

#include "zmatrix.h"
#include "tz_math.h"
#include "tz_utilities.h"

void ZFlyEmMisc::NormalizeSimmat(ZMatrix &simmat)
{
  for (int j = 0; j < simmat.getColumnNumber(); ++j) {
    double maxC = simmat.getValue(j, j);
    for (int i = 0; i < simmat.getRowNumber(); ++i) {
      if (i != j) {
        double maxR = simmat.getValue(i, i);
        simmat.set(i, j, simmat.getValue(i, j) / dmax2(maxC, maxR));
      }
    }
  }
}
