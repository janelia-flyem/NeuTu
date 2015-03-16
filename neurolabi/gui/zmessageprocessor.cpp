#include "zmessageprocessor.h"
#include <iostream>

ZMessageProcessor::ZMessageProcessor()
{
}

void ZMessageProcessor::processMessage(
    ZMessage */*message*/, QWidget *host) const
{

}

std::string ZMessageProcessor::toString() const
{
  return "Base ZMessageProcessor";
}

void ZMessageProcessor::print() const
{
  std::cout << toString() << std::endl;
}
