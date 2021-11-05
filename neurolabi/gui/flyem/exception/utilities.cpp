#include "utilities.h"

namespace flyem {

SplitErrorMessageBuilder::SplitErrorMessageBuilder() :
  ZWidgetMessageBuilder(neutu::EMessageType::ERROR)
{
  title("Split Error");
  m_message.setMessage(
        "Something wrong happend in the split. "
        "Please stop working on this body and report the issue ASAP.");
}

SplitErrorMessageBuilder& SplitErrorMessageBuilder::forBody(
    uint64_t bodyId, neutu::EBodyLabelType type)
{
  m_message.appendMessage(
        QString("%1 ID: %2").
        arg((type == neutu::EBodyLabelType::SUPERVOXEL) ? "Supervoxel" : "Body").
        arg(bodyId));
  return *this;
}

}
