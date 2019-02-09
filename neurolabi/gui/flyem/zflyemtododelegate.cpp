#include "zflyemtododelegate.h"

#include "zstackdoc.h"

ZFlyEmToDoDelegate::ZFlyEmToDoDelegate(ZStackDoc *doc) : m_doc(doc)
{

}

void ZFlyEmToDoDelegate::add(
    int x, int y, int z, bool checked, neutu::EToDoAction action,
    uint64_t bodyId)
{
  if (m_doc != NULL) {
    m_doc->executeAddTodoCommand(x, y, z, checked, action, bodyId);
  }
}
