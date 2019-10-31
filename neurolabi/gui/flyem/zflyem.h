#ifndef ZFLYEM_H
#define ZFLYEM_H

#include <cstdint>

namespace flyem {

enum EDataSet {
  DATA_TEM, DATA_FIB19, DATA_FIB25, DATA_FIB25_7C, DATA_UNKOWN
};

typedef uint64_t TBodyLabel;

}

#endif // ZFLYEM_H
