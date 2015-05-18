#include "zkeyeventmapper.h"
#include "zstackoperator.h"

ZKeyEventMapper::ZKeyEventMapper(ZInteractiveContext *context, ZStackDoc *doc)
{
}

ZStackOperator ZKeyEventMapper::getOperation(const QKeyEvent &event) const
{

}

////////////////////////////////////

ZStackOperator ZKeyPressEventMapper::getOperation(const QKeyEvent &event) const
{

}
