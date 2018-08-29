#include "zstackprinter.h"

#include <iostream>

#include "zstack.hxx"

ZStackPrinter::ZStackPrinter()
{

}

void ZStackPrinter::setDetailLevel(int level)
{
  m_detailLevel = level;
}

void ZStackPrinter::print(const Stack *stack)
{
  if (m_detailLevel == 0) {
    C_Stack::print(stack);
  } else if (m_detailLevel > 0) {
    C_Stack::printValue(stack);
  }
}

void ZStackPrinter::print(const ZStack *stack)
{
  for (int c = 0; c < stack->channelNumber(); ++c) {
    if (stack->channelNumber() > 1) {
      std::cout << "Channel " << c << ":" << std::endl;
    }

    print(stack->c_stack(c));
  }
}
