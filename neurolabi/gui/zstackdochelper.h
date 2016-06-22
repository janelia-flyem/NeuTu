#ifndef ZSTACKDOCHELPER_H
#define ZSTACKDOCHELPER_H

class ZStackDoc;

class ZStackDocHelper
{
public:
  ZStackDocHelper();
#if defined(_FLYEM_)
  void extractCurrentZ(const ZStackDoc *doc);
#endif
  int getCurrentZ() const;
  bool hasCurrentZ() const;

private:
  int m_currentZ;
  bool m_hasCurrentZ;
};

#endif // ZSTACKDOCHELPER_H
