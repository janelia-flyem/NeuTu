#include "zstringarray.h"
#include <iostream>

ZStringArray::ZStringArray()
{
}

void ZStringArray::print(const std::vector<std::string> &strArray)
{
  std::cout << strArray.size() << " strings." << std::endl;
  for (std::vector<std::string>::const_iterator iter = strArray.begin();
       iter != strArray.end(); ++iter) {
    std::cout << *iter << std::endl;
  }
}
