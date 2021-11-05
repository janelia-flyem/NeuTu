#ifndef SPLITEXCEPTION_H
#define SPLITEXCEPTION_H

#include <stdexcept>

namespace flyem {

class SplitException : public std::runtime_error
{
public:
  SplitException(const std::string& what_arg);
};

}

#endif // SPLITEXCEPTION_H
