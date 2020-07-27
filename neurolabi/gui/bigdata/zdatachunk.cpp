#include "zdatachunk.h"

ZDataChunk::ZDataChunk()
{

}

bool ZDataChunk::updateNeeded() const
{
  return isValid() && !isReady();
}
