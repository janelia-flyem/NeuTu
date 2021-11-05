#include "splitexception.h"

flyem::SplitException::SplitException(const std::string& what_arg) :
  std::runtime_error(what_arg)
{

}
