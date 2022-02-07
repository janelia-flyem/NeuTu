#include "mvcdef.h"


namespace neutu {

namespace mvc {

bool HasFlag(ViewInfoFlags host, ViewInfoFlags test)
{
  return (host & test) == test;
}

}

std::string ToString(mvc::EModification action)
{
  switch (action) {
  case mvc::EModification::CREATED:
    return "created";
  case mvc::EModification::DELETED:
    return "deleted";
  case mvc::EModification::UPDATED:
    return "updated";
  }
}

}
