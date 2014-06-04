#ifndef ZSTACKREADFACTORY_H
#define ZSTACKREADFACTORY_H

#include "zstackfactory.h"
#include "zstackfile.h"

class ZStackReadFactory : public ZStackFactory
{
public:
  ZStackReadFactory();

public:
  ZStack* makeStack(ZStack *stack) const;

public:
  inline void setSource(const ZStackFile &source) {
    m_source = source;
  }

private:
  ZStackFile m_source;
};

#endif // ZSTACKREADFACTORY_H
