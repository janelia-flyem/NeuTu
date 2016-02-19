#include "zstackdoclabelstackfactory.h"
#include "zstackdoc.h"

ZStackDocLabelStackFactory::ZStackDocLabelStackFactory() : m_doc(NULL)
{
}

ZStack* ZStackDocLabelStackFactory::makeStack(ZStack *stack) const
{
  if (getDocument() == NULL) {
    return NULL;
  }

  return getDocument()->makeLabelStack(stack);
}
