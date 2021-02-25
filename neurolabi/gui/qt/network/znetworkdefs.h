#ifndef ZNETWORKDEFS_H
#define ZNETWORKDEFS_H

namespace znetwork {

enum class EOperation {
  NONE, READ, READ_PARTIAL, READ_HEAD, READ_OPTIONS,
  IS_READABLE, HAS_HEAD, HAS_OPTIONS, POST
};

}

#endif // ZNETWORKDEFS_H
