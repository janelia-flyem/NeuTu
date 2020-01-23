#ifndef ZSTACKDOCMETA_H
#define ZSTACKDOCMETA_H

class ZJsonObject;

class ZStackDocMeta
{
public:
  ZStackDocMeta();

  bool allowingTracing() const;
  bool allowingSegmentation() const;

  void allowTracing(bool on);

  void loadJsonObject(const ZJsonObject &obj);

private:
  bool m_allowingTracing = false;
  bool m_allowingSegmentation = false;
};

#endif // ZSTACKDOCMETA_H
