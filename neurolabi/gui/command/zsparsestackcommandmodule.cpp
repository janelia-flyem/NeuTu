#include "zsparsestackcommandmodule.h"

#include <vector>
#include <string>

#include "zobject3dscan.h"
#include "zstack.hxx"
#include "zstackreader.h"

ZSparseStackCommand::ZSparseStackCommand()
{

}

int ZSparseStackCommand::run(
    const std::vector<std::string> &input, const std::string& output,
    const ZJsonObject &/*config*/)
{

  if(input.empty() ||output.empty())
  {
    return 1;
  }

  std::string imgFile = input.front();

  ZStack stack;
  stack.load(imgFile);

  ZObject3dScan obj;
  obj.loadStack(stack);

  if (!obj.isEmpty()) {
    obj.save(output);
  } else {
    std::cout << "Failed to produce result: " << "empty object." <<std::endl;
    return 1;
  }

  return 0;
}
