#include "zstackfactory.h"

ZStackFactory::ZStackFactory()
{
}

Stack *ZStackFactory::pileMatched(const std::vector<Stack*> stackArray)
{
  if (stackArray.empty()) {
    return NULL;
  }

  int kind = C_Stack::kind(stackArray[0]);
  int width = C_Stack::width(stackArray[0]);
  int height = C_Stack::height(stackArray[0]);
  int depth = 0;
  for (std::vector<Stack*>::const_iterator iter = stackArray.begin();
       iter != stackArray.end(); ++iter) {
    depth += C_Stack::depth(*iter);
  }

  Stack *out = C_Stack::make(kind, width, height, depth);

  int currentPlane = 0;
  for (std::vector<Stack*>::const_iterator iter = stackArray.begin();
       iter != stackArray.end(); ++iter) {
    const Stack* stack = *iter;
    Stack substack = C_Stack::sliceView(
          out, currentPlane, currentPlane + C_Stack::depth(stack) - 1);
    C_Stack::copyValue(stack, &substack);
    currentPlane += C_Stack::depth(stack);
  }

  return out;
}
