#include "mvcdef.h"


namespace neutu {

namespace mvc {

bool HasFlag(ViewInfoFlags host, ViewInfoFlags test)
{
  return (host & test) == test;
}

}

}
